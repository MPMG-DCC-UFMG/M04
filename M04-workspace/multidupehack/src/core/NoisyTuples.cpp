// Copyright (C) 2013-2021 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "NoisyTuples.h"

vector<unsigned int> getFirst2Index(const vector<pair<unsigned int, double>>& pairs, const unsigned int nbOfFirst)
{
  vector<unsigned int> oldIds2NewIds(nbOfFirst, numeric_limits<unsigned int>::max());
  unsigned int hyperplaneId = 0;
  for (const pair<unsigned int, double>& pair : pairs)
    {
      oldIds2NewIds[pair.first] = hyperplaneId++;
    }
  return oldIds2NewIds;
}

vector<unsigned int> NoisyTuples::minimalNbOfTuples;
vector<double> NoisyTuples::epsilonVector;
vector<unsigned int> NoisyTuples::symDimensionIds;
vector<vector<NoisyTuples*>> NoisyTuples::hyperplanes;
vector<Dimension*> NoisyTuples::dimensions;
vector<unordered_set<unsigned int>> NoisyTuples::hyperplanesToClear;

NoisyTuples::NoisyTuples(): tuples(), lowestMembershipInMinimalNSet(2)
{
}

bool NoisyTuples::empty() const
{
  return lowestMembershipInMinimalNSet == 3;
}

unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::const_iterator NoisyTuples::begin() const
{
  return tuples.begin();
}

unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::const_iterator NoisyTuples::end() const
{
  return tuples.end();
}

void NoisyTuples::insert(const vector<unsigned int>& tuple, const double membership)
{
  tuples[tuple] = membership;
}

bool NoisyTuples::erase(const vector<unsigned int>& tuple)
{
  const unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::iterator tupleToEraseIt = tuples.find(tuple);
  if (tupleToEraseIt != tuples.end())
    {
      // Because lowestMembershipInMinimalNSet initially is 2 and is set to 3 when an hyperplane is already/currently cleared, the following test can only pass for an already processed hyperplane that is yet to be found too noisy
      if (tupleToEraseIt->second >= lowestMembershipInMinimalNSet)
	{
	  tuples.erase(tupleToEraseIt);
	  return true;
	}
      tuples.erase(tupleToEraseIt);
    }
  return false;
}

void NoisyTuples::clear(const unsigned int dimensionId, const unsigned int hyperplaneId)
{
  lowestMembershipInMinimalNSet = 3; // To never test again if this hyperplane is too noisy
  dimensions[dimensionId]->decrementCardinality();
  for (unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::const_iterator tupleIt = tuples.begin(); tupleIt != tuples.end(); tupleIt = tuples.erase(tupleIt))
    {
      // Erasing the same tuple from the orthogonal hyperplanes
      vector<vector<NoisyTuples*>>::iterator hyperplanesInDimensionIt = hyperplanes.begin();
      vector<unsigned int>::const_iterator elementIt = tupleIt->first.begin();
      vector<unsigned int> orthogonalTuple;
      orthogonalTuple.reserve(tupleIt->first.size());
      // Constructing the first orthogonal tuple
      if (dimensionId < 2)
	{
	  orthogonalTuple.push_back(hyperplaneId);
	  orthogonalTuple.insert(orthogonalTuple.end(), elementIt + 1, tupleIt->first.end());
	}
      else
	{
	  const vector<unsigned int>::const_iterator suffixBegin = elementIt + dimensionId;
	  orthogonalTuple.insert(orthogonalTuple.end(), elementIt + 1, suffixBegin);
	  orthogonalTuple.push_back(hyperplaneId);
	  orthogonalTuple.insert(orthogonalTuple.end(), suffixBegin, tupleIt->first.end());
	}
      // Erasing the tuple from the orthogonal hyperplanes whose dimension ids are lesser than that of this hyperplane
      vector<unsigned int>::iterator orthogonalTupleIt = orthogonalTuple.begin();
      unsigned int orthogonalDimensionId = 0;
      for (; orthogonalDimensionId != dimensionId; ++orthogonalDimensionId)
	{
	  NoisyTuples& orthogonalHyperplane = *(*hyperplanesInDimensionIt)[*elementIt];
	  if (orthogonalHyperplane.erase(orthogonalTuple))
	    {
	      hyperplanesToClear[orthogonalDimensionId].insert(*elementIt);
	    }
	  *orthogonalTupleIt++ = *elementIt++;
	  ++hyperplanesInDimensionIt;
	}
      if (++orthogonalDimensionId != hyperplanes.size())
	{
	  // Erasing the tuple from the orthogonal hyperplanes whose dimension ids are greater than that of this hyperplane
	  for (*orthogonalTupleIt = hyperplaneId; ; *++orthogonalTupleIt = *elementIt++)
	    {
	      NoisyTuples& orthogonalHyperplane = *(*++hyperplanesInDimensionIt)[*elementIt];
	      if (orthogonalHyperplane.erase(orthogonalTuple))
		{
		  hyperplanesToClear[orthogonalDimensionId].insert(*elementIt);
		}
	      if (++orthogonalDimensionId == hyperplanes.size())
		{
		  break;
		}
	    }
	}
    }
}

void NoisyTuples::clearIfTooNoisy(const unsigned int dimensionId, const unsigned int hyperplaneId)
{
  // TODO: separately treat the case where the element is symmetric (isolating the code to compute highestMembershipValues in a separate function) to process both elements at once; for every epsilon of a third dimension, multiply the min sizes of the remaining n - 2 dimensions (yes, there is a symmetric dimension included), subtract 1 (because of the self loop) and subtract as many membership degrees taken at the beginnig of each lowestMembershipInMinimalNSet (of the symmetric elements); if epsilon is exceeded, clear
  // Given the minimal size constraints, computing the minimal possible noise in a pattern involving this hyperplane
  double minNoise = minimalNbOfTuples[dimensionId];
  if (!tuples.empty())
    {
      multiset<double> highestMembershipValues;
      unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::const_iterator tupleIt = tuples.begin();
      for (unsigned int minNbOfTuples = minimalNbOfTuples[dimensionId]; tupleIt != tuples.end() && minNbOfTuples; --minNbOfTuples)
	{
	  highestMembershipValues.insert(tupleIt->second);
	  ++tupleIt;
	}
      lowestMembershipInMinimalNSet = *(highestMembershipValues.begin());
      for (; tupleIt != tuples.end(); ++tupleIt)
	{
	  const double membership = tupleIt->second;
	  if (membership > lowestMembershipInMinimalNSet)
	    {
	      highestMembershipValues.erase(highestMembershipValues.begin());
	      highestMembershipValues.insert(membership);
	      lowestMembershipInMinimalNSet = *(highestMembershipValues.begin());
	    }
	}
      // TODO: For every epsilon of a different dimension, multiply the min sizes of the n - 2 remaining dimensions and subtract as many membership degrees taken at the beginning of lowestMembershipInMinimalNSet; if epsilon is exceeded, clear
      for (const double highestMembershipValue : highestMembershipValues)
	{
	  minNoise -= highestMembershipValue;
	}
    }
  if (minNoise > epsilonVector[dimensionId])
    {
      // This hyperplane cannot possibly be in a pattern respecting the minimal size constraints: clear it
      if (dimensions[dimensionId]->symmetric())
	{
	  for (const unsigned int symDimensionId : symDimensionIds)
	    {
	      hyperplanes[symDimensionId][hyperplaneId]->clear(symDimensionId, hyperplaneId);
	    }
	}
      else
	{
	  clear(dimensionId, hyperplaneId);
	}
    }
}

vector<Dimension*> NoisyTuples::unPreProcessedDimensions(const vector<unsigned int>& symDimensionIdsParam, const vector<unsigned int>& cardinalities)
{
  unsigned int dimensionId = 0;
  const vector<unsigned int>::const_iterator symDimensionEnd = symDimensionIds.end();
  vector<unsigned int>::const_iterator symDimensionIt = symDimensionIds.begin();
  for (const unsigned int cardinality : cardinalities)
    {
      if (symDimensionIt != symDimensionEnd && *symDimensionIt == dimensionId)
	{
	  ++symDimensionIt;
	  dimensions.push_back(new Dimension(dimensionId++, cardinality, true));
	}
      else
	{
	  dimensions.push_back(new Dimension(dimensionId++, cardinality, false));
	}
    }
  return dimensions;
}

vector<unsigned int> NoisyTuples::moveValuesRelatedToSymmetricDimensionsAtTheEnd(const vector<unsigned int>& values)
{
  vector<unsigned int> reordered;
  reordered.reserve(values.size());
  unsigned int dimensionId = 0;
  const vector<unsigned int>::const_iterator symDimensionIdEnd = symDimensionIds.end();
  vector<unsigned int>::const_iterator symDimensionIdIt = symDimensionIds.begin();
  for (const unsigned int value : values)
    {
      if (symDimensionIdIt != symDimensionIdEnd && *symDimensionIdIt == dimensionId++)
	{
	  ++symDimensionIdIt;
	}
      else
	{
	  reordered.push_back(value);
	}
    }
  reordered.insert(reordered.end(), 2, values[symDimensionIds.front()]);
  return reordered;
}

vector<Dimension*> NoisyTuples::preProcess(const vector<double>& epsilonVectorParam, const vector<unsigned int>& symDimensionIdsParam, const vector<unsigned int>& minSizeVector, const bool isMinAreaNull, const vector<unsigned int>& cardinalities, vector<vector<NoisyTuples*>>& hyperplanesParam)
{
  const unsigned int n = hyperplanesParam.size();
  dimensions.reserve(n);
#ifndef PRE_PROCESS
  return unPreProcessedDimensions(symDimensionIdsParam, cardinalities);
#endif
  // Do not pre-process if a valid pattern can have no element in a dimension
  if (isMinAreaNull)
    {
      for (const unsigned int minSize : minSizeVector)
	{
	  if (!minSize)
	    {
	      return unPreProcessedDimensions(symDimensionIdsParam, cardinalities);
	    }
	}
    }
  // Initialize minimalNbOfTuples and cardinalities considering the input data order of the attributes
  minimalNbOfTuples.reserve(n);
  vector<unsigned int> nbOfTuples;
  nbOfTuples.reserve(n);
  if (symDimensionIdsParam.empty())
    {
      IndistinctSkyPatterns::setParametersToComputePresentAndPotentialIrrelevancyThresholds(numeric_limits<unsigned int>::max(), 0);
      for (unsigned int dimensionId = 0; dimensionId != n; ++dimensionId)
	{
	  minimalNbOfTuples.push_back(IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(minSizeVector, dimensionId, 0));
	  nbOfTuples.push_back(IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(cardinalities, dimensionId, 0));
	}
    }
  else
    {
      symDimensionIds = symDimensionIdsParam;
      // Move the number and the minimal number of symmetric elements at the end (they need to be contiguous when calling IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern)
      const vector<unsigned int> reorderedCardinalities = moveValuesRelatedToSymmetricDimensionsAtTheEnd(cardinalities);
      const vector<unsigned int> reorderedMinSizes = moveValuesRelatedToSymmetricDimensionsAtTheEnd(minSizeVector);
      IndistinctSkyPatterns::setParametersToComputePresentAndPotentialIrrelevancyThresholds(n - 2, n - 1);
      const unsigned int nbOfTuplesInSymmetricHyperplane = IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(reorderedCardinalities, n - 1, reorderedCardinalities.back());
      const unsigned int minimalNbOfTuplesInSymmetricHyperplane = IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(reorderedMinSizes, n - 1, minSizeVector[symDimensionIds.front()]);
      unsigned int internalDimensionId = 0;
      const vector<unsigned int>::const_iterator symDimensionIdEnd = symDimensionIds.end();
      vector<unsigned int>::const_iterator symDimensionIdIt = symDimensionIds.begin();
      for (unsigned int dimensionId = 0; dimensionId != n; ++dimensionId)
	{
	  if (symDimensionIdIt != symDimensionIdEnd && *symDimensionIdIt == dimensionId)
	    {
	      ++symDimensionIdIt;
	      nbOfTuples.push_back(nbOfTuplesInSymmetricHyperplane);
	      minimalNbOfTuples.push_back(minimalNbOfTuplesInSymmetricHyperplane);
	    }
	  else
	    {
	      nbOfTuples.push_back(IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(reorderedCardinalities, internalDimensionId, reorderedCardinalities.back()));
	      minimalNbOfTuples.push_back(IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(reorderedMinSizes, internalDimensionId, minSizeVector[symDimensionIds.front()]));
	      ++internalDimensionId;
	    }
	}
    }
  epsilonVector = epsilonVectorParam;
  hyperplanes = hyperplanesParam;
  hyperplanesToClear.resize(n);
  // Consider the dimensions in decreasing order of (minimalNbOfTuples - epsilon) / nbOfTuples
  vector<pair<double, unsigned int>> order;
  order.reserve(n);
  const vector<unsigned int>::const_iterator symDimensionIdEnd = symDimensionIds.end();
  vector<unsigned int>::const_iterator symDimensionIdIt = symDimensionIds.begin();
  vector<vector<NoisyTuples*>>::iterator hyperplanesInDimensionIt = hyperplanes.begin();
  vector<double>::const_iterator epsilonIt = epsilonVector.begin();
  vector<unsigned int>::const_iterator minimalNbOfTuplesIt = minimalNbOfTuples.begin();
  vector<unsigned int>::const_iterator nbOfTuplesIt = nbOfTuples.begin();
  for (unsigned int dimensionId = 0; dimensionId != n; ++dimensionId)
    {
      order.emplace_back((*epsilonIt++ - *minimalNbOfTuplesIt++) / *nbOfTuplesIt++, dimensionId);
      if (symDimensionIdIt != symDimensionIdEnd && *symDimensionIdIt == dimensionId)
	{
	  ++symDimensionIdIt;
	  dimensions.push_back(new Dimension(dimensionId, hyperplanesInDimensionIt->size(), true));
	}
      else
	{
	  dimensions.push_back(new Dimension(dimensionId, hyperplanesInDimensionIt->size(), false));
	}
      ++hyperplanesInDimensionIt;
    }
  sort(order.begin(), order.end(), [](const pair<double, unsigned int>& entry1, const pair<double, unsigned int>& entry2) {return entry1.first < entry2.first;});
  for (const pair<double, unsigned int>& orderPair : order)
    {
      if (minimalNbOfTuples[orderPair.second])
	{
	  vector<NoisyTuples*>& hyperplanesInDimension = hyperplanes[orderPair.second];
	  vector<NoisyTuples*>::iterator hyperplaneIt = hyperplanesInDimension.begin();
	  for (unsigned int hyperplaneId = 0; hyperplaneId != hyperplanesInDimension.size(); ++hyperplaneId)
	    {
	      // TODO: remove the test after processing symmetric elements together
	      if ((*hyperplaneIt)->lowestMembershipInMinimalNSet != 3)
		{
		  // **hyperplaneIt is not a symmetric hyperplane that has already been erased
		  (*hyperplaneIt)->clearIfTooNoisy(orderPair.second, hyperplaneId);
		}
	      ++hyperplaneIt;
	    }
	}
    }
  vector<pair<double, unsigned int>>::const_iterator orderIt = order.begin();
  while (true)
    {
      for (; orderIt != order.end() && hyperplanesToClear[orderIt->second].empty(); ++orderIt)
	{
	}
      if (orderIt == order.end())
	{
	  break;
	}
      unordered_set<unsigned int>& hyperplaneIdsToClearInDimension = hyperplanesToClear[orderIt->second];
      vector<NoisyTuples*>& hyperplanesToClearInDimension = hyperplanes[orderIt->second];
      unordered_set<unsigned int>::iterator hyperplaneIdIt = hyperplaneIdsToClearInDimension.begin();
      for (; hyperplaneIdIt != hyperplaneIdsToClearInDimension.end() && hyperplanesToClearInDimension[*hyperplaneIdIt]->lowestMembershipInMinimalNSet == 3; hyperplaneIdIt = hyperplaneIdsToClearInDimension.erase(hyperplaneIdIt))
	{
	}
      if (hyperplaneIdIt == hyperplaneIdsToClearInDimension.end())
	{
	  ++orderIt;
	}
      else
	{
	  hyperplanesToClearInDimension[*hyperplaneIdIt]->clearIfTooNoisy(orderIt->second, *hyperplaneIdIt);
	  hyperplaneIdsToClearInDimension.erase(hyperplaneIdIt);
	  orderIt = order.begin();
	}
    }
  // PERF: Clear in a tau-contiguous dimension too small sets of elements that are more than tau-far distant from any other element in the dimension
  return dimensions;
}

// Order the new ids by increasing total membership to take advantage of the prefetch buffer
vector<unsigned int> NoisyTuples::createNewIds(const vector<NoisyTuples*>& hyperplanesInDimension, const unsigned int nbOfValidHyperplanes)
{
  vector<pair<unsigned int, double>> ids2TotalMemberships = getIds2TotalMemberships(hyperplanesInDimension, nbOfValidHyperplanes);
  sort(ids2TotalMemberships.begin(), ids2TotalMemberships.end(), [](const pair<unsigned int, double>& pair1, const pair<unsigned int, double>& pair2) {return pair1.second < pair2.second;});
  return getFirst2Index(ids2TotalMemberships, hyperplanesInDimension.size());
}

// Order the new ids of the symmetric elements by increasing total membership in all symmetric dimensions
vector<unsigned int> NoisyTuples::createNewIds(const vector<unsigned int>& symDimensionIdsParam, const vector<vector<NoisyTuples*>>& hyperplanesParam, const unsigned int nbOfValidHyperplanes)
{
  // TODO: simplification considering only two symmetric dimensions
  // vector<pair<unsigned int, double>> ids2TotalMemberships = getIds2TotalMemberships(hyperplanesParam[symDimensionIdsParam.back()], nbOfValidHyperplanes);
  // vector<pair<unsigned int, double>>::iterator id2TotalMembershipIt = ids2TotalMemberships.begin();
  // for (const NoisyTuples* hyperplane : hyperplanesParam[symDimensionIdsParam.front()])
  //   {
  //     if (hyperplane->lowestMembershipInMinimalNSet != 3)
  // 	{
  // 	  for (const pair<const vector<unsigned int>, double>& tuple : hyperplane->tuples)
  // 	    {
  // 	      id2TotalMembershipIt->second += tuple.second;
  // 	    }
  // 	  ++id2TotalMembershipIt;
  // 	}
  //   }
  // sort(ids2TotalMemberships.begin(), ids2TotalMemberships.end(), [](const pair<unsigned int, double>& pair1, const pair<unsigned int, double>& pair2) {return pair1.second < pair2.second;});
  // return getFirst2Index(ids2TotalMemberships, hyperplanesParam[symDimensionIdsParam.back()].size());

  const vector<unsigned int>::const_iterator lastSymDimensionIdIt = --symDimensionIdsParam.end();
  vector<pair<unsigned int, double>> ids2TotalMemberships = getIds2TotalMemberships(hyperplanesParam[*lastSymDimensionIdIt], nbOfValidHyperplanes);
  for (vector<unsigned int>::const_iterator symDimensionIdIt = symDimensionIdsParam.begin(); symDimensionIdIt != lastSymDimensionIdIt; ++symDimensionIdIt)
    {
      vector<pair<unsigned int, double>>::iterator id2TotalMembershipIt = ids2TotalMemberships.begin();
      for (const NoisyTuples* hyperplane : hyperplanesParam[*symDimensionIdIt])
	{
	  if (hyperplane->lowestMembershipInMinimalNSet != 3)
	    {
	      for (const pair<const vector<unsigned int>, double>& tuple : hyperplane->tuples)
		{
		  id2TotalMembershipIt->second += tuple.second;
		}
	      ++id2TotalMembershipIt;
	    }
	}
    }
  sort(ids2TotalMemberships.begin(), ids2TotalMemberships.end(), [](const pair<unsigned int, double>& pair1, const pair<unsigned int, double>& pair2) {return pair1.second < pair2.second;});
  return getFirst2Index(ids2TotalMemberships, hyperplanesParam[*lastSymDimensionIdIt].size());
}

// Order the new ids by increasing numeric label
vector<unsigned int> NoisyTuples::createNewIds(const vector<NoisyTuples*>& hyperplanesInDimension, const unsigned int nbOfValidHyperplanes, const vector<string>& numericLabels)
{
  vector<pair<unsigned int, double>> ids2NumericLabels;
  ids2NumericLabels.reserve(nbOfValidHyperplanes);
  unsigned int hyperplaneId = 0;
  for (const NoisyTuples* hyperplane : hyperplanesInDimension)
    {
      if (hyperplane->lowestMembershipInMinimalNSet != 3)
	{
	  ids2NumericLabels.emplace_back(hyperplaneId, lexical_cast<double>(numericLabels[hyperplaneId]));
	}
      ++hyperplaneId;
    }
  sort(ids2NumericLabels.begin(), ids2NumericLabels.end(), [](const pair<unsigned int, double>& pair1, const pair<unsigned int, double>& pair2) {return pair1.second < pair2.second;});
  return getFirst2Index(ids2NumericLabels, hyperplanesInDimension.size());
}

vector<pair<unsigned int, double>> NoisyTuples::getIds2TotalMemberships(const vector<NoisyTuples*>& hyperplanesInDimension, const unsigned int nbOfValidHyperplanes)
{
  vector<pair<unsigned int, double>> ids2TotalMemberships;
  ids2TotalMemberships.reserve(nbOfValidHyperplanes);
  unsigned int hyperplaneId = 0;
  for (const NoisyTuples* hyperplane : hyperplanesInDimension)
    {
      if (hyperplane->lowestMembershipInMinimalNSet != 3)
	{
	  double totalMembership = 0;
	  for (const pair<const vector<unsigned int>, double>& tuple : hyperplane->tuples)
	    {
	      totalMembership += tuple.second;
	    }
	  ids2TotalMemberships.emplace_back(hyperplaneId, totalMembership);
	}
      ++hyperplaneId;
    }
  return ids2TotalMemberships;
}
