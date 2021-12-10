// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef MIN_SUM_H_
#define MIN_SUM_H_

#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <boost/tokenizer.hpp>

#include "../utilities/NoInputException.h"
#include "../utilities/DataFormatException.h"
#include "TupleMeasure.h"
#include "TupleValueTrie.h"

using namespace boost;

class MinSum final : public TupleMeasure
{
 public:
  MinSum(const string& tupleValueFileName, const char* dimensionSeparator, const char* elementSeparator, const vector<unordered_map<string, unsigned int>>& labels2Ids, const vector<unsigned int>& dimensionOrder, const vector<unsigned int>& cardinalities, const float densityThreshold, const float threshold);
  MinSum* clone() const;

  bool monotone() const;

  bool violationAfterPresentIncreased(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent);
  bool violationAfterPresentAndPotentialDecreased(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent);
  float optimisticValue() const;

  static void deleteTupleValues();

 private:
  double minSum;

  static vector<unsigned int> dimensionIds;
  static bool isValuesAllPositive;
  static bool isValuesAllNegative;
  static AbstractTupleValueData* positiveTupleValues;
  static AbstractTupleValueData* negativeTupleValues;

  static float threshold;

  static AbstractTupleValueData* initializeTupleValues(const vector<unsigned int>& cardinalities);
};

#endif /*MIN_SUM_H_*/
