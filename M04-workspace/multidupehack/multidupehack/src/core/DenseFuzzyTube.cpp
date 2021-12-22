// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "DenseFuzzyTube.h"

unsigned int DenseFuzzyTube::size;

DenseFuzzyTube::DenseFuzzyTube(const vector<pair<unsigned int, unsigned int>>& sparseTube): tube(size, Attribute::noisePerUnit)
{
  for (const pair<unsigned int, unsigned int>& entry : sparseTube)
    {
      tube[entry.first] = entry.second;
    }
}

DenseFuzzyTube::DenseFuzzyTube(const unsigned int presentElement): tube(size, Attribute::noisePerUnit)
{
  tube[presentElement] = 0;
}

DenseFuzzyTube* DenseFuzzyTube::clone() const
{
  return new DenseFuzzyTube(*this);
}

void DenseFuzzyTube::print(vector<unsigned int>& prefix, ostream& out) const
{
  unsigned int hyperplaneId = 0;
  for (const unsigned int noise : tube)
    {
      if (noise != Attribute::noisePerUnit)
	{
	  printTuple(prefix, hyperplaneId, 1 - static_cast<double>(noise) / Attribute::noisePerUnit, out);
	}
      ++hyperplaneId;
    }
}

void DenseFuzzyTube::setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<vector<unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt)
{
  const unsigned int element = oldIds2NewIdsIt->at(tuple[*attributeIdIt]);
  (*attributeIt)->subtractPotentialNoise(element, membership);
  tube[element] = Attribute::noisePerUnit - membership;
}

unsigned int DenseFuzzyTube::noiseOnValue(const unsigned int valueDataId) const
{
  return tube[valueDataId];
}

unsigned int DenseFuzzyTube::noiseOnValues(const vector<unsigned int>& valueDataIds) const
{
  unsigned int oldNoise = 0;
  for (const unsigned int valueDataId : valueDataIds)
    {
      oldNoise += tube[valueDataId];
    }
  return oldNoise;
}

unsigned int DenseFuzzyTube::setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  // *this necessarily relates to the present attribute
  const Value& presentValue = (*attributeIt)->getChosenValue();
  const unsigned int noise = tube[presentValue.getDataId()];
  (*potentialOrAbsentValueIntersectionIt)[presentValue.getPresentIntersectionId()] += noise;
  return noise;
}

unsigned int DenseFuzzyTube::presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute) const
{
  unsigned int newNoise = 0;
  const vector<Value*>::iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = tube[(*valueIt)->getDataId()];
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

unsigned int DenseFuzzyTube::presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(const Attribute& currentAttribute, const vector<unsigned int>::iterator potentialOrAbsentValueIntersectionBegin) const
{
  unsigned int newNoise = 0;
  vector<unsigned int>::iterator potentialOrAbsentValueIntersectionIt = potentialOrAbsentValueIntersectionBegin;
  const vector<Value*>::const_iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::const_iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = tube[(*valueIt)->getDataId()];
      *potentialOrAbsentValueIntersectionIt++ += newNoiseInHyperplane;
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

void DenseFuzzyTube::presentFixPotentialOrAbsentValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int valueId = 0;
  const vector<Value*>::iterator end = currentAttribute.absentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = tube[(*valueIt)->getDataId()];
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
      ++valueId;
    }
}

void DenseFuzzyTube::presentFixPotentialOrAbsentValuesInSecondSymmetricAttributeAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int valueId = 0;
  vector<Value*>::iterator end = currentAttribute.absentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin() + 1; valueIt != end; ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = tube[(*valueIt)->getDataId()];
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
      ++valueId;
    }
}

unsigned int DenseFuzzyTube::absentFixPresentOrPotentialValuesAfterAbsentValueMet(Attribute& currentAttribute, vector<unsigned int>& absentValueIntersection) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = tube[(*valueIt)->getDataId()];
      (*valueIt)->subtractPotentialNoise(oldNoiseInHyperplane);
      absentValueIntersection[(*valueIt)->getPresentAndPotentialIntersectionId()] += oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      oldNoise += tube[(*valueIt)->getDataId()];
    }
  return oldNoise;
}

unsigned int DenseFuzzyTube::irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(Attribute& currentAttribute) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = tube[(*valueIt)->getDataId()];
      (*valueIt)->subtractPotentialNoise(oldNoiseInHyperplane);
      oldNoise += oldNoiseInHyperplane;
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      oldNoise += tube[(*valueIt)->getDataId()];
    }
  return oldNoise;
}

unsigned int DenseFuzzyTube::absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(Attribute& currentAttribute, vector<unsigned int>& absentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = tube[(*valueIt)->getDataId()];
      absentValueIntersectionIt[(*valueIt)->getPresentAndPotentialIntersectionId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      oldNoise += tube[(*valueIt)->getDataId()];
    }
  return oldNoise;
}

void DenseFuzzyTube::absentFixAbsentValuesAfterAbsentValueMet(const vector<Value*>::iterator absentBegin, const vector<Value*>::iterator absentEnd, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = absentBegin; valueIt != absentEnd; ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = tube[(*valueIt)->getDataId()];
      (*valueIt)->subtractPotentialNoise(oldNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getPresentAndPotentialIntersectionId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] -= oldNoiseInHyperplane;
	}
    }
}

unsigned int DenseFuzzyTube::noiseSum(const vector<vector<unsigned int>>::const_iterator dimensionIt) const
{
  unsigned int noise = 0;
  for (const unsigned int id : *dimensionIt)
    {
      noise += tube[id];
    }
  return noise;
}

void DenseFuzzyTube::setSize(const unsigned int sizeParam)
{
  size = sizeParam;
}

#ifdef ASSERT
unsigned int DenseFuzzyTube::noiseSumOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      return tube[value.getDataId()];
    }
  unsigned int noise = 0;
  const vector<Value*>::const_iterator end = (*attributeIt)->presentEnd();
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != end; ++valueIt)
    {
      noise += tube[(*valueIt)->getDataId()];
    }
  return noise;
}

unsigned int DenseFuzzyTube::noiseSumOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      return tube[value.getDataId()];
    }
  unsigned int noise = 0;
  const vector<Value*>::const_iterator end = (*attributeIt)->irrelevantEnd();
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != end; ++valueIt)
    {
      noise += tube[(*valueIt)->getDataId()];
    }
  return noise;
}
#endif
