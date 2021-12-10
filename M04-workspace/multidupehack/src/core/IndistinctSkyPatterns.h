// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef INDISTINCT_SKY_PATTERNS_H_
#define INDISTINCT_SKY_PATTERNS_H_

#include <vector>
#include <ostream>

using namespace std;

class IndistinctSkyPatterns
{
 public:
  IndistinctSkyPatterns(const vector<vector<unsigned int>>& pattern, const vector<float>& maximizedMeasures);

  bool operator==(const IndistinctSkyPatterns& indistinctSkyPatterns) const; /* equality if indistinct w.r.t. maximized sizes and area */
  friend ostream& operator<<(ostream& out, const IndistinctSkyPatterns& indistinctSkyPatterns);

  const vector<vector<vector<unsigned int>>>& getPatterns() const;
  void insert(const vector<vector<unsigned int>>& pattern);
  vector<unsigned int> getMinSizeMeasures() const; /* returns the maximized sizes followed by the area if it is maximized */
  bool distinct(const vector<float>& otherMaximizedMeasures) const;
  bool indistinctOrDominates(const vector<float>& otherMaximizedMeasures) const;
  bool indistinctOrDominatedBy(const vector<float>& otherMaximizedMeasures) const;
  bool minSizeDistinct(const vector<unsigned int>& minSizeMeasures) const; /* minSizeMeasures must contains the optimistic values for the maximized sizes and, if the area is maximized for that measure too (back of the vector) */
  bool minSizeIndistinctOrDominates(const vector<unsigned int>& minSizeMeasures) const; /* minSizeMeasures must contains the optimistic values for the maximized sizes and, if the area is maximized for that measure too (back of the vector) */
  bool minSizeIndistinctOrDominatedBy(const vector<unsigned int>& minSizeMeasures) const; /* minSizeMeasures must contains the optimistic values for the maximized sizes and, if the area is maximized for that measure too (back of the vector) */
#ifdef MIN_SIZE_ELEMENT_PRUNING
  vector<unsigned int> minNbOfNonSelfLoopsTuplesInANonDominatedPattern(const vector<unsigned int>& minimalPatternSizes, const vector<unsigned int>& maximalPatternSizes, const unsigned int areaOfMinimalPattern) const; /* some size or area must be to be maximized */

  static bool noSizeOrAreaMaximized();
#endif
  static void setParametersToComputePresentAndPotentialIrrelevancyThresholds(const unsigned int firstSymmetricAttributeId, const unsigned int lastSymmetricAttributeId);
  static void setParametersToComputePresentAndPotentialIrrelevancyThresholds(const vector<unsigned int>& maximizedSizeDimensionIds, const bool isAreaMaximized);
  static unsigned int nbOfNonSelfLoopTuplesInHyperplaneOfPattern(const vector<unsigned int>& sizes, const unsigned int hyperplaneDimensionId, const unsigned int nbOfSymmetricElements);
  /* static const unsigned int nbOfNonSelfLoopTuplesInPattern(const vector<unsigned int>& sizes, const unsigned int nbOfSymmetricElements); */

 private:
  vector<vector<vector<unsigned int>>> patterns;
  const vector<float> maximizedMeasures;
  unsigned int area;

  static unsigned int firstSymmetricAttributeId;
  static unsigned int lastSymmetricAttributeId;
  static vector<unsigned int> maximizedSizeDimensionIds;
  static bool isAreaMaximized;

#ifdef MIN_SIZE_ELEMENT_PRUNING
  unsigned int minNbOfNonSelfLoopTuplesInHyperplaneOfPatternAccordingToArea(const unsigned int maximalPatternSize, const unsigned int hyperplaneDimensionId, const unsigned int minNbOfSymmetricElements) const;
  unsigned int minNbOfNonSelfLoopsTuplesInHyperplaneOfIndistinctPattern(const vector<unsigned int>& minimalPatternSizes, const vector<unsigned int>& maximalPatternSizes, const unsigned int hyperplaneDimensionId, const unsigned int minNbOfSymmetricElements) const;
  unsigned int minNbOfNonSelfLoopsTuplesInHyperplaneOfNonDominatedDistinctPattern(const vector<unsigned int>& minimalPatternSizes, const vector<unsigned int>& maximalPatternSizes, const unsigned int hyperplaneDimensionId, const unsigned int minNbOfSymmetricElements, const unsigned int sizeOfSymmetricDimensionPlusOne) const;
#endif
};

#endif /*INDISTINCT_SKY_PATTERN_H_*/
