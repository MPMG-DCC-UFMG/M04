// Copyright (C) 2013-2021 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef NOISY_TUPLES_H_
#define NOISY_TUPLES_H_

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <boost/lexical_cast.hpp>

#include "../../Parameters.h"
#include "../utilities/vector_hash.h"
#include "Dimension.h"
#include "IndistinctSkyPatterns.h"

using namespace std;
using namespace boost;

class NoisyTuples
{
 public:
  NoisyTuples();

  bool empty() const;
  unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::const_iterator begin() const;
  unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::const_iterator end() const;

  void insert(const vector<unsigned int>& tuple, const double membership);

  static vector<Dimension*> preProcess(const vector<double>& epsilonVector, const vector<unsigned int>& symDimensionIds, const vector<unsigned int>& minSizeVector, const bool isMinAreaNull, const vector<unsigned int>& cardinalities, vector<vector<NoisyTuples*>>& hyperplanes);
  static vector<unsigned int> createNewIds(const vector<NoisyTuples*>& hyperplanesInDimension, const unsigned int nbOfValidHyperplanes); /* for regular attributes */
  static vector<unsigned int> createNewIds(const vector<unsigned int>& symDimensionIdsParam, const vector<vector<NoisyTuples*>>& hyperplanesParam, const unsigned int nbOfValidHyperplanes); /* for symmetric attributes */
  static vector<unsigned int> createNewIds(const vector<NoisyTuples*>& hyperplanesInDimension, const unsigned int nbOfValidHyperplanes, const vector<string>& numericLabels); /* for metric attributes */

 private:
  unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>> tuples;
  double lowestMembershipInMinimalNSet; /* 2 when the hyperplane is unprocessed; 3 when currently/already cleared */

  static vector<unsigned int> minimalNbOfTuples;
  static vector<double> epsilonVector;
  static vector<unsigned int> symDimensionIds;
  static vector<vector<NoisyTuples*>> hyperplanes;
  static vector<Dimension*> dimensions;
  static vector<unordered_set<unsigned int>> hyperplanesToClear;

  bool erase(const vector<unsigned int>& tuple); /* returns whether the (already processed) hyperplane should be checked again */
  void clear(const unsigned int dimensionId, const unsigned int hyperplaneId);
  void clearIfTooNoisy(const unsigned int dimensionId, const unsigned int hyperplaneId);

  static vector<Dimension*> unPreProcessedDimensions(const vector<unsigned int>& symDimensionIdsParam, const vector<unsigned int>& cardinalities);
  static vector<unsigned int> moveValuesRelatedToSymmetricDimensionsAtTheEnd(const vector<unsigned int>& v);
  static vector<pair<unsigned int, double>> getIds2TotalMemberships(const vector<NoisyTuples*>& hyperplanesInDimension, const unsigned int nbOfValidHyperplanes);
};

#endif /*NOISY_TUPLES_H_*/
