package br.ufmg.cs.systems.fractal.subgraph

import java.io.{DataInput, DataOutput}

import br.ufmg.cs.systems.fractal.conf.SparkConfiguration
import br.ufmg.cs.systems.fractal.graph.{BasicMainGraph, MainGraph}

import scala.collection.mutable.ArrayBuffer

/** An edge induced subgraph
  *
  * @param words integer array indicating the subgraph edges
  */
case class ESubgraph(var words: Array[(Int,Int)],
                     var mappedWords: ArrayBuffer[String] = null)
    extends ResultSubgraph[(Int,Int)] {

  // must have because we are messing around with Writables
  def this() = {
    this(null.asInstanceOf[Array[(Int,Int)]])
  }

  def toMappedSubgraph(config: SparkConfiguration[_]): ResultSubgraph[_] = {
    val mainGraph = config.getMainGraph[MainGraph[_,_]]
    mappedWords = ArrayBuffer.empty
    var i = 0
    while (i < words.length) {
      val (src, dest) = words(i)
      val edgeIdsCur = mainGraph.getVertexNeighbourhood(src).
        getEdgesWithNeighbourVertex(dest).cursor()
      while (edgeIdsCur.moveNext()) {
        mappedWords += mainGraph.getVertex(edgeIdsCur.elem()).getVertexOriginalId
      }
      i += 1
    }
    this
  }

  def toInternalSubgraph[E <: Subgraph](config: SparkConfiguration[E]): E = {
    val mainGraph = config.getMainGraph[BasicMainGraph[_,_]]
    val subgraph = config.createSubgraph[E]
    var i = 0
    while (i < words.length) {
      val (src, dest) = words(i)
      val edgeIdsCur = mainGraph.getVertexNeighbourhood(src).
        getEdgesWithNeighbourVertex(dest).cursor()
      while (edgeIdsCur.moveNext()) {
        subgraph.addWord(edgeIdsCur.elem())
      }
      i += 1
    }
    subgraph
  }

  def combinations(k: Int): Iterator[ESubgraph] = {
    words.combinations(k).map (new ESubgraph(_))
  }

  override def write(out: DataOutput): Unit = {
    out.writeInt (words.size)
    words.foreach {w => out.writeInt(w._1); out.writeInt(w._2)}
  }

  override def readFields(in: DataInput): Unit = {
    val wordsLen = in.readInt
    words = new Array [(Int,Int)] (wordsLen)
    for (i <- 0 until wordsLen) words(i) = (in.readInt, in.readInt)
  }

  override def toString = {
    s"ESubgraph(${vertices.mkString (", ")})"
  }
}

/**
  * An edge induced subgraph
  */
object ESubgraph {
  def apply (strSubgraph: String) = {
    val edgesStr = strSubgraph split "\\s+"
    val edges = new Array[(Int,Int)](edgesStr.size)
    for (i <- 0 until edges.size) {
      val words = (edgesStr(i) split "-").map (_.toInt)
      edges(i) = (words(0), words(1))
    }

    new ESubgraph (edges)
  }
}