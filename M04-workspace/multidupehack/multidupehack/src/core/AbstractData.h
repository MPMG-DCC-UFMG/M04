// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef ABSTRACT_DATA_H_
#define ABSTRACT_DATA_H_

#include <ostream>

#include "Attribute.h"

class AbstractData
{
 public:
  virtual ~AbstractData();
  virtual AbstractData* clone() const = 0;

  virtual bool isATrie() const;
  virtual bool isFullSparseTube() const;

  virtual void print(vector<unsigned int>& prefix, ostream& out) const = 0;
  virtual bool setSelfLoopsInSymmetricAttributes(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const unsigned int dimensionId) = 0;
  virtual void setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<vector<unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt) = 0;
  virtual void sortTubes();

  virtual unsigned int setPresent(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const = 0;
  virtual unsigned int setPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const = 0;
  virtual unsigned int setSymmetricPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const = 0;
  virtual unsigned int setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const = 0;
  virtual unsigned int setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const = 0;
  virtual unsigned int setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const = 0;
  virtual unsigned int setIrrelevant(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& irrelevantValueDataIds, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const = 0;
  virtual unsigned int setAbsentAfterAbsentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const = 0;
  virtual unsigned int setIrrelevantAfterIrrelevantValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const = 0;
  virtual unsigned int setSymmetricAbsentAfterAbsentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const = 0;
  virtual unsigned int setAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const = 0;
  virtual unsigned int setIrrelevantAfterAbsentUsed(const vector<Attribute*>::iterator irrelevantAttributeIt, const vector<unsigned int>& irrelevantValueDataIds, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const = 0;
  virtual unsigned int setAbsentAfterAbsentValueMetAndAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const = 0;

  virtual unsigned int noiseSum(const vector<vector<unsigned int>>::const_iterator dimensionIt) const = 0;

#ifdef ASSERT
  virtual unsigned int noiseSumOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const = 0;
  virtual unsigned int noiseSumOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const = 0;
#endif
};

#endif /*ABSTRACT_DATA_H_*/
