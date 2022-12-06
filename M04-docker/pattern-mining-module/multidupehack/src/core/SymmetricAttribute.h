// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef SYMMETRIC_ATTRIBUTE_H_
#define SYMMETRIC_ATTRIBUTE_H_

#include "Attribute.h"

/* Although the code in Tree works for any number of symmetric attributes, this implementation is specific to two such attributes */
class SymmetricAttribute final: public Attribute
{
 public:
  SymmetricAttribute();
  SymmetricAttribute(Attribute&& otherSymmetricAttribute) = delete;
  SymmetricAttribute(const vector<unsigned int>& nbOfValuesPerAttribute, const double epsilon, const unsigned int minSize, const vector<string>& labels);

  SymmetricAttribute* clone() const;

  SymmetricAttribute& operator=(const SymmetricAttribute& otherSymmetricAttribute) = delete;
  SymmetricAttribute& operator=(SymmetricAttribute&& otherSymmetricAttribute) = delete;

  void sortPotentialAndAbsentButChosenPresentValueIfNecessary(const unsigned int presentAttributeId);
  void sortPotentialIrrelevantAndAbsentButChosenAbsentValueIfNecessary(const unsigned int absentAttributeId);
  void sortPotentialIrrelevantAndAbsentIfNecessary(const unsigned int absentAttributeId);

  void setSymmetricAttribute(SymmetricAttribute* symmetricAttribute);
  bool symmetric() const;
  unsigned int getSymmetricAttributeId() const;

  void subtractSelfLoopsFromPotentialNoise(const unsigned int totalMembershipDueToSelfLoopsOnASymmetricValue, const unsigned int nbOfSymmetricElements);
  void repositionChosenPresentValue();
  void setChosenValuePresent();
  vector<unsigned int> setChosenValueIrrelevant(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd); /* to be called after chooseValue (on the same object) */
  void setLastIrrelevantValueChosen(const vector<Attribute*>::const_iterator thisAttributeIt, const vector<Attribute*>::const_iterator attributeEnd); /* to be called after setChosenValueIrrelevant (on the same object) */

  bool findIrrelevantValuesAndCheckTauContiguity(const vector<Attribute*>::iterator attributeBegin, const vector<Attribute*>::iterator attributeEnd);

#ifdef MIN_SIZE_ELEMENT_PRUNING
  pair<bool, vector<unsigned int>> findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity();
  void presentAndPotentialCleanAbsent(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd);
#ifdef MIN_AREA_REFINEMENT
  void computeMinNbOfNonSelfLoopsWithSymmetricAttributes(const vector<unsigned int>::const_reverse_iterator minSizeIt, const vector<float>::const_reverse_iterator minAreaDividedByProductOfSubsequentMinSizesIt, const vector<Attribute*>::const_iterator thisIt, const vector<Attribute*>::const_iterator attributeEnd, const unsigned int nbOfSymmetricElements, const unsigned int currentArea, unsigned int& minArea) const;
  void computeMinNbOfNonSelfLoopsOnASymmetricValue(const vector<unsigned int>::const_reverse_iterator minSizeIt, const vector<float>::const_reverse_iterator minAreaDividedByProductOfSubsequentMinSizesIt, const vector<Attribute*>::const_iterator thisIt, const vector<Attribute*>::const_iterator attributeEnd, const unsigned int nbOfSymmetricElements, const unsigned int currentArea, unsigned int& minArea) const;
#endif
#endif

  bool unclosed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
  void cleanAndSortAbsent(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd);

 private:
  SymmetricAttribute* symmetricAttribute;

  SymmetricAttribute(const SymmetricAttribute& parentAttribute);

  bool isEnumeratedAttribute(const unsigned int enumeratedAttributeId) const;

  SymmetricAttribute* thisOrFirstSymmetricAttribute();

  unsigned int getIndexOfValueToChoose(const double presentCoeff, const double presentAndPotentialCoeff, double& bestAppeal) const;
  unsigned int getIndexOfValueToChoose(const vector<Attribute*>& attributes, const Attribute* firstSymmetricAttribute, double& bestAppeal) const; /* returns the appeal and the index of the best value to enumerate */
  void chooseValue(const unsigned int indexOfValue);

  bool symmetricValuesDoNotExtendPresent(const Value& value, const Value& symmetricValue, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const; /* must only be called on the first symmetric attribute */
  bool symmetricValuesDoNotExtendPresentAndPotential(const Value& value, const Value& symmetricValue, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const; /* must only be called on the first symmetric attribute */

#ifdef MIN_SIZE_ELEMENT_PRUNING
  void presentAndPotentialCleanAbsent(const unsigned int threshold);
#endif
};

#endif /*SYMMETRIC_ATTRIBUTE_H_*/
