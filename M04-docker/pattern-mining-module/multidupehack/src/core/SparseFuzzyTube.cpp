// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "SparseFuzzyTube.h"

unsigned int SparseFuzzyTube::sizeLimit;

SparseFuzzyTube::SparseFuzzyTube(): tube()
{
}

SparseFuzzyTube* SparseFuzzyTube::clone() const
{
  return new SparseFuzzyTube(*this);
}

void SparseFuzzyTube::print(vector<unsigned int>& prefix, ostream& out) const
{
  for (const pair<unsigned int, unsigned int>& hyperplane : tube)
    {
      printTuple(prefix, hyperplane.first, 1 - static_cast<double>(hyperplane.second) / Attribute::noisePerUnit, out);
    }
}

bool SparseFuzzyTube::isFullSparseTube() const
{
  return tube.size() == sizeLimit;
}

DenseFuzzyTube* SparseFuzzyTube::getDenseRepresentation() const
{
  return new DenseFuzzyTube(tube);
}

bool SparseFuzzyTube::setSelfLoopsInSymmetricAttributes(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const unsigned int dimensionId)
{
  // Necessarily symmetric
  if (sizeLimit == 0)
    {
      // *this must be turned into a DenseFuzzyTube
      return true;
    }
  tube.emplace_back(hyperplaneId, 0);
  return false;
}

void SparseFuzzyTube::setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<vector<unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt)
{
  const unsigned int element = oldIds2NewIdsIt->at(tuple[*attributeIdIt]);
  (*attributeIt)->subtractPotentialNoise(element, membership);
  tube.emplace_back(element, Attribute::noisePerUnit - membership);
}

void SparseFuzzyTube::sortTubes()
{
  tube.shrink_to_fit();
  sort(tube.begin(), tube.end(), [](const pair<unsigned int, unsigned int>& entry1, const pair<unsigned int, unsigned int>& entry2) {return entry1.first < entry2.first;});
}

unsigned int SparseFuzzyTube::noiseOnValue(const unsigned int valueDataId) const
{
  const vector<pair<unsigned int, unsigned int>>::const_iterator tubeEnd = tube.end();
  const vector<pair<unsigned int, unsigned int>>::const_iterator noisyTupleIt = lower_bound(tube.begin(), tubeEnd, valueDataId, [](const pair<unsigned int, unsigned int>& entry, const unsigned int id) {return entry.first < id;});
  if (noisyTupleIt != tubeEnd && noisyTupleIt->first == valueDataId)
    {
      return noisyTupleIt->second;
    }
  return Attribute::noisePerUnit;
}

unsigned int SparseFuzzyTube::noiseOnValues(const vector<unsigned int>& valueDataIds) const
{
  unsigned int oldNoise = 0;
  const vector<pair<unsigned int, unsigned int>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, unsigned int>>::const_iterator tubeBegin = tube.begin();
  const vector<unsigned int>::const_iterator valueDataIdEnd = valueDataIds.end();
  for (vector<unsigned int>::const_iterator valueDataIdIt = valueDataIds.begin(); valueDataIdIt != valueDataIdEnd; ++valueDataIdIt)
    {
      tubeBegin = lower_bound(tubeBegin, tubeEnd, *valueDataIdIt, [](const pair<unsigned int, unsigned int>& entry, const unsigned int id) {return entry.first < id;});
      if (tubeBegin == tubeEnd)
	{
	  return (valueDataIdEnd - valueDataIdIt) * Attribute::noisePerUnit + oldNoise;
	}
      if (tubeBegin->first == *valueDataIdIt)
	{
	  oldNoise += tubeBegin->second;
	}
      else
	{
	  oldNoise += Attribute::noisePerUnit;
	}
    }
  return oldNoise;
}

unsigned int SparseFuzzyTube::setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  // *this necessarily relates to the present attribute
  const Value& presentValue = (*attributeIt)->getChosenValue();
  const unsigned int noise = noiseOnValue(presentValue.getDataId());
  (*potentialOrAbsentValueIntersectionIt)[presentValue.getPresentIntersectionId()] += noise;
  return noise;
}

unsigned int SparseFuzzyTube::presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute) const
{
  unsigned int newNoise = 0;
  const vector<pair<unsigned int, unsigned int>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, unsigned int>>::const_iterator tubeBegin = tube.begin();
  const vector<Value*>::iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId, [](const pair<unsigned int, unsigned int>& entry, const unsigned int id) {return entry.first < id;});
      if (tubeBegin == tubeEnd)
	{
	  newNoise += (end - valueIt) * Attribute::noisePerUnit;
	  do
	    {
	      (*valueIt)->addPresentNoise(Attribute::noisePerUnit);
	    } while (++valueIt != end);
	  return newNoise;
	}
      if (tubeBegin->first == valueDataId)
	{
	  (*valueIt)->addPresentNoise(tubeBegin->second);
	  newNoise += tubeBegin->second;
	}
      else
	{
	  (*valueIt)->addPresentNoise(Attribute::noisePerUnit);
	  newNoise += Attribute::noisePerUnit;
	}
    }
  return newNoise;
}

unsigned int SparseFuzzyTube::presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(const Attribute& currentAttribute, const vector<unsigned int>::iterator potentialOrAbsentValueIntersectionBegin) const
{
  unsigned int newNoise = 0;
  vector<unsigned int>::iterator potentialOrAbsentValueIntersectionIt = potentialOrAbsentValueIntersectionBegin;
  const vector<pair<unsigned int, unsigned int>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, unsigned int>>::const_iterator tubeBegin = tube.begin();
  const vector<Value*>::const_iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::const_iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId, [](const pair<unsigned int, unsigned int>& entry, const unsigned int id) {return entry.first < id;});
      if (tubeBegin == tubeEnd)
	{
	  newNoise += (end - valueIt) * Attribute::noisePerUnit;
	  do
	    {
	      *potentialOrAbsentValueIntersectionIt++ += Attribute::noisePerUnit;
	    } while (++valueIt != end);
	  return newNoise;
	}
      if (tubeBegin->first == valueDataId)
	{
	  *potentialOrAbsentValueIntersectionIt++ += tubeBegin->second;
	  newNoise += tubeBegin->second;
	}
      else
	{
	  *potentialOrAbsentValueIntersectionIt++ += Attribute::noisePerUnit;
	  newNoise += Attribute::noisePerUnit;
	}
    }
  return newNoise;
}

void SparseFuzzyTube::presentFixPotentialOrAbsentValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Value*>::iterator absentBegin = currentAttribute.absentBegin();
  presentFixPotentialOrAbsentValuesAfterPresentValueMet(currentAttribute.potentialBegin(), absentBegin, 0, intersectionIts);
  presentFixPotentialOrAbsentValuesAfterPresentValueMet(absentBegin, currentAttribute.absentEnd(), currentAttribute.sizeOfPotential(), intersectionIts);
}

void SparseFuzzyTube::presentFixPotentialOrAbsentValuesInSecondSymmetricAttributeAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Value*>::iterator absentBegin = currentAttribute.absentBegin();
  presentFixPotentialOrAbsentValuesAfterPresentValueMet(currentAttribute.potentialBegin() + 1, absentBegin, 0, intersectionIts);
  presentFixPotentialOrAbsentValuesAfterPresentValueMet(absentBegin, currentAttribute.absentEnd(), currentAttribute.sizeOfPotential() - 1, intersectionIts);
}

void SparseFuzzyTube::presentFixPotentialOrAbsentValuesAfterPresentValueMet(const vector<Value*>::iterator begin, const vector<Value*>::iterator end, const unsigned int firstValueId, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int valueId = firstValueId;
  const vector<pair<unsigned int, unsigned int>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, unsigned int>>::const_iterator tubeBegin = tube.begin();
  for (vector<Value*>::iterator valueIt = begin; valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId, [](const pair<unsigned int, unsigned int>& entry, const unsigned int id) {return entry.first < id;});
      if (tubeBegin == tubeEnd)
	{
	  do
	    {
	      (*valueIt)->addPresentNoise(Attribute::noisePerUnit);
	      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
		{
		  (*intersectionIt)[valueId] += Attribute::noisePerUnit;
		}
	      ++valueId;
	    } while (++valueIt != end);
	  return;
	}
      if (tubeBegin->first == valueDataId)
	{
	  (*valueIt)->addPresentNoise(tubeBegin->second);
	  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	    {
	      (*intersectionIt)[valueId] += tubeBegin->second;
	    }
	}
      else
	{
	  (*valueIt)->addPresentNoise(Attribute::noisePerUnit);
	  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	    {
	      (*intersectionIt)[valueId] += Attribute::noisePerUnit;
	    }
	}
      ++valueId;
    }
}

unsigned int SparseFuzzyTube::absentFixPresentOrPotentialValuesAfterAbsentValueMet(Attribute& currentAttribute, vector<unsigned int>& absentValueIntersection) const
{
  const vector<Value*>::iterator potentialBegin = currentAttribute.potentialBegin();
  return absentFixPresentOrPotentialValuesAfterAbsentValueMet(currentAttribute.presentBegin(), potentialBegin, absentValueIntersection) + absentFixPresentOrPotentialValuesAfterAbsentValueMet(potentialBegin, currentAttribute.potentialEnd(), absentValueIntersection) + noiseOnIrrelevant(currentAttribute);
}

unsigned int SparseFuzzyTube::absentFixPresentOrPotentialValuesAfterAbsentValueMet(const vector<Value*>::iterator begin, const vector<Value*>::iterator end, vector<unsigned int>& absentValueIntersection) const
{
  unsigned int oldNoise = 0;
  const vector<pair<unsigned int, unsigned int>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, unsigned int>>::const_iterator tubeBegin = tube.begin();
  for (vector<Value*>::iterator valueIt = begin; valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId, [](const pair<unsigned int, unsigned int>& entry, const unsigned int id) {return entry.first < id;});
      if (tubeBegin == tubeEnd)
	{
	  oldNoise += (end - valueIt) * Attribute::noisePerUnit;
	  do
	    {
	      (*valueIt)->subtractPotentialNoise(Attribute::noisePerUnit);
	      absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()] += Attribute::noisePerUnit;
	    } while (++valueIt != end);
	  return oldNoise;
	}
      if (tubeBegin->first == valueDataId)
      	{
      	  (*valueIt)->subtractPotentialNoise(tubeBegin->second);
	  absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()] += tubeBegin->second;
      	  oldNoise += tubeBegin->second;
      	}
      else
      	{
      	  (*valueIt)->subtractPotentialNoise(Attribute::noisePerUnit);
	  absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()] += Attribute::noisePerUnit;
      	  oldNoise += Attribute::noisePerUnit;
      	}
    }
  return oldNoise;
}

unsigned int SparseFuzzyTube::irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(Attribute& currentAttribute) const
{
  const vector<Value*>::iterator potentialBegin = currentAttribute.potentialBegin();
  return irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(currentAttribute.presentBegin(), potentialBegin) + irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(potentialBegin, currentAttribute.potentialEnd()) + noiseOnIrrelevant(currentAttribute);
}

unsigned int SparseFuzzyTube::irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(const vector<Value*>::iterator begin, const vector<Value*>::iterator end) const
{
  unsigned int oldNoise = 0;
  const vector<pair<unsigned int, unsigned int>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, unsigned int>>::const_iterator tubeBegin = tube.begin();
  for (vector<Value*>::iterator valueIt = begin; valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId, [](const pair<unsigned int, unsigned int>& entry, const unsigned int id) {return entry.first < id;});
      if (tubeBegin == tubeEnd)
	{
	  oldNoise += (end - valueIt) * Attribute::noisePerUnit;
	  do
	    {
	      (*valueIt)->subtractPotentialNoise(Attribute::noisePerUnit);
	    } while (++valueIt != end);
	  return oldNoise;
	}
      if (tubeBegin->first == valueDataId)
      	{
      	  (*valueIt)->subtractPotentialNoise(tubeBegin->second);
      	  oldNoise += tubeBegin->second;
      	}
      else
      	{
      	  (*valueIt)->subtractPotentialNoise(Attribute::noisePerUnit);
      	  oldNoise += Attribute::noisePerUnit;
      	}
    }
  return oldNoise;
}

unsigned int SparseFuzzyTube::noiseOnIrrelevant(const Attribute& currentAttribute) const
{
  unsigned int oldNoise = 0;
  const vector<pair<unsigned int, unsigned int>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, unsigned int>>::const_iterator tubeBegin = tube.begin();
  vector<Value*>::const_iterator end = currentAttribute.irrelevantEnd();
  for (vector<Value*>::const_iterator valueIt = currentAttribute.irrelevantBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId, [](const pair<unsigned int, unsigned int>& entry, const unsigned int id) {return entry.first < id;});
      if (tubeBegin == tubeEnd)
	{
	  return (end - valueIt) * Attribute::noisePerUnit + oldNoise;
	}
      if (tubeBegin->first == valueDataId)
	{
	  oldNoise += tubeBegin->second;
	}
      else
	{
	  oldNoise += Attribute::noisePerUnit;
	}
    }
  return oldNoise;
}

unsigned int SparseFuzzyTube::absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(Attribute& currentAttribute, vector<unsigned int>& absentValueIntersection) const
{
  const vector<Value*>::const_iterator potentialBegin = currentAttribute.potentialBegin();
  return absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(currentAttribute.presentBegin(), potentialBegin, absentValueIntersection) + absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(potentialBegin, currentAttribute.potentialEnd(), absentValueIntersection) + noiseOnIrrelevant(currentAttribute);
}

unsigned int SparseFuzzyTube::absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(const vector<Value*>::const_iterator begin, const vector<Value*>::const_iterator end, vector<unsigned int>& absentValueIntersection) const
{
  unsigned int oldNoise = 0;
  const vector<pair<unsigned int, unsigned int>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, unsigned int>>::const_iterator tubeBegin = tube.begin();
  for (vector<Value*>::const_iterator valueIt = begin; valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId, [](const pair<unsigned int, unsigned int>& entry, const unsigned int id) {return entry.first < id;});
      if (tubeBegin == tubeEnd)
	{
	  oldNoise += (end - valueIt) * Attribute::noisePerUnit;
	  do
	    {
	      absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()] -= Attribute::noisePerUnit;
	    } while (++valueIt != end);
	  return oldNoise;
	}
      if (tubeBegin->first == valueDataId)
	{
	  absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()] -= tubeBegin->second;
	  oldNoise += tubeBegin->second;
	}
      else
	{
	  absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()] -= Attribute::noisePerUnit;
	  oldNoise += Attribute::noisePerUnit;
	}
    }
  return oldNoise;
}

void SparseFuzzyTube::absentFixAbsentValuesAfterAbsentValueMet(const vector<Value*>::iterator absentBegin, const vector<Value*>::iterator absentEnd, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<pair<unsigned int, unsigned int>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, unsigned int>>::const_iterator tubeBegin = tube.begin();
  for (vector<Value*>::iterator valueIt = absentBegin; valueIt != absentEnd; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId, [](const pair<unsigned int, unsigned int>& entry, const unsigned int id) {return entry.first < id;});
      if (tubeBegin == tubeEnd)
	{
	  do
	    {
	      (*valueIt)->subtractPotentialNoise(Attribute::noisePerUnit);
	      const unsigned int valueId = (*valueIt)->getPresentAndPotentialIntersectionId();
	      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
		{
		  (*intersectionIt)[valueId] -= Attribute::noisePerUnit;
		}
	    } while (++valueIt != absentEnd);
	  return;
	}
      const unsigned int valueId = (*valueIt)->getPresentAndPotentialIntersectionId();
      if (tubeBegin->first == valueDataId)
	{
	  (*valueIt)->subtractPotentialNoise(tubeBegin->second);
	  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	    {
	      (*intersectionIt)[valueId] -= tubeBegin->second;
	    }
	}
      else
	{
	  (*valueIt)->subtractPotentialNoise(Attribute::noisePerUnit);
	  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	    {
	      (*intersectionIt)[valueId] -= Attribute::noisePerUnit;
	    }
	}
    }
}

unsigned int SparseFuzzyTube::noiseSum(const vector<vector<unsigned int>>::const_iterator dimensionIt) const
{
  unsigned int noise = 0;
  const vector<pair<unsigned int, unsigned int>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, unsigned int>>::const_iterator tubeBegin = tube.begin();
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  for (vector<unsigned int>::const_iterator idIt = dimensionIt->begin(); idIt != idEnd; ++idIt)
    {
      tubeBegin = lower_bound(tubeBegin, tubeEnd, *idIt, [](const pair<unsigned int, unsigned int>& entry, const unsigned int id) {return entry.first < id;});
      if (tubeBegin == tubeEnd)
	{
	  return (idEnd - idIt) * Attribute::noisePerUnit + noise;
	}
      if (tubeBegin->first == *idIt)
	{
	  noise += tubeBegin->second;
	}
      else
	{
	  noise += Attribute::noisePerUnit;
	}
    }
  return noise;
}

void SparseFuzzyTube::setSizeLimit(const unsigned int sizeLimitParam)
{
  sizeLimit = sizeLimitParam;
}

#ifdef ASSERT
unsigned int SparseFuzzyTube::noiseSumOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      return noiseOnValue(value.getDataId());
    }
  unsigned int noise = 0;
  const vector<Value*>::const_iterator end = (*attributeIt)->presentEnd();
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != end; ++valueIt)
    {
      noise += noiseOnValue((*valueIt)->getDataId());
    }
  return noise;
}

unsigned int SparseFuzzyTube::noiseSumOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      return noiseOnValue(value.getDataId());
    }
  unsigned int noise = 0;
  const vector<Value*>::const_iterator end = (*attributeIt)->irrelevantEnd();
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != end; ++valueIt)
    {
      noise += noiseOnValue((*valueIt)->getDataId());
    }
  return noise;
}
#endif
