// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "Value.h"

Value::Value(const unsigned int id, const unsigned int presentAndPotentialNoiseParam, const unsigned int nbOfSubsequentAttributes): dataId(id), presentIntersectionId(0), presentAndPotentialIntersectionId(id), presentNoise(0), presentAndPotentialNoise(presentAndPotentialNoiseParam), intersectionsWithPresentValues(nbOfSubsequentAttributes), intersectionsWithPresentAndPotentialValues(nbOfSubsequentAttributes)
{
}

Value::Value(const Value& parent, const unsigned int presentIntersectionIdParam): dataId(parent.dataId), presentIntersectionId(presentIntersectionIdParam), presentAndPotentialIntersectionId(0), presentNoise(parent.presentNoise), presentAndPotentialNoise(parent.presentAndPotentialNoise), intersectionsWithPresentValues(parent.intersectionsWithPresentValues.size()), intersectionsWithPresentAndPotentialValues()
{
}

unsigned int Value::getPresentIntersectionId() const
{
  return presentIntersectionId;
}

unsigned int Value::getPresentAndPotentialIntersectionId() const
{
  return presentAndPotentialIntersectionId;
}

unsigned int Value::getDataId() const
{
  return dataId;
}

unsigned int Value::getPresentNoise() const
{
  return presentNoise;
}

unsigned int Value::getPresentAndPotentialNoise() const
{
  return presentAndPotentialNoise;
}

vector<vector<unsigned int>>::iterator Value::setPresentAndPotentialIntersectionId(const unsigned int presentAndPotentialIntersectionIdParam)
{
  presentAndPotentialIntersectionId = presentAndPotentialIntersectionIdParam;
  intersectionsWithPresentAndPotentialValues.resize(intersectionsWithPresentValues.size());
  return intersectionsWithPresentAndPotentialValues.begin();
}

void Value::incrementPresentNoise()
{
  ++presentNoise;
}

void Value::addPresentNoise(const unsigned int noise)
{
  presentNoise += noise;
}

void Value::decrementPotentialNoise()
{
  --presentAndPotentialNoise;
}

void Value::subtractPotentialNoise(const unsigned int noise)
{
  presentAndPotentialNoise -= noise;
}

vector<vector<unsigned int>>::iterator Value::getIntersectionsBeginWithPresentValues()
{
  return intersectionsWithPresentValues.begin();
}

vector<vector<unsigned int>>::const_iterator Value::getIntersectionsBeginWithPresentValues() const
{
  return intersectionsWithPresentValues.begin();
}

vector<vector<unsigned int>>::iterator Value::getIntersectionsBeginWithPresentAndPotentialValues()
{
  return intersectionsWithPresentAndPotentialValues.begin();
}

vector<vector<unsigned int>>::const_iterator Value::getIntersectionsBeginWithPresentAndPotentialValues() const
{
  return intersectionsWithPresentAndPotentialValues.begin();
}

void Value::setNullPresentAndPotentialIntersectionsWithNewAbsentValue(const unsigned int intersectionIndex)
{
  intersectionsWithPresentAndPotentialValues[intersectionIndex].push_back(0);
}

bool Value::extendsPastPresent(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int intersectionIndex) const
{
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentNoise + (*valueIt)->intersectionsWithPresentValues[intersectionIndex][presentIntersectionId] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

bool Value::extendsFuturePresent(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int reverseAttributeIndex) const
{
  const vector<unsigned int>& presentIntersectionsWithFutureValues = intersectionsWithPresentValues[intersectionsWithPresentValues.size() - reverseAttributeIndex];
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentNoise + presentIntersectionsWithFutureValues[(*valueIt)->presentIntersectionId] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

bool Value::extendsPastPresentAndPotential(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int intersectionIndex) const
{
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentAndPotentialNoise + (*valueIt)->intersectionsWithPresentAndPotentialValues[intersectionIndex][presentAndPotentialIntersectionId] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

bool Value::extendsFuturePresentAndPotential(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int reverseAttributeIndex) const
{
  const vector<unsigned int>& presentAndPotentialIntersectionsWithFutureValues = intersectionsWithPresentAndPotentialValues[intersectionsWithPresentAndPotentialValues.size() - reverseAttributeIndex];
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentAndPotentialNoise + presentAndPotentialIntersectionsWithFutureValues[(*valueIt)->presentAndPotentialIntersectionId] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

bool Value::symmetricValuesExtendPastPresent(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int intersectionIndex) const
{
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentNoise + (*valueIt)->intersectionsWithPresentValues[intersectionIndex][presentIntersectionId] + (*valueIt)->intersectionsWithPresentValues[intersectionIndex + 1][presentIntersectionId] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

bool Value::symmetricValuesExtendFuturePresent(const Value& symmetricValue, const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int reverseAttributeIndex) const
{
  const unsigned int intersectionIndex = intersectionsWithPresentValues.size() - reverseAttributeIndex;
  const vector<unsigned int>& presentIntersectionsWithFutureValues1 = intersectionsWithPresentValues[intersectionIndex + 1];
  const vector<unsigned int>& presentIntersectionsWithFutureValues2 = symmetricValue.intersectionsWithPresentValues[intersectionIndex];
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentNoise + presentIntersectionsWithFutureValues1[(*valueIt)->presentIntersectionId] + presentIntersectionsWithFutureValues2[(*valueIt)->presentIntersectionId] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

bool Value::symmetricValuesExtendPastPresentAndPotential(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int intersectionIndex) const
{
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentAndPotentialNoise + (*valueIt)->intersectionsWithPresentAndPotentialValues[intersectionIndex][presentAndPotentialIntersectionId] + (*valueIt)->intersectionsWithPresentAndPotentialValues[intersectionIndex + 1][presentAndPotentialIntersectionId] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

bool Value::symmetricValuesExtendFuturePresentAndPotential(const Value& symmetricValue, const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int reverseAttributeIndex) const
{
  const unsigned int intersectionIndex = intersectionsWithPresentAndPotentialValues.size() - reverseAttributeIndex;
  const vector<unsigned int>& presentAndPotentialIntersectionsWithFutureValues1 = intersectionsWithPresentAndPotentialValues[intersectionIndex + 1];
  const vector<unsigned int>& presentAndPotentialIntersectionsWithFutureValues2 = symmetricValue.intersectionsWithPresentAndPotentialValues[intersectionIndex];
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentAndPotentialNoise + presentAndPotentialIntersectionsWithFutureValues1[(*valueIt)->presentAndPotentialIntersectionId] + presentAndPotentialIntersectionsWithFutureValues2[(*valueIt)->presentAndPotentialIntersectionId] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

#ifdef ASSERT
unsigned int Value::presentNoiseAtIntersectionWith(const Value& otherValue, const unsigned int intersectionIndex) const
{
  return intersectionsWithPresentValues[intersectionIndex][otherValue.presentIntersectionId];
}

unsigned int Value::presentAndPotentialNoiseAtIntersectionWith(const Value& otherValue, const unsigned int intersectionIndex) const
{
  return intersectionsWithPresentAndPotentialValues[intersectionIndex][otherValue.presentAndPotentialIntersectionId];
}
#endif

bool Value::smallerDataId(const Value* value, const Value* otherValue)
{
  return value->dataId < otherValue->dataId;
}
