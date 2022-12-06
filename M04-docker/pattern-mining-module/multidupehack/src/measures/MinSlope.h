// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef MIN_SLOPE_H_
#define MIN_SLOPE_H_

#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <boost/tokenizer.hpp>

#include "../utilities/NoInputException.h"
#include "../utilities/DataFormatException.h"
#include "TupleMeasure.h"
#include "TuplePointTrie.h"

using namespace boost;

class MinSlope final : public TupleMeasure
{
 public:
  MinSlope(const string& tuplePointFileName, const char* dimensionSeparator, const char* elementSeparator, const vector<unordered_map<string, unsigned int>>& labels2Ids, const vector<unsigned int>& dimensionOrder, const vector<unsigned int>& cardinalities, const float densityThreshold, const float threshold);
  MinSlope* clone() const;

  bool violationAfterPresentIncreased(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent);
  bool violationAfterPresentAndPotentialDecreased(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent);
  float optimisticValue() const;

  static void deleteTuplePoints();

 private:
  SlopeSums minSums;
  SlopeSums maxSums;
  float minSlope;

  static vector<unsigned int> dimensionIds;
  static AbstractTuplePointData* tuplePoints;
  static float threshold;

  void computeOptimisticValue();
};

#endif /*MIN_SLOPE_H_*/
