// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef SPARSE_CRISP_TUBE_H_
#define SPARSE_CRISP_TUBE_H_

#include "DenseCrispTube.h"

class SparseCrispTube final : public Tube
{
 public:
  SparseCrispTube();

  void print(vector<unsigned int>& prefix, ostream& out) const;
  bool isFullSparseTube() const;
  DenseCrispTube* getDenseRepresentation() const;

  bool setSelfLoopsInSymmetricAttributes(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const unsigned int dimensionId);
  void setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<vector<unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt);
  void sortTubes();

  unsigned int setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const;

  unsigned int noiseSum(const vector<vector<unsigned int>>::const_iterator dimensionIt) const;

#ifdef ASSERT
  unsigned int noiseSumOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const;
  unsigned int noiseSumOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const;
#endif

  static void setSizeLimit(const unsigned int sizeLimit);

 private:
  vector<unsigned int> tube;

  static unsigned int sizeLimit;

  SparseCrispTube* clone() const;

  unsigned int noiseOnValue(const unsigned int valueDataId) const;
  unsigned int noiseOnValues(const vector<unsigned int>& valueDataIds) const;

  unsigned int presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute) const;
  unsigned int presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(const Attribute& currentAttribute, const vector<unsigned int>::iterator potentialOrAbsentValueIntersectionBegin) const;
  void presentFixPotentialOrAbsentValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  void presentFixPotentialOrAbsentValuesInSecondSymmetricAttributeAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  void presentFixPotentialOrAbsentValuesAfterPresentValueMet(const vector<Value*>::iterator begin, const vector<Value*>::iterator end, const unsigned int firstValueId, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  unsigned int absentFixPresentOrPotentialValuesAfterAbsentValueMet(Attribute& currentAttribute, vector<unsigned int>& absentValueIntersection) const;
  unsigned int absentFixPresentOrPotentialValuesAfterAbsentValueMet(const vector<Value*>::iterator begin, const vector<Value*>::iterator end, vector<unsigned int>& absentValueIntersection) const;
  unsigned int irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(Attribute& currentAttribute) const;
  unsigned int irrelevantFixPresentOrPotentialValuesAfterIrrelevantValueMet(const vector<Value*>::iterator begin, const vector<Value*>::iterator end) const;
  unsigned int noiseOnIrrelevant(const Attribute& currentAttribute) const;
  unsigned int absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(Attribute& currentAttribute, vector<unsigned int>& absentValueIntersection) const;
  unsigned int absentFixPresentOrPotentialValuesAfterAbsentValueMetAndAbsentUsed(const vector<Value*>::const_iterator begin, const vector<Value*>::const_iterator end, vector<unsigned int>& absentValueIntersection) const;
  void absentFixAbsentValuesAfterAbsentValueMet(const vector<Value*>::iterator absentBegin, const vector<Value*>::iterator absentEnd, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
};

#endif /*SPARSE_CRISP_TUBE_H_*/
