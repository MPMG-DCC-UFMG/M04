// Copyright (C) 2013-2020 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "Attribute.h"

// // debug
// void Attribute::printPresentAndPotentialIntersectionsWithEnumeratedValueInNextAttribute(const Attribute& nextAttribute) const
// {
//   if (nextAttribute.absentIndex < nextAttribute.values.size())
//     {
//   cout << "intersections with ";
//   nextAttribute.printValueFromDataId(nextAttribute.values[nextAttribute.absentIndex]->getDataId(), cout);
//   cout << " aka " << nextAttribute.values[nextAttribute.absentIndex]->getDataId() << ':';
//   const unsigned int valueId = nextAttribute.values[nextAttribute.absentIndex]->getPresentAndPotentialIntersectionId();
//   for (const Value* value : values)
//     {
//       cout << ' ' << value->getIntersectionsBeginWithPresentAndPotentialValues()->at(valueId);
//     }
//   cout << '\n';
//     }
// }

unsigned int Attribute::noisePerUnit;

unsigned int Attribute::maxId = 0;
unsigned int Attribute::orderedAttributeId;
vector<unsigned int> Attribute::epsilonVector;
vector<vector<string>> Attribute::labelsVector;
unsigned int Attribute::minArea;
vector<bool> Attribute::isClosedVector;
vector<unsigned int> Attribute::minSizes;
bool Attribute::isAnAttributeSymmetric = false;

string Attribute::outputElementSeparator;
string Attribute::emptySetString;
string Attribute::elementNoiseSeparator;
bool Attribute::isNoisePrinted;

#if defined DEBUG || defined VERBOSE_ELEMENT_CHOICE || defined ASSERT
vector<unsigned int> Attribute::internal2ExternalAttributeOrder;
#endif

Attribute::Attribute(): id(0), values(), potentialIndex(0), irrelevantIndex(0), absentIndex(0), presentAndPotentialIrrelevancyThreshold(0), maxNbOfPresentAndPotentiaValuesEver(0), maxNbOfAbsentValuesEver(0)
{
}

Attribute::Attribute(const vector<unsigned int>& nbOfValuesPerAttribute, const double epsilon, const unsigned int minSize, const vector<string>& labels): id(maxId++), values(), potentialIndex(0), irrelevantIndex(0), absentIndex(0), presentAndPotentialIrrelevancyThreshold(0), maxNbOfPresentAndPotentiaValuesEver(0), maxNbOfAbsentValuesEver(0)
{
  const vector<unsigned int>::const_iterator nbOfValuesInThisAttributeIt = nbOfValuesPerAttribute.begin() + id;
  irrelevantIndex = *nbOfValuesInThisAttributeIt;
  absentIndex = irrelevantIndex;
  maxNbOfPresentAndPotentiaValuesEver = absentIndex;
  unsigned int sizeOfAValue = noisePerUnit;
  const unsigned int nbOfSubsequentAttributes = nbOfValuesPerAttribute.size() - id - 1;
  vector<unsigned int>::const_iterator nbOfValuesInAttributeIt = nbOfValuesPerAttribute.begin();
  for (; nbOfValuesInAttributeIt != nbOfValuesInThisAttributeIt; ++nbOfValuesInAttributeIt)
    {
      sizeOfAValue *= *nbOfValuesInAttributeIt;
    }
  while (++nbOfValuesInAttributeIt != nbOfValuesPerAttribute.end())
    {
      sizeOfAValue *= *nbOfValuesInAttributeIt;
    }
  values.reserve(irrelevantIndex);
  for (unsigned int valueId = 0; valueId != *nbOfValuesInThisAttributeIt; ++valueId)
    {
      values.push_back(new Value(valueId, sizeOfAValue, nbOfSubsequentAttributes));
    }
  epsilonVector.push_back(epsilon * noisePerUnit);
  minSizes.push_back(minSize);
  labelsVector.push_back(labels);
}

Attribute::Attribute(const Attribute& parentAttribute): id(parentAttribute.id), values(), potentialIndex(parentAttribute.potentialIndex), irrelevantIndex(parentAttribute.irrelevantIndex), absentIndex(parentAttribute.absentIndex), presentAndPotentialIrrelevancyThreshold(parentAttribute.presentAndPotentialIrrelevancyThreshold), maxNbOfPresentAndPotentiaValuesEver(0), maxNbOfAbsentValuesEver(0)
{
}

Attribute::~Attribute()
{
  for (const Value* value : values)
    {
      delete value;
    }
}

Attribute* Attribute::clone() const
{
  return new Attribute(*this);
}

ostream& operator<<(ostream& out, const Attribute& attribute)
{
  attribute.printValues(attribute.values.begin(), attribute.values.begin() + attribute.irrelevantIndex, out);
  return out;
}

void Attribute::subtractSelfLoopsFromPotentialNoise(const unsigned int totalMembershipDueToSelfLoopsOnASymmetricValue, const unsigned int nbOfSymmetricElements)
{
  const unsigned int totalMembershipDueToSelfLoopsOnValue = totalMembershipDueToSelfLoopsOnASymmetricValue / irrelevantIndex * nbOfSymmetricElements;
  for (Value* value : values)
    {
      value->subtractPotentialNoise(totalMembershipDueToSelfLoopsOnValue);
    }
}

void Attribute::setPresentIntersections(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const unsigned int presentAttributeId)
{
  values.reserve((*parentAttributeIt)->values.size());
  const vector<Attribute*>::const_iterator parentNextAttributeIt = parentAttributeIt + 1;
  unsigned int newId = 0;
  vector<Value*>::const_iterator parentValueIt = (*parentAttributeIt)->values.begin();
  vector<Value*>::const_iterator end = parentValueIt + potentialIndex;
  for (; parentValueIt != end; ++parentValueIt)
    {
      values.push_back(createPresentChildValue(**parentValueIt, newId++, parentNextAttributeIt, parentAttributeEnd, presentAttributeId));
    }
  if (isEnumeratedAttribute(presentAttributeId))
    {
      values.push_back(createChosenValue(**parentValueIt++, newId, parentNextAttributeIt, parentAttributeEnd));
      ++potentialIndex;
    }
  newId = 0;
  end = (*parentAttributeIt)->values.end();
  for (; parentValueIt != end; ++parentValueIt)
    {
      values.push_back(createPotentialOrAbsentChildValue(**parentValueIt, newId++, parentNextAttributeIt, parentAttributeEnd, presentAttributeId));
    }
  if (isEnumeratedAttribute(presentAttributeId))
    {
      --potentialIndex;
    }
}

Value* Attribute::createPresentChildValue(const Value& parentValue, const unsigned int newId, const vector<Attribute*>::const_iterator parentNextAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const unsigned int presentAttributeId) const
{
  Value* childValue = new Value(parentValue, newId);
  vector<vector<unsigned int>>::iterator noiseInIntersectionWithPresentValuesIt = childValue->getIntersectionsBeginWithPresentValues();
  vector<vector<unsigned int>>::const_iterator parentNoiseInIntersectionWithPresentValuesIt = parentValue.getIntersectionsBeginWithPresentValues();
  for (vector<Attribute*>::const_iterator parentAttributeIt = parentNextAttributeIt; parentAttributeIt != parentAttributeEnd; ++parentAttributeIt)
    {
      (*parentAttributeIt)->setPresentChildValuePresentIntersections(*parentNoiseInIntersectionWithPresentValuesIt++, *noiseInIntersectionWithPresentValuesIt++, presentAttributeId);
    }
  return childValue;
}

void Attribute::setPresentChildValuePresentIntersections(const vector<unsigned int>& noiseInIntersectionWithPresentValues, vector<unsigned int>& childNoiseInIntersectionWithPresentValues, const unsigned int presentAttributeId) const
{
  const vector<Value*>::const_iterator end = values.end();
  if (isEnumeratedAttribute(presentAttributeId))
    {
      childNoiseInIntersectionWithPresentValues.reserve(values.size() - potentialIndex - 1);
      for (vector<Value*>::const_iterator valueIt = values.begin() + potentialIndex + 1; valueIt != end; ++valueIt)
	{
	  childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[(*valueIt)->getPresentIntersectionId()]);
	}
      return;
    }
  childNoiseInIntersectionWithPresentValues.reserve(values.size() - potentialIndex);
  for (vector<Value*>::const_iterator valueIt = values.begin() + potentialIndex; valueIt != end; ++valueIt)
    {
      childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[(*valueIt)->getPresentIntersectionId()]);
    }
}

Value* Attribute::createChosenValue(const Value& parentValue, const unsigned int newId, const vector<Attribute*>::const_iterator parentNextAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd) const
{
  Value* chosenValue = new Value(parentValue, newId);
  vector<vector<unsigned int>>::iterator noiseInIntersectionWithPresentValuesIt = chosenValue->getIntersectionsBeginWithPresentValues();
  for (vector<Attribute*>::const_iterator parentAttributeIt = parentNextAttributeIt; parentAttributeIt != parentAttributeEnd; ++parentAttributeIt)
    {
      (*parentAttributeIt)->setChosenChildValuePresentIntersections(*noiseInIntersectionWithPresentValuesIt++);
    }
  return chosenValue;
}

void Attribute::setChosenChildValuePresentIntersections(vector<unsigned int>& childNoiseInIntersectionWithPresentValues) const
{
  childNoiseInIntersectionWithPresentValues.resize(values.size() - potentialIndex);
}

Value* Attribute::createPotentialOrAbsentChildValue(const Value& parentValue, const unsigned int newId, const vector<Attribute*>::const_iterator parentNextAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const unsigned int presentAttributeId) const
{
  Value* childValue = new Value(parentValue, newId);
  vector<vector<unsigned int>>::iterator noiseInIntersectionWithPresentValuesIt = childValue->getIntersectionsBeginWithPresentValues();
  vector<vector<unsigned int>>::const_iterator parentNoiseInIntersectionWithPresentValuesIt = parentValue.getIntersectionsBeginWithPresentValues();
  for (vector<Attribute*>::const_iterator parentAttributeIt = parentNextAttributeIt; parentAttributeIt != parentAttributeEnd; ++parentAttributeIt)
    {
      (*parentAttributeIt)->setPotentialOrAbsentChildValuePresentIntersections(*parentNoiseInIntersectionWithPresentValuesIt++, *noiseInIntersectionWithPresentValuesIt++, presentAttributeId);
    }
  return childValue;
}

void Attribute::setPotentialOrAbsentChildValuePresentIntersections(const vector<unsigned int>& noiseInIntersectionWithPresentValues, vector<unsigned int>& childNoiseInIntersectionWithPresentValues, const unsigned int presentAttributeId) const
{
  const vector<Value*>::const_iterator end = values.begin() + potentialIndex;
  if (isEnumeratedAttribute(presentAttributeId))
    {
      childNoiseInIntersectionWithPresentValues.reserve(potentialIndex + 1);
      for (vector<Value*>::const_iterator valueIt = values.begin(); valueIt != end; ++valueIt)
	{
	  childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[(*valueIt)->getPresentIntersectionId()]);
	}
      childNoiseInIntersectionWithPresentValues.push_back(0);
      return;
    }
  childNoiseInIntersectionWithPresentValues.reserve(potentialIndex);
  for (vector<Value*>::const_iterator valueIt = values.begin(); valueIt != end; ++valueIt)
    {
      childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[(*valueIt)->getPresentIntersectionId()]);
    }
}

void Attribute::setPresentAndPotentialIntersections(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator attributeIt, const vector<Attribute*>::const_iterator attributeEnd)
{
  maxNbOfPresentAndPotentiaValuesEver = irrelevantIndex;
  maxNbOfAbsentValuesEver = values.size() - absentIndex;
  const vector<Attribute*>::const_iterator parentNextAttributeIt = parentAttributeIt + 1;
  const vector<Attribute*>::const_iterator nextAttributeIt = attributeIt + 1;
  vector<Value*>::const_iterator parentValueIt = (*parentAttributeIt)->values.begin();
  unsigned int newId = 0;
  vector<Value*>::iterator end = values.begin() + irrelevantIndex;
  for (vector<Value*>::iterator valueIt = values.begin(); valueIt != end; ++valueIt)
    {
      vector<vector<unsigned int>>::iterator presentAndPotentialIntersectionsIt = (*valueIt)->setPresentAndPotentialIntersectionId(newId++);
      for (; (*parentValueIt)->getDataId() != (*valueIt)->getDataId(); ++parentValueIt)
	{
	}
      vector<vector<unsigned int>>::iterator parentPresentAndPotentialIntersectionsIt = (*parentValueIt)->getIntersectionsBeginWithPresentAndPotentialValues();
      vector<Attribute*>::const_iterator subsequentParentAttributeIt = parentNextAttributeIt;
      for (vector<Attribute*>::const_iterator subsequentAttributeIt = nextAttributeIt; subsequentAttributeIt != attributeEnd; ++subsequentAttributeIt)
	{
	  (*subsequentAttributeIt)->setPresentAndPotentialIntersectionsWithAbsentValues(**subsequentParentAttributeIt++, *parentPresentAndPotentialIntersectionsIt++, *presentAndPotentialIntersectionsIt++);
	}
      ++parentValueIt;
    }
  newId = 0;
  end = values.end();
  for (vector<Value*>::iterator valueIt = values.begin() + absentIndex; valueIt != end; ++valueIt)
    {
      vector<vector<unsigned int>>::iterator presentAndPotentialIntersectionsIt = (*valueIt)->setPresentAndPotentialIntersectionId(newId++);
      for (; (*parentValueIt)->getDataId() != (*valueIt)->getDataId(); ++parentValueIt)
	{
	}
      vector<vector<unsigned int>>::iterator parentPresentAndPotentialIntersectionsIt = (*parentValueIt)->getIntersectionsBeginWithPresentAndPotentialValues();
      vector<Attribute*>::const_iterator subsequentParentAttributeIt = parentNextAttributeIt;
      for (vector<Attribute*>::const_iterator subsequentAttributeIt = nextAttributeIt; subsequentAttributeIt != attributeEnd; ++subsequentAttributeIt)
	{
	  (*subsequentAttributeIt)->setPresentAndPotentialIntersectionsWithPresentAndPotentialValues(**subsequentParentAttributeIt++, *parentPresentAndPotentialIntersectionsIt++, *presentAndPotentialIntersectionsIt++);
	}
      ++parentValueIt;
    }
}

void Attribute::setPresentAndPotentialIntersectionsWithPresentAndPotentialValues(const Attribute& parentAttribute, const vector<unsigned int>& parentNoiseInIntersectionWithPresentAndPotentialValues, vector<unsigned int>& noiseInIntersectionWithPresentAndPotentialValues) const
{
  noiseInIntersectionWithPresentAndPotentialValues.reserve(irrelevantIndex);
  vector<Value*>::const_iterator parentValueIt = parentAttribute.values.begin();
  const vector<Value*>::const_iterator end = values.begin() + irrelevantIndex;
  for (vector<Value*>::const_iterator valueIt = values.begin(); valueIt != end; ++valueIt)
    {
      for (; (*parentValueIt)->getDataId() != (*valueIt)->getDataId(); ++parentValueIt)
	{
	}
      noiseInIntersectionWithPresentAndPotentialValues.push_back(parentNoiseInIntersectionWithPresentAndPotentialValues[(*parentValueIt)->getPresentAndPotentialIntersectionId()]);
      ++parentValueIt;
    }
}

void Attribute::setPresentAndPotentialIntersectionsWithAbsentValues(const Attribute& parentAttribute, const vector<unsigned int>& parentNoiseInIntersectionWithPresentAndPotentialValues, vector<unsigned int>& noiseInIntersectionWithPresentAndPotentialValues) const
{
  noiseInIntersectionWithPresentAndPotentialValues.reserve(maxNbOfAbsentValuesEver); // reserve(irrelevantIndex - potentialIndex + maxNbOfAbsentValuesEver), to avoid reallocs, worsen the time and space performances
  vector<Value*>::const_iterator parentValueIt = parentAttribute.absentBegin();
  const vector<Value*>::const_iterator end = values.end();
  for (vector<Value*>::const_iterator valueIt = values.begin() + absentIndex; valueIt != end; ++valueIt)
    {
      for (; (*parentValueIt)->getDataId() != (*valueIt)->getDataId(); ++parentValueIt)
	{
	}
      noiseInIntersectionWithPresentAndPotentialValues.push_back(parentNoiseInIntersectionWithPresentAndPotentialValues[(*parentValueIt)->getPresentAndPotentialIntersectionId()]);
      ++parentValueIt;
    }
}

unsigned int Attribute::getId() const
{
  return id;
}

bool Attribute::closedAttribute() const
{
  return isClosedVector[id];
}

bool Attribute::symmetric() const
{
  return false;
}

bool Attribute::metric() const
{
  return false;
}

Attribute* Attribute::thisOrFirstSymmetricAttribute()
{
  return this;
}

bool Attribute::isEnumeratedAttribute(const unsigned int enumeratedAttributeId) const
{
  return enumeratedAttributeId == id;
}

vector<unsigned int> Attribute::getPresentAndPotentialDataIds() const
{
  vector<unsigned int> dataIds;
  dataIds.reserve(irrelevantIndex);
  const vector<Value*>::const_iterator end = values.begin() + irrelevantIndex;
  for (vector<Value*>::const_iterator valueIt = values.begin(); valueIt != end; ++valueIt)
    {
      dataIds.push_back((*valueIt)->getDataId());
    }
  return dataIds;
}

vector<unsigned int> Attribute::getIrrelevantDataIds() const
{
  vector<unsigned int> dataIds;
  dataIds.reserve(absentIndex - irrelevantIndex);
  const vector<Value*>::const_iterator end = values.begin() + absentIndex;
  for (vector<Value*>::const_iterator valueIt = values.begin() + irrelevantIndex; valueIt != end; ++valueIt)
    {
      dataIds.push_back((*valueIt)->getDataId());
    }
  return dataIds;
}

vector<Value*>::const_iterator Attribute::presentBegin() const
{
  return values.begin();
}

vector<Value*>::const_iterator Attribute::presentEnd() const
{
  return values.begin() + potentialIndex;
}

vector<Value*>::const_iterator Attribute::potentialBegin() const
{
  return values.begin() + potentialIndex;
}

vector<Value*>::const_iterator Attribute::potentialEnd() const
{
  return values.begin() + irrelevantIndex;
}

vector<Value*>::const_iterator Attribute::irrelevantBegin() const
{
  return values.begin() + irrelevantIndex;
}

vector<Value*>::const_iterator Attribute::irrelevantEnd() const
{
  return values.begin() + absentIndex;
}

vector<Value*>::const_iterator Attribute::absentBegin() const
{
  return values.begin() + absentIndex;
}

vector<Value*>::const_iterator Attribute::absentEnd() const
{
  return values.end();
}

vector<Value*>::iterator Attribute::presentBegin()
{
  return values.begin();
}

vector<Value*>::iterator Attribute::presentEnd()
{
  return values.begin() + potentialIndex;
}

vector<Value*>::iterator Attribute::potentialBegin()
{
  return values.begin() + potentialIndex;
}

vector<Value*>::iterator Attribute::potentialEnd()
{
  return values.begin() + irrelevantIndex;
}

vector<Value*>::iterator Attribute::irrelevantBegin()
{
  return values.begin() + irrelevantIndex;
}

vector<Value*>::iterator Attribute::irrelevantEnd()
{
  return values.begin() + absentIndex;
}

vector<Value*>::iterator Attribute::absentBegin()
{
  return values.begin() + absentIndex;
}

vector<Value*>::iterator Attribute::absentEnd()
{
  return values.end();
}

unsigned int Attribute::sizeOfPresent() const
{
  return potentialIndex;
}

unsigned int Attribute::sizeOfPotential() const
{
  return irrelevantIndex - potentialIndex;
}

unsigned int Attribute::sizeOfPresentAndPotential() const
{
  return absentIndex;
}

bool Attribute::irrelevantEmpty() const
{
  return irrelevantIndex == absentIndex;
}

unsigned int Attribute::globalSize() const
{
  return values.size();
}

double Attribute::totalPresentAndPotentialNoise() const
{
  double totalNoise = 0;
  const vector<Value*>::const_iterator end = values.begin() + irrelevantIndex;
  for (vector<Value*>::const_iterator valueIt = values.begin(); valueIt != end; ++valueIt)
    {
      totalNoise += (*valueIt)->getPresentAndPotentialNoise();
    }
  return totalNoise;
}

double Attribute::averagePresentAndPotentialNoise() const
{
  return totalPresentAndPotentialNoise() / values.size();
}

void Attribute::sortPotentialAndAbsentButChosenPresentValueIfNecessary(const unsigned int presentAttributeId)
{
  if (id == orderedAttributeId && id != presentAttributeId)
    {
      sortPotentialAndAbsent(potentialIndex);
    }
}

void Attribute::sortPotentialIrrelevantAndAbsentButChosenAbsentValueIfNecessary(const unsigned int absentAttributeId)
{
  sortPotentialIrrelevantAndAbsentIfNecessary(absentAttributeId);
}

void Attribute::sortPotentialIrrelevantAndAbsentIfNecessary(const unsigned int absentAttributeId)
{
  if (id == orderedAttributeId && id != absentAttributeId)
    {
      sortPotentialIrrelevantAndAbsent(absentIndex);
    }
}

void Attribute::sortPotentialAndAbsent(const unsigned int realPotentialIndex)
{
  const vector<Value*>::iterator absentBegin = values.begin() + irrelevantIndex;
  sort(values.begin() + realPotentialIndex, absentBegin, Value::smallerDataId);
  sort(absentBegin, values.end(), Value::smallerDataId);
}

void Attribute::sortPotentialIrrelevantAndAbsent(const unsigned int realAbsentIndex)
{
  const vector<Value*>::iterator irrelevantBegin = values.begin() + irrelevantIndex;
  sort(values.begin() + potentialIndex, irrelevantBegin, Value::smallerDataId);
  sort(irrelevantBegin, values.begin() + absentIndex, Value::smallerDataId);
  sort(values.begin() + realAbsentIndex, values.end(), Value::smallerDataId);
}

unsigned int Attribute::getChosenValueDataId() const
{
  return values[potentialIndex]->getDataId();
}

Value& Attribute::getChosenValue() const
{
  return *values[potentialIndex];
}

void Attribute::repositionChosenPresentValue()
{
  if (id == orderedAttributeId)
    {
      repositionChosenPresentValueInOrderedAttribute();
    }
}

void Attribute::setChosenValuePresent()
{
  ++potentialIndex;
}

void Attribute::repositionChosenPresentValueInOrderedAttribute()
{
  const vector<Value*>::iterator valuesBegin = values.begin();
  vector<Value*>::iterator valueIt = valuesBegin + potentialIndex - 1;
  Value* chosenValue = *valueIt;
  const unsigned int chosenValueDataId = chosenValue->getDataId();
  if ((*valuesBegin)->getDataId() < chosenValueDataId)
    {
      for (vector<Value*>::iterator nextValueIt = valueIt - 1; (*nextValueIt)->getDataId() > chosenValueDataId; --nextValueIt)
	{
	  *valueIt = *nextValueIt;
	  --valueIt;
	}
      *valueIt = chosenValue;
      return;
    }
  rotate(valuesBegin, valueIt, valueIt + 1);
}

vector<unsigned int> Attribute::setChosenValueIrrelevant(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd)
{
  vector<unsigned int> irrelevantValueDataIds = setChosenValueIrrelevant();
  if (irrelevantValueDataIds.empty())
    {
      return irrelevantValueDataIds;
    }
#ifdef MIN_SIZE_ELEMENT_PRUNING
  if (!minArea)
    {
      return irrelevantValueDataIds;
    }
  bool isAbsentToBeCleaned = false;
  while (possiblePresentAndPotentialIrrelevancy(attributeBegin, attributeEnd))
    {
      if (presentAndPotentialIrrelevant())
	{
          return {};
        }
      isAbsentToBeCleaned = true;
      const vector<unsigned int> newIrrelevantValueDataIds = findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity().second;
      if (newIrrelevantValueDataIds.empty())
	{
          break;		// presentAndPotentialIrrelevancyThreshold will not change
        }
      irrelevantValueDataIds.insert(irrelevantValueDataIds.end(), newIrrelevantValueDataIds.begin(), newIrrelevantValueDataIds.end());
    }
  if (isAbsentToBeCleaned)
    {
      presentAndPotentialCleanAbsent();
    }
#endif
  return irrelevantValueDataIds;
}

vector<unsigned int> Attribute::setChosenValueIrrelevant()
{
  Value*& chosenAbsentValue = values[potentialIndex];
  const unsigned int chosenAbsentValueDataId = chosenAbsentValue->getDataId();
  swap(chosenAbsentValue, values[--irrelevantIndex]);
  return {chosenAbsentValueDataId};
}

unsigned int Attribute::getChosenAbsentValueDataId() const
{
  return values[absentIndex]->getDataId();
}

Value& Attribute::getChosenAbsentValue() const
{
  return *values[absentIndex];
}

void Attribute::setNullPresentAndPotentialIntersectionsWithNewAbsentValue(const unsigned int intersectionIndex)
{
  vector<Value*>::iterator valueIt = values.begin();
  const vector<Value*>::iterator end = valueIt + irrelevantIndex;
  for (; valueIt != end; ++valueIt)
    {
      (*valueIt)->setNullPresentAndPotentialIntersectionsWithNewAbsentValue(intersectionIndex);
    }
}

void Attribute::setLastIrrelevantValueChosen(const vector<Attribute*>::const_iterator thisAttributeIt, const vector<Attribute*>::const_iterator attributeEnd)
{
  Value& absentValue = *values[--absentIndex];
  absentValue.setPresentAndPotentialIntersectionId(maxNbOfAbsentValuesEver++);
  vector<vector<unsigned int>>::iterator presentAndPotentialIntersectionsIt = absentValue.getIntersectionsBeginWithPresentAndPotentialValues();
  for (vector<Attribute*>::const_iterator attributeIt = thisAttributeIt; ++attributeIt != attributeEnd; ++presentAndPotentialIntersectionsIt)
    {
      *presentAndPotentialIntersectionsIt = vector<unsigned int>((*attributeIt)->maxNbOfPresentAndPotentiaValuesEver);
    }
}

// It should be called after initialization only (values is ordered)
vector<vector<unsigned int>>::iterator Attribute::getIntersectionsBeginWithPotentialValues(const unsigned int valueId)
{
  return values[valueId]->getIntersectionsBeginWithPresentAndPotentialValues();
}

// It should be called after initialization only (values is ordered)
void Attribute::decrementPotentialNoise(const unsigned int valueId)
{
  values[valueId]->decrementPotentialNoise();
}

// It should be called after initialization only (values is ordered)
void Attribute::subtractPotentialNoise(const unsigned int valueId, const unsigned int noise)
{
  values[valueId]->subtractPotentialNoise(noise);
}

void Attribute::printValues(const vector<Value*>::const_iterator begin, const vector<Value*>::const_iterator end, ostream& out) const
{
  bool isFirstElement = true;
  const vector<string>& labels = labelsVector[id];
  for (vector<Value*>::const_iterator valueIt = begin; valueIt != end; ++valueIt)
    {
      if (isFirstElement)
	{
	  isFirstElement = false;
	}
      else
	{
	  out << outputElementSeparator;
	}
      out << labels[(*valueIt)->getDataId()];
      if (isNoisePrinted)
	{
	  out << elementNoiseSeparator << static_cast<float>((*valueIt)->getPresentAndPotentialNoise()) / noisePerUnit;
	}
    }
  if (isFirstElement)
    {
      out << emptySetString;
    }
}

void Attribute::printValueFromDataId(const unsigned int valueDataId, ostream& out) const
{
  out << labelsVector[id][valueDataId];
}

#ifdef DEBUG
void Attribute::printPresent(ostream& out) const
{
  printValues(values.begin(), values.begin() + potentialIndex, out);
}

void Attribute::printPotential(ostream& out) const
{
  printValues(values.begin() + potentialIndex, values.begin() + irrelevantIndex, out);
}

void Attribute::printAbsent(ostream& out) const
{
  printValues(values.begin() + absentIndex, values.end(), out);
}

void Attribute::printChosenValue(ostream& out) const
{
  out << labelsVector[id][values[potentialIndex]->getDataId()] << " in attribute " << internal2ExternalAttributeOrder[id];
}
#endif

#ifdef ASSERT
void Attribute::printValue(const Value& value, ostream& out) const
{
  out << labelsVector[id][value.getDataId()] << " in attribute " << internal2ExternalAttributeOrder[id];
}
#endif

#if defined DEBUG || defined VERBOSE_ELEMENT_CHOICE || defined ASSERT
void Attribute::setInternal2ExternalAttributeOrder(const vector<unsigned int>& internal2ExternalAttributeOrderParam)
{
  internal2ExternalAttributeOrder = internal2ExternalAttributeOrderParam;
}
#endif

Attribute* Attribute::chooseValue(const vector<Attribute*>& attributes)
{
  bool isNoEpsilonReachedChoosingASymmetricAttribute = true;
  Attribute* firstSymmetricAttribute = nullptr;
  vector<unsigned int>::const_iterator epsilonIt = epsilonVector.begin();
  const vector<Attribute*>::const_iterator attributeEnd = attributes.end();
  const vector<Attribute*>::const_iterator attributeBegin = attributes.begin();
  vector<Attribute*>::const_iterator attributeIt = attributeBegin;
  if (isAnAttributeSymmetric)
    {
      for (; !(*attributeIt)->symmetric(); ++attributeIt)
	{
	}
      firstSymmetricAttribute = *attributeIt;
      attributeIt = attributeBegin;
#ifdef TWO_MODE_ELEMENT_CHOICE
      if (firstSymmetricAttribute->potentialIndex != firstSymmetricAttribute->irrelevantIndex)
	{
	  // Testing whether choosing a symmetric attribute allows to reach an epsilon
	  const unsigned int nbOfPresentSymmetricElementsPlusOne = firstSymmetricAttribute->potentialIndex + 1;
	  for (; attributeIt != attributeEnd; ++attributeIt)
	    {
	      unsigned int presentArea = nbOfPresentSymmetricElementsPlusOne * noisePerUnit;
	      vector<Attribute*>::const_iterator otherAttributeIt = attributeBegin;
	      if (*attributeIt == firstSymmetricAttribute)
		{
		  for (; *otherAttributeIt != firstSymmetricAttribute; ++otherAttributeIt)
		    {
		      presentArea *= (*attributeIt)->potentialIndex;
		    }
		  for (++otherAttributeIt; ++otherAttributeIt != attributeEnd; )
		    {
		      presentArea *= (*attributeIt)->potentialIndex;
		    }
		  if (presentArea > *epsilonIt || presentArea > *++epsilonIt)
		    {
		      break;
		    }
		  ++attributeIt;
		}
	      else
		{
		  for (; *otherAttributeIt != firstSymmetricAttribute; ++otherAttributeIt)
		    {
		      if (otherAttributeIt != attributeIt)
			{
			  presentArea *= (*attributeIt)->potentialIndex;
			}
		    }
		  for (++otherAttributeIt; ++otherAttributeIt != attributeEnd; )
		    {
		      if (otherAttributeIt != attributeIt)
			{
			  presentArea *= (*attributeIt)->potentialIndex;
			}
		    }
		  if (presentArea * nbOfPresentSymmetricElementsPlusOne > *epsilonIt)
		    {
		      break;
		    }
		}
	      ++epsilonIt;
	    }
	  if (attributeIt == attributeEnd)
	    {
	      // No epsilon reached by choosing a symmetric attribute: resetting variables to test as if no attribute was symmetric
	      attributeIt = attributeBegin;
	      epsilonIt = epsilonVector.begin();
	    }
	  else
	    {
	      // *epsilonIt reached by choosing a symmetric attribute: setting attributeIt to attributeEnd to not test choosing other attributes
	      attributeIt = attributeEnd;
	      isNoEpsilonReachedChoosingASymmetricAttribute = false;
	    }
	}
    }
  for (; attributeIt != attributeEnd; ++attributeIt)
    {
      unsigned int presentArea = noisePerUnit;
      unsigned int smallestOtherPresentSize = numeric_limits<unsigned int>::max();
      for (vector<Attribute*>::const_iterator otherAttributeIt = attributeBegin; otherAttributeIt != attributeEnd; ++otherAttributeIt)
	{
	  if (otherAttributeIt != attributeIt)
	    {
	      if ((*otherAttributeIt)->potentialIndex < smallestOtherPresentSize && (*otherAttributeIt)->potentialIndex != (*otherAttributeIt)->irrelevantIndex)
		{
		  if (smallestOtherPresentSize != numeric_limits<unsigned int>::max())
		    {
		      presentArea *= smallestOtherPresentSize;
		    }
		  smallestOtherPresentSize = (*otherAttributeIt)->potentialIndex;
		}
	      else
		{
		  presentArea *= (*otherAttributeIt)->potentialIndex;
		}
	    }
	}
      // TODO: the code below subtracts the self loops; it cannot be used without multiplicativePartition doing the same
      // if (firstSymmetricAttribute != nullptr && presentArea)
      // 	{
      // 	  presentArea -= presentArea / firstSymmetricAttribute->potentialIndex;
      // 	}
      if (!++smallestOtherPresentSize || smallestOtherPresentSize * presentArea > *epsilonIt++)
	{
	  // only *attributeIt has potential elements || *epsilonIt can be reached
	  break;
	}
    }
  if (attributeIt == attributeEnd && isNoEpsilonReachedChoosingASymmetricAttribute)
    {
#ifdef VERBOSE_ELEMENT_CHOICE
      cout << "No value set present allows to reach an epsilon\n";
#endif
      Attribute* bestAttribute;
      double bestCost = numeric_limits<double>::infinity();
      vector<Attribute*> orderedAttributes = attributes;
      sort(orderedAttributes.begin(), orderedAttributes.end());
      const vector<Attribute*>::iterator orderedAttributeEnd = orderedAttributes.end();
      for (vector<Attribute*>::iterator orderedAttributeIt = orderedAttributes.begin(); orderedAttributeIt != orderedAttributeEnd; ++orderedAttributeIt)
  	{
  	  Attribute* currentAttribute = *orderedAttributeIt;
#ifdef VERBOSE_ELEMENT_CHOICE
  	  cout << "Multiplicative partitions of attribute " << internal2ExternalAttributeOrder[currentAttribute->id] << "'s floor(epsilon) + 1: " << epsilonVector[currentAttribute->id] / noisePerUnit + 1 << '\n';
#endif
  	  vector<Attribute*>::const_iterator nextAttributeIt = orderedAttributes.erase(orderedAttributeIt);
  	  vector<unsigned int> factorization;
  	  factorization.reserve(maxId - 1);
	  currentAttribute->multiplicativePartition(epsilonVector[currentAttribute->id] / noisePerUnit + 1, maxId - 1, 1, orderedAttributes, factorization, bestCost, bestAttribute);
  	  orderedAttributeIt = orderedAttributes.insert(nextAttributeIt, currentAttribute);
  	}
      if (bestCost == numeric_limits<double>::infinity())
	{
	  // impossible to reach any epsilon (only happens if some attribute with potential values is unclosed)
	  vector<bool>::const_iterator isClosedIt = isClosedVector.begin();
	  for (attributeIt = attributeBegin; *isClosedIt++ || (*attributeIt)->potentialIndex == (*attributeIt)->irrelevantIndex; ++attributeIt)
	    {
	    }
#ifdef VERBOSE_ELEMENT_CHOICE
	  cout << "Any subset of potential values in unclosed attributes leads to a closed-" << maxId << "-set: choosing first potential value in attribute " << internal2ExternalAttributeOrder[(*attributeIt)->id] << '\n';
#endif
	  return *attributeIt;
	}
      bestAttribute = bestAttribute->thisOrFirstSymmetricAttribute();
      double appeal = -1;
      bestAttribute->chooseValue(bestAttribute->getIndexOfValueToChoose(attributes, firstSymmetricAttribute, appeal));
      return bestAttribute;
    }
#endif
  for (attributeIt = attributeBegin; (*attributeIt)->potentialIndex == (*attributeIt)->irrelevantIndex; ++attributeIt)
    {
    }
  Attribute* bestAttribute = *attributeIt;
  double bestAppeal = -1;
  unsigned int indexOfValueToChoose = bestAttribute->getIndexOfValueToChoose(attributes, firstSymmetricAttribute, bestAppeal);
  while (++attributeIt != attributeEnd)
    {
      if ((*attributeIt)->potentialIndex != (*attributeIt)->irrelevantIndex)
	{
	  double appeal = bestAppeal;
	  const unsigned int indexOfValueToChooseInAttribute = (*attributeIt)->getIndexOfValueToChoose(attributes, firstSymmetricAttribute, appeal);
	  if (appeal > bestAppeal)
	    {
	      bestAppeal = appeal;
	      indexOfValueToChoose = indexOfValueToChooseInAttribute;
	      bestAttribute = *attributeIt;
	    }
	}
    }
  bestAttribute->chooseValue(indexOfValueToChoose);
  return bestAttribute;
}

#ifdef TWO_MODE_ELEMENT_CHOICE
void Attribute::findBetterEnumerationStrategy(const vector<Attribute*>& attributes, const vector<unsigned int>& factorization, const bool isAnElementTakenInThis, double& bestCost, Attribute*& bestAttribute) const
{
  Attribute* bestAttributeForCurrentPermutation = const_cast<Attribute*>(this);
  double cost;
  if (isAnElementTakenInThis)
    {
      cost = irrelevantIndex - potentialIndex;
    }
  else
    {
      cost = 1;
    }
  bool isNotFirstSymmetric = false;
  vector<unsigned int>::const_reverse_iterator factorIt = factorization.rbegin(); // reverse to take the greatest factor among those for symmetric attributes
  for (Attribute* attribute : attributes)
    {
      if (attribute->irrelevantIndex < *factorIt)
	{
	  return;
	}
      if (attribute->symmetric())
	{
	  if (isNotFirstSymmetric)
	    {
	      ++factorIt;
	      continue;
	    }
	  isNotFirstSymmetric = true;
	}
      cost *= math::binomial_coefficient<double>(attribute->irrelevantIndex, *factorIt);
      if (attribute->potentialIndex < *factorIt && bestAttributeForCurrentPermutation == this)
	{
	  bestAttributeForCurrentPermutation = attribute;
	}
      ++factorIt;
    }
  if (cost < bestCost)
    {
      bestCost = cost;
      bestAttribute = bestAttributeForCurrentPermutation;
    }
}

void Attribute::multiplicativePartition(const double number, const unsigned int nbOfFactors, const unsigned int begin, vector<Attribute*>& attributes, vector<unsigned int>& factorization, double& bestCost, Attribute*& bestAttribute) const
{
  if (nbOfFactors == 1)
    {
      const unsigned int factor = ceil(number);
      if (factor >= begin)
	{
	  factorization.push_back(factor);
#ifdef VERBOSE_ELEMENT_CHOICE
	  cout << ' ';
	  for (const unsigned int f : factorization)
	    {
	      cout << ' ' << f;
	    }
	  cout << '\n';
#endif
	  if (isAnAttributeSymmetric)
	    {
	      // the whole factorization is at one element of exceeding epsilon because of the self loops with no noise
	      do
		{
		  findBetterEnumerationStrategy(attributes, factorization, false, bestCost, bestAttribute);
		} while (next_permutation(attributes.begin(), attributes.end()));
	      factorization.pop_back();
	      return;
	    }
	  do
	    {
	      // one element to remove to be at one element of exceeding epsilon
	      unsigned int previousFactor = 0;
	      for (unsigned int& f : factorization)
	      	{
	      	  if (f == previousFactor)
	      	    {
	      	      continue;
	      	    }
	      	  --f;
	      	  findBetterEnumerationStrategy(attributes, factorization, !potentialIndex, bestCost, bestAttribute);
	      	  previousFactor = ++f;
	      	}
	      findBetterEnumerationStrategy(attributes, factorization, false, bestCost, bestAttribute);
	    } while (next_permutation(attributes.begin(), attributes.end()));
	  factorization.pop_back();
	}
      return;
    }
  const unsigned int end = ceil(pow(number, 1. / nbOfFactors)) + 1;
  for (unsigned int factor = begin; factor != end; ++factor)
    {
      factorization.push_back(factor);
      multiplicativePartition(number / factor, nbOfFactors - 1, factor, attributes, factorization, bestCost, bestAttribute);
      factorization.pop_back();
    }
}
#endif

unsigned int Attribute::getIndexOfValueToChoose(const double presentCoeff, const double presentAndPotentialCoeff, double& bestAppeal) const
{
  const vector<Value*>::const_iterator end = values.begin() + irrelevantIndex;
  vector<Value*>::const_iterator valueToChooseIt = values.begin() + potentialIndex;
  for (vector<Value*>::const_iterator potentialValueIt = valueToChooseIt; potentialValueIt != end; ++potentialValueIt)
    {
      const double appealOfValue = presentCoeff * (*potentialValueIt)->getPresentNoise() + presentAndPotentialCoeff * (*potentialValueIt)->getPresentAndPotentialNoise();
      if (appealOfValue > bestAppeal)
	{
#if defined VERBOSE_ELEMENT_CHOICE && ENUMERATION_PROCESS == 1
	  cout << "Appeal of attribute " << internal2ExternalAttributeOrder[id] << ": " << appealOfValue / noisePerUnit << " with element " << labelsVector[id][(*potentialValueIt)->getDataId()] << '\n';
#endif
	  bestAppeal = appealOfValue;
	  valueToChooseIt = potentialValueIt;
	}
    }
  return valueToChooseIt - values.begin();
}

unsigned int Attribute::getIndexOfValueToChoose(const vector<Attribute*>& attributes, const Attribute* symmetricAttribute, double& bestAppeal) const
{
  unsigned int testedArea = 0;
  unsigned int presentArea = 1;
  unsigned int presentAndPotentialArea = 1;
  for (const Attribute* potentialAttribute : attributes)
    {
      if (potentialAttribute != this)
      	{
	  presentArea *= potentialAttribute->potentialIndex;
	  presentAndPotentialArea *= potentialAttribute->irrelevantIndex;
	  unsigned int yesFactor = potentialAttribute->irrelevantIndex - potentialAttribute->potentialIndex;
      	  for (const Attribute* presentAttribute : attributes)
      	    {
      	      if (presentAttribute != this && presentAttribute != potentialAttribute)
      	  	{
      	  	  yesFactor *= presentAttribute->potentialIndex;
      	  	}
      	    }
	  if (isAnAttributeSymmetric && !potentialAttribute->symmetric() && yesFactor)
	    {
	      yesFactor -= yesFactor / symmetricAttribute->potentialIndex;
	    }
	  testedArea += yesFactor;
	}
    }
  if (!testedArea)
    {
#if ENUMERATION_PROCESS == 1
      if (!presentArea)
	{
#endif
#ifdef TWO_MODE_ELEMENT_CHOICE
	  return potentialIndex;
#endif
	  if (!potentialIndex)
	    {
#ifdef VERBOSE_ELEMENT_CHOICE
	      cout << "Appeal of attribute " << internal2ExternalAttributeOrder[id] << ": 0\n";
#endif
	      if (bestAppeal > 0)
		{
	           return 0;
	        }
	      bestAppeal = 0;
	      double appeal = 0;
	      return getIndexOfValueToChoose(1, 0, appeal);
	    }
#ifdef VERBOSE_ELEMENT_CHOICE
	  cout << "Appeal of attribute " << internal2ExternalAttributeOrder[id] << ": -1\n";
#endif
	  if (bestAppeal == -1)
	    {
	      double appeal = 0;
	      return getIndexOfValueToChoose(1, 0, appeal);
	    }
	  return 0;
#if ENUMERATION_PROCESS == 1
	}
      // only this has potential elements: with the code below, the denominator of presentAndPotentialCoeff is 0
      return getIndexOfValueToChoose(1, 0, bestAppeal);
#endif
    }
#if ENUMERATION_PROCESS == 0
#ifdef VERBOSE_ELEMENT_CHOICE
  cout << "Appeal of attribute " << internal2ExternalAttributeOrder[id] << ": " << testedArea << '\n';
#endif
  if (testedArea > bestAppeal)
    {
      bestAppeal = testedArea;
      double appeal = 0;
      return getIndexOfValueToChoose(1, 0, appeal);
    }
#else
  if (isAnAttributeSymmetric && presentAndPotentialArea)
    {
      presentAndPotentialArea -= presentAndPotentialArea / symmetricAttribute->irrelevantIndex;
      if (presentArea)
	{
	  presentArea -= presentArea / symmetricAttribute->potentialIndex;
	}
    }
  if (static_cast<double>(testedArea + (maxId - 1) * presentArea) * noisePerUnit > bestAppeal)
    {
      // the best possible appeal with this attribute can exceed bestAppeal
      const double presentAndPotentialCoeff = static_cast<double>(testedArea) / (presentAndPotentialArea - presentArea);
      return getIndexOfValueToChoose(-presentAndPotentialCoeff + maxId - 1, presentAndPotentialCoeff, bestAppeal);
    }
#if defined VERBOSE_ELEMENT_CHOICE && ENUMERATION_PROCESS == 1
  else
    {
      cout << "Appeal of attribute " << internal2ExternalAttributeOrder[id] << " cannot be higher than " << bestAppeal / noisePerUnit << '\n';
    }
#endif
#endif
  return 0;
}

void Attribute::chooseValue(const unsigned int indexOfValue)
{
  swap(values[potentialIndex], values[indexOfValue]);
}

void Attribute::setPotentialValueIrrelevant(const vector<Value*>::iterator potentialValueIt)
{
  swap(*potentialValueIt, values[--irrelevantIndex]);
}

bool Attribute::findIrrelevantValuesAndCheckTauContiguity(const vector<Attribute*>::iterator attributeBegin, const vector<Attribute*>::iterator attributeEnd)
{
  // A potential value is irrelevant if at least one previous present values is not extensible with it
  vector<Value*>::iterator potentialEnd = values.begin() + irrelevantIndex;
  for (vector<Value*>::iterator potentialValueIt = values.begin() + potentialIndex; potentialValueIt != potentialEnd; )
    {
      if (valueDoesNotExtendPresent(**potentialValueIt, attributeBegin, attributeEnd))
	{
#ifdef DEBUG
	  cout << labelsVector[id][(*potentialValueIt)->getDataId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " will never be present nor extend any future pattern\n";
#endif
	  iter_swap(potentialValueIt, --potentialEnd);
	  --irrelevantIndex;
	}
      else
	{
	  ++potentialValueIt;
	}
    }
  return false;
}

#ifdef MIN_SIZE_ELEMENT_PRUNING
void Attribute::resetPresentAndPotentialIrrelevancyThreshold()
{
  presentAndPotentialIrrelevancyThreshold = 0;
}

bool Attribute::possiblePresentAndPotentialIrrelevancy(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd)
{
  if ((presentAndPotentialIrrelevancyThreshold && !minArea) || !irrelevantIndex) // only !irrelevantIndex needed for correctness but minSizeIrrelevancyThreshold takes time, whereas presentAndPotentialIrrelevancyThreshold && !minArea does not
    {
      return false;
    }
  const unsigned int threshold = minSizeIrrelevancyThreshold(attributeBegin, attributeEnd);
  if (threshold <= presentAndPotentialIrrelevancyThreshold)
    {
      return false;
    }
  presentAndPotentialIrrelevancyThreshold = threshold;
  return true;
}

bool Attribute::presentAndPotentialIrrelevant() const
{
  const vector<Value*>::const_iterator end = values.begin() + potentialIndex;
  vector<Value*>::const_iterator presentValueIt = values.begin();
  for (; presentValueIt != end && (*presentValueIt)->getPresentAndPotentialNoise() <= presentAndPotentialIrrelevancyThreshold; ++presentValueIt)
    {
    }
#ifdef DEBUG
  if (presentValueIt != end)
    {
      cout << labelsVector[id][(*presentValueIt)->getDataId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " contains too much potential noise given the size constraints -> Prune!\n";
    }
#endif
  return presentValueIt != end;
}

pair<bool, vector<unsigned int>> Attribute::findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity()
{
  vector<unsigned int> newIrrelevantValueDataIds;
  const vector<Value*>::iterator potentialBegin = values.begin() + potentialIndex;
  vector<Value*>::iterator potentialEnd = values.begin() + irrelevantIndex;
  for (vector<Value*>::iterator potentialValueIt = potentialBegin; potentialValueIt != potentialEnd; )
    {
      // **potentialValueIt is irrelevant if it contains too much noise in any extension satisfying the minimal size constraints
      if (presentAndPotentialIrrelevantValue(**potentialValueIt))
	{
	  newIrrelevantValueDataIds.push_back((*potentialValueIt)->getDataId());
	  iter_swap(potentialValueIt, --potentialEnd);
	  --irrelevantIndex;
	}
      else
	{
	  ++potentialValueIt;
	}
    }
  return {false, newIrrelevantValueDataIds};
}

bool Attribute::presentAndPotentialIrrelevantValue(const Value& value) const
{
#ifdef DEBUG
  if (value.getPresentAndPotentialNoise() > presentAndPotentialIrrelevancyThreshold)
    {
      cout << "Given the minimal size constraints, " << labelsVector[id][value.getDataId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " will never be present nor extend any future pattern\n";
    }
#endif
  return value.getPresentAndPotentialNoise() > presentAndPotentialIrrelevancyThreshold;
}

void Attribute::presentAndPotentialCleanAbsent()
{
  const vector<Value*>::iterator absentBegin = values.begin() + absentIndex;
  for (vector<Value*>::iterator absentValueIt = absentBegin; absentValueIt != values.end(); )
    {
      if ((*absentValueIt)->getPresentAndPotentialNoise() > presentAndPotentialIrrelevancyThreshold)
	{
	  removeAbsentValue(absentValueIt);
	}
      else
	{
	  ++absentValueIt;
	}
    }
}

unsigned int Attribute::minSizeIrrelevancyThreshold(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  // Sum epsilon with the number of non-self-loops tuples in the maximal pattern and subtract the number of non-self-loops tuples in the minimal pattern
  bool isOneMinSize0 = false;
  unsigned int minNbOfSymmetricElements = 0;
  unsigned int maxNbOfSymmetricElements = 0;
  vector<float> minAreaDividedByProductsOfSubsequentMinSizes;
  minAreaDividedByProductsOfSubsequentMinSizes.reserve(maxId);
  minAreaDividedByProductsOfSubsequentMinSizes.push_back(-.5 + minArea); // -.5 to avoid round-off issues
  vector<unsigned int> minSizesOfOrthogonalAttributes;
  minSizesOfOrthogonalAttributes.reserve(maxId - 1);
  unsigned int maxAreaOfValue = 1;
  vector<Attribute*> orthogonalAttributes;
  orthogonalAttributes.reserve(maxId - 1);
  vector<Attribute*>::const_iterator attributeIt = attributeBegin;
  for (; *attributeIt != this; ++attributeIt)
    {
      orthogonalAttributes.push_back(*attributeIt);
    }
  while (++attributeIt != attributeEnd)
    {
      orthogonalAttributes.push_back(*attributeIt);
    }
  const vector<Attribute*>::const_reverse_iterator rend = orthogonalAttributes.rend();
  for (vector<Attribute*>::const_reverse_iterator orthogonalAttributeIt = orthogonalAttributes.rbegin(); orthogonalAttributeIt != rend; ++orthogonalAttributeIt)
    {
      const unsigned int minSize = max((*orthogonalAttributeIt)->potentialIndex, minSizes[(*orthogonalAttributeIt)->id]);
      if (minSize)
	{
	  minSizesOfOrthogonalAttributes.push_back(minSize);
	  minAreaDividedByProductsOfSubsequentMinSizes.push_back(minAreaDividedByProductsOfSubsequentMinSizes.back() / minSize);
	}
      else
	{
	  minSizesOfOrthogonalAttributes.push_back(1);
	  minAreaDividedByProductsOfSubsequentMinSizes.push_back(minAreaDividedByProductsOfSubsequentMinSizes.back());
	  isOneMinSize0 = true;
	}
      maxAreaOfValue *= (*orthogonalAttributeIt)->absentIndex;
      if ((*orthogonalAttributeIt)->symmetric())
	{
	  if (symmetric())
	    {
	      // To compute the minimal number of non-self loops on a symmetric value, the cardinality of this dimension is not fixed to the maximum
	      minAreaDividedByProductsOfSubsequentMinSizes.back() /= minSize;
	    }
	  if ((*orthogonalAttributeIt)->absentIndex > maxNbOfSymmetricElements) // absentIndex can differ between symmetric attributes!
      	    {
	      maxNbOfSymmetricElements = (*orthogonalAttributeIt)->absentIndex;
	      minNbOfSymmetricElements = minSize;
	    }
	}
    }
  if (!maxNbOfSymmetricElements)
    {
      if (minAreaDividedByProductsOfSubsequentMinSizes.back() > irrelevantIndex)
  	{
#ifdef MIN_AREA_REFINEMENT
	  unsigned int actualMinArea = numeric_limits<unsigned int>::max();
  	  orthogonalAttributes.front()->computeMinArea(minSizesOfOrthogonalAttributes.rbegin(), minAreaDividedByProductsOfSubsequentMinSizes.rbegin() + 1, orthogonalAttributes.begin(), orthogonalAttributes.end(), irrelevantIndex, actualMinArea);
  	  return epsilonVector[id] + (maxAreaOfValue - actualMinArea / irrelevantIndex) * noisePerUnit;
#else
	  return epsilonVector[id] + (maxAreaOfValue - ceil(minArea / irrelevantIndex)) * noisePerUnit;
#endif
  	}
      if (isOneMinSize0)
  	{
  	  return epsilonVector[id] + (maxAreaOfValue - ceil(minArea / irrelevantIndex)) * noisePerUnit;
  	}
      return epsilonVector[id] + (maxAreaOfValue - round(1. / minAreaDividedByProductsOfSubsequentMinSizes.back() * (-.5 + minArea))) * noisePerUnit;
    }
  if (symmetric())
    {
      const unsigned int minSize = max(potentialIndex, minSizes[id]);
      if (minAreaDividedByProductsOfSubsequentMinSizes.back() * minSize > irrelevantIndex)
	{
#ifdef MIN_AREA_REFINEMENT
	  unsigned int minNbOfNonSelfLoopsOnASymmetricValue = numeric_limits<unsigned int>::max();
	  orthogonalAttributes.front()->computeMinNbOfNonSelfLoopsOnASymmetricValue(minSizesOfOrthogonalAttributes.rbegin(), minAreaDividedByProductsOfSubsequentMinSizes.rbegin() + 1, orthogonalAttributes.begin(), orthogonalAttributes.end(), 0, 1, minNbOfNonSelfLoopsOnASymmetricValue);
	  return epsilonVector[id] + (maxAreaOfValue - maxAreaOfValue / maxNbOfSymmetricElements - minNbOfNonSelfLoopsOnASymmetricValue) * noisePerUnit;
#else
	  return epsilonVector[id] + (maxAreaOfValue - ceil(minArea / irrelevantIndex)) * noisePerUnit;
#endif
	}
      if (isOneMinSize0)
	{
	  return epsilonVector[id] + (maxAreaOfValue - ceil(minArea / irrelevantIndex)) * noisePerUnit;
	}
      const unsigned int minAreaOfValue = round(1. / minAreaDividedByProductsOfSubsequentMinSizes.back() * (-.5 + minArea) / minSize);
      return epsilonVector[id] + (maxAreaOfValue - maxAreaOfValue / maxNbOfSymmetricElements - minAreaOfValue + minAreaOfValue / minNbOfSymmetricElements) * noisePerUnit;
    }
  if (minAreaDividedByProductsOfSubsequentMinSizes.back() > irrelevantIndex)
    {
#ifdef MIN_AREA_REFINEMENT
      unsigned int minNbOfNonSelfLoops = numeric_limits<unsigned int>::max();
      orthogonalAttributes.front()->computeMinNbOfNonSelfLoopsWithSymmetricAttributes(minSizesOfOrthogonalAttributes.rbegin(), minAreaDividedByProductsOfSubsequentMinSizes.rbegin() + 1, orthogonalAttributes.begin(), orthogonalAttributes.end(), 0, irrelevantIndex, minNbOfNonSelfLoops);
      return epsilonVector[id] + (maxAreaOfValue - maxAreaOfValue / maxNbOfSymmetricElements - minNbOfNonSelfLoops / irrelevantIndex) * noisePerUnit;
#else
      return epsilonVector[id] + (maxAreaOfValue - ceil(minArea / irrelevantIndex)) * noisePerUnit;
#endif
    }
  if (isOneMinSize0)
    {
      return epsilonVector[id] + (maxAreaOfValue - ceil(minArea / irrelevantIndex)) * noisePerUnit;
    }
  const unsigned int minAreaOfValue = round(1. / minAreaDividedByProductsOfSubsequentMinSizes.back() * (-.5 + minArea));
  return epsilonVector[id] + (maxAreaOfValue - maxAreaOfValue / maxNbOfSymmetricElements - minAreaOfValue + minAreaOfValue / minNbOfSymmetricElements) * noisePerUnit;
}

// CLEAN: factorize with minSizeIrrelevancyThreshold; it is the same code ignoring the symmetry because there is no self loop in the present and potential hyperplane related to an absent symmetric value
unsigned int Attribute::minSizeIrrelevancyThresholdIgnoringSymmetry(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  // Sum epsilon with the number of non-self-loops tuples in the maximal pattern and subtract the number of non-self-loops tuples in the minimal pattern
  bool isOneMinSize0 = false;
  vector<float> minAreaDividedByProductsOfSubsequentMinSizes;
  minAreaDividedByProductsOfSubsequentMinSizes.reserve(maxId);
  minAreaDividedByProductsOfSubsequentMinSizes.push_back(-.5 + minArea); // -.5 to avoid round-off issues
  vector<unsigned int> minSizesOfOrthogonalAttributes;
  minSizesOfOrthogonalAttributes.reserve(maxId - 1);
  unsigned int maxAreaOfValue = 1;
  vector<Attribute*> orthogonalAttributes;
  orthogonalAttributes.reserve(maxId - 1);
  vector<Attribute*>::const_iterator attributeIt = attributeBegin;
  for (; *attributeIt != this; ++attributeIt)
    {
      orthogonalAttributes.push_back(*attributeIt);
    }
  while (++attributeIt != attributeEnd)
    {
      orthogonalAttributes.push_back(*attributeIt);
    }
  const vector<Attribute*>::const_reverse_iterator rend = orthogonalAttributes.rend();
  for (vector<Attribute*>::const_reverse_iterator orthogonalAttributeIt = orthogonalAttributes.rbegin(); orthogonalAttributeIt != rend; ++orthogonalAttributeIt)
    {
      const unsigned int minSize = max((*orthogonalAttributeIt)->potentialIndex, minSizes[(*orthogonalAttributeIt)->id]);
      if (minSize)
	{
	  minSizesOfOrthogonalAttributes.push_back(minSize);
	  minAreaDividedByProductsOfSubsequentMinSizes.push_back(minAreaDividedByProductsOfSubsequentMinSizes.back() / minSize);
	}
      else
	{
	  minSizesOfOrthogonalAttributes.push_back(1);
	  minAreaDividedByProductsOfSubsequentMinSizes.push_back(minAreaDividedByProductsOfSubsequentMinSizes.back());
	  isOneMinSize0 = true;
	}
      maxAreaOfValue *= (*orthogonalAttributeIt)->absentIndex;
    }
  if (minAreaDividedByProductsOfSubsequentMinSizes.back() > irrelevantIndex)
    {
#ifdef MIN_AREA_REFINEMENT
      unsigned int actualMinArea = numeric_limits<unsigned int>::max();
      orthogonalAttributes.front()->computeMinArea(minSizesOfOrthogonalAttributes.rbegin(), minAreaDividedByProductsOfSubsequentMinSizes.rbegin() + 1, orthogonalAttributes.begin(), orthogonalAttributes.end(), irrelevantIndex, actualMinArea);
      return epsilonVector[id] + (maxAreaOfValue - actualMinArea / irrelevantIndex) * noisePerUnit;
#else
      return epsilonVector[id] + (maxAreaOfValue - ceil(minArea / irrelevantIndex)) * noisePerUnit;
#endif
    }
  if (isOneMinSize0)
    {
      return epsilonVector[id] + (maxAreaOfValue - ceil(minArea / irrelevantIndex)) * noisePerUnit;
    }
  return epsilonVector[id] + (maxAreaOfValue - round(1. / minAreaDividedByProductsOfSubsequentMinSizes.back() * (-.5 + minArea))) * noisePerUnit;
}

#ifdef PRE_PROCESS
void Attribute::setPresentAndPotentialIrrelevancyThreshold(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd)
{
  presentAndPotentialIrrelevancyThreshold = minSizeIrrelevancyThreshold(attributeBegin, attributeEnd);
}
#endif

#ifdef MIN_AREA_REFINEMENT
void Attribute::computeMinArea(const vector<unsigned int>::const_reverse_iterator minSizeIt, const vector<float>::const_reverse_iterator minAreaDividedByProductOfSubsequentMinSizesIt, const vector<Attribute*>::const_iterator thisIt, const vector<Attribute*>::const_iterator attributeEnd, const unsigned int currentArea, unsigned int& minArea) const
{
  const vector<Attribute*>::const_iterator nextIt = thisIt + 1;
  if (nextIt == attributeEnd)
    {
      unsigned int factor = ceil(*minAreaDividedByProductOfSubsequentMinSizesIt / currentArea);
      if (factor <= irrelevantIndex)
      	{
	  if (factor < *minSizeIt)
	    {
	      factor = *minSizeIt;
	    }
	  factor *= currentArea;
      	  if (factor < minArea)
      	    {
      	      minArea = factor;
      	    }
      	}
      return;
    }
  const vector<unsigned int>::const_reverse_iterator nextMinSizesIt = minSizeIt + 1;
  const vector<float>::const_reverse_iterator nextMinAreaDividedByProductOfSubsequentMinSizesIt = minAreaDividedByProductOfSubsequentMinSizesIt + 1;
  const unsigned int end = min(static_cast<unsigned int>(ceil(*minAreaDividedByProductOfSubsequentMinSizesIt / currentArea)), irrelevantIndex);
  for (unsigned int factor = *minSizeIt; factor <= end; ++factor)
    {
      (*nextIt)->computeMinArea(nextMinSizesIt, nextMinAreaDividedByProductOfSubsequentMinSizesIt, nextIt, attributeEnd, factor * currentArea, minArea);
    }
}

void Attribute::computeMinNbOfNonSelfLoopsWithSymmetricAttributes(const vector<unsigned int>::const_reverse_iterator minSizeIt, const vector<float>::const_reverse_iterator minAreaDividedByProductOfSubsequentMinSizesIt, const vector<Attribute*>::const_iterator thisIt, const vector<Attribute*>::const_iterator attributeEnd, const unsigned int nbOfSymmetricElements, const unsigned int currentArea, unsigned int& minNbOfNonSelfLoops) const
{
  const vector<Attribute*>::const_iterator nextIt = thisIt + 1;
  if (nextIt == attributeEnd)
    {
      unsigned int factor = ceil(*minAreaDividedByProductOfSubsequentMinSizesIt / currentArea);
      if (factor <= irrelevantIndex)
	{
	  if (factor < *minSizeIt)
	    {
	      factor = *minSizeIt;
	    }
	  factor *= currentArea;
	  factor -= factor / nbOfSymmetricElements;
	  if (factor < minNbOfNonSelfLoops)
	    {
	      minNbOfNonSelfLoops = factor;
	    }
	}
      return;
    }
  const vector<unsigned int>::const_reverse_iterator nextMinSizesIt = minSizeIt + 1;
  const vector<float>::const_reverse_iterator nextMinAreaDividedByProductOfSubsequentMinSizesIt = minAreaDividedByProductOfSubsequentMinSizesIt + 1;
  const unsigned int end = min(static_cast<unsigned int>(ceil(*minAreaDividedByProductOfSubsequentMinSizesIt / currentArea)), irrelevantIndex);
  for (unsigned int factor = *minSizeIt; factor <= end; ++factor)
    {
      (*nextIt)->computeMinNbOfNonSelfLoopsWithSymmetricAttributes(nextMinSizesIt, nextMinAreaDividedByProductOfSubsequentMinSizesIt, nextIt, attributeEnd, nbOfSymmetricElements, factor * currentArea, minNbOfNonSelfLoops);
    }
}

void Attribute::computeMinNbOfNonSelfLoopsOnASymmetricValue(const vector<unsigned int>::const_reverse_iterator minSizeIt, const vector<float>::const_reverse_iterator minAreaDividedByProductOfSubsequentMinSizesIt, const vector<Attribute*>::const_iterator thisIt, const vector<Attribute*>::const_iterator attributeEnd, const unsigned int nbOfSymmetricElements, const unsigned int currentArea, unsigned int& minNbOfNonSelfLoopsOnAValue) const
{
  const vector<Attribute*>::const_iterator nextIt = thisIt + 1;
  if (nextIt == attributeEnd)
    {
      unsigned int factor = ceil(*minAreaDividedByProductOfSubsequentMinSizesIt / currentArea);
      if (factor <= irrelevantIndex)
	{
	  if (factor < *minSizeIt)
	    {
	      factor = *minSizeIt;
	    }
	  factor *= currentArea / nbOfSymmetricElements;
	  factor -= factor / nbOfSymmetricElements;
	  if (factor < minNbOfNonSelfLoopsOnAValue)
	    {
	      minNbOfNonSelfLoopsOnAValue = factor;
	    }
	}
      return;
    }
  const vector<unsigned int>::const_reverse_iterator nextMinSizesIt = minSizeIt + 1;
  const vector<float>::const_reverse_iterator nextMinAreaDividedByProductOfSubsequentMinSizesIt = minAreaDividedByProductOfSubsequentMinSizesIt + 1;
  const unsigned int end = min(static_cast<unsigned int>(ceil(*minAreaDividedByProductOfSubsequentMinSizesIt / currentArea)), irrelevantIndex);
  for (unsigned int factor = *minSizeIt; factor <= end; ++factor)
    {
      (*nextIt)->computeMinNbOfNonSelfLoopsOnASymmetricValue(nextMinSizesIt, nextMinAreaDividedByProductOfSubsequentMinSizesIt, nextIt, attributeEnd, nbOfSymmetricElements, factor * currentArea, minNbOfNonSelfLoopsOnAValue);
    }
}
#endif
#endif

bool Attribute::valueDoesNotExtendPresent(const Value& value, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  const vector<unsigned int>::const_iterator thisEpsilonIt = epsilonVector.begin() + id;
  if (value.getPresentNoise() > *thisEpsilonIt)
    {
      return true;
    }
  vector<Attribute*>::const_iterator attributeIt = attributeBegin;
  vector<unsigned int>::const_iterator epsilonIt = epsilonVector.begin() + (*attributeIt)->id;
  for (unsigned int intersectionIndex = id; epsilonIt != thisEpsilonIt && value.extendsPastPresent((*attributeIt)->presentBegin(), (*attributeIt)->presentEnd(), *epsilonIt, --intersectionIndex); ++epsilonIt)
    {
      ++attributeIt;
    }
  if (epsilonIt != thisEpsilonIt)
    {
      return true;
    }
  for (unsigned int reverseAttributeIndex = maxId - id; ++attributeIt != attributeEnd && value.extendsFuturePresent((*attributeIt)->presentBegin(), (*attributeIt)->presentEnd(), *(++epsilonIt), --reverseAttributeIndex); )
    {
    }
  return attributeIt != attributeEnd;
}

vector<unsigned int> Attribute::eraseIrrelevantValues()
{
  vector<unsigned int> dataIds;
  dataIds.reserve(absentIndex - irrelevantIndex);
  const vector<Value*>::iterator begin = values.begin() + irrelevantIndex;
  const vector<Value*>::iterator end = values.begin() + absentIndex;
  for (vector<Value*>::iterator irrelevantValueIt = begin; irrelevantValueIt != end; ++irrelevantValueIt)
    {
      dataIds.push_back((*irrelevantValueIt)->getDataId());
      delete *irrelevantValueIt;
    }
  if (id == orderedAttributeId)
    {
      sort(dataIds.begin(), dataIds.end());
    }
  values.erase(begin, end);
  absentIndex = irrelevantIndex;
  return dataIds;
}

bool Attribute::unclosed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  const vector<Value*>::const_iterator absentEnd = values.end();
  vector<Value*>::const_iterator absentValueIt = values.begin() + irrelevantIndex;
  for (; absentValueIt != absentEnd && valueDoesNotExtendPresentAndPotential(**absentValueIt, attributeBegin, attributeEnd); ++absentValueIt)
    {
    }
#ifdef DEBUG
  if (absentValueIt != absentEnd)
    {
      cout << labelsVector[id][(*absentValueIt)->getDataId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " extends any future pattern -> Prune!\n";
    }
#endif
  return absentValueIt != absentEnd;
}

bool Attribute::valueDoesNotExtendPresentAndPotential(const Value& value, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  const vector<unsigned int>::const_iterator thisEpsilonIt = epsilonVector.begin() + id;
  if (value.getPresentAndPotentialNoise() > *thisEpsilonIt)
    {
      return true;
    }
  vector<Attribute*>::const_iterator attributeIt = attributeBegin;
  vector<unsigned int>::const_iterator epsilonIt = epsilonVector.begin() + (*attributeIt)->id;
  for (unsigned int intersectionIndex = id; epsilonIt != thisEpsilonIt && value.extendsPastPresentAndPotential((*attributeIt)->presentBegin(), (*attributeIt)->potentialEnd(), *epsilonIt, --intersectionIndex); ++epsilonIt)
    {
      ++attributeIt;
    }
  if (epsilonIt != thisEpsilonIt)
    {
      return true;
    }
  for (unsigned int reverseAttributeIndex = maxId - id; ++attributeIt != attributeEnd && value.extendsFuturePresentAndPotential((*attributeIt)->presentBegin(), (*attributeIt)->potentialEnd(), *(++epsilonIt), --reverseAttributeIndex); )
    {
    }
  return attributeIt != attributeEnd;
}

void Attribute::cleanAndSortAbsent(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd)
{
  const vector<Value*>::iterator absentBegin = values.begin() + absentIndex;
  for (vector<Value*>::iterator absentValueIt = absentBegin; absentValueIt != values.end(); )
    {
      if (valueDoesNotExtendPresent(**absentValueIt, attributeBegin, attributeEnd))
	{
	  removeAbsentValue(absentValueIt);
	}
      else
	{
	  ++absentValueIt;
	}
    }
  sort(absentBegin, values.end(), Value::smallerDataId);
}

void Attribute::sortPotential()
{
  sort(values.begin() + potentialIndex, values.begin() + irrelevantIndex, Value::smallerDataId);
}

void Attribute::sortPotentialAndAbsentButChosenValue(const unsigned int presentAttributeId)
{
  if (id != orderedAttributeId || id == presentAttributeId) // else already sorted by sortPotentialAndAbsentButChosenPresentValueIfNecessary
    {
      const vector<Value*>::iterator absentBegin = values.begin() + absentIndex;
      sort(absentBegin, values.end(), Value::smallerDataId);
      if (isEnumeratedAttribute(presentAttributeId))
	{
	  sort(values.begin() + potentialIndex + 1, absentBegin, Value::smallerDataId);
	  return;
	}
      sort(values.begin() + potentialIndex, absentBegin, Value::smallerDataId);
    }
}

void Attribute::removeAbsentValue(vector<Value*>::iterator absentValueIt)
{
#ifdef DEBUG
  cout << labelsVector[id][(*absentValueIt)->getDataId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " will never extend any future pattern\n";
#endif
  delete *absentValueIt;
  *absentValueIt = values.back();
  values.pop_back();
}

bool Attribute::finalizable() const
{
  if (isClosedVector[id] || potentialIndex == irrelevantIndex)
    {
      const unsigned int epsilon = epsilonVector[id];
      const vector<Value*>::const_iterator end = values.begin() + irrelevantIndex;
      vector<Value*>::const_iterator valueIt = values.begin();
      for (; valueIt != end && (*valueIt)->getPresentAndPotentialNoise() <= epsilon; ++valueIt)
	{
	}
      return valueIt == end;
    }
  return false;
}

vector<unsigned int> Attribute::finalize() const
{
  vector<unsigned int> dataIdsOfValuesSetPresent;
  dataIdsOfValuesSetPresent.reserve(irrelevantIndex - potentialIndex);
  const vector<Value*>::const_iterator potentialEnd = values.begin() + irrelevantIndex;
  for (vector<Value*>::const_iterator potentialValueIt = values.begin() + potentialIndex; potentialValueIt != potentialEnd; ++potentialValueIt)
    {
      dataIdsOfValuesSetPresent.push_back((*potentialValueIt)->getDataId());
    }
  return dataIdsOfValuesSetPresent;
}

unsigned int Attribute::lastAttributeId()
{
  return maxId - 1;
}

const vector<unsigned int>& Attribute::getEpsilonVector()
{
  return epsilonVector;
}

unsigned int Attribute::getMinSize(const unsigned int attributeId)
{
  return minSizes[attributeId];
}

unsigned int Attribute::getMinArea()
{
  return minArea;
}

void Attribute::setMinAreaIsClosedVectorAndIsStorageAllDense(const unsigned int minAreaParam, const vector<bool>& isClosedVectorParam, const bool isStorageAllDense)
{
  unsigned int minAreaAccordingToMinSizes = 1;
  for (const unsigned int minSize : minSizes)
    {
      minAreaAccordingToMinSizes *= minSize;
    }
  if (minAreaAccordingToMinSizes < minAreaParam)
    {
      minArea = minAreaParam;
    }
  else
    {
      minArea = 0;
    }
  isClosedVector = isClosedVectorParam;
  if (isStorageAllDense)
    {
      orderedAttributeId = numeric_limits<unsigned int>::max();
      return;
    }
  orderedAttributeId = maxId - 1;
}

void Attribute::setIsAnAttributeSymmetric()
{
  isAnAttributeSymmetric = true;
}

void Attribute::setOutputFormat(const char* outputElementSeparatorParam, const char* emptySetStringParam, const char* elementNoiseSeparatorParam, const bool isNoisePrintedParam)
{
  outputElementSeparator = outputElementSeparatorParam;
  emptySetString = emptySetStringParam;
  elementNoiseSeparator = elementNoiseSeparatorParam;
  isNoisePrinted = isNoisePrintedParam;
}

void Attribute::printOutputElementSeparator(ostream& out)
{
  out << outputElementSeparator;
}

void Attribute::printEmptySetString(ostream& out)
{
  out << emptySetString;
}

bool Attribute::lessAppealingIrrelevant(const Attribute* attribute, const Attribute* otherAttribute)
{
  return (attribute->absentIndex - attribute->irrelevantIndex) * otherAttribute->values.size() < (otherAttribute->absentIndex - otherAttribute->irrelevantIndex) * attribute->values.size();
}
