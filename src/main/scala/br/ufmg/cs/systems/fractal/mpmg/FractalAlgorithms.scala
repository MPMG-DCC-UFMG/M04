package br.ufmg.cs.systems.fractal.mpmg

import br.ufmg.cs.systems.fractal._
import br.ufmg.cs.systems.fractal.computation.Computation
import br.ufmg.cs.systems.fractal.graph.MainGraph
import br.ufmg.cs.systems.fractal.subgraph.{EdgeInducedSubgraph, VertexInducedSubgraph}
import br.ufmg.cs.systems.fractal.util._
import br.ufmg.cs.systems.fractal.util.collection.IntArrayList
import br.ufmg.cs.systems.fractal.util.pool.IntIntMapPool
import org.apache.hadoop.io.{IntWritable, Text}

import scala.collection.mutable.Map

class FractalAlgorithms extends Logging {

   def mapVertices(fgraph: FractalGraph): Map[IntWritable, Text] = {
      fgraph.vfractoid.
         expand(1).
         aggregate[IntWritable, Text]("mapv",
            (subg, comp, value) => {
               val v = subg.getVertices
               val n = new IntWritable(v.get(0))
               n
            },
            (subg, comp, value) => {
               val v = subg.getVertices
               val vertex = v.get(0)
               new Text(comp.getConfig().getMainGraph[MainGraph[_, _]]()
                  .getVertex(vertex).getVertexOriginalId.toString)
            },
            (value1, value2) => {
               value1
            }).aggregationMap[IntWritable, Text]("mapv")
   }

   /**
    * All-cliques listing implementing the efficient DAG structure from
    * [[https://dl.acm.org/citation.cfm?id=3186125]]
    *
    * @param cliqueSize
    * @return Fractoid with the initial state for cliques
    */
   def cliques(fgraph: FractalGraph, cliqueSize: Int): Fractoid[VertexInducedSubgraph] = {
      fgraph.vfractoid.
         expand(1).
         set("subgraph_enumerator",
            "br.ufmg.cs.systems.fractal.gmlib.clique.KClistEnumerator")
   }

   /**
    * Shortest paths (SPs) listing implemented with aggregations
    *
    * @param numSteps maximum number of exploration steps
    * @return Fractoid with the initial state for the SPs
    */
   def spaths(fgraph: FractalGraph, numSteps: Int): Map[(Int, Int), IntArrayList] = {
      val SP_AGG = "sps"

      val logFunc = (msg: String) => {
         Logging.getLogger("PATHS_DEBUG").info(msg)
      }

      val checkPath: (EdgeInducedSubgraph) => (Boolean,Int,Int) =
         s => {
         val vertices = s.getVertices
         val numVertices = vertices.size()
         val degreeMap = IntIntMapPool.instance().createObject()
         val edges = s.getEdges
         val numEdges = edges.size()
         var i = 0
         while (i < numEdges) {
            val e = s.edge(edges.get(i))
            degreeMap.addValue(e.getSourceId, 1, 0)
            degreeMap.addValue(e.getDestinationId, 1, 0)
            i += 1
         }

         i = 0
         var firstVertex = -1
         var lastVertex = -1
         var numDegreeOne = 0
         var numDegreeTwo = 0
         var numDegreeGreaterTwo = 0
         while (i < numVertices) {
            val u = vertices.get(i)
            val d = degreeMap.get(u)
            if (d == 1) {
               numDegreeOne += 1
               if (firstVertex == -1) firstVertex = u
               else lastVertex = u
            } else if (d == 2) {
               numDegreeTwo += 1
            } else {
               numDegreeGreaterTwo += 1
            }
            i += 1
         }

         val isPath = numDegreeOne == 2 && numDegreeGreaterTwo == 0

         if (lastVertex < firstVertex) {
            val aux = lastVertex
            lastVertex = firstVertex
            firstVertex = aux
         }

         (isPath, firstVertex, lastVertex)
      }

      val mapKeyFunc: (EdgeInducedSubgraph, Computation[EdgeInducedSubgraph], PairWritable[IntWritable,
         IntWritable]) => (PairWritable[IntWritable,IntWritable]) = (s,c,k) => {
         val (_, firstVertex, lastVertex) = checkPath(s)
         val p = new PairWritable(new IntWritable(firstVertex),
            new IntWritable(lastVertex))
         logFunc(s"MapKey iteration=-1 vertices=${s.getVertices} " +
            s"key=${p}")
         p
      }

      val mapValueFunc: (EdgeInducedSubgraph,
         Computation[EdgeInducedSubgraph], IntArrayList) => (IntArrayList) =
         (s,c,v) => {
         logFunc(s"MapValue iteration=-1 vertices=${s.getVertices} " +
            s"value=${s.getVertices}")
         s.getVertices
      }

      val reduceFunc: (IntArrayList, IntArrayList) => IntArrayList =
         (value1, value2) => {
         logFunc(s"Reduce iteration=-1 value1=${value1} value2=${value2}")
         value1
      }


      val bootstrap = fgraph.efractoid.
         expand(1).
         aggregate[PairWritable[IntWritable, IntWritable], IntArrayList](
            SP_AGG,
            mapKeyFunc,
            mapValueFunc,
            reduceFunc,
            isIncremental = true)

      var fpaths = bootstrap
      var spaths = fpaths.aggregationMap[PairWritable[IntWritable, IntWritable], IntArrayList](SP_AGG)

      val sc = fgraph.fractalContext.sparkContext
      var allPaths = Map.empty[(Int, Int), IntArrayList]
      spaths.foreach(kv => allPaths.put((kv._1.getLeft().get(), kv._1.getRight().get()), kv._2))

      spaths.foreach(kv => logFunc(s"Spaths iteration=-1 ${kv}"))
      allPaths.foreach(kv => logFunc(s"AllPaths iteration=-1 ${kv}"))

      var remainingSteps = numSteps - 1
      var continue = true
      var iteration = 0

      while (continue) {
         val previous_size = allPaths.size
         val allPathsBc = sc.broadcast(allPaths)

         fpaths = fpaths.
            filter ((s, c) => {
               val (isPath, firstVertex, lastVertex) = checkPath(s)
               isPath &&
                  allPathsBc.value.contains((firstVertex, lastVertex))
            }).
            expand(1).
            filter { (s, c) => {
               val (isPath, firstVertex, lastVertex) = checkPath(s)
               val pathNotFound = !allPathsBc.value.contains(
                  (firstVertex, lastVertex))

               logFunc(s"Filtering iteration=${iteration} step=${c.getStep}" +
                  s" partitionId=${c.getPartitionId}" +
                  s" subgraph=${s.getVertices} pathFound=${pathNotFound}" +
                  s" isPath=${isPath} mapSize=${allPathsBc.value.size}" +
                  s" mapIdentity=${System.identityHashCode(allPathsBc.value)}")

               pathNotFound && isPath
            }
            }.
            aggregate[PairWritable[IntWritable, IntWritable], IntArrayList](
               SP_AGG,
               mapKeyFunc,
               mapValueFunc,
               reduceFunc,
               isIncremental = true)

         spaths = fpaths.aggregationMap[PairWritable[IntWritable, IntWritable], IntArrayList](SP_AGG)
         val newAllPaths = Map.empty[(Int, Int), IntArrayList]
         allPaths.foreach(kv => newAllPaths.put(kv._1, kv._2))
         spaths.foreach(kv =>
            newAllPaths.put((kv._1.getLeft().get(), kv._1.getRight().get()), kv._2))
         allPaths = newAllPaths
         val new_size = allPaths.size

         spaths.foreach(kv => logFunc(s"Spaths" +
            s" iteration=${iteration} ${kv}"))
         allPaths.foreach(kv => logFunc(s"AllPaths" +
            s" iteration=${iteration} ${kv}"))

         remainingSteps -= 1
         continue = previous_size < new_size && remainingSteps > 0
         logFunc(s"IterationFinished iteration=${iteration}" +
            s" remainingSteps=${remainingSteps}" +
            s" previousSize=${previous_size} newSize=${new_size}" +
            s" continue=${continue}")

         iteration += 1
      }

      allPaths
   }
}
