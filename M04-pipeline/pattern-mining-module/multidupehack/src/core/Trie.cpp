// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "Trie.h"

NoNoiseTube Trie::noNoiseTube = NoNoiseTube();

Trie::Trie(const Trie& otherTrie): hyperplanes()
{
  copy(otherTrie);
}

Trie::Trie(Trie&& otherTrie): hyperplanes(std::move(otherTrie.hyperplanes))
{
}

Trie::Trie(const vector<unsigned int>::const_iterator cardinalityIt, const vector<unsigned int>::const_iterator cardinalityEnd): hyperplanes()
{
  unsigned int cardinality = *cardinalityIt;
  hyperplanes.reserve(cardinality);
  if (cardinalityIt + 2 == cardinalityEnd)
    {
      if (Attribute::noisePerUnit == 1)
	{
	  do
	    {
	      hyperplanes.push_back(new SparseCrispTube());
	    }
	  while (--cardinality);
	  return;
	}
      do
	{
	  hyperplanes.push_back(new SparseFuzzyTube());
	}
      while (--cardinality);
      return;
    }
  const vector<unsigned int>::const_iterator nextCardinalityIt = cardinalityIt + 1;
  do
    {
      hyperplanes.push_back(new Trie(nextCardinalityIt, cardinalityEnd));
    }
  while (--cardinality);
}

Trie::~Trie()
{
  const vector<AbstractData*>::iterator hyperplaneEnd = hyperplanes.end();
  vector<AbstractData*>::iterator hyperplaneIt = hyperplanes.begin();
  if ((*hyperplaneIt)->isATrie())
    {
      do
	{
	  delete *hyperplaneIt;
	}
      while (++hyperplaneIt != hyperplaneEnd);
      return;
    }
  do
    {
      if (*hyperplaneIt != &noNoiseTube)
	{
	  delete *hyperplaneIt;
	}
    }
  while (++hyperplaneIt != hyperplaneEnd);
}

Trie* Trie::clone() const
{
  return new Trie(*this);
}

Trie& Trie::operator=(const Trie& otherTrie)
{
  copy(otherTrie);
  return *this;
}

Trie& Trie::operator=(Trie&& otherTrie)
{
  hyperplanes = std::move(otherTrie.hyperplanes);
  return *this;
}

ostream& operator<<(ostream& out, const Trie& trie)
{
  vector<unsigned int> prefix;
  trie.print(prefix, out);
  return out;
}

void Trie::copy(const Trie& otherTrie)
{
  hyperplanes.reserve(otherTrie.hyperplanes.size());
  for (const AbstractData* hyperplane : otherTrie.hyperplanes)
    {
      hyperplanes.push_back(hyperplane->clone());
    }
}

bool Trie::isATrie() const
{
  return true;
}

void Trie::print(vector<unsigned int>& prefix, ostream& out) const
{
  unsigned int hyperplaneId = 0;
  for (AbstractData* hyperplane : hyperplanes)
    {
      prefix.push_back(hyperplaneId++);
      hyperplane->print(prefix, out);
      prefix.pop_back();
    }
}

void Trie::setSelfLoops(const unsigned int firstSymmetricAttributeId, const unsigned int lastSymmetricAttributeId)
{
  setSelfLoopsBeforeSymmetricAttributes(firstSymmetricAttributeId, lastSymmetricAttributeId, 0);
}

void Trie::setSelfLoopsBeforeSymmetricAttributes(const unsigned int firstSymmetricAttributeId, const unsigned int lastSymmetricAttributeId, const unsigned int dimensionId)
{
  const unsigned int nextDimensionId = dimensionId + 1;
  if (dimensionId == firstSymmetricAttributeId)
    {
      unsigned int hyperplaneId = 0;
      for (AbstractData*& hyperplane : hyperplanes)
	{
	  setSelfLoopsInSymmetricAttributes(hyperplane, hyperplaneId++, lastSymmetricAttributeId, nextDimensionId);
	}
      return;
    }
  for (AbstractData* hyperplane : hyperplanes)
    {
      static_cast<Trie*>(hyperplane)->setSelfLoopsBeforeSymmetricAttributes(firstSymmetricAttributeId, lastSymmetricAttributeId, nextDimensionId);
    }
}

void Trie::setSelfLoopsInSymmetricAttributes(AbstractData*& hyperplane, const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const unsigned int nextDimensionId)
{
  if (hyperplane->setSelfLoopsInSymmetricAttributes(hyperplaneId, lastSymmetricAttributeId, nextDimensionId))
    {
      delete hyperplane;
      if (Attribute::noisePerUnit == 1)
	{
	  hyperplane = new DenseCrispTube(hyperplaneId);
	  return;
	}
      hyperplane = new DenseFuzzyTube(hyperplaneId);
    }
}

bool Trie::setSelfLoopsInSymmetricAttributes(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const unsigned int dimensionId)
{
  AbstractData*& hyperplane = hyperplanes[hyperplaneId];
  if (dimensionId == lastSymmetricAttributeId)
    {
      if (hyperplane->isATrie())
	{
	  static_cast<Trie*>(hyperplane)->setSelfLoopsAfterSymmetricAttributes();
	  return false;
	}
      delete hyperplane;
      hyperplane = &noNoiseTube;
      return false;
    }
  setSelfLoopsInSymmetricAttributes(hyperplane, hyperplaneId, lastSymmetricAttributeId, dimensionId + 1);
  return false;
}

void Trie::setSelfLoopsAfterSymmetricAttributes()
{
  const vector<AbstractData*>::iterator end = hyperplanes.end();
  vector<AbstractData*>::iterator hyperplaneIt = hyperplanes.begin();
  if ((*hyperplaneIt)->isATrie())
    {
      for (; hyperplaneIt != end; ++hyperplaneIt)
	{
	  static_cast<Trie*>(*hyperplaneIt)->setSelfLoopsAfterSymmetricAttributes();
	}
      return;
    }
  for (; hyperplaneIt != end; ++hyperplaneIt)
    {
      delete *hyperplaneIt;
      *hyperplaneIt = &noNoiseTube;
    }
}

void Trie::setHyperplane(const unsigned int hyperplaneOldId, const unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::const_iterator begin, const unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::const_iterator end, const vector<unsigned int>& attributeOrder, const vector<vector<unsigned int>>& oldIds2NewIds, vector<Attribute*>& attributes)
{
  const vector<Attribute*>::iterator nextAttributeIt = attributes.begin() + 1;
  const unsigned int hyperplaneId = oldIds2NewIds.front().at(hyperplaneOldId);
  AbstractData*& hyperplane = hyperplanes[hyperplaneId];
  for (unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::const_iterator tupleIt = begin; tupleIt != end; ++tupleIt)
    {
      const unsigned int membership = ceil(tupleIt->second * Attribute::noisePerUnit); // ceil to guarantee that a super-pattern of every pattern to be output is output
      setTuple(hyperplane, tupleIt->first, membership, attributeOrder.begin(), ++oldIds2NewIds.begin(), nextAttributeIt);
      attributes.front()->subtractPotentialNoise(hyperplaneId, membership);
    }
  sortTubes();
}

void Trie::setTuple(AbstractData*& hyperplane, const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<vector<unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator nextAttributeIt)
{
  if (hyperplane->isFullSparseTube())
    {
      Tube* newHyperplane;
      if (Attribute::noisePerUnit == 1)
	{
	  newHyperplane = static_cast<SparseCrispTube*>(hyperplane)->getDenseRepresentation();
	}
      else
	{
	  newHyperplane = static_cast<SparseFuzzyTube*>(hyperplane)->getDenseRepresentation();
	}
      delete hyperplane;
      hyperplane = newHyperplane;
    }
  hyperplane->setTuple(tuple, membership, attributeIdIt, oldIds2NewIdsIt, nextAttributeIt);
}

void Trie::setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<vector<unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt)
{
  const unsigned int element = oldIds2NewIdsIt->at(tuple[*attributeIdIt]);
  (*attributeIt)->subtractPotentialNoise(element, membership);
  setTuple(hyperplanes[element], tuple, membership, ++attributeIdIt, ++oldIds2NewIdsIt, attributeIt + 1);
}

void Trie::sortTubes()
{
  for (AbstractData* hyperplane : hyperplanes)
    {
      hyperplane->sortTubes();
    }
}

void Trie::setPresent(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeBegin) const
{
  vector<vector<vector<unsigned int>>::iterator> intersectionIts;
  intersectionIts.reserve(Attribute::lastAttributeId());
  setPresent(presentAttributeIt, attributeBegin, intersectionIts);
}

void Trie::setSymmetricPresent(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeBegin) const
{
  vector<vector<vector<unsigned int>>::iterator> intersectionIts;
  intersectionIts.reserve(Attribute::lastAttributeId());
  setSymmetricPresent(presentAttributeIt, attributeBegin, intersectionIts);
}

void Trie::setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeBegin) const
{
  vector<vector<vector<unsigned int>>::iterator> intersectionIts;
  intersectionIts.reserve(Attribute::lastAttributeId());
  setAbsent(absentAttributeIt, attributeBegin, intersectionIts);
}

void Trie::setIrrelevant(const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueDataIds, const vector<Attribute*>::iterator attributeBegin) const
{
  vector<vector<vector<unsigned int>>::iterator> intersectionIts;
  intersectionIts.reserve(Attribute::lastAttributeId());
  setIrrelevant(irrelevantAttributeIt, irrelevantValueDataIds, attributeBegin, intersectionIts);
}

void Trie::setSymmetricAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeBegin) const
{
  vector<vector<vector<unsigned int>>::iterator> intersectionIts;
  intersectionIts.reserve(Attribute::lastAttributeId());
  setSymmetricAbsent(absentAttributeIt, attributeBegin, intersectionIts);
}

unsigned int Trie::setPresent(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  if (attributeIt == presentAttributeIt)
    {
      Value& presentValue = (*presentAttributeIt)->getChosenValue();
      nextIntersectionIts.push_back(presentValue.getIntersectionsBeginWithPresentValues());
      return hyperplanes[presentValue.getDataId()]->setPresentAfterPresentValueMet(nextAttributeIt, nextIntersectionIts);
    }
  presentFixPotentialOrAbsentValues((*attributeIt)->potentialBegin(), (*attributeIt)->absentEnd(), presentAttributeIt, nextAttributeIt, intersectionIts);
  return presentFixPresentValues(**attributeIt, presentAttributeIt, nextAttributeIt, nextIntersectionIts);
}

unsigned int Trie::setSymmetricPresent(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  if (attributeIt == presentAttributeIt)
    {
      // *this necessarily relates to the first symmetric attribute
      presentFixPotentialOrAbsentValues((*attributeIt)->potentialBegin() + 1, (*attributeIt)->absentEnd(), nextAttributeIt, nextAttributeIt, intersectionIts);
      const unsigned int newNoise = presentFixPresentValues(**attributeIt, nextAttributeIt, nextAttributeIt, nextIntersectionIts);
      Value& presentValue = (*presentAttributeIt)->getChosenValue();
      nextIntersectionIts.push_back(presentValue.getIntersectionsBeginWithPresentValues());
      return newNoise + hyperplanes[presentValue.getDataId()]->setSymmetricPresentAfterPresentValueMet(nextAttributeIt, nextIntersectionIts);
    }
  presentFixPotentialOrAbsentValuesBeforeSymmetricAttributes(**attributeIt, presentAttributeIt, nextAttributeIt, intersectionIts);
  return presentFixPresentValuesBeforeSymmetricAttributes(**attributeIt, presentAttributeIt, nextAttributeIt, nextIntersectionIts);
}

unsigned int Trie::setPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  presentFixPotentialOrAbsentValuesAfterPresentValueMet((*attributeIt)->potentialBegin(), (*attributeIt)->absentEnd(), nextAttributeIt, intersectionIts);
  return presentFixPresentValuesAfterPresentValueMet(**attributeIt, nextAttributeIt, nextIntersectionIts);
}

unsigned int Trie::setSymmetricPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  presentFixPotentialOrAbsentValuesAfterPresentValueMet((*attributeIt)->potentialBegin() + 1, (*attributeIt)->absentEnd(), nextAttributeIt, intersectionIts);
  return presentFixPresentValuesAfterPresentValueMet(**attributeIt, nextAttributeIt, nextIntersectionIts);
}

unsigned int Trie::setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  if (attributeIt == presentAttributeIt)
    {
      const Value& presentValue = (*presentAttributeIt)->getChosenValue();
      const unsigned int newNoiseInHyperplane = hyperplanes[presentValue.getDataId()]->setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(attributeIt + 1, potentialOrAbsentValueIntersectionIt + 1);
      (*potentialOrAbsentValueIntersectionIt)[presentValue.getPresentIntersectionId()] += newNoiseInHyperplane;
      return newNoiseInHyperplane;
    }
  return presentFixPresentValuesAfterPotentialOrAbsentUsed(**attributeIt, presentAttributeIt, attributeIt + 1, potentialOrAbsentValueIntersectionIt->begin(), potentialOrAbsentValueIntersectionIt + 1);
}

unsigned int Trie::setSymmetricPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  if (attributeIt == presentAttributeIt)
    {
      // *this necessarily relates to the first symmetric attribute
      const Value& presentValue = (*presentAttributeIt)->getChosenValue();
      const unsigned int newNoiseInHyperplane = hyperplanes[presentValue.getDataId()]->setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(attributeIt + 1, potentialOrAbsentValueIntersectionIt + 1);
      (*potentialOrAbsentValueIntersectionIt)[presentValue.getPresentIntersectionId()] += newNoiseInHyperplane;
      return newNoiseInHyperplane + presentFixPresentValuesAfterPotentialOrAbsentUsed(**attributeIt, presentAttributeIt + 1, attributeIt + 1, potentialOrAbsentValueIntersectionIt->begin(), potentialOrAbsentValueIntersectionIt + 1);
    }
  return presentFixPresentValuesBeforeSymmetricAttributesAfterPotentialOrAbsentUsed(**attributeIt, presentAttributeIt, attributeIt + 1, potentialOrAbsentValueIntersectionIt->begin(), potentialOrAbsentValueIntersectionIt + 1);
}

unsigned int Trie::setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  return presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(**attributeIt, attributeIt + 1, potentialOrAbsentValueIntersectionIt->begin(), potentialOrAbsentValueIntersectionIt + 1);
}

unsigned int Trie::presentFixPresentValues(Attribute& currentAttribute, const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int newNoise = 0;
  const vector<Value*>::iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentValues());
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setPresent(presentAttributeIt, nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

unsigned int Trie::presentFixPresentValuesBeforeSymmetricAttributes(Attribute& currentAttribute, const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int newNoise = 0;
  const vector<Value*>::iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentValues());
      // Since this is before the symmetric attributes, hyperplanes necessarily are tries
      const unsigned int newNoiseInHyperplane = static_cast<Trie*>(hyperplanes[(*valueIt)->getDataId()])->setSymmetricPresent(presentAttributeIt, nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

unsigned int Trie::presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int newNoise = 0;
  const vector<Value*>::iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentValues());
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setPresentAfterPresentValueMet(nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

unsigned int Trie::presentFixPresentValuesAfterPotentialOrAbsentUsed(Attribute& currentAttribute, const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator nextAttributeIt, const vector<unsigned int>::iterator potentialOrAbsentValueIntersectionBegin, const vector<vector<unsigned int>>::iterator nextPotentialOrAbsentValueIntersectionIt) const
{
  unsigned int newNoise = 0;
  vector<unsigned int>::iterator potentialOrAbsentValueIntersectionIt = potentialOrAbsentValueIntersectionBegin;
  const vector<Value*>::iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setPresentAfterPotentialOrAbsentUsed(presentAttributeIt, nextAttributeIt, nextPotentialOrAbsentValueIntersectionIt);
      *potentialOrAbsentValueIntersectionIt++ += newNoiseInHyperplane;
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

unsigned int Trie::presentFixPresentValuesBeforeSymmetricAttributesAfterPotentialOrAbsentUsed(Attribute& currentAttribute, const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator nextAttributeIt, const vector<unsigned int>::iterator potentialOrAbsentValueIntersectionBegin, const vector<vector<unsigned int>>::iterator nextPotentialOrAbsentValueIntersectionIt) const
{
  unsigned int newNoise = 0;
  vector<unsigned int>::iterator potentialOrAbsentValueIntersectionIt = potentialOrAbsentValueIntersectionBegin;
  const vector<Value*>::iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      // Since this is before the symmetric attributes, hyperplanes necessarily are tries
      const unsigned int newNoiseInHyperplane = static_cast<Trie*>(hyperplanes[(*valueIt)->getDataId()])->setSymmetricPresentAfterPotentialOrAbsentUsed(presentAttributeIt, nextAttributeIt, nextPotentialOrAbsentValueIntersectionIt);
      *potentialOrAbsentValueIntersectionIt++ += newNoiseInHyperplane;
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

unsigned int Trie::presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(const Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, const vector<unsigned int>::iterator potentialOrAbsentValueIntersectionBegin, const vector<vector<unsigned int>>::iterator nextPotentialOrAbsentValueIntersectionIt) const
{
  unsigned int newNoise = 0;
  vector<unsigned int>::iterator potentialOrAbsentValueIntersectionIt = potentialOrAbsentValueIntersectionBegin;
  const vector<Value*>::const_iterator end = currentAttribute.presentEnd();
  for (vector<Value*>::const_iterator valueIt = currentAttribute.presentBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(nextAttributeIt, nextPotentialOrAbsentValueIntersectionIt);
      *potentialOrAbsentValueIntersectionIt++ += newNoiseInHyperplane;
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

void Trie::presentFixPotentialOrAbsentValues(const vector<Value*>::iterator begin, const vector<Value*>::iterator end, const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int valueId = 0;
  for (vector<Value*>::iterator valueIt = begin; valueIt != end; ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setPresentAfterPotentialOrAbsentUsed(presentAttributeIt, nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentValues());
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
      ++valueId;
    }
}

void Trie::presentFixPotentialOrAbsentValuesBeforeSymmetricAttributes(Attribute& currentAttribute, const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int valueId = 0;
  const vector<Value*>::iterator end = currentAttribute.absentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != end; ++valueIt)
    {
      // Since this is before the symmetric attributes, hyperplanes necessarily are tries
      const unsigned int newNoiseInHyperplane = static_cast<Trie*>(hyperplanes[(*valueIt)->getDataId()])->setSymmetricPresentAfterPotentialOrAbsentUsed(presentAttributeIt, nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentValues());
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
      ++valueId;
    }
}

void Trie::presentFixPotentialOrAbsentValuesAfterPresentValueMet(const vector<Value*>::iterator begin, const vector<Value*>::iterator end, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int valueId = 0;
  for (vector<Value*>::iterator valueIt = begin; valueIt != end; ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentValues());
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
      ++valueId;
    }
}

unsigned int Trie::setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  if (attributeIt == absentAttributeIt)
    {
      Value& absentValue = (*absentAttributeIt)->getChosenAbsentValue();
      const unsigned int oldNoiseInHyperplane = hyperplanes[absentValue.getDataId()]->setAbsentAfterAbsentValueMet(nextAttributeIt, nextIntersectionIts, absentValue.getIntersectionsBeginWithPresentAndPotentialValues());
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  intersectionIt->back() += oldNoiseInHyperplane;
	}
      return oldNoiseInHyperplane;
    }
  absentFixAbsentValues(**attributeIt, absentAttributeIt, nextAttributeIt, intersectionIts);
  return absentFixPresentOrPotentialValues(**attributeIt, absentAttributeIt, nextAttributeIt, intersectionIts, nextIntersectionIts);
}

unsigned int Trie::setIrrelevant(const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueDataIds, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  if (attributeIt == irrelevantAttributeIt)
    {
      unsigned int oldNoise = 0;
      for (const unsigned int irrelevantValueDataId : irrelevantValueDataIds)
	{
	  oldNoise += hyperplanes[irrelevantValueDataId]->setIrrelevantAfterIrrelevantValueMet(nextAttributeIt, nextIntersectionIts);
	}
      return oldNoise;
    }
  irrelevantFixAbsentValues(**attributeIt, irrelevantAttributeIt, irrelevantValueDataIds, nextAttributeIt, intersectionIts);
  return irrelevantFixPresentOrPotentialValues(**attributeIt, irrelevantAttributeIt, irrelevantValueDataIds, nextAttributeIt, intersectionIts, nextIntersectionIts);
}

unsigned int Trie::setSymmetricAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  if (attributeIt == absentAttributeIt)
    {
      // *this necessarily relates to the first symmetric attribute
      absentFixAbsentValues(**attributeIt, nextAttributeIt, nextAttributeIt, intersectionIts);
      Value& absentValue = (*absentAttributeIt)->getChosenAbsentValue();
      const unsigned int oldNoiseInHyperplane = hyperplanes[absentValue.getDataId()]->setSymmetricAbsentAfterAbsentValueMet(nextAttributeIt, nextIntersectionIts, absentValue.getIntersectionsBeginWithPresentAndPotentialValues());
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  intersectionIt->back() += oldNoiseInHyperplane;
	}
      return oldNoiseInHyperplane + absentFixPresentOrPotentialValues(**attributeIt, nextAttributeIt, nextAttributeIt, intersectionIts, nextIntersectionIts);
    }
  absentFixAbsentValuesBeforeSymmetricAttributes(**attributeIt, absentAttributeIt, nextAttributeIt, intersectionIts);
  return absentFixPresentOrPotentialValuesBeforeSymmetricAttributes(**attributeIt, absentAttributeIt, nextAttributeIt, intersectionIts, nextIntersectionIts);
}

unsigned int Trie::setAbsentAfterAbsentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  absentFixAbsentValuesAfterAbsentValueMet((*attributeIt)->absentBegin(), (*attributeIt)->absentEnd(), nextAttributeIt, intersectionIts);
  return absentFixPresentOrPotentialValuesAfterAbsentValueMet(**attributeIt, nextAttributeIt, intersectionIts, nextIntersectionIts, absentValueIntersectionIt);
}

unsigned int Trie::setIrrelevantAfterIrrelevantValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  absentFixAbsentValuesAfterAbsentValueMet((*attributeIt)->absentBegin(), (*attributeIt)->absentEnd(), nextAttributeIt, intersectionIts);
  return irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(**attributeIt, nextAttributeIt, intersectionIts, nextIntersectionIts);
}

unsigned int Trie::setSymmetricAbsentAfterAbsentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  // *this necessarily relates to the second symmetric attribute
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  // The first absent value actually is the value set absent and there is no noise to be found at the intersection of a vertex (seen as an outgoing vertex) and itself (seen as an ingoing vertex)
  absentFixAbsentValuesAfterAbsentValueMet((*attributeIt)->absentBegin() + 1, (*attributeIt)->absentEnd(), nextAttributeIt, intersectionIts);
  return absentFixPresentOrPotentialValuesAfterAbsentValueMet(**attributeIt, nextAttributeIt, intersectionIts, nextIntersectionIts, absentValueIntersectionIt);
}

unsigned int Trie::setAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  if (attributeIt == absentAttributeIt)
    {
      return hyperplanes[(*absentAttributeIt)->getChosenAbsentValueDataId()]->setAbsentAfterAbsentValueMetAndAbsentUsed(attributeIt + 1, absentValueIntersectionIt + 1);
    }
  return absentFixPresentOrPotentialValuesAfterAbsentUsed(**attributeIt, absentAttributeIt, attributeIt + 1, absentValueIntersectionIt, absentValueIntersectionIt + 1);
}

unsigned int Trie::setIrrelevantAfterAbsentUsed(const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueDataIds, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  const vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt = absentValueIntersectionIt + 1;
  if (attributeIt == irrelevantAttributeIt)
    {
      unsigned int oldNoise = 0;
      for (const unsigned int irrelevantValueDataId : irrelevantValueDataIds)
	{
	  oldNoise += hyperplanes[irrelevantValueDataId]->setAbsentAfterAbsentValueMetAndAbsentUsed(nextAttributeIt, nextAbsentValueIntersectionIt);
	}
      return oldNoise;
    }
  return irrelevantFixPresentOrPotentialValuesAfterAbsentUsed(**attributeIt, irrelevantAttributeIt, irrelevantValueDataIds, nextAttributeIt, absentValueIntersectionIt, nextAbsentValueIntersectionIt);
}

unsigned int Trie::setSymmetricAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  const vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt = absentValueIntersectionIt + 1;
  if (attributeIt == absentAttributeIt)
    {
      // *this necessarily relates to the first symmetric attribute
      return hyperplanes[(*absentAttributeIt)->getChosenAbsentValueDataId()]->setAbsentAfterAbsentValueMetAndAbsentUsed(nextAttributeIt, nextAbsentValueIntersectionIt) + absentFixPresentOrPotentialValuesAfterAbsentUsed(**attributeIt, nextAttributeIt, nextAttributeIt, absentValueIntersectionIt, nextAbsentValueIntersectionIt);
    }
  return absentFixPresentOrPotentialValuesBeforeSymmetricAttributesAfterAbsentUsed(**attributeIt, absentAttributeIt, nextAttributeIt, absentValueIntersectionIt, nextAbsentValueIntersectionIt);
}

unsigned int Trie::setAbsentAfterAbsentValueMetAndAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  return absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(**attributeIt, attributeIt + 1, absentValueIntersectionIt, absentValueIntersectionIt + 1);
}

unsigned int Trie::absentFixPresentOrPotentialValues(Attribute& currentAttribute, const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setAbsent(absentAttributeIt, nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
      (*valueIt)->subtractPotentialNoise(oldNoiseInHyperplane);
      oldNoise += oldNoiseInHyperplane;
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      oldNoise += hyperplanes[(*valueIt)->getDataId()]->setAbsent(absentAttributeIt, nextAttributeIt, nextIntersectionIts);
    }
  return oldNoise;
}

unsigned int Trie::irrelevantFixPresentOrPotentialValues(Attribute& currentAttribute, const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueDataIds, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setIrrelevant(irrelevantAttributeIt, irrelevantValueDataIds, nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
      (*valueIt)->subtractPotentialNoise(oldNoiseInHyperplane);
      oldNoise += oldNoiseInHyperplane;
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      oldNoise += hyperplanes[(*valueIt)->getDataId()]->setIrrelevant(irrelevantAttributeIt, irrelevantValueDataIds, nextAttributeIt, nextIntersectionIts);
    }
  return oldNoise;
}

unsigned int Trie::absentFixPresentOrPotentialValuesBeforeSymmetricAttributes(Attribute& currentAttribute, const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      // Since this is before the symmetric attributes, hyperplanes necessarily are tries
      const unsigned int oldNoiseInHyperplane = static_cast<Trie*>(hyperplanes[(*valueIt)->getDataId()])->setSymmetricAbsent(absentAttributeIt, nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
      (*valueIt)->subtractPotentialNoise(oldNoiseInHyperplane);
      oldNoise += oldNoiseInHyperplane;
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      oldNoise += static_cast<Trie*>(hyperplanes[(*valueIt)->getDataId()])->setSymmetricAbsent(absentAttributeIt, nextAttributeIt, nextIntersectionIts);
    }
  return oldNoise;
}

unsigned int Trie::absentFixPresentOrPotentialValuesAfterAbsentValueMet(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts, vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt = absentValueIntersectionIt + 1;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setAbsentAfterAbsentValueMet(nextAttributeIt, nextIntersectionIts, nextAbsentValueIntersectionIt);
      nextIntersectionIts.pop_back();
      (*valueIt)->subtractPotentialNoise(oldNoiseInHyperplane);
      (*absentValueIntersectionIt)[(*valueIt)->getPresentAndPotentialIntersectionId()] += oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      oldNoise += hyperplanes[(*valueIt)->getDataId()]->setAbsentAfterAbsentValueMet(nextAttributeIt, nextIntersectionIts, nextAbsentValueIntersectionIt);
    }
  return oldNoise;
}

unsigned int Trie::irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setIrrelevantAfterIrrelevantValueMet(nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
      (*valueIt)->subtractPotentialNoise(oldNoiseInHyperplane);
      oldNoise += oldNoiseInHyperplane;
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      oldNoise += hyperplanes[(*valueIt)->getDataId()]->setIrrelevantAfterIrrelevantValueMet(nextAttributeIt, nextIntersectionIts);
    }
  return oldNoise;
}

unsigned int Trie::absentFixPresentOrPotentialValuesAfterAbsentUsed(Attribute& currentAttribute, const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator nextAttributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt, const vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setAbsentAfterAbsentUsed(absentAttributeIt, nextAttributeIt, nextAbsentValueIntersectionIt);
      (*absentValueIntersectionIt)[(*valueIt)->getPresentAndPotentialIntersectionId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      oldNoise += hyperplanes[(*valueIt)->getDataId()]->setAbsentAfterAbsentUsed(absentAttributeIt, nextAttributeIt, nextAbsentValueIntersectionIt);
    }
  return oldNoise;
}

unsigned int Trie::irrelevantFixPresentOrPotentialValuesAfterAbsentUsed(Attribute& currentAttribute, const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueDataIds, const vector<Attribute*>::iterator nextAttributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt, const vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setIrrelevantAfterAbsentUsed(irrelevantAttributeIt, irrelevantValueDataIds, nextAttributeIt, nextAbsentValueIntersectionIt);
      (*absentValueIntersectionIt)[(*valueIt)->getPresentAndPotentialIntersectionId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      oldNoise += hyperplanes[(*valueIt)->getDataId()]->setIrrelevantAfterAbsentUsed(irrelevantAttributeIt, irrelevantValueDataIds, nextAttributeIt, nextAbsentValueIntersectionIt);
    }
  return oldNoise;
}

unsigned int Trie::absentFixPresentOrPotentialValuesBeforeSymmetricAttributesAfterAbsentUsed(Attribute& currentAttribute, const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator nextAttributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt, const vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      // Since this is before the symmetric attributes, hyperplanes necessarily are tries
      const unsigned int oldNoiseInHyperplane = static_cast<Trie*>(hyperplanes[(*valueIt)->getDataId()])->setSymmetricAbsentAfterAbsentUsed(absentAttributeIt, nextAttributeIt, nextAbsentValueIntersectionIt);
      (*absentValueIntersectionIt)[(*valueIt)->getPresentAndPotentialIntersectionId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      oldNoise += static_cast<Trie*>(hyperplanes[(*valueIt)->getDataId()])->setSymmetricAbsentAfterAbsentUsed(absentAttributeIt, nextAttributeIt, nextAbsentValueIntersectionIt);
    }
  return oldNoise;
}

unsigned int Trie::absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt, const vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  vector<Value*>::iterator end = currentAttribute.potentialEnd();
  vector<Value*>::iterator valueIt = currentAttribute.presentBegin();
  for (; valueIt != end; ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setAbsentAfterAbsentValueMetAndAbsentUsed(nextAttributeIt, nextAbsentValueIntersectionIt);
      (*absentValueIntersectionIt)[(*valueIt)->getPresentAndPotentialIntersectionId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  end = currentAttribute.irrelevantEnd();
  for (; valueIt != end; ++valueIt)
    {
      oldNoise += hyperplanes[(*valueIt)->getDataId()]->setAbsentAfterAbsentValueMetAndAbsentUsed(nextAttributeIt, nextAbsentValueIntersectionIt);
    }
  return oldNoise;
}

void Trie::absentFixAbsentValues(Attribute& currentAttribute, const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Value*>::iterator end = currentAttribute.absentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setAbsentAfterAbsentUsed(absentAttributeIt, nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      (*valueIt)->subtractPotentialNoise(oldNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getPresentAndPotentialIntersectionId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] -= oldNoiseInHyperplane;
	}
    }
}

void Trie::irrelevantFixAbsentValues(Attribute& currentAttribute, const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueDataIds, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Value*>::iterator end = currentAttribute.absentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != end; ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setIrrelevantAfterAbsentUsed(irrelevantAttributeIt, irrelevantValueDataIds, nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      (*valueIt)->subtractPotentialNoise(oldNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getPresentAndPotentialIntersectionId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] -= oldNoiseInHyperplane;
	}
    }
}

void Trie::absentFixAbsentValuesBeforeSymmetricAttributes(Attribute& currentAttribute, const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Value*>::iterator end = currentAttribute.absentEnd();
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != end; ++valueIt)
    {
      // Since this is before the symmetric attributes, hyperplanes necessarily are tries
      const unsigned int oldNoiseInHyperplane = static_cast<Trie*>(hyperplanes[(*valueIt)->getDataId()])->setSymmetricAbsentAfterAbsentUsed(absentAttributeIt, nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      (*valueIt)->subtractPotentialNoise(oldNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getPresentAndPotentialIntersectionId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] -= oldNoiseInHyperplane;
	}
    }
}

void Trie::absentFixAbsentValuesAfterAbsentValueMet(const vector<Value*>::iterator absentBegin, const vector<Value*>::iterator absentEnd, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = absentBegin; valueIt != absentEnd; ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getDataId()]->setAbsentAfterAbsentValueMetAndAbsentUsed(nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      (*valueIt)->subtractPotentialNoise(oldNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getPresentAndPotentialIntersectionId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] -= oldNoiseInHyperplane;
	}
    }
}

double Trie::noiseSum(const vector<vector<unsigned int>>& nSet) const
{
  double noise = 0;
  const vector<vector<unsigned int>>::const_iterator dimensionIt = nSet.begin();
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  for (const unsigned int id : *dimensionIt)
    {
      noise += hyperplanes[id]->noiseSum(nextDimensionIt);
    }
  return noise;
}

unsigned int Trie::noiseSum(const vector<vector<unsigned int>>::const_iterator dimensionIt) const
{
  unsigned int noise = 0;
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  for (const unsigned int id : *dimensionIt)
    {
      noise += hyperplanes[id]->noiseSum(nextDimensionIt);
    }
  return noise;
}

#ifdef ASSERT
unsigned int Trie::noiseSumOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  const vector<Attribute*>::const_iterator nextAttributeIt = attributeIt + 1;
  if (attributeIt == valueAttributeIt)
    {
      return hyperplanes[value.getDataId()]->noiseSumOnPresent(valueAttributeIt, value, nextAttributeIt);
    }
  unsigned int noise = 0;
  const vector<Value*>::const_iterator end = (*attributeIt)->presentEnd();
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != end; ++valueIt)
    {
      noise += hyperplanes[(*valueIt)->getDataId()]->noiseSumOnPresent(valueAttributeIt, value, nextAttributeIt);
    }
  return noise;
}

unsigned int Trie::noiseSumOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  const vector<Attribute*>::const_iterator nextAttributeIt = attributeIt + 1;
  if (attributeIt == valueAttributeIt)
    {
      return hyperplanes[value.getDataId()]->noiseSumOnPresentAndPotential(valueAttributeIt, value, nextAttributeIt);
    }
  unsigned int noise = 0;
  const vector<Value*>::const_iterator end = (*attributeIt)->irrelevantEnd();
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != end; ++valueIt)
    {
      noise += hyperplanes[(*valueIt)->getDataId()]->noiseSumOnPresentAndPotential(valueAttributeIt, value, nextAttributeIt);
    }
  return noise;
}

unsigned int Trie::noiseSumOnPresent(const vector<Attribute*>::const_iterator firstValueAttributeIt, const Value& firstValue, const vector<Attribute*>::const_iterator secondValueAttributeIt, const Value& secondValue, const vector<Attribute*>::const_iterator attributeIt) const
{
  const vector<Attribute*>::const_iterator nextAttributeIt = attributeIt + 1;
  if (attributeIt == firstValueAttributeIt)
    {
      return hyperplanes[firstValue.getDataId()]->noiseSumOnPresent(secondValueAttributeIt, secondValue, nextAttributeIt);
    }
  unsigned int noise = 0;
  const vector<Value*>::const_iterator end = (*attributeIt)->presentEnd();
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != end; ++valueIt)
    {
      noise += static_cast<Trie*>(hyperplanes[(*valueIt)->getDataId()])->noiseSumOnPresent(firstValueAttributeIt, firstValue, secondValueAttributeIt, secondValue, nextAttributeIt);
    }
  return noise;
}

unsigned int Trie::noiseSumOnPresentAndPotential(const vector<Attribute*>::const_iterator firstValueAttributeIt, const Value& firstValue, const vector<Attribute*>::const_iterator secondValueAttributeIt, const Value& secondValue, const vector<Attribute*>::const_iterator attributeIt) const
{
  const vector<Attribute*>::const_iterator nextAttributeIt = attributeIt + 1;
  if (attributeIt == firstValueAttributeIt)
    {
      return hyperplanes[firstValue.getDataId()]->noiseSumOnPresentAndPotential(secondValueAttributeIt, secondValue, nextAttributeIt);
    }
  unsigned int noise = 0;
  const vector<Value*>::const_iterator end = (*attributeIt)->irrelevantEnd();
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != end; ++valueIt)
    {
      noise += static_cast<Trie*>(hyperplanes[(*valueIt)->getDataId()])->noiseSumOnPresentAndPotential(firstValueAttributeIt, firstValue, secondValueAttributeIt, secondValue, nextAttributeIt);
    }
  return noise;
}
#endif

vector<vector<vector<unsigned int>>::iterator> Trie::incrementIterators(const vector<vector<vector<unsigned int>>::iterator>& iterators)
{
  vector<vector<vector<unsigned int>>::iterator> nextIterators;
  nextIterators.reserve(iterators.size());
  for (const vector<vector<unsigned int>>::iterator it : iterators)
    {
      nextIterators.push_back(it + 1);
    }
  return nextIterators;
}
