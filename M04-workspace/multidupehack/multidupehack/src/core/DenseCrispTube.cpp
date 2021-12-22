// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "DenseCrispTube.h"

unsigned int DenseCrispTube::size;

DenseCrispTube::DenseCrispTube(const vector<unsigned int>& sparseTube): tube()
{
  tube.resize(size, true);
  for (const unsigned int elementId : sparseTube)
    {
      tube.set(elementId, false);
    }
}

DenseCrispTube::DenseCrispTube(const unsigned int presentElement): tube()
{
  tube.resize(size, true);
  tube.set(presentElement, false);
}

DenseCrispTube* DenseCrispTube::clone() const
{
  return new DenseCrispTube(*this);
}

void DenseCrispTube::print(vector<unsigned int>& prefix, ostream& out) const
{
  unsigned int hyperplaneId = 0;
  for (dynamic_bitset<>::size_type absentHyperplaneId = tube.find_first(); absentHyperplaneId != dynamic_bitset<>::npos; absentHyperplaneId = tube.find_next(absentHyperplaneId))
    {
      for (; hyperplaneId != absentHyperplaneId; ++hyperplaneId)
	{
	  printTuple(prefix, hyperplaneId, 1, out);
	}
      ++hyperplaneId;
    }
  const unsigned int end = size + 1;
  while (++hyperplaneId != end)
    {
      printTuple(prefix, hyperplaneId, 1, out);
    }
}

void DenseCrispTube::setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<vector<unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt)
{
  const unsigned int element = oldIds2NewIdsIt->at(tuple[*attributeIdIt]);
  (*attributeIt)->decrementPotentialNoise(element);
  tube.set(element, false);
}

unsigned int DenseCrispTube::noiseOnValue(const unsigned int valueDataId) const
{
  if (tube[valueDataId])
    {
      return 1;
    }
  return 0;
}

unsigned int DenseCrispTube::noiseOnValues(const vector<unsigned int>& valueDataIds) const
{
  unsigned int oldNoise = 0;
  for (const unsigned int valueDataId : valueDataIds)
    {
      if (tube[valueDataId])
	{
	  ++oldNoise;
	}
    }
  return oldNoise;
}

unsigned int DenseCrispTube::setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  // *this necessarily relates to the present attribute
  const Value& presentValue = (*attributeIt)->getChosenValue();
  if (tube[presentValue.getDataId()])
    {
      ++(*potentialOrAbsentValueIntersectionIt)[presentValue.getPresentIntersectionId()];
      return 1;
    }
  return 0;
}

unsigned int DenseCrispTube::presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute) const
{
  unsigned int newNoise = 0;
  const vector<Value*>::iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
	{
	  (*valueIt)->incrementPresentNoise();
	  ++newNoise;
	}
    }
  return newNoise;
}

unsigned int DenseCrispTube::presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(const Attribute& currentAttribute, const vector<unsigned int>::iterator potentialOrAbsentValueIntersectionBegin) const
{
  unsigned int newNoise = 0;
  vector<unsigned int>::iterator potentialOrAbsentValueIntersectionIt = potentialOrAbsentValueIntersectionBegin;
  const vector<Value*>::const_iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::const_iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
	{
	  ++*potentialOrAbsentValueIntersectionIt;
	  ++newNoise;
	}
      ++potentialOrAbsentValueIntersectionIt;
    }
  return newNoise;
}

void DenseCrispTube::presentFixPotentialOrAbsentValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int valueId = 0;
  const vector<Value*>::iterator end = currentAttribute.absentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != end; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
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

void DenseCrispTube::presentFixPotentialOrAbsentValuesInSecondSymmetricAttributeAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int valueId = 0;
  const vector<Value*>::iterator end = currentAttribute.absentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin() + 1; valueIt != end; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
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

unsigned int DenseCrispTube::absentFixPresentOrPotentialValuesAfterAbsentValueMet(Attribute& currentAttribute, vector<unsigned int>& absentValueIntersection) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
	{
	  (*valueIt)->decrementPotentialNoise();
	  ++absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()];
	  ++oldNoise;
	}
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
	{
	  ++oldNoise;
	}
    }
  return oldNoise;
}

unsigned int DenseCrispTube::irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(Attribute& currentAttribute) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
	{
	  (*valueIt)->decrementPotentialNoise();
	  ++oldNoise;
	}
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
	{
	  ++oldNoise;
	}
    }
  return oldNoise;
}

unsigned int DenseCrispTube::absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(Attribute& currentAttribute, vector<unsigned int>& absentValueIntersection) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
	{
	  --absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()];
	  ++oldNoise;
	}
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
	{
	  ++oldNoise;
	}
    }
  return oldNoise;
}

void DenseCrispTube::absentFixAbsentValuesAfterAbsentValueMet(const vector<Value*>::iterator absentBegin, const vector<Value*>::iterator absentEnd, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = absentBegin; valueIt != absentEnd; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
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

unsigned int DenseCrispTube::noiseSum(const vector<vector<unsigned int>>::const_iterator dimensionIt) const
{
  unsigned int noise = 0;
  for (const unsigned int id : *dimensionIt)
    {
      if (tube[id])
	{
	  ++noise;
	}
    }
  return noise;
}

void DenseCrispTube::setSize(const unsigned int sizeParam)
{
  size = sizeParam;
}

#ifdef ASSERT
unsigned int DenseCrispTube::noiseSumOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      if (tube[value.getDataId()])
	{
	  return 1;
	}
      return 0;
    }
  unsigned int noise = 0;
  const vector<Value*>::const_iterator end = (*attributeIt)->presentEnd();
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != end; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
	{
	  ++noise;
	}
    }
  return noise;
}

unsigned int DenseCrispTube::noiseSumOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      if (tube[value.getDataId()])
	{
	  return 1;
	}
      return 0;
    }
  unsigned int noise = 0;
  const vector<Value*>::const_iterator end = (*attributeIt)->irrelevantEnd();
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != end; ++valueIt)
    {
      if (tube[(*valueIt)->getDataId()])
	{
	  ++noise;
	}
    }
  return noise;
}
#endif
