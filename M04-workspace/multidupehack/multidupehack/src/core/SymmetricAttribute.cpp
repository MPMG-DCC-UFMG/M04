// Copyright (C) 2013-2020 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "SymmetricAttribute.h"

SymmetricAttribute::SymmetricAttribute(): Attribute(), symmetricAttribute(nullptr)
{
}

SymmetricAttribute::SymmetricAttribute(const vector<unsigned int>& nbOfValuesPerAttribute, const double epsilon, const unsigned int minSize, const vector<string>& labels): Attribute(nbOfValuesPerAttribute, epsilon, minSize, labels), symmetricAttribute(nullptr)
{
}

SymmetricAttribute::SymmetricAttribute(const SymmetricAttribute& parentAttribute): Attribute(parentAttribute), symmetricAttribute(nullptr)
{
}

SymmetricAttribute* SymmetricAttribute::clone() const
{
  return new SymmetricAttribute(*this);
}

void SymmetricAttribute::sortPotentialAndAbsentButChosenPresentValueIfNecessary(const unsigned int presentAttributeId)
{
  if (id == orderedAttributeId)
    {
      if (id == presentAttributeId + 1)
	{
	  Attribute::sortPotentialAndAbsent(potentialIndex + 1);
	  symmetricAttribute->Attribute::sortPotentialAndAbsent(potentialIndex + 1);
	  return;
	}
      Attribute::sortPotentialAndAbsent(potentialIndex);
      symmetricAttribute->Attribute::sortPotentialAndAbsent(potentialIndex);
    }
}

void SymmetricAttribute::sortPotentialIrrelevantAndAbsentButChosenAbsentValueIfNecessary(const unsigned int absentAttributeId)
{
  if (id == orderedAttributeId)
    {
      if (id == absentAttributeId + 1)
	{
	  Attribute::sortPotentialIrrelevantAndAbsent(absentIndex + 1);
	  symmetricAttribute->Attribute::sortPotentialIrrelevantAndAbsent(absentIndex + 1);
	  return;
	}
      Attribute::sortPotentialIrrelevantAndAbsent(absentIndex);
      symmetricAttribute->Attribute::sortPotentialIrrelevantAndAbsent(absentIndex);
    }
}

void SymmetricAttribute::sortPotentialIrrelevantAndAbsentIfNecessary(const unsigned int absentAttributeId)
{
  if (id == orderedAttributeId && id != absentAttributeId)
    {
      Attribute::sortPotentialIrrelevantAndAbsent(absentIndex);
      symmetricAttribute->Attribute::sortPotentialIrrelevantAndAbsent(symmetricAttribute->absentIndex);
    }
}

bool SymmetricAttribute::symmetric() const
{
  return true;
}

unsigned int SymmetricAttribute::getSymmetricAttributeId() const
{
  return symmetricAttribute->id;
}

SymmetricAttribute* SymmetricAttribute::thisOrFirstSymmetricAttribute()
{
  if (id < symmetricAttribute->id)
    {
      return this;
    }
  return symmetricAttribute;
}

bool SymmetricAttribute::isEnumeratedAttribute(const unsigned int enumeratedAttributeId) const
{
  return enumeratedAttributeId == id || enumeratedAttributeId == symmetricAttribute->id;
}

void SymmetricAttribute::setSymmetricAttribute(SymmetricAttribute* symmetricAttributeParam)
{
  symmetricAttribute = symmetricAttributeParam;
}

void SymmetricAttribute::subtractSelfLoopsFromPotentialNoise(const unsigned int totalMembershipDueToSelfLoopsOnASymmetricValue, const unsigned int nbOfSymmetricElements)
{
  for (Value* value : values)
    {
      value->subtractPotentialNoise(totalMembershipDueToSelfLoopsOnASymmetricValue);
    }
}

void SymmetricAttribute::repositionChosenPresentValue()
{
  // The chosen symmetric Attribute cannot be the last one (given chooseValue)
  if (symmetricAttribute->id == orderedAttributeId)
    {
      repositionChosenPresentValueInOrderedAttribute();
      symmetricAttribute->repositionChosenPresentValueInOrderedAttribute();
    }
}

void SymmetricAttribute::setChosenValuePresent()
{
  ++potentialIndex;
  ++(symmetricAttribute->potentialIndex);
}

vector<unsigned int> SymmetricAttribute::setChosenValueIrrelevant(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd)
{
  swap(symmetricAttribute->values[potentialIndex], symmetricAttribute->values[--(symmetricAttribute->irrelevantIndex)]);
  Value*& chosenAbsentValue = values[potentialIndex];
  const unsigned int chosenAbsentValueDataId = chosenAbsentValue->getDataId();
  swap(chosenAbsentValue, values[--irrelevantIndex]);
#ifdef MIN_SIZE_ELEMENT_PRUNING
  if (!minArea)
    {
      return {chosenAbsentValueDataId};
    }
  bool isAbsentToBeCleaned = false;
  vector<unsigned int> irrelevantValueDataIds = {chosenAbsentValueDataId};
  while (possiblePresentAndPotentialIrrelevancy(attributeBegin, attributeEnd))
    {
      symmetricAttribute->presentAndPotentialIrrelevancyThreshold = presentAndPotentialIrrelevancyThreshold;
      if (presentAndPotentialIrrelevant() || symmetricAttribute->presentAndPotentialIrrelevant())
	{
          return {};
        }
      isAbsentToBeCleaned = true;
      vector<unsigned int> newIrrelevantValueDataIds = findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity().second;
      const bool isNoIrrelevantValueFound = newIrrelevantValueDataIds.empty();
      if (!isNoIrrelevantValueFound)
	{
	  irrelevantValueDataIds.insert(irrelevantValueDataIds.end(), newIrrelevantValueDataIds.begin(), newIrrelevantValueDataIds.end());
	}
      newIrrelevantValueDataIds = symmetricAttribute->findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity().second;
      if (newIrrelevantValueDataIds.empty())
	{
	  if (isNoIrrelevantValueFound)
	    {
	      break;		// presentAndPotentialIrrelevancyThreshold will not change
	    }
	}
      else
	{
	  irrelevantValueDataIds.insert(irrelevantValueDataIds.end(), newIrrelevantValueDataIds.begin(), newIrrelevantValueDataIds.end());
	}
    }
  if (isAbsentToBeCleaned)
    {
      const unsigned int threshold = minSizeIrrelevancyThresholdIgnoringSymmetry(attributeBegin, attributeEnd); // different from presentAndPotentialIrrelevancyThreshold, because there is no self loop in the present and potential hyperplane related to an absent value
      presentAndPotentialCleanAbsent(threshold);
      symmetricAttribute->presentAndPotentialCleanAbsent(threshold);
    }
  return irrelevantValueDataIds;
#endif
  return {chosenAbsentValueDataId};
}

void SymmetricAttribute::setLastIrrelevantValueChosen(const vector<Attribute*>::const_iterator thisAttributeIt, const vector<Attribute*>::const_iterator attributeEnd)
{
  Attribute::setLastIrrelevantValueChosen(thisAttributeIt, attributeEnd);
  symmetricAttribute->Attribute::setLastIrrelevantValueChosen(thisAttributeIt + 1, attributeEnd);
}

unsigned int SymmetricAttribute::getIndexOfValueToChoose(const double presentCoeff, const double presentAndPotentialCoeff, double& bestAppeal) const
{
  vector<Value*>::const_iterator symmetricPotentialValueIt = symmetricAttribute->potentialBegin();
  const vector<Value*>::const_iterator end = values.begin() + irrelevantIndex;
  vector<Value*>::const_iterator valueToChooseIt = values.begin() + potentialIndex;
  for (vector<Value*>::const_iterator potentialValueIt = valueToChooseIt; potentialValueIt != end; ++potentialValueIt)
    {
      const double appealOfValue = presentCoeff * ((*potentialValueIt)->getPresentNoise() + (*symmetricPotentialValueIt)->getPresentNoise()) + presentAndPotentialCoeff * ((*potentialValueIt)->getPresentAndPotentialNoise() + (*symmetricPotentialValueIt)->getPresentAndPotentialNoise());
      if (appealOfValue > bestAppeal)
	{
#if defined VERBOSE_ELEMENT_CHOICE && ENUMERATION_PROCESS == 1
	  cout << "Appeal of symmetric attributes: " << appealOfValue / noisePerUnit << " with element " << labelsVector[id][(*potentialValueIt)->getDataId()] << '\n';
#endif
	  bestAppeal = appealOfValue;
	  valueToChooseIt = potentialValueIt;
	}
      ++symmetricPotentialValueIt;
    }
  return valueToChooseIt - values.begin();
}

unsigned int SymmetricAttribute::getIndexOfValueToChoose(const vector<Attribute*>& attributes, const Attribute* firstSymmetricAttribute, double& bestAppeal) const
{
  if (firstSymmetricAttribute == this)
    {
      unsigned int testedArea = 0;
      unsigned int presentArea = 1;
      unsigned int presentAndPotentialArea = 1;
      const vector<Attribute*>::const_iterator attributeEnd = attributes.end();
      const vector<Attribute*>::const_iterator attributeBegin = attributes.begin();
      for (vector<Attribute*>::const_iterator potentialAttributeIt = attributeBegin; potentialAttributeIt != attributeEnd; ++potentialAttributeIt)
	{
	  presentArea *= (*potentialAttributeIt)->sizeOfPresent();
	  unsigned int yesFactor;
	  if (*potentialAttributeIt == this)
	    {
	      presentAndPotentialArea *= irrelevantIndex - 1;
	      yesFactor = (irrelevantIndex - potentialIndex - 1);
	      ++potentialAttributeIt;
	    }
	  else
	    {
	      presentAndPotentialArea *= (*potentialAttributeIt)->sizeOfPresentAndPotential();
	      yesFactor = (*potentialAttributeIt)->sizeOfPotential();
	    }
	  for (const Attribute* presentAttribute : attributes)
	    {
	      if (presentAttribute != this && presentAttribute != *potentialAttributeIt)
		{
		  yesFactor *= presentAttribute->sizeOfPresent();
		}
	    }
	  testedArea += yesFactor;
	}
      if (!testedArea)
	{
#if ENUMERATION_PROCESS == 1
	  if (!presentArea)
	    {
#endif
#ifdef TWO_MODE_ELEMENT_CHOICE

	      return getIndexOfValueToChoose(0, 1, bestAppeal);

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
      testedArea *= 2;
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
      if (static_cast<double>(testedArea + (maxId - 1) * presentArea) * noisePerUnit * 2 > bestAppeal)
	{
	  const double presentAndPotentialCoeff = static_cast<double>(testedArea) / (presentAndPotentialArea - presentArea);
	  return getIndexOfValueToChoose(-presentAndPotentialCoeff + maxId - 1, presentAndPotentialCoeff, bestAppeal);
	}
#if defined VERBOSE_ELEMENT_CHOICE && ENUMERATION_PROCESS == 1
      else
	{
	  cout << "Appeal of symmetric attributes cannot be higher than " << bestAppeal / noisePerUnit << '\n';
	}
#endif
  // the following return is executed when no epsilon can be reached and every potential value has no noise on present and potential (appeal == bestAppeal == 0)
#endif
    }
  return potentialIndex;
}

void SymmetricAttribute::chooseValue(const unsigned int indexOfValue)
{
  swap(values[potentialIndex], values[indexOfValue]);
  iter_swap(symmetricAttribute->potentialBegin(), symmetricAttribute->presentBegin() + indexOfValue);
}

bool SymmetricAttribute::findIrrelevantValuesAndCheckTauContiguity(const vector<Attribute*>::iterator attributeBegin, const vector<Attribute*>::iterator attributeEnd)
{
  if (id < symmetricAttribute->id)
    {
      vector<Value*>::iterator symmetricPotentialValueIt = symmetricAttribute->potentialBegin();
      vector<Value*>::iterator potentialEnd = values.begin() + irrelevantIndex;
      for (vector<Value*>::iterator potentialValueIt = values.begin() + potentialIndex; potentialValueIt != potentialEnd; )
	{
	  if (symmetricValuesDoNotExtendPresent(**potentialValueIt, **symmetricPotentialValueIt, attributeBegin, attributeEnd))
	    {
#ifdef DEBUG
	      cout << labelsVector[id][(*potentialValueIt)->getDataId()] << " in symmetric attributes will never be present nor extend any future pattern\n";
#endif
	      iter_swap(potentialValueIt, --potentialEnd);
	      --irrelevantIndex;
	      symmetricAttribute->setPotentialValueIrrelevant(symmetricPotentialValueIt);
	    }
	  else
	    {
	      ++potentialValueIt;
	      ++symmetricPotentialValueIt;
	    }
	}
    }
  return false;
}

#ifdef MIN_SIZE_ELEMENT_PRUNING
pair<bool, vector<unsigned int>> SymmetricAttribute::findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity()
{
  vector<unsigned int> newIrrelevantValueDataIds;
  const vector<Value*>::iterator symmetricPotentialBegin = symmetricAttribute->potentialBegin();
  vector<Value*>::iterator symmetricPotentialValueIt = symmetricPotentialBegin;
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
	  symmetricAttribute->setPotentialValueIrrelevant(symmetricPotentialValueIt);
	}
      else
	{
	  ++potentialValueIt;
	  ++symmetricPotentialValueIt;
	}
    }
  return {false, newIrrelevantValueDataIds};
}

void SymmetricAttribute::presentAndPotentialCleanAbsent(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd)
{
  presentAndPotentialCleanAbsent(minSizeIrrelevancyThresholdIgnoringSymmetry(attributeBegin, attributeEnd));
}

void SymmetricAttribute::presentAndPotentialCleanAbsent(const unsigned int threshold)
{
  vector<Value*>::iterator symmetricAbsentValueIt = symmetricAttribute->absentBegin();
  for (vector<Value*>::iterator absentValueIt = values.begin() + absentIndex; absentValueIt != values.end(); )
    {
      if ((*absentValueIt)->getPresentAndPotentialNoise() > threshold)
	{
	  removeAbsentValue(absentValueIt);
	  symmetricAttribute->removeAbsentValue(symmetricAbsentValueIt);
	}
      else
	{
	  ++absentValueIt;
	  ++symmetricAbsentValueIt;
	}
    }
}

#ifdef MIN_AREA_REFINEMENT
void SymmetricAttribute::computeMinNbOfNonSelfLoopsWithSymmetricAttributes(const vector<unsigned int>::const_reverse_iterator minSizeIt, const vector<float>::const_reverse_iterator minAreaDividedByProductOfSubsequentMinSizesIt, const vector<Attribute*>::const_iterator thisIt, const vector<Attribute*>::const_iterator attributeEnd, const unsigned int nbOfSymmetricElements, const unsigned int currentArea, unsigned int& minNbOfNonSelfLoops) const
{
  const vector<Attribute*>::const_iterator nextIt = thisIt + 2;
  if (nextIt == attributeEnd)
    {
      unsigned int factor = ceil(sqrt(*minAreaDividedByProductOfSubsequentMinSizesIt / currentArea));
      if (factor <= irrelevantIndex)
	{
	  if (factor < *minSizeIt)
	    {
	      factor = *minSizeIt;
	    }
	  factor *= (factor - 1) * currentArea;
	  if (factor < minNbOfNonSelfLoops)
	    {
	      minNbOfNonSelfLoops = factor;
	    }
	}
      return;
    }
  const vector<unsigned int>::const_reverse_iterator nextMinSizesIt = minSizeIt + 2;
  const vector<float>::const_reverse_iterator nextMinAreaDividedByProductOfSubsequentMinSizesIt = minAreaDividedByProductOfSubsequentMinSizesIt + 2;
  const unsigned int end = min(static_cast<unsigned int>(ceil(sqrt(*(minAreaDividedByProductOfSubsequentMinSizesIt + 1) / currentArea))), irrelevantIndex);
  for (unsigned int factor = *minSizeIt; factor <= end; ++factor)
    {
      (*nextIt)->computeMinNbOfNonSelfLoopsWithSymmetricAttributes(nextMinSizesIt, nextMinAreaDividedByProductOfSubsequentMinSizesIt, nextIt, attributeEnd, factor, factor * factor * currentArea, minNbOfNonSelfLoops);
    }
}

void SymmetricAttribute::computeMinNbOfNonSelfLoopsOnASymmetricValue(const vector<unsigned int>::const_reverse_iterator minSizeIt, const vector<float>::const_reverse_iterator minAreaDividedByProductOfSubsequentMinSizesIt, const vector<Attribute*>::const_iterator thisIt, const vector<Attribute*>::const_iterator attributeEnd, const unsigned int nbOfSymmetricElements, const unsigned int currentArea, unsigned int& minNbOfNonSelfLoopsOnAValue) const
{
  const vector<Attribute*>::const_iterator nextIt = thisIt + 1;
  if (nextIt == attributeEnd)
    {
      unsigned int factor = ceil(sqrt(*minAreaDividedByProductOfSubsequentMinSizesIt / currentArea));
      if (factor <= irrelevantIndex)
	{
	  if (factor < *minSizeIt)
	    {
	      factor = *minSizeIt;
	    }
	  --factor;
	  factor *= currentArea;
	  if (factor < minNbOfNonSelfLoopsOnAValue)
	    {
	      minNbOfNonSelfLoopsOnAValue = factor;
	    }
	}
      return;
    }
  const vector<unsigned int>::const_reverse_iterator nextMinSizesIt = minSizeIt + 1;
  const vector<float>::const_reverse_iterator nextMinAreaDividedByProductOfSubsequentMinSizesIt = minAreaDividedByProductOfSubsequentMinSizesIt + 1;
  const unsigned int end = min(static_cast<unsigned int>(ceil(sqrt(*minAreaDividedByProductOfSubsequentMinSizesIt / currentArea))), irrelevantIndex);
  for (unsigned int factor = *minSizeIt; factor <= end; ++factor)
    {
      (*nextIt)->computeMinNbOfNonSelfLoopsOnASymmetricValue(nextMinSizesIt, nextMinAreaDividedByProductOfSubsequentMinSizesIt, nextIt, attributeEnd, factor, factor * factor * currentArea, minNbOfNonSelfLoopsOnAValue);
    }
}
#endif
#endif

bool SymmetricAttribute::symmetricValuesDoNotExtendPresent(const Value& value, const Value& symmetricValue, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  const vector<unsigned int>::const_iterator thisEpsilonIt = epsilonVector.begin() + id;
  if (value.getPresentNoise() > *thisEpsilonIt || symmetricValue.getPresentNoise() > *(thisEpsilonIt + 1))
    {
      return true;
    }
  vector<Attribute*>::const_iterator attributeIt = attributeBegin;
  vector<unsigned int>::const_iterator epsilonIt = epsilonVector.begin();
  for (unsigned int intersectionIndex = id; epsilonIt != thisEpsilonIt && value.symmetricValuesExtendPastPresent((*attributeIt)->presentBegin(), (*attributeIt)->presentEnd(), *epsilonIt, --intersectionIndex); ++epsilonIt)
    {
      ++attributeIt;
    }
  unsigned int reverseAttributeIndex = maxId - id - 1;
  if (!(epsilonIt == thisEpsilonIt && symmetricValue.extendsPastPresent(values.begin(), values.begin() + potentialIndex, *epsilonIt, 0) && value.extendsFuturePresent(symmetricAttribute->presentBegin(), symmetricAttribute->presentEnd(), *++epsilonIt, reverseAttributeIndex)))
    {
      return true;
    }
  for (attributeIt += 2; attributeIt != attributeEnd && value.symmetricValuesExtendFuturePresent(symmetricValue, (*attributeIt)->presentBegin(), (*attributeIt)->presentEnd(), *++epsilonIt, reverseAttributeIndex--); ++attributeIt)
    {
    }
  return attributeIt != attributeEnd;
}

bool SymmetricAttribute::unclosed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  if (id > symmetricAttribute->id)
    {
      return false;
    }
  vector<Value*>::const_iterator symmetricAbsentValueIt = symmetricAttribute->absentBegin();
  const vector<Value*>::const_iterator absentEnd = values.end();
  vector<Value*>::const_iterator absentValueIt = values.begin() + irrelevantIndex;
  for (; absentValueIt != absentEnd && symmetricValuesDoNotExtendPresentAndPotential(**absentValueIt, **symmetricAbsentValueIt, attributeBegin, attributeEnd); ++absentValueIt)
    {
      ++symmetricAbsentValueIt;
    }
#ifdef DEBUG
  if (absentValueIt != absentEnd)
    {
      cout << labelsVector[id][(*absentValueIt)->getDataId()] << " in symmetric attributes extends the pattern -> Prune!\n";
    }
#endif
  return absentValueIt != absentEnd;
}

bool SymmetricAttribute::symmetricValuesDoNotExtendPresentAndPotential(const Value& value, const Value& symmetricValue, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  const vector<unsigned int>::const_iterator thisEpsilonIt = epsilonVector.begin() + id;
  if (value.getPresentAndPotentialNoise() > *thisEpsilonIt || symmetricValue.getPresentAndPotentialNoise() > *(thisEpsilonIt + 1))
    {
      return true;
    }
  vector<Attribute*>::const_iterator attributeIt = attributeBegin;
  vector<unsigned int>::const_iterator epsilonIt = epsilonVector.begin();
  for (unsigned int intersectionIndex = id; epsilonIt != thisEpsilonIt && value.symmetricValuesExtendPastPresentAndPotential((*attributeIt)->presentBegin(), (*attributeIt)->potentialEnd(), *epsilonIt, --intersectionIndex); ++epsilonIt)
    {
      ++attributeIt;
    }
  unsigned int reverseAttributeIndex = maxId - id - 1;
  if (!(epsilonIt == thisEpsilonIt && symmetricValue.extendsPastPresentAndPotential(values.begin(), values.begin() + irrelevantIndex, *epsilonIt, 0) && value.extendsFuturePresentAndPotential(symmetricAttribute->presentBegin(), symmetricAttribute->potentialEnd(), *++epsilonIt, reverseAttributeIndex)))
    {
      return true;
    }
  for (attributeIt += 2; attributeIt != attributeEnd && value.symmetricValuesExtendFuturePresentAndPotential(symmetricValue, (*attributeIt)->presentBegin(), (*attributeIt)->potentialEnd(), *++epsilonIt, reverseAttributeIndex--); ++attributeIt)
    {
    }
  return attributeIt != attributeEnd;
}

void SymmetricAttribute::cleanAndSortAbsent(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd)
{
  if (id < symmetricAttribute->id)
    {
      vector<Value*>::iterator symmetricAbsentValueIt = symmetricAttribute->absentBegin();
      const vector<Value*>::iterator absentBegin = values.begin() + absentIndex;
      for (vector<Value*>::iterator absentValueIt = absentBegin; absentValueIt != values.end(); )
	{
	  if (symmetricValuesDoNotExtendPresent(**absentValueIt, **symmetricAbsentValueIt, attributeBegin, attributeEnd))
	    {
	      removeAbsentValue(absentValueIt);
	      symmetricAttribute->removeAbsentValue(symmetricAbsentValueIt);
	    }
	  else
	    {
	      ++absentValueIt;
	      ++symmetricAbsentValueIt;
	    }
	}
      sort(absentBegin, values.end(), Value::smallerDataId);
      sort(symmetricAttribute->absentBegin(), symmetricAttribute->absentEnd(), Value::smallerDataId);
    }
}
