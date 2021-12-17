// Copyright (C) 2013-2020 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "MetricAttribute.h"

vector<double> MetricAttribute::tauVector;
vector<vector<double>> MetricAttribute::timestampsVector;

MetricAttribute::MetricAttribute(const vector<unsigned int>& nbOfValuesPerAttribute, const double epsilon, const unsigned int minSize, const vector<string>& labels, const double tau): Attribute(nbOfValuesPerAttribute, epsilon, minSize, labels)
{
  tauVector.resize(id + 1);
  tauVector.back() = tau;
  timestampsVector.resize(id + 1);
  vector<double>& timestampsOfThisAttribute = timestampsVector.back();
  timestampsOfThisAttribute.reserve(labels.size());
  for (const string& label : labels)
    {
      timestampsOfThisAttribute.push_back(lexical_cast<double>(label));
    }
}

MetricAttribute::MetricAttribute(const MetricAttribute& parentAttribute): Attribute(parentAttribute)
{
}

MetricAttribute* MetricAttribute::clone() const
{
  return new MetricAttribute(*this);
}

void MetricAttribute::sortPotentialAndAbsentButChosenPresentValueIfNecessary(const unsigned int presentAttributeId) const
{
  // already sorted
}

void MetricAttribute::sortPotentialIrrelevantAndAbsentButChosenAbsentValueIfNecessary(const unsigned int absentAttributeId) const
{
  // already sorted
}

void MetricAttribute::sortPotentialIrrelevantAndAbsentIfNecessary(const unsigned int absentAttributeId)
{
  // potential and absent values are already sorted
  if (id == orderedAttributeId && id != absentAttributeId)
    {
      sort(values.begin() + irrelevantIndex, values.begin() + absentIndex, Value::smallerDataId);
    }
}

bool MetricAttribute::metric() const
{
  return true;
}

void MetricAttribute::chooseValue(const unsigned int indexOfValue)
{
  const vector<Value*>::iterator valueIt = values.begin() + indexOfValue;
  rotate(values.begin() + potentialIndex, valueIt, valueIt + 1);
}

void MetricAttribute::repositionChosenPresentValue()
{
  repositionChosenPresentValueInOrderedAttribute();
}

vector<Value*>::iterator MetricAttribute::repositionChosenValueAndGetIt()
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
      return valueIt;
    }
  rotate(valuesBegin, valueIt, valueIt + 1);
  return valuesBegin;
}

vector<unsigned int> MetricAttribute::setChosenValueIrrelevant() // returns an empty vector if the tau-contiguity is violated, the dataId of the value set absent and of the tau-far irrelevant values otherwise
{
  const unsigned int minPresentDataId = values.front()->getDataId();
  if (!potentialIndex)
    {
      rotate(values.begin(), values.begin() + 1, values.begin() + irrelevantIndex--);
      unsigned int potentialValueId = values.front()->getDataId();
      if (potentialValueId > minPresentDataId)
	{
	  eraseAbsentValuesBeneathTimestamp(timestampsVector[id][potentialValueId] - tauVector[id]);
	  return {minPresentDataId};
	}
      potentialValueId = values[irrelevantIndex - 1]->getDataId();
      if (potentialValueId < minPresentDataId)
	{
	  eraseAbsentValuesBeyondTimestamp(timestampsVector[id][potentialValueId] + tauVector[id]);
	}
      return {minPresentDataId};
    }
  vector<Value*>::iterator potentialBegin = values.begin() + potentialIndex;
  const Value* absentValue = *potentialBegin;
  const unsigned int absentValueDataId = (*potentialBegin)->getDataId();
  const double tau = tauVector[id];
  const vector<double>& timestamps = timestampsVector[id];
  const vector<Value*>::iterator potentialEnd = values.begin() + irrelevantIndex;
  vector<Value*>::iterator potentialBorderIt = lower_bound(++potentialBegin, potentialEnd, absentValue, Value::smallerDataId);
  if (minPresentDataId > absentValueDataId)
    {
      // absentValue is beneath the present range
      double scopeTimestamp;
      if (potentialBorderIt != potentialEnd && (*potentialBorderIt)->getDataId() < minPresentDataId)
	{
	  scopeTimestamp = timestamps[(*potentialBorderIt)->getDataId()] - tau;
	}
      else
	{
	  scopeTimestamp = timestamps[minPresentDataId] - tau;
	}
      if (potentialBorderIt == potentialBegin)
	{
	  eraseAbsentValuesBeneathTimestamp(scopeTimestamp);
	  rotate(potentialBegin - 1, potentialBegin, potentialEnd);
	  --irrelevantIndex;
	  return {absentValueDataId};
	}
      if (scopeTimestamp > timestamps[(*(potentialBorderIt - 1))->getDataId()])
	{
#ifdef DEBUG
	  cout << "In the " << tau << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every potential value beneath " << scopeTimestamp << " is irrelevant\n";
#endif
	  // Potential values with too small timestamps are irrelevant
	  eraseAbsentValuesBeneathTimestamp(scopeTimestamp);
	  rotate(potentialBegin - 1, potentialBegin, potentialBorderIt);
	  vector<unsigned int> irrelevantValueDataIds;
	  irrelevantValueDataIds.reserve(potentialBorderIt - --potentialBegin);
	  for (vector<Value*>::const_iterator tauFarPotentialValueIt = potentialBegin; tauFarPotentialValueIt != potentialBorderIt; ++tauFarPotentialValueIt)
	    {
	      irrelevantValueDataIds.push_back((*tauFarPotentialValueIt)->getDataId());
	    }
	  rotate(potentialBegin, potentialBorderIt, potentialEnd);
	  irrelevantIndex -= potentialBorderIt - potentialBegin;
	  return irrelevantValueDataIds;
	}
      rotate(potentialBegin - 1, potentialBegin, potentialEnd);
      --irrelevantIndex;
      return {absentValueDataId};
    }
  const unsigned int maxPresentDataId = (*(potentialBegin - 2))->getDataId();
  if (maxPresentDataId < absentValueDataId)
    {
      // absentValue is beyond the present range
      double scopeTimestamp;
      if (potentialBorderIt == potentialBegin || (*(potentialBorderIt - 1))->getDataId() < maxPresentDataId)
	{
	  scopeTimestamp = timestamps[maxPresentDataId] + tau;
	}
      else
	{
	  scopeTimestamp = timestamps[(*(potentialBorderIt - 1))->getDataId()] + tau;
	}
      if (potentialBorderIt == potentialEnd)
	{
	  eraseAbsentValuesBeyondTimestamp(scopeTimestamp);
	  rotate(potentialBegin - 1, potentialBegin, potentialEnd);
	  --irrelevantIndex;
	  return {absentValueDataId};
	}
      if (scopeTimestamp < timestamps[(*potentialBorderIt)->getDataId()])
	{
#ifdef DEBUG
	  cout << "In the " << tau << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every potential value beyond " << scopeTimestamp << " is irrelevant\n";
#endif
	  // Potential values with too great timestamps are irrelevant
	  eraseAbsentValuesBeyondTimestamp(scopeTimestamp);
	  rotate(potentialBegin - 1, potentialBegin, potentialEnd);
	  irrelevantIndex -= potentialEnd - --potentialBorderIt;
	  vector<unsigned int> irrelevantValueDataIds;
	  irrelevantValueDataIds.reserve(potentialEnd - potentialBorderIt + 1);
	  for (; potentialBorderIt != potentialEnd; ++potentialBorderIt)
	    {
	      irrelevantValueDataIds.push_back((*potentialBorderIt)->getDataId());
	    }
	  return irrelevantValueDataIds;
	}
      rotate(potentialBegin - 1, potentialBegin, potentialEnd);
      --irrelevantIndex;
      return {absentValueDataId};
    }
  // absentValue is inside the present range
  vector<Value*>::iterator presentBorderIt = lower_bound(values.begin(), potentialBegin - 1, absentValue, Value::smallerDataId);
  double previousTimestamp;
  if (potentialBorderIt == potentialBegin)
    {
      previousTimestamp = timestamps[(*(presentBorderIt - 1))->getDataId()];
    }
  else
    {
      previousTimestamp = timestamps[max((*(presentBorderIt - 1))->getDataId(), (*(potentialBorderIt - 1))->getDataId())];
    }
  double nextTimestamp;
  if (potentialBorderIt == potentialEnd)
    {
      nextTimestamp = timestamps[(*presentBorderIt)->getDataId()];
    }
  else
    {
      nextTimestamp = timestamps[min((*presentBorderIt)->getDataId(), (*potentialBorderIt)->getDataId())];
    }
  if (previousTimestamp + tau < nextTimestamp)
    {
#ifdef DEBUG
      cout << tau << "-contiguity constraint on attribute " << internal2ExternalAttributeOrder[id] << " not verified -> Prune!\n";
#endif
      return {};
    }
  rotate(potentialBegin - 1, potentialBegin, potentialEnd);
  --irrelevantIndex;
  return {absentValueDataId};
}

void MetricAttribute::repositionChosenAbsentValue()
{
  vector<Value*>::iterator valueIt = values.begin() + absentIndex;
  Value* chosenValue = *valueIt;
  const unsigned int chosenValueDataId = chosenValue->getDataId();
  if (values.back()->getDataId() > chosenValueDataId)
    {
      for (vector<Value*>::iterator nextValueIt = valueIt; (*++nextValueIt)->getDataId() < chosenValueDataId; )
	{
	  *valueIt++ = *nextValueIt;
	}
      *valueIt = chosenValue;
      return;
    }
  rotate(valueIt, valueIt + 1, values.end());
}

// PERF: decrease of the absent intervals on which binary searches are performed (use of the last iterator returned as a new bound)
bool MetricAttribute::findIrrelevantValuesAndCheckTauContiguity(const vector<Attribute*>::iterator attributeBegin, const vector<Attribute*>::iterator attributeEnd)
{
  if (potentialIndex == irrelevantIndex)
    {
      return false;
    }
  if (!potentialIndex)
    {
      Attribute::findIrrelevantValuesAndCheckTauContiguity(attributeBegin, attributeEnd);
      sort(values.begin(), values.begin() + irrelevantIndex, Value::smallerDataId);
      return false;
    }
  vector<Value*>::iterator chosenValueIt = repositionChosenValueAndGetIt(); // If *this was chosen, the chosen value currently is at the end of present values
  const double tau = tauVector[id];
  const vector<double>& timestamps = timestampsVector[id];
  const vector<Value*>::iterator potentialBegin = values.begin() + potentialIndex;
  vector<Value*>::iterator potentialEnd = values.begin() + irrelevantIndex;
  vector<Value*>::iterator lowerPotentialBorderIt = lower_bound(potentialBegin, potentialEnd, values.front(), Value::smallerDataId);
  vector<Value*>::iterator upperPotentialBorderIt = lower_bound(potentialBegin, potentialEnd, *(potentialBegin - 1), Value::smallerDataId);
  if (lowerPotentialBorderIt != potentialEnd)
    {
      // There are some potential values that are greater than the smallest present value
      // Values between the lowest and the greatest present timestamps
      vector<Value*>::const_iterator presentIt = values.begin();
      double presentTimestamp = timestamps[(*presentIt)->getDataId()];
      double scopeTimestamp = presentTimestamp;
      for (vector<Value*>::iterator potentialValueIt = lowerPotentialBorderIt; potentialValueIt != upperPotentialBorderIt; )
	{
	  // Increase scopeTimestamp w.r.t. present timestamps within scope
	  while (presentTimestamp <= scopeTimestamp && presentIt != potentialBegin)
	    {
	      scopeTimestamp = presentTimestamp + tau;
	      if (++presentIt != potentialBegin)
		{
		  presentTimestamp = timestamps[(*presentIt)->getDataId()];
		}
	    }
	  if (valueDoesNotExtendPresent(**potentialValueIt, attributeBegin, attributeEnd))
	    {
	      if (potentialValueIt == --upperPotentialBorderIt && presentIt != potentialBegin)
		{
#ifdef DEBUG
		  cout << tau << "-contiguity constraint on attribute " << internal2ExternalAttributeOrder[id] << " not verified -> Prune!\n";
#endif
		  return true;
		}
#ifdef DEBUG
	      cout << labelsVector[id][(*potentialValueIt)->getDataId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " will never be present nor extend any future pattern\n";
#endif
	      rotate(potentialValueIt, potentialValueIt + 1, potentialEnd--);
	      --irrelevantIndex;
	    }
	  else
	    {
	      // Check tau-contiguity
	      const double potentialTimestamp = timestamps[(*potentialValueIt)->getDataId()];
	      if (potentialTimestamp > scopeTimestamp)
		{
#ifdef DEBUG
		  cout << tau << "-contiguity constraint on attribute " << internal2ExternalAttributeOrder[id] << " not verified -> Prune!\n";
#endif
		  return true;
		}
	      // Increase (if possible) scopeTimestamp w.r.t. potentialTimestamp
	      const double scopePotentialTimestamp = potentialTimestamp + tau;
	      if (scopePotentialTimestamp > scopeTimestamp)
		{
		  scopeTimestamp = scopePotentialTimestamp;
		}
	      ++potentialValueIt;
	    }
	}
    }
  // Values beyond the greatest present timestamp
  if (upperPotentialBorderIt != potentialEnd)
    {
      // There are some potential values that are greater than the largest present value
      double scopeTimestamp = timestamps[(*(potentialBegin - 1))->getDataId()] + tau;
      while (upperPotentialBorderIt != potentialEnd)
	{
	  // Check tau-contiguity
	  const double nextTimestamp = timestamps[(*upperPotentialBorderIt)->getDataId()];
	  if (nextTimestamp > scopeTimestamp)
	    {
#ifdef DEBUG
	      cout << "In the " << tau << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every potential value beyond " << scopeTimestamp << " is irrelevant\n";
#endif
	      irrelevantIndex -= potentialEnd - upperPotentialBorderIt;
	      potentialEnd = upperPotentialBorderIt;
	      break;
	    }
	  if (valueDoesNotExtendPresent(**upperPotentialBorderIt, attributeBegin, attributeEnd))
	    {
#ifdef DEBUG
	      cout << labelsVector[id][(*upperPotentialBorderIt)->getDataId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " will never be present nor extend any future pattern\n";
#endif
	      rotate(upperPotentialBorderIt, upperPotentialBorderIt + 1, potentialEnd--);
	      --irrelevantIndex;
	    }
	  else
	    {
	      scopeTimestamp = nextTimestamp + tau;
	      ++upperPotentialBorderIt;
	    }
	}
      eraseAbsentValuesBeyondTimestamp(scopeTimestamp);
    }
  // Values beneath the lowest present timestamp
  if (lowerPotentialBorderIt != potentialBegin)
    {
      // There are some potential values that are lower than the smallest present value
      double scopeTimestamp = timestamps[values.front()->getDataId()] - tau;
      do
	{
	  // Check tau-contiguity
	  const double nextTimestamp = timestamps[(*(--lowerPotentialBorderIt))->getDataId()];
	  if (nextTimestamp < scopeTimestamp)
	    {
#ifdef DEBUG
	      cout << "In the " << tau << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every potential value beneath " << scopeTimestamp << " is irrelevant\n";
#endif
	      rotate(potentialBegin, ++lowerPotentialBorderIt, potentialEnd);
	      irrelevantIndex -= lowerPotentialBorderIt - potentialBegin;
	      break;
	    }
	  if (valueDoesNotExtendPresent(**lowerPotentialBorderIt, attributeBegin, attributeEnd))
	    {
#ifdef DEBUG
	      cout << labelsVector[id][(*lowerPotentialBorderIt)->getDataId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " will never be present nor extend of any future pattern\n";
#endif
	      rotate(lowerPotentialBorderIt, lowerPotentialBorderIt + 1, potentialEnd--);
	      --irrelevantIndex;
	    }
	  else
	    {
	      scopeTimestamp = nextTimestamp - tau;
	    }
	} while (lowerPotentialBorderIt != potentialBegin);
      eraseAbsentValuesBeneathTimestamp(scopeTimestamp);
    }
  if (chosenValueIt != potentialBegin)
    {
      // Put the chosen value back to where it was for Attribute::setPresentAndPotentialIntersections
      rotate(chosenValueIt, chosenValueIt + 1, potentialBegin);
    }
  return false;
}

#ifdef MIN_SIZE_ELEMENT_PRUNING
pair<bool, vector<unsigned int>> MetricAttribute::findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity()
{
  if (potentialIndex == irrelevantIndex)
    {
      return {false, {}};
    }
  if (!potentialIndex)
    {
      const pair<bool, vector<unsigned int>> newIrrelevantValueDataIds = Attribute::findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity();
      sort(values.begin() + potentialIndex, values.begin() + irrelevantIndex, Value::smallerDataId);
      return newIrrelevantValueDataIds;
    }
  vector<unsigned int> newIrrelevantValueDataIds;
  const double tau = tauVector[id];
  const vector<double>& timestamps = timestampsVector[id];
  const vector<Value*>::iterator potentialBegin = values.begin() + potentialIndex;
  vector<Value*>::iterator potentialEnd = values.begin() + irrelevantIndex;
  vector<Value*>::iterator lowerPotentialBorderIt = lower_bound(potentialBegin, potentialEnd, values.front(), Value::smallerDataId);
  vector<Value*>::iterator upperPotentialBorderIt = lower_bound(potentialBegin, potentialEnd, *(potentialBegin - 1), Value::smallerDataId);
  if (lowerPotentialBorderIt != potentialEnd)
    {
      // There are some potential values that are greater than the smallest present value
      // Values between the lowest and the greatest present timestamps
      vector<Value*>::const_iterator presentIt = values.begin();
      double presentTimestamp = timestamps[(*presentIt)->getDataId()];
      double scopeTimestamp = presentTimestamp;
      for (vector<Value*>::iterator potentialValueIt = lowerPotentialBorderIt; potentialValueIt != upperPotentialBorderIt; )
	{
	  // Increase scopeTimestamp w.r.t. present timestamps within scope
	  while (presentTimestamp <= scopeTimestamp && presentIt != potentialBegin)
	    {
	      scopeTimestamp = presentTimestamp + tau;
	      if (++presentIt != potentialBegin)
		{
		  presentTimestamp = timestamps[(*presentIt)->getDataId()];
		}
	    }
	  if (presentAndPotentialIrrelevantValue(**potentialValueIt))
	    {
	      newIrrelevantValueDataIds.push_back((*potentialValueIt)->getDataId());
	      if (potentialValueIt == --upperPotentialBorderIt && presentIt != potentialBegin)
		{
#ifdef DEBUG
		  cout << tau << "-contiguity constraint on attribute " << internal2ExternalAttributeOrder[id] << " not verified -> Prune!\n";
#endif
		  return {true, newIrrelevantValueDataIds};
		}
	      rotate(potentialValueIt, potentialValueIt + 1, potentialEnd--);
	      --irrelevantIndex;
	    }
	  else
	    {
	      // Check tau-contiguity
	      const double potentialTimestamp = timestamps[(*potentialValueIt)->getDataId()];
	      if (potentialTimestamp > scopeTimestamp)
		{
#ifdef DEBUG
		  cout << tau << "-contiguity constraint on attribute " << internal2ExternalAttributeOrder[id] << " not verified -> Prune!\n";
#endif
		  return {true, newIrrelevantValueDataIds};
		}
	      // Increase (if possible) scopeTimestamp w.r.t. potentialTimestamp
	      const double scopePotentialTimestamp = potentialTimestamp + tau;
	      if (scopePotentialTimestamp > scopeTimestamp)
		{
		  scopeTimestamp = scopePotentialTimestamp;
		}
	      ++potentialValueIt;
	    }
	}
    }
  // Values beyond the greatest present timestamp
  if (upperPotentialBorderIt != potentialEnd)
    {
      // There are some potential values that are greater than the largest present value
      double scopeTimestamp = timestamps[(*(potentialBegin - 1))->getDataId()] + tau;
      while (upperPotentialBorderIt != potentialEnd)
	{
	  // Check tau-contiguity
	  const double nextTimestamp = timestamps[(*upperPotentialBorderIt)->getDataId()];
	  if (nextTimestamp > scopeTimestamp)
	    {
#ifdef DEBUG
	      cout << "In the " << tau << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every potential value beyond " << scopeTimestamp << " is irrelevant\n";
#endif
	      irrelevantIndex -= potentialEnd - upperPotentialBorderIt;
	      while (upperPotentialBorderIt != potentialEnd)
	      	{
	      	  newIrrelevantValueDataIds.push_back((*--potentialEnd)->getDataId());
	      	}
	      break;
	    }
	  if (presentAndPotentialIrrelevantValue(**upperPotentialBorderIt))
	    {
	      newIrrelevantValueDataIds.push_back((*upperPotentialBorderIt)->getDataId());
	      rotate(upperPotentialBorderIt, upperPotentialBorderIt + 1, potentialEnd--);
	      --irrelevantIndex;
	    }
	  else
	    {
	      scopeTimestamp = nextTimestamp + tau;
	      ++upperPotentialBorderIt;
	    }
	}
      eraseAbsentValuesBeyondTimestamp(scopeTimestamp);
    }
  // Values beneath the lowest present timestamp
  if (lowerPotentialBorderIt != potentialBegin)
    {
      // There are some potential values that are lower than the smallest present value
      double scopeTimestamp = timestamps[values.front()->getDataId()] - tau;
      do
	{
	  // Check tau-contiguity
	  const double nextTimestamp = timestamps[(*(--lowerPotentialBorderIt))->getDataId()];
	  if (nextTimestamp < scopeTimestamp)
	    {
#ifdef DEBUG
	      cout << "In the " << tau << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every potential value beneath " << scopeTimestamp << " is irrelevant\n";
#endif
	      irrelevantIndex -= ++lowerPotentialBorderIt - potentialBegin;
	      for (vector<Value*>::iterator potentialValueIt = potentialBegin; potentialValueIt != lowerPotentialBorderIt; ++potentialValueIt)
	      	{
	      	  newIrrelevantValueDataIds.push_back((*potentialValueIt)->getDataId());
	      	}
	      rotate(potentialBegin, lowerPotentialBorderIt, potentialEnd);
	      break;
	    }
	  if (presentAndPotentialIrrelevantValue(**lowerPotentialBorderIt))
	    {
	      newIrrelevantValueDataIds.push_back((*lowerPotentialBorderIt)->getDataId());
	      rotate(lowerPotentialBorderIt, lowerPotentialBorderIt + 1, potentialEnd--);
	      --irrelevantIndex;
	    }
	  else
	    {
	      scopeTimestamp = nextTimestamp - tau;
	    }
	} while (lowerPotentialBorderIt != potentialBegin);
      eraseAbsentValuesBeneathTimestamp(scopeTimestamp);
    }
  return {false, newIrrelevantValueDataIds};
}

void MetricAttribute::sortAbsent()
{
  sort(values.begin() + absentIndex, values.end(), Value::smallerDataId);
}
#endif

void MetricAttribute::eraseAbsentValuesBeneathTimestamp(const double timestamp)
{
#ifdef DEBUG
  cout << "In the " << tauVector[id] << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every absent value beneath " << timestamp << " cannot extend any future pattern\n";
#endif
  const vector<double>& timestamps = timestampsVector[id];
  vector<Value*>::iterator valueIt = values.begin() + absentIndex;
  const vector<Value*>::iterator absentBegin = valueIt;
  for (; valueIt != values.end() && timestamps[(*valueIt)->getDataId()] < timestamp; ++valueIt)
    {
      delete *valueIt;
    }
  values.erase(absentBegin, valueIt);
}

void MetricAttribute::eraseAbsentValuesBeyondTimestamp(const double timestamp)
{
#ifdef DEBUG
  cout << "In the " << tauVector[id] << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every absent value beyond " << timestamp << " cannot extend any future pattern\n";
#endif
  if (absentIndex != values.size())
    {
      const vector<double>& timestamps = timestampsVector[id];
      vector<Value*>::iterator absentBegin = values.begin() + absentIndex;
      if (timestamps[(*absentBegin)->getDataId()] > timestamp)
	{
	  for (vector<Value*>::iterator valueIt = absentBegin; valueIt != values.end(); ++valueIt)
	    {
	      delete *valueIt;
	    }
	  values.resize(absentIndex);
	  return;
	}
      vector<Value*>::iterator valueIt = --(values.end());
      for (; timestamps[(*valueIt)->getDataId()] > timestamp; --valueIt)
	{
	  delete *valueIt;
	}
      values.erase(valueIt + 1, values.end());
    }
}

bool MetricAttribute::unclosed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  if (!potentialIndex || irrelevantIndex == values.size())
    {
      return false;
    }
  const double tau = tauVector[id];
  const Value* presentBack = values[potentialIndex - 1];
  const vector<double>& timestamps = timestampsVector[id];
  double scopeTimestamp = timestamps[presentBack->getDataId()] + tau;
  const vector<Value*>::const_iterator absentBegin = values.begin() + irrelevantIndex;
  const vector<Value*>::const_iterator end = values.end();
  vector<Value*>::const_iterator tauCloseAbsentEnd = lower_bound(absentBegin, end, presentBack, Value::smallerDataId);
  for (; tauCloseAbsentEnd != end && timestamps[(*tauCloseAbsentEnd)->getDataId()] <= scopeTimestamp; ++tauCloseAbsentEnd)
    {
    }
  scopeTimestamp = timestamps[values.front()->getDataId()] - tau;
  vector<Value*>::const_iterator tauCloseAbsentBegin;
  if (timestamps[(*absentBegin)->getDataId()] < scopeTimestamp)
    {
      tauCloseAbsentBegin = lower_bound(absentBegin, end, values.front(), Value::smallerDataId);
      while (timestamps[(*(--tauCloseAbsentBegin))->getDataId()] >= scopeTimestamp)
	{
	}
      ++tauCloseAbsentBegin;
    }
  else
    {
      tauCloseAbsentBegin = absentBegin;
    }
  for (; tauCloseAbsentBegin != tauCloseAbsentEnd && valueDoesNotExtendPresentAndPotential(**tauCloseAbsentBegin, attributeBegin, attributeEnd); ++tauCloseAbsentBegin)
    {
    }
#ifdef DEBUG
  if (tauCloseAbsentBegin != tauCloseAbsentEnd)
    {
      cout << labelsVector[id][(*tauCloseAbsentBegin)->getDataId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " extends any future pattern -> Prune!\n";
    }
#endif
  return tauCloseAbsentBegin != tauCloseAbsentEnd;
}

bool MetricAttribute::globallyUnclosed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  return Attribute::unclosed(attributeBegin, attributeEnd);
}

void MetricAttribute::sortPotentialAndAbsentButChosenValue(const unsigned int presentAttributeId) const
{
  // Already sorted
}

bool MetricAttribute::finalizable() const
{
  return (potentialIndex || potentialIndex == irrelevantIndex) && Attribute::finalizable();
}

vector<unsigned int> MetricAttribute::finalize()
{
  vector<unsigned int> dataIdsOfValuesSetPresent;
  dataIdsOfValuesSetPresent.reserve(irrelevantIndex - potentialIndex);
  // WARNING: present and potential must be ordered
  const vector<Value*>::iterator begin = values.begin() + potentialIndex;
  const vector<Value*>::iterator end = values.begin() + irrelevantIndex;
  for (vector<Value*>::iterator valueIt = begin; valueIt != end; ++valueIt)
    {
      dataIdsOfValuesSetPresent.push_back((*valueIt)->getDataId());
    }
  inplace_merge(values.begin(), begin, end, Value::smallerDataId);
  return dataIdsOfValuesSetPresent;
}
