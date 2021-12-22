// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef SKY_PATTERN_TREE_H_
#define SKY_PATTERN_TREE_H_

#include "Tree.h"

struct skyPatternHasher
{
  size_t operator()(const IndistinctSkyPatterns* indistinctSkyPattern) const
  {
    const vector<unsigned int> minSizeMeasures = indistinctSkyPattern->getMinSizeMeasures();
    return boost::hash_range(minSizeMeasures.begin(), minSizeMeasures.end());
  }
};

struct skyPatternEqual
{
  bool operator()(const IndistinctSkyPatterns* indistinctSkyPattern1, const IndistinctSkyPatterns* indistinctSkyPattern2) const
  {
    return *indistinctSkyPattern1 == *indistinctSkyPattern2;
  }
};

class SkyPatternTree final : public Tree
{
 public:
  SkyPatternTree() = delete;
  SkyPatternTree(const SkyPatternTree&) = delete;
  SkyPatternTree(SkyPatternTree&&) = delete;
  SkyPatternTree(const char* dataFileName, const float densityThreshold, const vector<double>& epsilonVector, const vector<unsigned int>& cliqueDimensions, const vector<double>& tauVector, const vector<unsigned int>& minSizes, const unsigned int minArea, const bool isReductionOnly, const vector<unsigned int>& unclosedDimensions, const char* inputElementSeparator, const char* inputDimensionSeparator, const char* outputFileName, const char* outputDimensionSeparator, const char* patternSizeSeparator, const char* sizeSeparator, const char* sizeAreaSeparator, const bool isSizePrinted, const bool isAreaPrinted, const bool isIntermediateSkylinePrinted);

  ~SkyPatternTree();

  SkyPatternTree& operator=(const SkyPatternTree&) = delete;
  SkyPatternTree& operator=(SkyPatternTree&&) = delete;

  void initMeasures(const vector<unsigned int>& maxSizes, const int maxArea, const vector<unsigned int>& maximizedSizeDimensions, const vector<unsigned int>& minimizedSizeDimensions, const bool isAreaMaximized, const bool isAreaMinimized, const vector<string>& groupFileNames, const vector<unsigned int>& groupMinSizes, const vector<unsigned int>& groupMaxSizes, const vector<vector<float>>& groupMinRatios, const vector<vector<float>>& groupMinPiatetskyShapiros, const vector<vector<float>>& groupMinLeverages, const vector<vector<float>>& groupMinForces, const vector<vector<float>>& groupMinYulesQs, const vector<vector<float>>& groupMinYulesYs, const char* groupElementSeparator, const char* groupDimensionElementsSeparator, vector<unsigned int>& groupMaximizedSizes, const vector<unsigned int>& groupMinimizedSizes, const vector<vector<float>>& groupMaximizedRatios, const vector<vector<float>>& groupMaximizedPiatetskyShapiros, const vector<vector<float>>& groupMaximizedLeverages, const vector<vector<float>>& groupMaximizedForces, const vector<vector<float>>& groupMaximizedYulesQs, const vector<vector<float>>& groupMaximizedYulesYs, const char* sumValueFileName, const float minSum, const char* valueElementSeparator, const char* valueDimensionSeparator, const bool isSumMaximized, const char* slopePointFileName, const float minSlope, const char* pointElementSeparator, const char* pointDimensionSeparator, const bool isSlopeMaximized, const float densityThreshold);
  void terminate();

 private:
  vector<Measure*> measuresToMaximize;

  static unsigned int nonMinSizeMeasuresIndex;
  static vector<IndistinctSkyPatterns*> skyPatterns;
  static unordered_set<IndistinctSkyPatterns*, skyPatternHasher, skyPatternEqual> minSizeSkyline;

  static bool isSomeOptimizedMeasureNotMonotone;
  static bool isIntermediateSkylinePrinted;

  SkyPatternTree(const SkyPatternTree& parent, const unsigned int presentAttributeId, const vector<Measure*>& mereConstraints, const vector<Measure*>& measuresToMaximize);

  bool leftSubtree(const Attribute& presentAttribute);

  bool violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent);
  bool violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent);
  bool dominated();
/* #ifdef MIN_SIZE_ELEMENT_PRUNING */
/*   vector<unsigned int> minSizeIrrelevancyThresholds() const; */
/* #endif */
  void validPattern() const;
  void printNSets(const vector<vector<vector<unsigned int>>>& nSets, ostream& out) const;

  static bool dominated(const vector<Measure*>& measuresToMaximize);
};

#endif /*SKY_PATTERN_TREE_H_*/
