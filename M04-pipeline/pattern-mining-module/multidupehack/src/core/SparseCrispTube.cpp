// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "SparseCrispTube.h"

unsigned int SparseCrispTube::sizeLimit;

SparseCrispTube::SparseCrispTube(): tube()
{
}

SparseCrispTube* SparseCrispTube::clone() const
{
  return new SparseCrispTube(*this);
}

void SparseCrispTube::print(vector<unsigned int>& prefix, ostream& out) const
{
  for (const unsigned int hyperplane : tube)
    {
      printTuple(prefix, hyperplane, 1, out);
    }
}

bool SparseCrispTube::isFullSparseTube() const
{
  return tube.size() == sizeLimit;
}

DenseCrispTube* SparseCrispTube::getDenseRepresentation() const
{
  return new DenseCrispTube(tube);
}

bool SparseCrispTube::setSelfLoopsInSymmetricAttributes(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const unsigned int dimensionId)
{
  // Necessarily symmetric
  if (sizeLimit == 0)
    {
      // *this must be turned into a DenseCrispTube
      return true;
    }
  tube.push_back(hyperplaneId);
  return false;
}

void SparseCrispTube::setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<vector<unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt)
{
  const unsigned int element = oldIds2NewIdsIt->at(tuple[*attributeIdIt]);
  (*attributeIt)->decrementPotentialNoise(element);
  tube.push_back(element);
}

void SparseCrispTube::sortTubes()
{
  tube.shrink_to_fit();
  sort(tube.begin(), tube.end());
}

unsigned int SparseCrispTube::noiseOnValue(const unsigned int valueDataId) const
{
  if (binary_search(tube.begin(), tube.end(), valueDataId))
    {
      return 0;
    }
  return 1;
}

unsigned int SparseCrispTube::noiseOnValues(const vector<unsigned int>& valueDataIds) const
{
  unsigned int oldNoise = 0;
  const vector<unsigned int>::const_iterator tubeEnd = tube.end();
  vector<unsigned int>::const_iterator tubeBegin = tube.begin();
  const vector<unsigned int>::const_iterator valueDataIdEnd = valueDataIds.end();
  for (vector<unsigned int>::const_iterator valueDataIdIt = valueDataIds.begin(); valueDataIdIt != valueDataIdEnd; ++valueDataIdIt)
    {
      tubeBegin = lower_bound(tubeBegin, tubeEnd, *valueDataIdIt);
      if (tubeBegin == tubeEnd)
	{
	  return valueDataIdEnd - valueDataIdIt + oldNoise;
	}
      if (*tubeBegin != *valueDataIdIt)
	{
	  ++oldNoise;
	}
    }
  return oldNoise;
}

unsigned int SparseCrispTube::setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  // *this necessarily relates to the present attribute
  const Value& presentValue = (*attributeIt)->getChosenValue();
  if (binary_search(tube.begin(), tube.end(), presentValue.getDataId()))
    {
      return 0;
    }
  ++(*potentialOrAbsentValueIntersectionIt)[presentValue.getPresentIntersectionId()];
  return 1;
}

unsigned int SparseCrispTube::presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute) const
{
  unsigned int newNoise = 0;
  const vector<unsigned int>::const_iterator tubeEnd = tube.end();
  vector<unsigned int>::const_iterator tubeBegin = tube.begin();
  const vector<Value*>::iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId);
      if (tubeBegin == tubeEnd)
	{
	  newNoise += end - valueIt;
	  do
	    {
	      (*valueIt)->incrementPresentNoise();
	    } while (++valueIt != end);
	  return newNoise;
	}
      if (*tubeBegin != valueDataId)
	{
	  (*valueIt)->incrementPresentNoise();
	  ++newNoise;
	}
    }
  return newNoise;
}

unsigned int SparseCrispTube::presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(const Attribute& currentAttribute, const vector<unsigned int>::iterator potentialOrAbsentValueIntersectionBegin) const
{
  unsigned int newNoise = 0;
  vector<unsigned int>::iterator potentialOrAbsentValueIntersectionIt = potentialOrAbsentValueIntersectionBegin;
  const vector<unsigned int>::const_iterator tubeEnd = tube.end();
  vector<unsigned int>::const_iterator tubeBegin = tube.begin();
  const vector<Value*>::const_iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::const_iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId);
      if (tubeBegin == tubeEnd)
	{
	  newNoise += end - valueIt;
	  do
	    {
	      ++*potentialOrAbsentValueIntersectionIt;
	      ++potentialOrAbsentValueIntersectionIt;
	    } while (++valueIt != end);
	  return newNoise;
	}
      if (*tubeBegin != valueDataId)
	{
	  ++*potentialOrAbsentValueIntersectionIt;
	  ++newNoise;
	}
      ++potentialOrAbsentValueIntersectionIt;
    }
  return newNoise;
}

void SparseCrispTube::presentFixPotentialOrAbsentValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Value*>::iterator absentBegin = currentAttribute.absentBegin();
  presentFixPotentialOrAbsentValuesAfterPresentValueMet(currentAttribute.potentialBegin(), absentBegin, 0, intersectionIts);
  presentFixPotentialOrAbsentValuesAfterPresentValueMet(absentBegin, currentAttribute.absentEnd(), currentAttribute.sizeOfPotential(), intersectionIts);
}

void SparseCrispTube::presentFixPotentialOrAbsentValuesInSecondSymmetricAttributeAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Value*>::iterator absentBegin = currentAttribute.absentBegin();
  presentFixPotentialOrAbsentValuesAfterPresentValueMet(currentAttribute.potentialBegin() + 1, absentBegin, 0, intersectionIts);
  presentFixPotentialOrAbsentValuesAfterPresentValueMet(absentBegin, currentAttribute.absentEnd(), currentAttribute.sizeOfPotential() - 1, intersectionIts);
}

void SparseCrispTube::presentFixPotentialOrAbsentValuesAfterPresentValueMet(const vector<Value*>::iterator begin, const vector<Value*>::iterator end, const unsigned int firstValueId, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int valueId = firstValueId;
  const vector<unsigned int>::const_iterator tubeEnd = tube.end();
  vector<unsigned int>::const_iterator tubeBegin = tube.begin();
  for (vector<Value*>::iterator valueIt = begin; valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId);
      if (tubeBegin == tubeEnd)
	{
	  do
	    {
	      (*valueIt)->incrementPresentNoise();
	      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
		{
		  ++(*intersectionIt)[valueId];
		}
	      ++valueId;
	    } while (++valueIt != end);
	  return;
	}
      if (*tubeBegin != valueDataId)
	{
	  (*valueIt)->incrementPresentNoise();
	  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	    {
	      ++(*intersectionIt)[valueId];
	    }
	}
      ++valueId;
    }
}

unsigned int SparseCrispTube::absentFixPresentOrPotentialValuesAfterAbsentValueMet(Attribute& currentAttribute, vector<unsigned int>& absentValueIntersection) const
{
  const vector<Value*>::iterator potentialBegin = currentAttribute.potentialBegin();
  return absentFixPresentOrPotentialValuesAfterAbsentValueMet(currentAttribute.presentBegin(), potentialBegin, absentValueIntersection) + absentFixPresentOrPotentialValuesAfterAbsentValueMet(potentialBegin, currentAttribute.potentialEnd(), absentValueIntersection) + noiseOnIrrelevant(currentAttribute);
}

unsigned int SparseCrispTube::absentFixPresentOrPotentialValuesAfterAbsentValueMet(const vector<Value*>::iterator begin, const vector<Value*>::iterator end, vector<unsigned int>& absentValueIntersection) const
{
  unsigned int oldNoise = 0;
  const vector<unsigned int>::const_iterator tubeEnd = tube.end();
  vector<unsigned int>::const_iterator tubeBegin = tube.begin();
  for (vector<Value*>::iterator valueIt = begin; valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId);
      if (tubeBegin == tubeEnd)
	{
	  oldNoise += end - valueIt;
	  do
	    {
	      (*valueIt)->decrementPotentialNoise();
	      ++absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()];
	    } while (++valueIt != end);
	  return oldNoise;
	}
      if (*tubeBegin != valueDataId)
	{
	  (*valueIt)->decrementPotentialNoise();
	  ++absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()];
	  ++oldNoise;
	}
    }
  return oldNoise;
}

unsigned int SparseCrispTube::irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(Attribute& currentAttribute) const
{
  const vector<Value*>::iterator potentialBegin = currentAttribute.potentialBegin();
  return irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(currentAttribute.presentBegin(), potentialBegin) + irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(potentialBegin, currentAttribute.potentialEnd()) + noiseOnIrrelevant(currentAttribute);
}

unsigned int SparseCrispTube::irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(const vector<Value*>::iterator begin, const vector<Value*>::iterator end) const
{
  unsigned int oldNoise = 0;
  const vector<unsigned int>::const_iterator tubeEnd = tube.end();
  vector<unsigned int>::const_iterator tubeBegin = tube.begin();
  for (vector<Value*>::iterator valueIt = begin; valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId);
      if (tubeBegin == tubeEnd)
	{
	  oldNoise += end - valueIt;
	  do
	    {
	      (*valueIt)->decrementPotentialNoise();
	    } while (++valueIt != end);
	  return oldNoise;
	}
      if (*tubeBegin != valueDataId)
	{
	  (*valueIt)->decrementPotentialNoise();
	  ++oldNoise;
	}
    }
  return oldNoise;
}

unsigned int SparseCrispTube::noiseOnIrrelevant(const Attribute& currentAttribute) const
{
  unsigned int oldNoise = 0;
  const vector<unsigned int>::const_iterator tubeEnd = tube.end();
  vector<unsigned int>::const_iterator tubeBegin = tube.begin();
  vector<Value*>::const_iterator end = currentAttribute.irrelevantEnd();
  for (vector<Value*>::const_iterator valueIt = currentAttribute.irrelevantBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId);
      if (tubeBegin == tubeEnd)
	{
	  return end - valueIt + oldNoise;
	}
      if (*tubeBegin != valueDataId)
	{
	  ++oldNoise;
	}
    }
  return oldNoise;
}

unsigned int SparseCrispTube::absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(Attribute& currentAttribute, vector<unsigned int>& absentValueIntersection) const
{
  vector<Value*>::const_iterator potentialBegin = currentAttribute.potentialBegin();
  return absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(currentAttribute.presentBegin(), potentialBegin, absentValueIntersection) + absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(potentialBegin, currentAttribute.potentialEnd(), absentValueIntersection) + noiseOnIrrelevant(currentAttribute);
}

unsigned int SparseCrispTube::absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(const vector<Value*>::const_iterator begin, const vector<Value*>::const_iterator end, vector<unsigned int>& absentValueIntersection) const
{
  unsigned int oldNoise = 0;
  const vector<unsigned int>::const_iterator tubeEnd = tube.end();
  vector<unsigned int>::const_iterator tubeBegin = tube.begin();
  for (vector<Value*>::const_iterator valueIt = begin; valueIt != end; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId);
      if (tubeBegin == tubeEnd)
	{
	  oldNoise += end - valueIt;
	  do
	    {
	      --absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()];
	    } while (++valueIt != end);
	  return oldNoise;
	}
      if (*tubeBegin != valueDataId)
	{
	  --absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()];
	  ++oldNoise;
	}
    }
  return oldNoise;
}

void SparseCrispTube::absentFixAbsentValuesAfterAbsentValueMet(const vector<Value*>::iterator absentBegin, const vector<Value*>::iterator absentEnd, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<unsigned int>::const_iterator tubeEnd = tube.end();
  vector<unsigned int>::const_iterator tubeBegin = tube.begin();
  for (vector<Value*>::iterator valueIt = absentBegin; valueIt != absentEnd; ++valueIt)
    {
      const unsigned int valueDataId = (*valueIt)->getDataId();
      tubeBegin = lower_bound(tubeBegin, tubeEnd, valueDataId);
      if (tubeBegin == tubeEnd)
	{
	  do
	    {
	      (*valueIt)->decrementPotentialNoise();
	      const unsigned int valueId = (*valueIt)->getPresentAndPotentialIntersectionId();
	      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
		{
		  --(*intersectionIt)[valueId];
		}
	    } while (++valueIt != absentEnd);
	  return;
	}
      if (*tubeBegin != valueDataId)
	{
	  (*valueIt)->decrementPotentialNoise();
	  const unsigned int valueId = (*valueIt)->getPresentAndPotentialIntersectionId();
	  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	    {
	      --(*intersectionIt)[valueId];
	    }
	}
    }
}

unsigned int SparseCrispTube::noiseSum(const vector<vector<unsigned int>>::const_iterator dimensionIt) const
{
  unsigned int noise = 0;
  const vector<unsigned int>::const_iterator tubeEnd = tube.end();
  vector<unsigned int>::const_iterator tubeBegin = tube.begin();
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  for (vector<unsigned int>::const_iterator idIt = dimensionIt->begin(); idIt != idEnd; ++idIt)
    {
      tubeBegin = lower_bound(tubeBegin, tubeEnd, *idIt);
      if (tubeBegin == tubeEnd)
	{
	  return idEnd - idIt + noise;
	}
      if (*tubeBegin != *idIt)
	{
	  ++noise;
	}
    }
  return noise;
}

void SparseCrispTube::setSizeLimit(const unsigned int sizeLimitParam)
{
  sizeLimit = sizeLimitParam;
}

#ifdef ASSERT
unsigned int SparseCrispTube::noiseSumOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      if (binary_search(tube.begin(), tube.end(), value.getDataId()))
	{
	  return 0;
	}
      return 1;
    }
  unsigned int noise = 0;
  const vector<Value*>::const_iterator end = (*attributeIt)->presentEnd();
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != end; ++valueIt)
    {
      if (!binary_search(tube.begin(), tube.end(), (*valueIt)->getDataId()))
	{
	  ++noise;
	}
    }
  return noise;
}

unsigned int SparseCrispTube::noiseSumOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      if (binary_search(tube.begin(), tube.end(), value.getDataId()))
	{
	  return 0;
	}
      return 1;
    }
  unsigned int noise = 0;
  const vector<Value*>::const_iterator end = (*attributeIt)->irrelevantEnd();
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != end; ++valueIt)
    {
      if (!binary_search(tube.begin(), tube.end(), (*valueIt)->getDataId()))
	{
	  ++noise;
	}
    }
  return noise;
}
#endif
