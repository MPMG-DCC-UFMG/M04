package br.ufmg.cs.systems.fractal.mpmg

import br.ufmg.cs.systems.fractal._
import br.ufmg.cs.systems.fractal.subgraph._
import br.ufmg.cs.systems.fractal.util.collection.IntArrayList
import br.ufmg.cs.systems.fractal.util.{Logging, PairWritable}
import org.apache.hadoop.fs.Path
import org.apache.hadoop.io.{IntWritable, Text}
import org.apache.spark.SparkConf
import org.apache.spark.sql.{DataFrame, SparkSession}
import com.hortonworks.spark.sql.hive.llap.HiveWarehouseBuilder;
import com.hortonworks.spark.sql.hive.llap.HiveWarehouseSession;
import com.hortonworks.spark.sql.hive.llap.HiveWarehouseSession._;

import org.apache.spark.SparkContext
import org.apache.spark.graphx.{GraphLoader, PartitionStrategy}
import org.apache.spark.graphx.Graph
import org.apache.spark.graphx.VertexId

import java.io.{BufferedWriter, File, FileWriter}

import scala.collection.mutable.Map
import scala.collection.mutable.ArrayBuffer

import java.io.BufferedOutputStream
import org.graphframes.GraphFrame
import org.apache.spark.sql.DataFrame
import org.apache.spark.sql.Row
import scala.collection.immutable.ListMap
import org.apache.spark.sql.functions._
import org.apache.spark.sql.functions.col
/*
* Intern apps -- they can be used insider the apps that users can call.
*/

class UtilApp() extends Logging {
  def readConfig(configPath: String): ujson.Value = {
    logInfo(s"Reading config file from ${configPath}")
    ujson.read(scala.reflect.io.File(configPath).slurp)
  }

  def getCreateSparkSession(config: ujson.Value = null, configPath: String, configName: String): SparkSession = {
    logInfo("Creating Spark Session")
    val useConfig = if (config == null) readConfig(configPath) else config
    var conf = new SparkConf()
    useConfig(configName).arr.foreach(setting => {
      conf = conf.set(setting("name").str, setting("value").str)
    })

    SparkSession.builder.config(conf).enableHiveSupport().getOrCreate()
  }

  def getCreateHiveSession(ss: SparkSession): HiveWarehouseSession = {
    logInfo("Creating Hive Session (and respective spark session)")
    HiveWarehouseBuilder.session(ss).build()
  }
}

class HiveApp(val configPath: String) extends Logging {
  val utilApp = new UtilApp
  var databaseConfigs: ujson.Value = _
  var currentConfig: ujson.Value = _
  var hiveSession: HiveWarehouseSession = _

  def initConfigs: Unit = {
    currentConfig = utilApp.readConfig(configPath)
    databaseConfigs = currentConfig("database")
  }

  def initHiveConnector {
    val sparkSession = utilApp.getCreateSparkSession(currentConfig, null, "spark_database")
    hiveSession = HiveWarehouseBuilder.session(sparkSession).build()
  }

  initConfigs
  initHiveConnector

  /**
   * Read from hive/database write in disk
   */
  def readWriteInput(filePath: String): Unit = {
    if (filePath.isEmpty) return

    logInfo(s"\tLoading edges with query: ${databaseConfigs("input_query").str}")
    val edges = hiveSession.executeQuery(databaseConfigs("input_query").str)
    //logInfo(s"\tEdges loaded sample of 10: ${edges.take(10)}")
    //    todo: write using spark
    logInfo(s"\tWriting data to CSV at: ${filePath}")

    val buffer = new BufferedWriter(new FileWriter(new File(filePath)))
    edges.collect.foreach(edge => {
      buffer.write(s"${edge.get(0)} ${edge.get(1)}\n")
    })
    buffer.close()
    //edges.coalesce(1).write.format("com.databricks.spark.csv").mode("overwrite").save(filePath)
  }
}

class MapVerticesApp(val fractalGraph: FractalGraph,
                     algs: FractalAlgorithms) extends FractalSparkApp {
  var app: Map[IntWritable, Text] = _

  def execute: Unit = {
    val (mapf, elapsed) = FractalSparkRunner.time {
      algs.mapVertices(fractalGraph)
    }
    logInfo(s"MapVerticesApp" +
      s" graph=${fractalGraph} elapsed=${elapsed}"
    )

    app = mapf
  }
}

/*
* Apps that users can use directly via config.
*/

trait MPMGApp extends Logging {
  def execute: Unit

  def writeResults(filePath: String, delimiter: String): Unit

  def writeResults(filePath: String, vertexMap: Map[IntWritable, Text]): Unit
}

class CliquesApp(
                  val fractalGraph: FractalGraph,
                  algs: FractalAlgorithms,
                  size: Int) extends FractalSparkApp with MPMGApp {
  var app: Fractoid[VertexInducedSubgraph] = _

  def execute: Unit = {
    val cliquesRes = algs.cliques(fractalGraph, size).
      explore(size-1)

    val (accums, elapsed) = FractalSparkRunner.time {
      cliquesRes.compute()
    }

    logInfo(s"CliquesOptApp" +
      s" size=${size}" +
      s" graph=${fractalGraph} " +
      s" numValidSubgraphs=${cliquesRes.numValidSubgraphs()} elapsed=${elapsed}"
    )

    app = cliquesRes
  }

  def writeResults(filePath: String, vertexMap: Map[IntWritable, Text]): Unit = {

    val conf = new org.apache.hadoop.conf.Configuration();
    val path = new Path(filePath)
    val fs = path.getFileSystem(conf)

    // Output file can be created from file system.
    val output = fs.create(path);

    // But BufferedOutputStream must be used to output an actual text file.
    val os = new BufferedOutputStream(output)
    //os.write("Identificador da clique,Identificador do vértice participante\n".getBytes("UTF-8"))

    var i = 1
    var iter_var = app.mappedSubgraphs.toLocalIterator
    while (iter_var.hasNext) {
     val subgraph = iter_var.next()
     for (vertex: String <- subgraph.mappedWords) {
        os.write(s"${i},${vertex}\n".getBytes("UTF-8"))
     }
      i += 1 
    }

    os.close()
  }

  def writeResults(filePath: String, delimiter: String): Unit = { }
}

class ShortestPathsApp(
                        val fractalGraph: FractalGraph,
                        algs: FractalAlgorithms,
                        explorationSteps: Int) extends FractalSparkApp with MPMGApp {
  var app: Fractoid[EdgeInducedSubgraph] = _

  def execute: Unit = {
    val (pathsf, elapsed) = FractalSparkRunner.time {
      algs.spaths(fractalGraph, explorationSteps)
    }
    logInfo(s"ShortestPathsApp" +
      s" explorationSteps=${explorationSteps}" +
      s" graph=${fractalGraph} " +
      s" numValidSubgraphs=${pathsf.numValidSubgraphs()} elapsed=${elapsed}"
    )

    app = pathsf
  }

  override def writeResults(filePath: String, vertexMap: Map[IntWritable, Text]): Unit = {
    val outputBuffer = new BufferedWriter(new FileWriter(new File(filePath)))
    outputBuffer.write("Identificador do caminho,Identificador do vértice participante,Vértice origem,Vértice destino\n")

    var i = 1
    app.aggregationMap[PairWritable[IntWritable, IntWritable], IntArrayList]("sps").foreach {
      case (pair, path) => {
        val it = path.iterator
        while (it.hasNext) {
          val id = new IntWritable(it.next())
          outputBuffer.write(s"${i},${vertexMap(id)},${vertexMap(pair.getLeft)},${vertexMap(pair.getRight)}\n")
        }
        i += 1 // todo: validate if is don't collide
      }
    }
    outputBuffer.close()
  }

  def writeResults(filePath: String, delimiter: String): Unit = {}
}

class ConnectedComponentsApp(var graph:GraphFrame) extends MPMGApp {
  var app:DataFrame = _

  def execute: Unit = {

    val cc = graph.connectedComponents.run()
    app = cc
  }

  def writeResults(filePath: String, vertexMap: Map[IntWritable, Text]): Unit = { }

  def writeResults(filePath: String, delimiter: String): Unit = {

    app.write.format("com.databricks.spark.csv").option("delimiter", delimiter).mode("overwrite").save(filePath)
  }

  def characterize(spark: SparkSession, sc: SparkContext, filePath: String, delimiter: String): Unit = {

    val longs = sc.textFile(filePath)
    val counts = longs.map(x=>{x.split(delimiter)(1).toLong}).countByValue()
    val sorted = ListMap(counts.toSeq.sortWith(_._2 > _._2):_*)

    val df = spark.createDataFrame(sorted.toSeq)
    df.write.format("com.databricks.spark.csv").option("delimiter", delimiter).mode("overwrite").save(filePath + ".out")
  }
}

class TriangleCountingApp(var graph:GraphFrame) extends MPMGApp {

  var app:DataFrame = _

  def execute: Unit = {

    val results = graph.triangleCount.run()
    app = results.select("id", "count").orderBy(col("count").desc, col("id"))
  }

  def writeResults(filePath: String, vertexMap: Map[IntWritable, Text]): Unit = {}

  def writeResults(filePath: String, delimiter: String): Unit = {

    app.write.format("com.databricks.spark.csv").option("delimiter", delimiter).mode("overwrite").save(filePath)
  }

  def characterize(spark: SparkSession, filePath: String, delimiter: String): Unit = {

    val longs = spark.sparkContext.textFile(filePath)
    val counts = longs.map(x=>{val arr = x.split(delimiter); (arr(0).toString,arr(1).toLong)})
    val df = spark.createDataFrame(counts.collect())
    val total_triangle = df.agg(sum("_2")).first.getLong(0) / 3
    import spark.implicits._ 
    val total_triangle_df = List(total_triangle).toDF()
    total_triangle_df.write.format("com.databricks.spark.csv").option("delimiter", delimiter).mode("overwrite").save(filePath + ".out")
  }
}

class ShortestPathsComponentsApp(
    spark:SparkSession,
    inputPath:String,
    ccInputPath:String,
    outputPath:String,
    delimiter:String,
    max_size:Int,
    ccid:Option[Long],
    fc:FractalContext, 
    algs: FractalAlgorithms) extends MPMGApp {

  def execute: Unit = {

    // arquivo temporario onde sera escrita a componente a ser passada para o spaths
    val inputForSpathsCC = inputPath.substring(0, inputPath.lastIndexOf("/") + 1) + "tmp_input_generated_for_spaths.csv"

    //1. le a saída gerada pela caracterizacao de componentes conexos
    var components = spark.read.option("inferSchema","true").option("delimiter", delimiter).csv(ccInputPath + ".out").toDF("cc", "qtd")

    //2. le o arquivo que contem o componente conexo a qual pertence cada vertice
    var allVertexCC = spark.read.option("inferSchema","true").option("delimiter", delimiter).csv(ccInputPath).toDF("vertex", "cc")

    //3. carrega os pares de vertices do grafo completo
    //var wholeGraphDF = spark.read.option("inferSchema","true").option("delimiter", delimiter).csv(inputPath).toDF("src", "dst", "edgeType").drop("edgeType")
    var wholeGraphDF = spark.read.option("inferSchema","true").option("delimiter", delimiter).csv(inputPath).toDF("src", "dst")
    
    var tmpComponents:Any = None
    if(ccid == None) tmpComponents = components.collect()
    else {
      val a:Array[Row] = new Array[Row](1)
      a(0) = Row(ccid.get)
      tmpComponents = a
    }

    val localComponents:Array[Row] = tmpComponents.asInstanceOf[Array[Row]]
    for (ccRow <- localComponents) {

      //4. descobre todos os nós para cada componente conexo
      val vertexCC = allVertexCC.where(col("cc") === ccRow(0))

      //5. Salva em um arquivo as arestas do componente conexo
      val vertexIds = vertexCC.select("vertex").collect.flatMap(_.toSeq).map(x=>x.toString);
      val directedEdges = wholeGraphDF.filter( (col("src").isin(vertexIds:_*)) || (col("dst").isin(vertexIds:_*)));
      val edges = directedEdges.union(directedEdges.select(col("dst"),col("src")));
      edges.write.mode("overwrite").option("sep"," ").csv(inputForSpathsCC) // o fractal so aceita espacos

      val fractalGraph = fc.textFile(inputForSpathsCC, "br.ufmg.cs.systems.fractal.graph.EdgeListGraph")

      val vertexMap = new MapVerticesApp(fractalGraph, algs) 
      vertexMap.execute 
    
      val app = new ShortestPathsApp(fractalGraph, algs, max_size)
      app.execute

      app.writeResults(outputPath, vertexMap.app)
    }
  }

  override def writeResults(filePath: String, vertexMap: Map[IntWritable, Text]): Unit = {} 
  
  def writeResults(filePath: String, delimiter: String): Unit = {  }

}

class BreadthFirstSearchApp(
    spark:SparkSession, 
    inputPath:String, 
    ccInputPath:String, 
    outputPath:String, 
    delimiter:String, 
    max_size:Int, 
    ccid:Option[Long]) extends MPMGApp {

  def execute:Unit = {

    //1. le a saída gerada pela caracterizacao de componentes conexos
    var components = spark.read.option("inferSchema","true").option("delimiter", delimiter).csv(ccInputPath + ".out").toDF("cc", "qtd")

    //2. lê o arquivo que contem o componente conexo a qual pertence cada vertice
    var allVertexCC = spark.read.option("inferSchema","true").option("delimiter", delimiter).csv(ccInputPath).toDF("vertex", "cc")

    //3. carrega os pares de vertices do grafo completo
    //var wholeGraphDF = spark.read.option("inferSchema","true").option("delimiter", delimiter).csv(inputPath).toDF("src", "dst", "edgeType").drop("edgeType")
    var wholeGraphDF = spark.read.option("inferSchema","true").option("delimiter", delimiter).csv(inputPath).toDF("src", "dst")

    var tmpComponents:Any = None 
    if(ccid == None) tmpComponents = components.collect()
    else { 
      val a:Array[Row] = new Array[Row](1)
      a(0) = Row(ccid.get)
      tmpComponents = a
    }

    val localComponents:Array[Row] = tmpComponents.asInstanceOf[Array[Row]]
    for (ccRow <- localComponents) {

      //4. descobre todos os nós para cada componente conexo
      val vertexCC = allVertexCC.where(col("cc") === ccRow(0))
      print(vertexCC.collect())

      //5. lista todos os pares de vertices para cada componente conexo
      val crossJoin = vertexCC.as("from").crossJoin(vertexCC.as("to")).orderBy("from.vertex")

      //6. Carrega um grafo contendo apenas os vertices do componente conexo
      val vertexIds = vertexCC.select("vertex").collect.flatMap(_.toSeq).map(x=>x.toString);
      val directedEdges = wholeGraphDF.filter( (col("src").isin(vertexIds:_*)) || (col("dst").isin(vertexIds:_*)));
      val edges = directedEdges.union(directedEdges.select(col("dst"),col("src")));
      val graph = GraphFrame.fromEdges(edges);

      //7. calcula o caminho mínimo para cada par de vértice do grafo descoberto
      val localCJ = crossJoin.collect();

      var allPathsCC:ArrayBuffer[Array[Array[String]]] = new ArrayBuffer[Array[Array[String]]]()
      localCJ.foreach(row => {

        val fromV = row(0).asInstanceOf[String]
        val toV = row(2).asInstanceOf[String]
        if ( (fromV < toV) ) { // && (!fromV.startsWith("tel")) && (!toV.startsWith("tel")) ){
          val paths = graph.bfs.fromExpr("id = '" + fromV + "'").toExpr("id = '" + toV + "'").maxPathLength(max_size).run()
          val localPaths = paths.collect()
          val localPathsSize = localPaths(0).size
          val vertexPathSize = localPathsSize/2 + 1
          val localPathsStringArr = localPaths.map(row => {
            var pathStringArr:Array[String] = new Array[String](vertexPathSize)
            pathStringArr(0) = row(0).toString
            pathStringArr(1) = row(localPathsSize - 1).toString
            for (vindex <- 2 to (vertexPathSize - 1)){
              pathStringArr(vindex) = row( (vindex-1)*2 ).toString
            }
            pathStringArr
          })
          allPathsCC.append(localPathsStringArr)
        }
      })
    
      val buffer = new BufferedWriter(new FileWriter(new File(outputPath), true))
      for(pathsPairOfVertices <- allPathsCC){
        for(path <- pathsPairOfVertices) {
          val line = path.mkString(delimiter).replaceAll("[\\[\\]]","")
          buffer.write(s"${line}\n")
        }
      }
      buffer.close
    }
  }

  override def writeResults(filePath: String, vertexMap: Map[IntWritable, Text]): Unit = {}

  def writeResults(filePath: String, delimiter: String): Unit = {}

}

class WriteDatabaseApp(
        spark:SparkSession, 
	val hs: HiveWarehouseSession, 
	inputPath:String, 
        databaseName:String,
	tableName:String, 
	columnNames:String, 
	delimiter:String) extends MPMGApp {
  
  def execute: Unit = {

    var dbName = databaseName
    if (dbName != "") {
    	hs.setDatabase(databaseName)
	dbName = dbName + "."
     }
    //hs.createTable(tableName).ifNotExists().column("clique_id", "bigint").column("vertex_id", "bigint").create()
    val dropQuery = "DROP TABLE IF EXISTS " + dbName + tableName
    hs.executeUpdate(dropQuery)

    var createQuery = "CREATE TABLE " + tableName + " ("
    
    val columnNamesArr = columnNames.split(",")
    columnNamesArr.foreach(colName => {createQuery = createQuery + colName + " String,"})
    createQuery = createQuery.substring(0, createQuery.length - 1)
    createQuery = createQuery + ") ROW FORMAT DELIMITED FIELDS TERMINATED BY ',' LINES TERMINATED BY '\n'"
    hs.executeUpdate(createQuery)
    
    //var df = spark.read.option("inferSchema","true").option("delimiter", delimiter).csv(inputPath).toDF("clique_id", "vertex_id")
    var df = spark.read.option("inferSchema","true").option("delimiter", delimiter).csv(inputPath).toDF(columnNamesArr: _*)
    df.write.format(HIVE_WAREHOUSE_CONNECTOR).mode("overwrite").option("table", tableName).save()

  }

  override def writeResults(filePath: String, vertexMap: Map[IntWritable, Text]): Unit = { }

  def writeResults(filePath: String, delimiter: String): Unit = {}

}
class ReadDatabaseApp(val hs: HiveWarehouseSession, val query: String
                     ) extends MPMGApp {
  var app: DataFrame = _

  def execute: Unit = {
    logInfo(s"\tLoading edges with query: " + query)
    app = hs.executeQuery(query)
  }

  override def writeResults(filePath: String, vertexMap: Map[IntWritable, Text]): Unit = {}

  def writeResults(filePath: String, delimiter: String): Unit = {
  
    if (filePath.isEmpty) return

    logInfo(s"\tWriting data to CSV at: ${filePath}")
    val conf = new org.apache.hadoop.conf.Configuration();
    val path = new Path(filePath)
    val fs = path.getFileSystem(conf)

    // Output file can be created from file system.
    //val output = fs.create(path);

    app.write.format("com.databricks.spark.csv").option("delimiter", delimiter).mode("overwrite").save(filePath)

    //app.coalesce(1).write.format("com.databricks.spark.csv").option("delimiter", " ").mode("overwrite").(filePath)
  }
}

object MPMGSparkRunner {
  def time[R](block: => R): (R, Long) = {
    val t0 = System.currentTimeMillis()
    val result = block // call-by-name
    val t1 = System.currentTimeMillis()
    (result, t1 - t0)
  }

  def main(args: Array[String]) {
    //args
    val configPath = args(0)
    val utilApp = new UtilApp
    val config = utilApp.readConfig(configPath)

    //create input file for fractal 
    val appConfig = config("app")

    //running fractal application
    val algs = new FractalAlgorithms //TODO: extends this class to be Algorithms (Fractal and Database algorithms)

    appConfig("name").str.toLowerCase match {
      case "cliques" => {
        /* fractal and its spark initialization */
        val ss = utilApp.getCreateSparkSession(config, null, "spark_fractal")
        if (!ss.sparkContext.isLocal) Thread.sleep(10000) // TODO: this is ugly but have to make sure all spark executors are up by the time we start executing fractal applications
        val fc = new FractalContext(ss.sparkContext)
        val inputPath = appConfig("input_path").str
        if (inputPath.isEmpty) throw new RuntimeException(s"Unknown input_fractal_path")
        val outputPath = appConfig("output_path").str
        val fractalGraph = fc.textFile(inputPath, "br.ufmg.cs.systems.fractal.graph.EdgeListGraph")

        /* Execute app */
        val vertexMap = new MapVerticesApp(fractalGraph, algs) //TODO: put this as an application (map the input and the output of fractal)
        //vertexMap.execute
        val app = new CliquesApp(fractalGraph, algs, appConfig("size").num.toInt)
        app.execute

        //write output results
        app.writeResults(outputPath, vertexMap.app)
        fc.stop()
        ss.stop()

      }
      case "spaths" => {
        /* fractal and its spark initialization */
        val ss = utilApp.getCreateSparkSession(config, null, "spark_fractal")
        if (!ss.sparkContext.isLocal) Thread.sleep(10000) // TODO: this is ugly but have to make sure all spark executors are up by the time we start executing fractal applications
        val fc = new FractalContext(ss.sparkContext)
        val inputPath = appConfig("input_path").str
        if (inputPath.isEmpty) throw new RuntimeException(s"Unknown input_path")
        val outputPath = appConfig("output_path").str
        val fractalGraph = fc.textFile(inputPath, "br.ufmg.cs.systems.fractal.graph.EdgeListGraph")

        /* Execute app */
        val vertexMap = new MapVerticesApp(fractalGraph, algs) //TODO: put this as an application (map the input and the output of fractal)
        vertexMap.execute //TODO: remove this from here, use as an application
        val app = new ShortestPathsApp(fractalGraph, algs, appConfig("max_size").num.toInt)
        app.execute

        //write output results
        app.writeResults(outputPath, vertexMap.app)
        fc.stop()
        ss.stop()
      }
      case "read_database" => {
        /*  database/hive and its spark initialization	 */
        val ss = utilApp.getCreateSparkSession(config, null, "spark_database")
        val hs = utilApp.getCreateHiveSession(ss)

        var delimiter = appConfig("delimiter").str
        if (delimiter.isEmpty) delimiter = " "
        val outputPath = appConfig("output_path").str

        /* Execute app */
        val app = new ReadDatabaseApp(hs, appConfig("query").str)
        app.execute
        app.writeResults(outputPath, delimiter)
        ss.close()
      }
      case "write_database" => {
        /*  database/hive and its spark initialization   */
        val ss = utilApp.getCreateSparkSession(config, null, "spark_database")
        val hs = utilApp.getCreateHiveSession(ss)

        val inputPath = appConfig("input_path").str
        val databaseName = appConfig("database_name").str
        val tableName = appConfig("table_name").str
        var columnNames = appConfig("column_names").str
        var delimiter = appConfig("delimiter").str


        /* Execute app */
        val app = new WriteDatabaseApp(ss, hs, inputPath, databaseName, tableName, columnNames, delimiter)
        app.execute
        ss.close()
      }
      case "connected_components" => {

        val inputPath = appConfig("input_path").str
        if (inputPath.isEmpty) throw new RuntimeException(s"Unknown input_path")

        var delimiter = appConfig("delimiter").str
        if (delimiter.isEmpty) delimiter = ","

        var checkpointDir = appConfig("checkpoint_dir").str
        if (checkpointDir.isEmpty) checkpointDir = "/checkpoint-dir"

        val outputPath = appConfig("output_path").str

        /* fractal and its spark initialization */
        val ss = utilApp.getCreateSparkSession(config, null, "spark_fractal")
        if (!ss.sparkContext.isLocal) Thread.sleep(10000) // TODO: this is ugly but have to make sure all spark executors are up by the time we start executing fractal applications

        val sc = ss.sparkContext
        
        sc.setCheckpointDir(checkpointDir)
        var df:DataFrame = null.asInstanceOf[DataFrame]
        try {
          df = ss.read.option("inferSchema","true").option("delimiter", delimiter).csv(inputPath).toDF("src", "dst", "tipo_aresta")
        } catch {
          case e:Exception => df = ss.read.option("inferSchema","true").option("delimiter", delimiter).csv(inputPath).toDF("src", "dst")
        }
	df = df.repartition(300) 

        val graph = GraphFrame.fromEdges(df)
        val app = new ConnectedComponentsApp(graph)
        
        app.execute

        //write output results
        app.writeResults(outputPath, delimiter)

        app.characterize(ss, sc, outputPath, delimiter)
        
        sc.stop()
        ss.stop()

      }
      case "tricount" => {

        val inputPath = appConfig("input_path").str
        if (inputPath.isEmpty) throw new RuntimeException(s"Unknown input_path")

        var delimiter = appConfig("delimiter").str
        if (delimiter.isEmpty) delimiter = ","

        var checkpointDir = appConfig("checkpoint_dir").str
        if (checkpointDir.isEmpty) checkpointDir = "/checkpoint-dir"

        val outputPath = appConfig("output_path").str

        /* fractal and its spark initialization */
        val ss = utilApp.getCreateSparkSession(config, null, "spark_fractal")
        if (!ss.sparkContext.isLocal) Thread.sleep(10000) // TODO: this is ugly but have to make sure all spark executors are up by the time we start executing fractal applications

        val sc = ss.sparkContext

        // Load the edges in canonical order and partition the graph for triangle count

        sc.setCheckpointDir(checkpointDir)
        var df:DataFrame = null.asInstanceOf[DataFrame]
        try {
          df = ss.read.option("inferSchema","true").option("delimiter", delimiter).csv(inputPath).toDF("src", "dst", "tipo_aresta")
        } catch {
          case e:Exception => df = ss.read.option("inferSchema","true").option("delimiter", delimiter).csv(inputPath).toDF("src", "dst")
        }

        df = df.repartition(300)
        val graph = GraphFrame.fromEdges(df)
        val app = new TriangleCountingApp(graph)
        app.execute 

        //write output results
        app.writeResults(outputPath, delimiter)

        app.characterize(ss, outputPath, delimiter)
        sc.stop()
        ss.stop()
      }
      case "bfs" => {

        val inputPath = appConfig("input_path").str
        val ccInputPath = appConfig("cc_input_path").str
	val outputPath = appConfig("output_path").str
        val delimiter = appConfig("delimiter").str
        val max_size = appConfig("max_size").num.toInt
        var ccid:Option[Long] = None
        try{
          val ccid_aux = appConfig("ccid").num.toLong
          ccid = Some(ccid_aux)
        } catch { case _:Throwable => {}}

        val ss = utilApp.getCreateSparkSession(config, null, "spark_fractal")
        if (!ss.sparkContext.isLocal) Thread.sleep(10000) // TODO: this is ugly but have to make sure all spark executors are up by the time we start executing fractal applications

        val sc = ss.sparkContext

        val app = new BreadthFirstSearchApp(ss, inputPath, ccInputPath, outputPath, delimiter, max_size, ccid)

        app.execute

        sc.stop()
        ss.stop()
      }
      case "spaths_cc" => {
        
         /* fractal and its spark initialization */
        val ss = utilApp.getCreateSparkSession(config, null, "spark_fractal")
        if (!ss.sparkContext.isLocal) Thread.sleep(10000) // TODO: this is ugly but have to make sure all spark executors are up by the time we start executing fractal applications
        val fc = new FractalContext(ss.sparkContext)
       
        val inputPath = appConfig("input_path").str
        val ccInputPath = appConfig("cc_input_path").str
        val outputPath = appConfig("output_path").str
        val delimiter = appConfig("delimiter").str
        val max_size = appConfig("max_size").num.toInt
        var ccid:Option[Long] = None
        try{
          val ccid_aux = appConfig("ccid").num.toLong
          ccid = Some(ccid_aux)
        } catch { case _:Throwable => {}}
        
        val app = new ShortestPathsComponentsApp(ss, inputPath, ccInputPath, outputPath, delimiter, max_size, ccid, fc, algs)

        app.execute

        fc.stop()
        ss.stop()

      }
      case appName => {
        throw new RuntimeException(s"Unknown app: ${appName}")
      }
    }
  }
}

