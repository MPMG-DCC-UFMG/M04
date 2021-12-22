// Copyright (C) 2014-2020 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "TupleMeasure.h"

vector<bool> TupleMeasure::relevantDimensionsForMonotoneMeasures;
vector<bool> TupleMeasure::relevantDimensionsForAntiMonotoneMeasures;
vector<TupleMeasure*> TupleMeasure::firstMeasures;
vector<pair<vector<vector<unsigned int>>, vector<vector<unsigned int>>>> TupleMeasure::stack;
vector<unsigned int> TupleMeasure::sortedElements;

TupleMeasure::TupleMeasure(const unsigned int n)
{
  if (relevantDimensionsForMonotoneMeasures.empty())
    {
      relevantDimensionsForMonotoneMeasures.resize(n);
      relevantDimensionsForAntiMonotoneMeasures.resize(n);
    }
}

TupleMeasure::TupleMeasure(const TupleMeasure& otherTupleMeasure)
{
  if (&otherTupleMeasure == firstMeasures.back())
    {
      firstMeasures.push_back(this);
      stack.emplace_back(stack.back());
    }
}

TupleMeasure::TupleMeasure(TupleMeasure&& otherTupleMeasure)
{
}

TupleMeasure::~TupleMeasure()
{
  if (!firstMeasures.empty() && firstMeasures.back() == this)
    {
      firstMeasures.pop_back();
      if (!stack.empty())	// test required in case of exception raised in the constructor of a child class
	{
	  stack.pop_back();
	}
    }
}

TupleMeasure& TupleMeasure::operator=(const TupleMeasure& otherTupleMeasure)
{
  if (&otherTupleMeasure == firstMeasures.back())
    {
      stack.emplace_back(stack.back());
    }
  return *this;
}

TupleMeasure& TupleMeasure::operator=(TupleMeasure&& otherTupleMeasure)
{
  return *this;
}

void TupleMeasure::initStack(const vector<unsigned int>& cardinalities)
{
  vector<vector<unsigned int>> potential;
  potential.reserve(cardinalities.size());
  bool isNoMeasureMonotone = true;
  const unsigned int end = relevantDimensionsForMonotoneMeasures.size();
  for (unsigned int dimensionId = 0; dimensionId != end; ++dimensionId)
    {
      vector<unsigned int> potentialDimension;
      if (relevantDimensionsForMonotoneMeasures[dimensionId])
	{
	  isNoMeasureMonotone = false;
	  const unsigned int cardinality = cardinalities[dimensionId];
	  potentialDimension.reserve(cardinality);
	  for (unsigned int elementId = 0; elementId != cardinality; ++elementId)
	    {
	      potentialDimension.push_back(elementId);
	    }
	}
      potential.emplace_back(potentialDimension);
    }
  // If unnecessary, clear potential vector (in this way, do not copy them at every copy of the tuple measures)
  if (isNoMeasureMonotone)
    {
      stack.emplace_back(vector<vector<unsigned int>>(cardinalities.size()), vector<vector<unsigned int>>());
      return;
    }
  stack.emplace_back(vector<vector<unsigned int>>(cardinalities.size()), potential);
}

void TupleMeasure::allMeasuresSet(const vector<unsigned int>& cardinalities, vector<Measure*>& mereConstraints)
{
  if (!relevantDimensionsForMonotoneMeasures.empty())
    {
      initStack(cardinalities);
      // Set the first groupMeasure* in firstMeasures
      vector<Measure*>::iterator measureIt = mereConstraints.begin();
      for (; !dynamic_cast<TupleMeasure*>(*measureIt); ++measureIt)
	{
	}
      firstMeasures.push_back(static_cast<TupleMeasure*>(*measureIt));
    }
}

void TupleMeasure::allMeasuresSet(const vector<unsigned int>& cardinalities, vector<Measure*>& mereConstraints, vector<Measure*>& nonMinSizeMeasuresToMaximize)
{
  if (!relevantDimensionsForMonotoneMeasures.empty())
    {
      initStack(cardinalities);
      // Set the first groupMeasure* in firstMeasures
      for (Measure* measure : mereConstraints)
	{
	  if (dynamic_cast<TupleMeasure*>(measure))
	    {
	      firstMeasures.push_back(static_cast<TupleMeasure*>(measure));
	      return;
	    }
	}
      vector<Measure*>::iterator measureIt = nonMinSizeMeasuresToMaximize.begin();
      for (; !dynamic_cast<TupleMeasure*>(*measureIt); ++measureIt)
	{
	}
      firstMeasures.push_back(static_cast<TupleMeasure*>(*measureIt));
    }
}

const vector<vector<unsigned int>>& TupleMeasure::present()
{
  return stack.back().first;
}

const vector<vector<unsigned int>>& TupleMeasure::presentAndPotential()
{
  return stack.back().second;
}

bool TupleMeasure::violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent)
{
  if (relevantDimensionsForAntiMonotoneMeasures[dimensionIdOfElementsSetPresent])
    {
      if (this == firstMeasures.back())
	{
	  sortedElements = elementsSetPresent;
	  sort(sortedElements.begin(), sortedElements.end());
	  vector<unsigned int>& presentDimension = stack.back().first[dimensionIdOfElementsSetPresent];
	  vector<unsigned int> newPresent(presentDimension.size() + sortedElements.size());
	  merge(presentDimension.begin(), presentDimension.end(), sortedElements.begin(), sortedElements.end(), newPresent.begin());
	  presentDimension.swap(newPresent);
	}
      return violationAfterPresentIncreased(dimensionIdOfElementsSetPresent, sortedElements);
    }
  return false;
}

bool TupleMeasure::violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent)
{
  if (relevantDimensionsForMonotoneMeasures[dimensionIdOfElementsSetAbsent])
    {
      if (this == firstMeasures.back())
	{
	  sortedElements = elementsSetAbsent;
	  sort(sortedElements.begin(), sortedElements.end());
	  vector<unsigned int>& presentAndPotentialDimension = stack.back().second[dimensionIdOfElementsSetAbsent];
	  vector<unsigned int> newPresentAndPotential(presentAndPotentialDimension.size() - sortedElements.size());
	  set_difference(presentAndPotentialDimension.begin(), presentAndPotentialDimension.end(), sortedElements.begin(), sortedElements.end(), newPresentAndPotential.begin());
	  presentAndPotentialDimension.swap(newPresentAndPotential);
        }
      return violationAfterPresentAndPotentialDecreased(dimensionIdOfElementsSetAbsent, sortedElements);
    }
  return false;
}

bool TupleMeasure::violationAfterPresentIncreased(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent)
{
  return false;
}

bool TupleMeasure::violationAfterPresentAndPotentialDecreased(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent)
{
  return false;
}
