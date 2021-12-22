// Copyright (C) 2013-2020 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "IndistinctSkyPatterns.h"

unsigned int IndistinctSkyPatterns::firstSymmetricAttributeId;
unsigned int IndistinctSkyPatterns::lastSymmetricAttributeId;
vector<unsigned int> IndistinctSkyPatterns::maximizedSizeDimensionIds;
bool IndistinctSkyPatterns::isAreaMaximized;

IndistinctSkyPatterns::IndistinctSkyPatterns(const vector<vector<unsigned int>>& pattern, const vector<float>& maximizedMeasuresParam): patterns {pattern}, maximizedMeasures(maximizedMeasuresParam), area(1)
{
  if (isAreaMaximized)
    {
      for (const vector<unsigned int>& dimension : pattern)
	{
	  area *= dimension.size();
	}
    }
}

bool IndistinctSkyPatterns::operator==(const IndistinctSkyPatterns& otherIndistinctSkyPatterns) const
{
  vector<unsigned int>::const_iterator maximizedSizeDimensionIdIt = maximizedSizeDimensionIds.begin();
  for (; maximizedSizeDimensionIdIt != maximizedSizeDimensionIds.end() && patterns.back()[*maximizedSizeDimensionIdIt].size() == otherIndistinctSkyPatterns.patterns.back()[*maximizedSizeDimensionIdIt].size(); ++maximizedSizeDimensionIdIt)
    {
    }
  if (maximizedSizeDimensionIdIt == maximizedSizeDimensionIds.end())
    {
      return area == otherIndistinctSkyPatterns.area;
    }
  return false;
}

ostream& operator<<(ostream& out, const IndistinctSkyPatterns& indistinctSkyPatterns)
{
  out << '(';
  bool isFirst = true;
  for (const float maximizedMeasure : indistinctSkyPatterns.maximizedMeasures)
    {
      if (isFirst)
	{
	  isFirst = false;
	}
      else
	{
	  out << ',';
	}
      out << maximizedMeasure;
    }
  for (const unsigned int maximizedSizeDimensionId : indistinctSkyPatterns.maximizedSizeDimensionIds)
    {
      if (isFirst)
	{
	  isFirst = false;
	}
      else
	{
	  out << ',';
	}
      out << indistinctSkyPatterns.patterns.back()[maximizedSizeDimensionId].size();
    }
  if (IndistinctSkyPatterns::isAreaMaximized)
    {
      if (isFirst)
	{
	  return out << indistinctSkyPatterns.area << ')';
	}
      return out << ',' << indistinctSkyPatterns.area << ')';
    }
  return out << ')';
}

const vector<vector<vector<unsigned int>>>& IndistinctSkyPatterns::getPatterns() const
{
  return patterns;
}

void IndistinctSkyPatterns::insert(const vector<vector<unsigned int>>& pattern)
{
  patterns.push_back(pattern);
}

vector<unsigned int> IndistinctSkyPatterns::getMinSizeMeasures() const
{
  vector<unsigned int> minSizeMeasures;
  if (isAreaMaximized)
    {
      minSizeMeasures.reserve(maximizedSizeDimensionIds.size() + 1);
      for (const unsigned int maximizedSizeDimensionId : maximizedSizeDimensionIds)
	{
	  minSizeMeasures.push_back(patterns.back()[maximizedSizeDimensionId].size());
	}
      minSizeMeasures.push_back(area);
      return minSizeMeasures;
    }
  minSizeMeasures.reserve(maximizedSizeDimensionIds.size());
  for (const unsigned int maximizedSizeDimensionId : maximizedSizeDimensionIds)
    {
      minSizeMeasures.push_back(patterns.back()[maximizedSizeDimensionId].size());
    }
  return minSizeMeasures;
}

bool IndistinctSkyPatterns::distinct(const vector<float>& otherMaximizedMeasures) const
{
  vector<float>::const_iterator otherMeasureIt = otherMaximizedMeasures.begin();
  for (vector<float>::const_iterator maximizedMeasureIt = maximizedMeasures.begin(); maximizedMeasureIt != maximizedMeasures.end() && *maximizedMeasureIt == *otherMeasureIt; ++maximizedMeasureIt)
    {
      ++otherMeasureIt;
    }
  return otherMeasureIt != otherMaximizedMeasures.end();
}

bool IndistinctSkyPatterns::indistinctOrDominates(const vector<float>& otherMaximizedMeasures) const
{
  vector<float>::const_iterator otherMeasureIt = otherMaximizedMeasures.begin();
  for (vector<float>::const_iterator maximizedMeasureIt = maximizedMeasures.begin(); maximizedMeasureIt != maximizedMeasures.end() && *maximizedMeasureIt >= *otherMeasureIt; ++maximizedMeasureIt)
    {
      ++otherMeasureIt;
    }
  return otherMeasureIt == otherMaximizedMeasures.end();
}

bool IndistinctSkyPatterns::indistinctOrDominatedBy(const vector<float>& otherMaximizedMeasures) const
{
  vector<float>::const_iterator otherMeasureIt = otherMaximizedMeasures.begin();
  for (vector<float>::const_iterator maximizedMeasureIt = maximizedMeasures.begin(); maximizedMeasureIt != maximizedMeasures.end() && *maximizedMeasureIt <= *otherMeasureIt; ++maximizedMeasureIt)
    {
      ++otherMeasureIt;
    }
  return otherMeasureIt == otherMaximizedMeasures.end();
}

bool IndistinctSkyPatterns::minSizeDistinct(const vector<unsigned int>& minSizeMeasures) const
{
  vector<unsigned int>::const_iterator maximizedSizeDimensionIdIt = maximizedSizeDimensionIds.begin();
  for (vector<unsigned int>::const_iterator measureIt = minSizeMeasures.begin(); maximizedSizeDimensionIdIt != maximizedSizeDimensionIds.end() && patterns.back()[*maximizedSizeDimensionIdIt].size() == *measureIt; ++measureIt)
    {
      ++maximizedSizeDimensionIdIt;
    }
  if (maximizedSizeDimensionIdIt == maximizedSizeDimensionIds.end())
    {
      return isAreaMaximized && area != minSizeMeasures.back();
    }
  return true;
}

bool IndistinctSkyPatterns::minSizeIndistinctOrDominates(const vector<unsigned int>& minSizeMeasures) const
{
  vector<unsigned int>::const_iterator maximizedSizeDimensionIdIt = maximizedSizeDimensionIds.begin();
  for (vector<unsigned int>::const_iterator measureIt = minSizeMeasures.begin(); maximizedSizeDimensionIdIt != maximizedSizeDimensionIds.end() && patterns.back()[*maximizedSizeDimensionIdIt].size() >= *measureIt; ++measureIt)
    {
      ++maximizedSizeDimensionIdIt;
    }
  if (maximizedSizeDimensionIdIt != maximizedSizeDimensionIds.end())
    {
      return false;
    }
  if (isAreaMaximized)
    {
      return area >= minSizeMeasures.back();
    }
  return true;
}

bool IndistinctSkyPatterns::minSizeIndistinctOrDominatedBy(const vector<unsigned int>& minSizeMeasures) const
{
  vector<unsigned int>::const_iterator maximizedSizeDimensionIdIt = maximizedSizeDimensionIds.begin();
  for (vector<unsigned int>::const_iterator measureIt = minSizeMeasures.begin(); maximizedSizeDimensionIdIt != maximizedSizeDimensionIds.end() && patterns.back()[*maximizedSizeDimensionIdIt].size() <= *measureIt; ++measureIt)
    {
      ++maximizedSizeDimensionIdIt;
    }
  if (maximizedSizeDimensionIdIt != maximizedSizeDimensionIds.end())
    {
      return false;
    }
  if (isAreaMaximized)
    {
      return area <= minSizeMeasures.back();
    }
  return true;
}

#ifdef MIN_SIZE_ELEMENT_PRUNING
unsigned int IndistinctSkyPatterns::minNbOfNonSelfLoopTuplesInHyperplaneOfPatternAccordingToArea(const unsigned int maximalPatternSize, const unsigned int hyperplaneDimensionId, const unsigned int minNbOfSymmetricElements) const
{
  if (!maximalPatternSize)
    {
      return numeric_limits<unsigned int>::max();
    }
  if (!minNbOfSymmetricElements)
    {
      return area / maximalPatternSize + 1;
    }
  if (hyperplaneDimensionId < firstSymmetricAttributeId || hyperplaneDimensionId > lastSymmetricAttributeId)
    {
      // TODO: check whether round-off errors are problematic (round instead of floor?)
      return area / maximalPatternSize - floor(static_cast<double>(area) / pow(minNbOfSymmetricElements, lastSymmetricAttributeId - firstSymmetricAttributeId)) + 1;
    }
  // TODO: check whether round-off errors are problematic (round instead of floor?)
  return area / maximalPatternSize - floor(static_cast<double>(area) / pow(minNbOfSymmetricElements, lastSymmetricAttributeId - firstSymmetricAttributeId + 1)) + 1;
}

unsigned int IndistinctSkyPatterns::minNbOfNonSelfLoopsTuplesInHyperplaneOfIndistinctPattern(const vector<unsigned int>& minimalPatternSizes, const vector<unsigned int>& maximalPatternSizes, const unsigned int hyperplaneDimensionId, const unsigned int minNbOfSymmetricElements) const
{
  vector<unsigned int> indistinctPatternSizes(minimalPatternSizes);
  const vector<vector<unsigned int>>& onePattern = patterns.back();
  for (const unsigned int maximizedSizeDimensionId : maximizedSizeDimensionIds)
    {
      if (maximizedSizeDimensionId != hyperplaneDimensionId)
	{
	  const unsigned int size = onePattern[maximizedSizeDimensionId].size();
	  if (size > maximalPatternSizes[maximizedSizeDimensionId])
	    {
	      // The indistinct pattern cannot be obtained given the maximal pattern
	      return numeric_limits<unsigned int>::max();
	    }
	  indistinctPatternSizes[maximizedSizeDimensionId] = size;
	}
    }
  return nbOfNonSelfLoopTuplesInHyperplaneOfPattern(indistinctPatternSizes, hyperplaneDimensionId, minNbOfSymmetricElements);
}

unsigned int IndistinctSkyPatterns::minNbOfNonSelfLoopsTuplesInHyperplaneOfNonDominatedDistinctPattern(const vector<unsigned int>& minimalPatternSizes, const vector<unsigned int>& maximalPatternSizes, const unsigned int hyperplaneDimensionId, const unsigned int minNbOfSymmetricElements, const unsigned int sizeOfSymmetricDimensionPlusOne) const
{
  unsigned int nbOfNonSelfLoopTuples = numeric_limits<unsigned int>::max();
  const vector<vector<unsigned int>>& onePattern = patterns.back();
  for (const unsigned int maximizedSizeDimensionId : maximizedSizeDimensionIds)
    {
      if (maximizedSizeDimensionId < firstSymmetricAttributeId || maximizedSizeDimensionId > lastSymmetricAttributeId)
	{
	  if (maximizedSizeDimensionId != hyperplaneDimensionId)
	    {
	      const unsigned int size = onePattern[maximizedSizeDimensionId].size();
	      if (size < maximalPatternSizes[maximizedSizeDimensionId])
		{
		  // The non-dominated distinct pattern can be obtained given the maximal pattern
		  const unsigned int oldValue = minimalPatternSizes[maximizedSizeDimensionId];
		  const_cast<vector<unsigned int>&>(minimalPatternSizes)[maximizedSizeDimensionId] = size + 1;
		  const unsigned int tmp = nbOfNonSelfLoopTuplesInHyperplaneOfPattern(minimalPatternSizes, hyperplaneDimensionId, minNbOfSymmetricElements);
		  if (tmp < nbOfNonSelfLoopTuples)
		    {
		      nbOfNonSelfLoopTuples = tmp;
		    }
		  const_cast<vector<unsigned int>&>(minimalPatternSizes)[maximizedSizeDimensionId] = oldValue;
		}
	    }
        }
      else
	{
	  if (sizeOfSymmetricDimensionPlusOne && maximizedSizeDimensionId == lastSymmetricAttributeId)
	    {
	      const unsigned int oldValue = minimalPatternSizes[maximizedSizeDimensionId];
	      for (unsigned int otherSymmetricAttributeId = firstSymmetricAttributeId; otherSymmetricAttributeId <= lastSymmetricAttributeId; ++otherSymmetricAttributeId)
		{
		  const_cast<vector<unsigned int>&>(minimalPatternSizes)[otherSymmetricAttributeId] = sizeOfSymmetricDimensionPlusOne;
	        }
              const unsigned int tmp = nbOfNonSelfLoopTuplesInHyperplaneOfPattern(minimalPatternSizes, hyperplaneDimensionId, minNbOfSymmetricElements);
	      if (tmp < nbOfNonSelfLoopTuples)
		{
		  nbOfNonSelfLoopTuples = tmp;
		}
	      for (unsigned int otherSymmetricAttributeId = firstSymmetricAttributeId; otherSymmetricAttributeId <= lastSymmetricAttributeId; ++otherSymmetricAttributeId)
		{
		  const_cast<vector<unsigned int>&>(minimalPatternSizes)[otherSymmetricAttributeId] = oldValue;
	        }
	    }
	}
    }
  return nbOfNonSelfLoopTuples;
}

vector<unsigned int> IndistinctSkyPatterns::minNbOfNonSelfLoopsTuplesInANonDominatedPattern(const vector<unsigned int>& minimalPatternSizes, const vector<unsigned int>& maximalPatternSizes, const unsigned int minimalPatternArea) const
{
  if (isAreaMaximized && minimalPatternArea > area)
    {
      // *this does not dominate the minimal pattern w.r.t. area
      return {};
    }
  const vector<vector<unsigned int>>& onePattern = patterns.back();
  for (const unsigned int maximizedSizeDimensionId : maximizedSizeDimensionIds)
    {
      if (minimalPatternSizes[maximizedSizeDimensionId] > onePattern[maximizedSizeDimensionId].size())
	{
	  // *this does not dominate the minimal pattern w.r.t. this dimension
	  return {};
	}
    }
  const unsigned int n = minimalPatternSizes.size();
  unsigned int minNbOfSymmetricElements;
  unsigned int minNbOfSymmetricElementsAccordingToArea;
  unsigned int sizeOfSymmetricDimensionPlusOne;
  if (firstSymmetricAttributeId == numeric_limits<unsigned int>::max())
    {
      minNbOfSymmetricElements = 0;
      minNbOfSymmetricElementsAccordingToArea = 0;
      sizeOfSymmetricDimensionPlusOne = 0;
    }
  else
    {
      minNbOfSymmetricElements = minimalPatternSizes[firstSymmetricAttributeId];
      if (isAreaMaximized)
	{
	  double minDoubleNbOfSymmetricElementsAccordingToArea = area;
	  vector<unsigned int>::const_iterator maximalPatternSizeIt = maximalPatternSizes.begin();
	  for (unsigned int dimensionId = 0; dimensionId != n; ++dimensionId)
	    {
	      if (dimensionId < firstSymmetricAttributeId || dimensionId > lastSymmetricAttributeId)
		{
		  minDoubleNbOfSymmetricElementsAccordingToArea /= *maximalPatternSizeIt;
		}
	      ++maximalPatternSizeIt;
	    }
	  // TODO: check whether round-off errors are problematic (round instead of ceil?)
	  minNbOfSymmetricElementsAccordingToArea = ceil(pow(minDoubleNbOfSymmetricElementsAccordingToArea, 1. / (lastSymmetricAttributeId - firstSymmetricAttributeId + 1)));
	  if (minNbOfSymmetricElementsAccordingToArea < minNbOfSymmetricElements)
	    {
	      minNbOfSymmetricElementsAccordingToArea = minNbOfSymmetricElements;
	    }
	}
      else
	{
	  minNbOfSymmetricElementsAccordingToArea = 0;
	}
      sizeOfSymmetricDimensionPlusOne = onePattern[firstSymmetricAttributeId].size() + 1;
      if (sizeOfSymmetricDimensionPlusOne > maximalPatternSizes[firstSymmetricAttributeId])
	{
	  // Given the maximal pattern, the non-dominated distinct pattern can be obtained with more symmetric elements
	  sizeOfSymmetricDimensionPlusOne = 0;
	}
    }
  vector<unsigned int> nbOfNonSelfLoopTuples(n, numeric_limits<unsigned int>::max());
  vector<unsigned int>::iterator nbOfNonSelfLoopTuplesIt = nbOfNonSelfLoopTuples.begin();
  vector<vector<unsigned int>>::const_iterator dimensionIt = onePattern.begin();
  vector<unsigned int>::const_iterator maximalPatternSizeIt = maximalPatternSizes.begin();
  vector<unsigned int>::const_iterator maximizedSizeDimensionIdIt = maximizedSizeDimensionIds.begin();
  for (unsigned int dimensionId = 0; dimensionId != n; ++dimensionId)
    {
      if (maximizedSizeDimensionIdIt != maximizedSizeDimensionIds.end() && *maximizedSizeDimensionIdIt == dimensionId)
	{
	  if (*maximalPatternSizeIt > dimensionIt->size())
	    {
	      *nbOfNonSelfLoopTuplesIt = 0;
	    }
	  else
	    {
	      if (*maximalPatternSizeIt == dimensionIt->size())
		{
		  *nbOfNonSelfLoopTuplesIt = minNbOfNonSelfLoopsTuplesInHyperplaneOfIndistinctPattern(minimalPatternSizes, maximalPatternSizes, dimensionId, minNbOfSymmetricElements);
		}
	      const unsigned int tmp = minNbOfNonSelfLoopsTuplesInHyperplaneOfNonDominatedDistinctPattern(minimalPatternSizes, maximalPatternSizes, dimensionId, minNbOfSymmetricElements, sizeOfSymmetricDimensionPlusOne);
	      if (tmp < *nbOfNonSelfLoopTuplesIt)
		{
		  *nbOfNonSelfLoopTuplesIt = tmp;
		}
	      if (isAreaMaximized)
		{
		  const unsigned int minNbOfNonSelfLoopsTuplesAccordingToArea = minNbOfNonSelfLoopTuplesInHyperplaneOfPatternAccordingToArea(maximalPatternSizes[dimensionId], dimensionId, minNbOfSymmetricElementsAccordingToArea);
		  if (minNbOfNonSelfLoopsTuplesAccordingToArea < *nbOfNonSelfLoopTuplesIt)
		    {
		      *nbOfNonSelfLoopTuplesIt = minNbOfNonSelfLoopsTuplesAccordingToArea;
		    }
		}
	    }
	  ++maximizedSizeDimensionIdIt;
	}
      else
	{
	  if (!maximizedSizeDimensionIds.empty())
	    {
	      *nbOfNonSelfLoopTuplesIt = minNbOfNonSelfLoopsTuplesInHyperplaneOfIndistinctPattern(minimalPatternSizes, maximalPatternSizes, dimensionId, minNbOfSymmetricElements);
	      const unsigned int tmp = minNbOfNonSelfLoopsTuplesInHyperplaneOfNonDominatedDistinctPattern(minimalPatternSizes, maximalPatternSizes, dimensionId, minNbOfSymmetricElements, sizeOfSymmetricDimensionPlusOne);
	      if (tmp < *nbOfNonSelfLoopTuplesIt)
		{
		  *nbOfNonSelfLoopTuplesIt = tmp;
		}
	    }
	  if (isAreaMaximized)
	    {
	      const unsigned int minNbOfNonSelfLoopsTuplesAccordingToArea = minNbOfNonSelfLoopTuplesInHyperplaneOfPatternAccordingToArea(maximalPatternSizes[dimensionId], dimensionId, minNbOfSymmetricElementsAccordingToArea);
	      if (minNbOfNonSelfLoopsTuplesAccordingToArea < *nbOfNonSelfLoopTuplesIt)
		{
		  *nbOfNonSelfLoopTuplesIt = minNbOfNonSelfLoopsTuplesAccordingToArea;
		}
	    }
	}
      ++nbOfNonSelfLoopTuplesIt;
      ++dimensionIt;
      ++maximalPatternSizeIt;
    }
  return nbOfNonSelfLoopTuples;
}

bool IndistinctSkyPatterns::noSizeOrAreaMaximized()
{
  return maximizedSizeDimensionIds.empty() && !isAreaMaximized;
}
#endif

void IndistinctSkyPatterns::setParametersToComputePresentAndPotentialIrrelevancyThresholds(const unsigned int firstSymmetricAttributeIdParam, const unsigned int lastSymmetricAttributeIdParam)
{
  firstSymmetricAttributeId = firstSymmetricAttributeIdParam;
  lastSymmetricAttributeId = lastSymmetricAttributeIdParam;
}

void IndistinctSkyPatterns::setParametersToComputePresentAndPotentialIrrelevancyThresholds(const vector<unsigned int>& maximizedSizeDimensionIdsParam, const bool isAreaMaximizedParam)
{
  maximizedSizeDimensionIds = maximizedSizeDimensionIdsParam;
  isAreaMaximized = isAreaMaximizedParam;
}

unsigned int IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(const vector<unsigned int>& sizes, const unsigned int hyperplaneDimensionId, const unsigned int nbOfSymmetricElements)
{
  const unsigned int n = sizes.size();
  unsigned int nbOfTuplesInHyperplane = 1;
  vector<unsigned int>::const_iterator sizeIt = sizes.begin();
  for (unsigned int dimensionId = 0; dimensionId != n; ++dimensionId)
    {
      if (dimensionId != hyperplaneDimensionId)
  	{
  	  nbOfTuplesInHyperplane *= *sizeIt;
  	}
      ++sizeIt;
    }
  if (!nbOfSymmetricElements)
    {
      return nbOfTuplesInHyperplane;
    }
  unsigned int nbOfSelfLoopsInHyperplane = 1;
  bool isHyperplaneSymmetric = false;
  sizeIt = sizes.begin();
  for (unsigned int dimensionId = 0; dimensionId != n; ++dimensionId)
    {
      if (dimensionId < firstSymmetricAttributeId || dimensionId > lastSymmetricAttributeId)
  	{
  	  if (dimensionId != hyperplaneDimensionId)
  	    {
  	      nbOfSelfLoopsInHyperplane *= *sizeIt;
  	    }
  	}
      else
  	{
  	  if (dimensionId == hyperplaneDimensionId)
  	    {
  	      isHyperplaneSymmetric = true;
  	    }
  	}
      ++sizeIt;
    }
  if (isHyperplaneSymmetric)
    {
      return nbOfTuplesInHyperplane - nbOfSelfLoopsInHyperplane;
    }
  return nbOfTuplesInHyperplane - nbOfSymmetricElements * nbOfSelfLoopsInHyperplane;
}
