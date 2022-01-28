// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef METRIC_ATTRIBUTE_H_
#define METRIC_ATTRIBUTE_H_

#include "Attribute.h"

class MetricAttribute final: public Attribute
{
 public:
  MetricAttribute(const vector<unsigned int>& nbOfValuesPerAttribute, const double epsilon, const unsigned int minSize, const vector<string>& labels, const double tau);

  MetricAttribute* clone() const;

  void sortPotentialAndAbsentButChosenPresentValueIfNecessary(const unsigned int presentAttributeId) const;
  void sortPotentialIrrelevantAndAbsentButChosenAbsentValueIfNecessary(const unsigned int absentAttributeId) const;
  void sortPotentialIrrelevantAndAbsentIfNecessary(const unsigned int absentAttributeId);

  bool metric() const;

  void repositionChosenPresentValue(); /* to be called on the attribute that is the child of the one where the value was chosen */
  void repositionChosenAbsentValue(); /* to be called after setLastIrrelevantValueChosen() (on the same object) */

  bool findIrrelevantValuesAndCheckTauContiguity(const vector<Attribute*>::iterator attributeBegin, const vector<Attribute*>::iterator attributeEnd);
#ifdef MIN_SIZE_ELEMENT_PRUNING
  pair<bool, vector<unsigned int>> findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity();
  void sortAbsent();
#endif
  bool unclosed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
  bool globallyUnclosed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
  bool finalizable() const;
  vector<unsigned int> finalize(); /* returns the original ids of the elements moved to present */

  void eraseAbsentValuesBeneathTimestamp(const double timestamp);
  void eraseAbsentValuesBeyondTimestamp(const double timestamp);

 private:
  static vector<double> tauVector; // 0 if the attribute is not totally ordered
  static vector<vector<double>> timestampsVector;

  MetricAttribute(const MetricAttribute& parentAttribute);

  void chooseValue(const unsigned int indexOfValue);
  vector<Value*>::iterator repositionChosenValueAndGetIt(); /* to be called after setPresentIntersections (on the same object) */
  vector<unsigned int> setChosenValueIrrelevant(); /* to be called after chooseValue (on the same object) */

  void sortPotentialAndAbsentButChosenValue(const unsigned int presentAttributeId) const;
};

#endif /*METRIC_ATTRIBUTE_H_*/
