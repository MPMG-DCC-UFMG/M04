// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef VALUE_H_
#define VALUE_H_

#include "../../Parameters.h"

#include <vector>

using namespace std;

class Value
{
 public:
  Value(const unsigned int id, const unsigned int presentAndPotentialNoise, const unsigned int nbOfSubsequentAttributes);
  Value(const Value& otherValue, const unsigned int presentIntersectionId);

  unsigned int getPresentIntersectionId() const;
  unsigned int getPresentAndPotentialIntersectionId() const;
  unsigned int getDataId() const;
  unsigned int getPresentNoise() const;
  unsigned int getPresentAndPotentialNoise() const;
  vector<vector<unsigned int>>::iterator setPresentAndPotentialIntersectionId(const unsigned int presentAndPotentialIntersectionId);
  void incrementPresentNoise();
  void addPresentNoise(const unsigned int noise);
  void decrementPotentialNoise();
  void subtractPotentialNoise(const unsigned int noise);
  vector<vector<unsigned int>>::iterator getIntersectionsBeginWithPresentValues();
  vector<vector<unsigned int>>::const_iterator getIntersectionsBeginWithPresentValues() const;
  vector<vector<unsigned int>>::iterator getIntersectionsBeginWithPresentAndPotentialValues();
  vector<vector<unsigned int>>::const_iterator getIntersectionsBeginWithPresentAndPotentialValues() const;
  void setNullPresentAndPotentialIntersectionsWithNewAbsentValue(const unsigned int intersectionIndex);

  bool extendsPastPresent(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int intersectionIndex) const;
  bool extendsFuturePresent(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int reverseAttributeIndex) const;
  bool extendsPastPresentAndPotential(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int intersectionIndex) const;
  bool extendsFuturePresentAndPotential(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int reverseAttributeIndex) const;

  bool symmetricValuesExtendPastPresent(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int intersectionIndex) const;
  bool symmetricValuesExtendFuturePresent(const Value& symmetricValue, const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int reverseAttributeIndex) const;
  bool symmetricValuesExtendPastPresentAndPotential(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int intersectionIndex) const;
  bool symmetricValuesExtendFuturePresentAndPotential(const Value& symmetricValue, const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int reverseAttributeIndex) const;

#ifdef ASSERT
  unsigned int presentNoiseAtIntersectionWith(const Value& otherValue, const unsigned int intersectionIndex) const;
  unsigned int presentAndPotentialNoiseAtIntersectionWith(const Value& otherValue, const unsigned int intersectionIndex) const;
#endif

  static bool smallerDataId(const Value* value, const Value* otherValue);

 private:
  const unsigned int dataId;
  const unsigned int presentIntersectionId;
  unsigned int presentAndPotentialIntersectionId;
  unsigned int presentNoise;
  unsigned int presentAndPotentialNoise;
  vector<vector<unsigned int>> intersectionsWithPresentValues;
  vector<vector<unsigned int>> intersectionsWithPresentAndPotentialValues;
};

#endif /*VALUE_H_*/
