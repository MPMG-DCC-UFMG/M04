// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "Tube.h"

Tube::~Tube()
{
}

bool Tube::setSelfLoopsInSymmetricAttributes(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const unsigned int dimensionId)
{
  // Never called because self loops are inserted first, the tubes are initially sparse and the method is overridden in those classes
  return false;
}

unsigned int Tube::setPresent(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  // *this necessarily relates to the present attribute
  return noiseOnValue((*attributeIt)->getChosenValueDataId());
}

unsigned int Tube::setPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  presentFixPotentialOrAbsentValuesAfterPresentValueMet(**attributeIt, intersectionIts);
  return presentFixPresentValuesAfterPresentValueMet(**attributeIt);
}

unsigned int Tube::setSymmetricPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  presentFixPotentialOrAbsentValuesInSecondSymmetricAttributeAfterPresentValueMet(**attributeIt, intersectionIts);
  return presentFixPresentValuesAfterPresentValueMet(**attributeIt);
}

unsigned int Tube::setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  return presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(**attributeIt, potentialOrAbsentValueIntersectionIt->begin());
}

unsigned int Tube::setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  // *this necessarily relates to the absent attribute
  const unsigned int oldNoiseInHyperplane = noiseOnValue((*absentAttributeIt)->getChosenAbsentValueDataId());
  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
    {
      intersectionIt->back() += oldNoiseInHyperplane;
    }
  return oldNoiseInHyperplane;
}

unsigned int Tube::setIrrelevant(const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueDataIds, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  // *this necessarily relates to the irrelevant attribute
  return noiseOnValues(irrelevantValueDataIds);
}

unsigned int Tube::setAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  // *this necessarily relates to the absent attribute
  return noiseOnValue((*absentAttributeIt)->getChosenAbsentValueDataId());
}

unsigned int Tube::setIrrelevantAfterAbsentUsed(const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueDataIds, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  // *this necessarily relates to the irrelevant attribute
  return noiseOnValues(irrelevantValueDataIds);
}

unsigned int Tube::setAbsentAfterAbsentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  absentFixAbsentValuesAfterAbsentValueMet((*attributeIt)->absentBegin(), (*attributeIt)->absentEnd(), intersectionIts);
  return absentFixPresentOrPotentialValuesAfterAbsentValueMet(**attributeIt, *absentValueIntersectionIt);
}

unsigned int Tube::setIrrelevantAfterIrrelevantValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  absentFixAbsentValuesAfterAbsentValueMet((*attributeIt)->absentBegin(), (*attributeIt)->absentEnd(), intersectionIts);
  return irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(**attributeIt);
}

unsigned int Tube::setSymmetricAbsentAfterAbsentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  // The first absent value actually is the value set absent and there is no noise to be found at the intersection of a vertex (seen as an outgoing vertex) and itself (seen as an ingoing vertex)
  absentFixAbsentValuesAfterAbsentValueMet((*attributeIt)->absentBegin() + 1, (*attributeIt)->absentEnd(), intersectionIts);
  return absentFixPresentOrPotentialValuesAfterAbsentValueMet(**attributeIt, *absentValueIntersectionIt);
}

unsigned int Tube::setAbsentAfterAbsentValueMetAndAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  return absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(**attributeIt, *absentValueIntersectionIt);
}

void Tube::printTuple(const vector<unsigned int>& prefix, const unsigned int lastElement, const float membership, ostream& out)
{
  for (const unsigned int id : prefix)
    {
      out << id << ' ';
    }
  out << lastElement << ' ' << membership << '\n';
}
