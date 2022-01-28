// Copyright (C) 2014-2020 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef TUPLE_MEASURE_H
#define TUPLE_MEASURE_H

#include <algorithm>

#include "Measure.h"

class TupleMeasure : public Measure
{
 public:
  TupleMeasure(const unsigned int n);
  TupleMeasure(const TupleMeasure& otherTupleMeasure);
  TupleMeasure(TupleMeasure&& otherTupleMeasure);
  virtual ~TupleMeasure();

  TupleMeasure& operator=(const TupleMeasure& otherTupleMeasure);
  TupleMeasure& operator=(TupleMeasure&& otherTupleMeasure);

  static void allMeasuresSet(const vector<unsigned int>& cardinalities, vector<Measure*>& mereConstraints); /* must be called after all measures are sorted in Tree::initMeasures */
  static void allMeasuresSet(const vector<unsigned int>& cardinalities, vector<Measure*>& mereConstraints, vector<Measure*>& nonMinSizeMeasuresToMaximize); /* must be called after all measures are sorted in SkyPatternTree::initMeasures */
  static const vector<vector<unsigned int>>& present(); /* elements are sorted */
  static const vector<vector<unsigned int>>& presentAndPotential(); /* elements are sorted */

  bool violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent);
  bool violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent);

 protected:
  static vector<bool> relevantDimensionsForMonotoneMeasures;
  static vector<bool> relevantDimensionsForAntiMonotoneMeasures;

 private:
  static vector<TupleMeasure*> firstMeasures;
  static vector<pair<vector<vector<unsigned int>>, vector<vector<unsigned int>>>> stack; /* elements are sorted */
  static vector<unsigned int> sortedElements;

  virtual bool violationAfterPresentIncreased(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent); /* elementsSetPresent is sorted */
  virtual bool violationAfterPresentAndPotentialDecreased(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent); /* elementsSetAbsent is sorted */

  static void initStack(const vector<unsigned int>& cardinalities);
};

#endif /*TUPLE_MEASURE_H*/
