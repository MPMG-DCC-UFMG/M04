// Copyright (C) 2013-2021 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef TREE_H_
#define TREE_H_

#include "../utilities/NoOutputException.h"
#include "../measures/MinSize.h"
#include "../measures/MaxSize.h"
#include "../measures/MinArea.h"
#include "../measures/MaxArea.h"
#include "../measures/MinGroupCover.h"
#include "../measures/MaxGroupCover.h"
#include "../measures/MinGroupCoverRatio.h"
#include "../measures/MinGroupCoverPiatetskyShapiro.h"
#include "../measures/MinGroupCoverLeverage.h"
#include "../measures/MinGroupCoverForce.h"
#include "../measures/MinGroupCoverYulesQ.h"
#include "../measures/MinGroupCoverYulesY.h"
#include "../measures/MinSum.h"
#include "../measures/MinSlope.h"
#include "SymmetricAttribute.h"
#include "MetricAttribute.h"
#include "Trie.h"
#include "NoisyTupleFileReader.h"
#include "NoisyTuples.h"

#if defined TIME || defined DETAILED_TIME || defined ELEMENT_CHOICE_TIME || defined MIN_SIZE_ELEMENT_PRUNING_TIME
#include <chrono>

using namespace std::chrono;
#endif

class Tree
{
 public:
  Tree() = delete;
  Tree(const Tree&) = delete;
  Tree(Tree&&) = delete;
  Tree(const char* dataFileName, const float densityThreshold, const vector<double>& epsilonVector, const vector<unsigned int>& cliqueDimensions, const vector<double>& tauVector, const vector<unsigned int>& minSizes, const unsigned int minArea, const bool isReductionOnly, const vector<unsigned int>& unclosedDimensions, const char* inputElementSeparator, const char* inputDimensionSeparator, const char* outputFileName, const char* outputDimensionSeparator, const char* patternSizeSeparator, const char* sizeSeparator, const char* sizeAreaSeparator, const bool isSizePrinted, const bool isAreaPrinted);

  virtual ~Tree();

  Tree& operator=(const Tree&) = delete;
  Tree& operator=(Tree&&) = delete;

  void initMeasures(const vector<unsigned int>& maxSizes, const int maxArea, const vector<string>& groupFileNames, const vector<unsigned int>& groupMinSizes, const vector<unsigned int>& groupMaxSizes, const vector<vector<float>>& groupMinRatios, const vector<vector<float>>& groupMinPiatetskyShapiros, const vector<vector<float>>& groupMinLeverages, const vector<vector<float>>& groupMinForces, const vector<vector<float>>& groupMinYulesQs, const vector<vector<float>>& groupMinYulesYs, const char* groupElementSeparator, const char* groupDimensionElementsSeparator, const char* sumValueFileName, const float minSum, const char* valueElementSeparator, const char* valueDimensionSeparator, const char* slopePointFileName, const float minSlope, const char* pointElementSeparator, const char* pointDimensionSeparator, const float densityThreshold);
  void mine();
  virtual void terminate();

 protected:
  vector<Attribute*> attributes;
  vector<Measure*> mereConstraints;
  bool isEnumeratedElementPotentiallyPreventingClosedness;

  static vector<unsigned int> external2InternalAttributeOrder;
  static vector<unordered_map<string, unsigned int>> labels2Ids;
  static Trie* data;

  static ofstream outputFile;
  static string outputDimensionSeparator;
  static string patternSizeSeparator;
  static string sizeSeparator;
  static string sizeAreaSeparator;
  static bool isSizePrinted;
  static bool isAreaPrinted;

#ifdef NB_OF_CLOSED_N_SETS
  static unsigned int nbOfClosedNSets;
#endif

  Tree(const Tree& parent, const unsigned int presentAttributeId, const vector<Measure*>& mereConstraints);

  void setPresent(const unsigned int presentAttributeId, const vector<Attribute*>& parentAttributes);

  virtual bool violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent);
  virtual bool violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent);

  static void setMinParametersInClique(vector<unsigned int>& parameterVector);
  static vector<Measure*> childMeasures(const vector<Measure*>& parentMeasures, const unsigned int presentAttributeId, const unsigned int presentValueId);
  static void deleteMeasures(vector<Measure*>& measures);
  static bool monotone(const Measure* measure);

 private:
  static unsigned int firstSymmetricAttributeId;
  static unsigned int secondSymmetricAttributeId;

#ifdef TIME
  static steady_clock::time_point overallBeginning;
#endif
#ifdef DETAILED_TIME
  static steady_clock::time_point startingPoint;
  static double parsingDuration;
  static double preProcessingDuration;
#endif
#ifdef ELEMENT_CHOICE_TIME
  static double valueChoiceDuration;
#endif
#ifdef MIN_SIZE_ELEMENT_PRUNING_TIME
  static double minSizeElementPruningDuration;
#endif
  // CLEAN: All these static attributes should not be static for a detailed analysis of the enumeration
#ifdef NB_OF_LEFT_NODES
  static unsigned int nbOfLeftNodes;
#endif

  friend ostream& operator<<(ostream& out, const Tree& tree);
#ifdef DEBUG
  void printNode(ostream& out) const;
#endif

  void peel();

  virtual bool leftSubtree(const Attribute& presentAttribute);
  void rightSubtree(Attribute& absentAttribute, const bool isLastEnumeratedElementPotentiallyPreventingClosedness);

  bool setIrrelevant();
#ifdef MIN_SIZE_ELEMENT_PRUNING
  void resetPresentAndPotentialIrrelevancyThresholdsExceptOn(const vector<Attribute*>::iterator absentAttributeIt);
  bool findMinSizeIrrelevantValuesAndCheckConstraints();
#endif

  virtual bool dominated();
  virtual void validPattern() const;

#ifdef ASSERT
  void assertNoiseOnValue(const vector<Attribute*>::const_iterator attributeIt, const Value& value) const;
  void assertPresentNoiseAtIntersection(const vector<Attribute*>::const_iterator firstAttributeIt, const Value& firstValue, const vector<Attribute*>::const_iterator secondAttributeIt, const Value& secondValue) const;
  void assertPresentAndPotentialNoiseAtIntersection(const vector<Attribute*>::const_iterator firstAttributeIt, const Value& firstValue, const vector<Attribute*>::const_iterator secondAttributeIt, const Value& secondValue) const;
  void assertNoiseAtIntersectionsWithPotentialAndAbsent(const vector<Attribute*>::const_iterator presentAttributeIt, const Value& presentValue) const;
  void assertNoiseAtIntersectionsWithPresentAndPotential(const vector<Attribute*>::const_iterator absentAttributeIt, const Value& absentValue) const;
#endif

  static void setMaxParametersInClique(vector<unsigned int>& parameterVector);
};

#endif /*TREE_H_*/
