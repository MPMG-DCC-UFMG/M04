// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef NO_NOISE_TUBE_H_
#define NO_NOISE_TUBE_H_

#include "AbstractData.h"

class NoNoiseTube final : public AbstractData
{
 public:
  void print(vector<unsigned int>& ids, ostream& out) const;
  bool setSelfLoopsInSymmetricAttributes(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const unsigned int dimensionId);
  void setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<vector<unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt);

  unsigned int setPresent(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  unsigned int setPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  unsigned int setSymmetricPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  unsigned int setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const;
  unsigned int setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const;
  unsigned int setSymmetricPresentAfterPresentValueMetAndPotentialOrAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const;
  unsigned int setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  unsigned int setIrrelevant(const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueIds, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  unsigned int setAbsentAfterAbsentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const;
  unsigned int setIrrelevantAfterIrrelevantValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  unsigned int setSymmetricAbsentAfterAbsentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const;
  unsigned int setAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const;
  unsigned int setIrrelevantAfterAbsentUsed(const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueIds, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const;
  unsigned int setAbsentAfterAbsentValueMetAndAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const;
  unsigned int setIrrelevantAfterIrrelevantValuesMetAndAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const;
  unsigned int setSymmetricAbsentAfterAbsentValueMetAndAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const;

  unsigned int noiseSum(const vector<vector<unsigned int>>::const_iterator dimensionIt) const;

#ifdef ASSERT
  unsigned int noiseSumOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const;
  unsigned int noiseSumOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const;
#endif

 private:
  NoNoiseTube* clone() const;
};

#endif /*NO_NOISE_TUBE_H_*/
