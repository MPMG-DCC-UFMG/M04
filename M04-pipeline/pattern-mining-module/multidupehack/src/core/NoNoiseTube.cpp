// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "NoNoiseTube.h"

NoNoiseTube* NoNoiseTube::clone() const
{
  return new NoNoiseTube(*this);
}

void NoNoiseTube::print(vector<unsigned int>& prefix, ostream& out) const
{
  for (const unsigned int id : prefix)
    {
      out << id << ' ';
    }
  out << "- 1\n";
}

bool NoNoiseTube::setSelfLoopsInSymmetricAttributes(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const unsigned int dimensionId)
{
  // Never called
  return false;
}

void NoNoiseTube::setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<vector<unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt)
{
  // Never called
}

unsigned int NoNoiseTube::setPresent(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  return 0;
}

unsigned int NoNoiseTube::setPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  return 0;
}

unsigned int NoNoiseTube::setSymmetricPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  return 0;
}

unsigned int NoNoiseTube::setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  return 0;
}

unsigned int NoNoiseTube::setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  return 0;
}

unsigned int NoNoiseTube::setSymmetricPresentAfterPresentValueMetAndPotentialOrAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  // Never called
  return 0;
}

unsigned int NoNoiseTube::setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  return 0;
}

unsigned int NoNoiseTube::setIrrelevant(const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueIds, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  return 0;
}

unsigned int NoNoiseTube::setAbsentAfterAbsentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  return 0;
}

unsigned int NoNoiseTube::setIrrelevantAfterIrrelevantValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  return 0;
}

unsigned int NoNoiseTube::setSymmetricAbsentAfterAbsentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  // Never called
  return 0;
}

unsigned int NoNoiseTube::setAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  return 0;
}

unsigned int NoNoiseTube::setIrrelevantAfterAbsentUsed(const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueIds, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator irrelevantValueIntersectionIt) const
{
  return 0;
}

unsigned int NoNoiseTube::setAbsentAfterAbsentValueMetAndAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  return 0;
}

unsigned int NoNoiseTube::setIrrelevantAfterIrrelevantValuesMetAndAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator irrelevantValueIntersectionIt) const
{
  return 0;
}

unsigned int NoNoiseTube::setSymmetricAbsentAfterAbsentValueMetAndAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  // Never called
  return 0;
}

unsigned int NoNoiseTube::noiseSum(const vector<vector<unsigned int>>::const_iterator dimensionIt) const
{
  return 0;
}

#ifdef ASSERT
unsigned int NoNoiseTube::noiseSumOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  return 0;
}

unsigned int NoNoiseTube::noiseSumOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  return 0;
}
#endif
