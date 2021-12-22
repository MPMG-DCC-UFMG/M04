// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef ATTRIBUTE_H_
#define ATTRIBUTE_H_

#include <string>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/math/special_functions/binomial.hpp>

#include "Value.h"

#if defined DEBUG || defined VERBOSE_ELEMENT_CHOICE
#include <iostream>
#endif

using namespace boost;

class Attribute
{
 public:
  /* /\* debug *\/ */
  /* void printPresentAndPotentialIntersectionsWithEnumeratedValueInNextAttribute(const Attribute& nextAttribute) const; */

  static unsigned int noisePerUnit; /* 1 if the data is crisp */

#ifdef DEBUG
  void printPresent(ostream& out) const;
  void printPotential(ostream& out) const;
  void printAbsent(ostream& out) const;
  void printChosenValue(ostream& out) const;
#endif
#ifdef ASSERT
  void printValue(const Value& value, ostream& out) const;
#endif

#if defined DEBUG || defined VERBOSE_ELEMENT_CHOICE || defined ASSERT
  static void setInternal2ExternalAttributeOrder(const vector<unsigned int>& internal2ExternalAttributeOrder);
#endif

  Attribute();
  Attribute(Attribute&& otherAttribute) = delete;
  Attribute(const vector<unsigned int>& nbOfValuesPerAttribute, const double epsilon, const unsigned int minSize, const vector<string>& labels);
  virtual ~Attribute();

  virtual Attribute* clone() const;
  virtual void subtractSelfLoopsFromPotentialNoise(const unsigned int totalMembershipDueToSelfLoopsOnASymmetricValue, const unsigned int nbOfSymmetricElements);
  void setPresentIntersections(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const unsigned int presentAttributeId);
  void setPresentAndPotentialIntersections(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator attributeIt, const vector<Attribute*>::const_iterator attributeEnd);

  Attribute& operator=(const Attribute& otherAttribute) = delete;
  Attribute& operator=(Attribute&& otherAttribute) = delete;
  friend ostream& operator<<(ostream& out, const Attribute& attribute);
  void printValueFromDataId(const unsigned int valueDataId, ostream& out) const;

  unsigned int getId() const;
  bool closedAttribute() const;
  virtual bool symmetric() const;
  virtual bool metric() const;
  vector<unsigned int> getPresentAndPotentialDataIds() const;
  vector<unsigned int> getIrrelevantDataIds() const;
  vector<Value*>::const_iterator presentBegin() const;
  vector<Value*>::const_iterator presentEnd() const;
  vector<Value*>::const_iterator potentialBegin() const;
  vector<Value*>::const_iterator potentialEnd() const;
  vector<Value*>::const_iterator irrelevantBegin() const;
  vector<Value*>::const_iterator irrelevantEnd() const;
  vector<Value*>::const_iterator absentBegin() const;
  vector<Value*>::const_iterator absentEnd() const;
  vector<Value*>::iterator presentBegin();
  vector<Value*>::iterator presentEnd();
  vector<Value*>::iterator potentialBegin();
  vector<Value*>::iterator potentialEnd();
  vector<Value*>::iterator irrelevantBegin();
  vector<Value*>::iterator irrelevantEnd();
  vector<Value*>::iterator absentBegin();
  vector<Value*>::iterator absentEnd();
  unsigned int sizeOfPresent() const;
  unsigned int sizeOfPotential() const;
  unsigned int sizeOfPresentAndPotential() const;
  bool irrelevantEmpty() const;
  unsigned int globalSize() const;
  double totalPresentAndPotentialNoise() const;
  double averagePresentAndPotentialNoise() const;

  virtual void sortPotentialAndAbsentButChosenPresentValueIfNecessary(const unsigned int presentAttributeId);
  virtual void sortPotentialIrrelevantAndAbsentButChosenAbsentValueIfNecessary(const unsigned int absentAttributeId);
  virtual void sortPotentialIrrelevantAndAbsentIfNecessary(const unsigned int absentAttributeId);

  unsigned int getChosenValueDataId() const; /* to be called after chooseValue (on the same attribute or on its child) */
  Value& getChosenValue() const; /* to be called after chooseValue (on the same attribute or on its child) */
  virtual void repositionChosenPresentValue();	/* to be called after setPresentIntersections (on the same object) */
  virtual void setChosenValuePresent();
  virtual vector<unsigned int> setChosenValueIrrelevant(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd); /* to be called after chooseValue (on the same object) */
  unsigned int getChosenAbsentValueDataId() const; /* to be called after setLastIrrelevantValueChosen (on the same object) */
  Value& getChosenAbsentValue() const; /* to be called after setLastIrrelevantValueChosen (on the same object) */
  void setNullPresentAndPotentialIntersectionsWithNewAbsentValue(const unsigned int intersectionIndex);
  virtual void setLastIrrelevantValueChosen(const vector<Attribute*>::const_iterator thisAttributeIt, const vector<Attribute*>::const_iterator attributeEnd); /* to be called after setChosenValueIrrelevant (on the same object) */
  void setPotentialValueIrrelevant(const vector<Value*>::iterator potentialValueIt);
  vector<unsigned int> eraseIrrelevantValues(); /* returns the sorted data ids of the erased elements */

  // WARNING: These two methods should be called after initialization only (all values ordered in potential)
  vector<vector<unsigned int>>::iterator getIntersectionsBeginWithPotentialValues(const unsigned int valueId);
  void decrementPotentialNoise(const unsigned int valueId);
  void subtractPotentialNoise(const unsigned int valueId, const unsigned int noise);

  virtual bool findIrrelevantValuesAndCheckTauContiguity(const vector<Attribute*>::iterator attributeBegin, const vector<Attribute*>::iterator attributeEnd);

#ifdef MIN_SIZE_ELEMENT_PRUNING
  void resetPresentAndPotentialIrrelevancyThreshold();
  bool possiblePresentAndPotentialIrrelevancy(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd);
  bool presentAndPotentialIrrelevant() const;
  virtual pair<bool, vector<unsigned int>> findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity();
  virtual void presentAndPotentialCleanAbsent();
#ifdef PRE_PROCESS
  void setPresentAndPotentialIrrelevancyThreshold(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd);
#endif
#ifdef MIN_AREA_REFINEMENT
  virtual void computeMinNbOfNonSelfLoopsWithSymmetricAttributes(const vector<unsigned int>::const_reverse_iterator minSizeIt, const vector<float>::const_reverse_iterator minAreaDividedByProductOfSubsequentMinSizesIt, const vector<Attribute*>::const_iterator thisIt, const vector<Attribute*>::const_iterator attributeEnd, const unsigned int nbOfSymmetricElements, const unsigned int currentArea, unsigned int& minArea) const;
  virtual void computeMinNbOfNonSelfLoopsOnASymmetricValue(const vector<unsigned int>::const_reverse_iterator minSizeIt, const vector<float>::const_reverse_iterator minAreaDividedByProductOfSubsequentMinSizesIt, const vector<Attribute*>::const_iterator thisIt, const vector<Attribute*>::const_iterator attributeEnd, const unsigned int nbOfSymmetricElements, const unsigned int currentArea, unsigned int& minArea) const;
#endif
#endif

  virtual bool unclosed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
  virtual void cleanAndSortAbsent(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd);
  void sortPotential();
  virtual void sortPotentialAndAbsentButChosenValue(const unsigned int presentAttributeId);
  virtual bool finalizable() const;
  virtual vector<unsigned int> finalize() const; /* returns the original ids of the elements moved to present */

  static unsigned int getMinSize(const unsigned int attributeId);
  static unsigned int getMinArea();
  static void setMinAreaIsClosedVectorAndIsStorageAllDense(const unsigned int minArea, const vector<bool>& isClosedVector, const bool isStorageAllDense);
  static void setIsAnAttributeSymmetric();
  static void setOutputFormat(const char* outputElementSeparator, const char* emptySetString, const char* elementNoiseSeparator, const bool isNoisePrinted);

  static unsigned int lastAttributeId();
  static const vector<unsigned int>& getEpsilonVector();
  static Attribute* chooseValue(const vector<Attribute*>& attributes);

  static void printOutputElementSeparator(ostream& out);
  static void printEmptySetString(ostream& out);

  static bool lessAppealingIrrelevant(const Attribute* attribute, const Attribute* otherAttribute);

 protected:
  const unsigned int id;
  vector<Value*> values;	/* present, then potential, then irrelevant, then absent */
  unsigned int potentialIndex;
  unsigned int irrelevantIndex;
  unsigned int absentIndex;
  unsigned int presentAndPotentialIrrelevancyThreshold;

  static unsigned int maxId;
  static unsigned int orderedAttributeId;
  static vector<unsigned int> epsilonVector;
  static vector<vector<string>> labelsVector;
  static unsigned int minArea;

#if defined DEBUG || defined VERBOSE_ELEMENT_CHOICE || defined ASSERT
  static vector<unsigned int> internal2ExternalAttributeOrder;
#endif

  Attribute(const Attribute& parentAttribute);

  void sortPotentialAndAbsent(const unsigned int realPotentialIndex);
  void sortPotentialIrrelevantAndAbsent(const unsigned int realAbsentIndex);
  void repositionChosenPresentValueInOrderedAttribute();

  bool valueDoesNotExtendPresent(const Value& value, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
  bool valueDoesNotExtendPresentAndPotential(const Value& value, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
  void removeAbsentValue(vector<Value*>::iterator valueIt);

#ifdef MIN_SIZE_ELEMENT_PRUNING
  bool presentAndPotentialIrrelevantValue(const Value& value) const;
  unsigned int minSizeIrrelevancyThresholdIgnoringSymmetry(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
#endif

 private:
  unsigned int maxNbOfPresentAndPotentiaValuesEver;
  unsigned int maxNbOfAbsentValuesEver;

  static vector<bool> isClosedVector;
  static vector<unsigned int> minSizes;
  static bool isAnAttributeSymmetric;

  static string outputElementSeparator;
  static string emptySetString;
  static string elementNoiseSeparator;
  static bool isNoisePrinted;

  virtual bool isEnumeratedAttribute(const unsigned int enumeratedAttributeId) const;
  void printValues(const vector<Value*>::const_iterator begin, const vector<Value*>::const_iterator end, ostream& out) const;

  virtual Attribute* thisOrFirstSymmetricAttribute();

  Value* createPresentChildValue(const Value& parentValue, const unsigned int newId, const vector<Attribute*>::const_iterator parentNextAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const unsigned int presentAttributeId) const;
  void setPresentChildValuePresentIntersections(const vector<unsigned int>& noiseInIntersectionWithPresentValues, vector<unsigned int>& childNoiseInIntersectionWithPresentValues, const unsigned int presentAttributeId) const;
  Value* createChosenValue(const Value& parentValue, const unsigned int newId, const vector<Attribute*>::const_iterator parentNextAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd) const;
  void setChosenChildValuePresentIntersections(vector<unsigned int>& childNoiseInIntersectionWithPresentValues) const;
  Value* createPotentialOrAbsentChildValue(const Value& parentValue, const unsigned int newId, const vector<Attribute*>::const_iterator parentNextAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const unsigned int presentAttributeId) const;
  void setPotentialOrAbsentChildValuePresentIntersections(const vector<unsigned int>& noiseInIntersectionWithPresentAndPotentialValues, vector<unsigned int>& childNoiseInIntersectionWithPresentAndPotentialValues, const unsigned int presentAttributeId) const;
  void setPresentAndPotentialIntersectionsWithPresentAndPotentialValues(const Attribute& parentAttribute, const vector<unsigned int>& parentNoiseInIntersectionWithPresentAndPotentialValues, vector<unsigned int>& noiseInIntersectionWithPresentAndPotentialValues) const;
  void setPresentAndPotentialIntersectionsWithAbsentValues(const Attribute& parentAttribute, const vector<unsigned int>& parentNoiseInIntersectionWithPresentAndPotentialValues, vector<unsigned int>& noiseInIntersectionWithPresentAndPotentialValues) const;

#ifdef TWO_MODE_ELEMENT_CHOICE
  void findBetterEnumerationStrategy(const vector<Attribute*>& attributes, const vector<unsigned int>& factorization, const bool isAnElementTakenInThis, double& bestCost, Attribute*& bestAttribute) const;
  void multiplicativePartition(const double number, const unsigned int nbOfSubsequentFactors, const unsigned int begin, vector<Attribute*>& attributes, vector<unsigned int>& factorization, double& bestCost, Attribute*& bestAttribute) const;
#endif
  virtual unsigned int getIndexOfValueToChoose(const double presentCoeff, const double presentAndPotentialCoeff, double& bestAppeal) const;
  virtual unsigned int getIndexOfValueToChoose(const vector<Attribute*>& attributes, const Attribute* symmetricAttribute, double& bestAppeal) const; /* returns the appeal and the index of the best value to enumerate */
  virtual void chooseValue(const unsigned int indexOfValue);
  virtual vector<unsigned int> setChosenValueIrrelevant(); /* to be called after chooseValue (on the same object) */

#ifdef MIN_SIZE_ELEMENT_PRUNING
  unsigned int minSizeIrrelevancyThreshold(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
#ifdef MIN_AREA_REFINEMENT
  void computeMinArea(const vector<unsigned int>::const_reverse_iterator minSizeIt, const vector<float>::const_reverse_iterator minAreaDividedByProductOfSubsequentMinSizesIt, const vector<Attribute*>::const_iterator thisIt, const vector<Attribute*>::const_iterator attributeEnd, const unsigned int currentArea, unsigned int& minArea) const;
#endif
#endif
};

#endif /*ATTRIBUTE_H_*/
