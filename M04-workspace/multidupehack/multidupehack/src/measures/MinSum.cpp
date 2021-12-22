// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "MinSum.h"

vector<unsigned int> MinSum::dimensionIds;
bool MinSum::isValuesAllPositive = true;
bool MinSum::isValuesAllNegative = true;
AbstractTupleValueData* MinSum::positiveTupleValues;
AbstractTupleValueData* MinSum::negativeTupleValues;
float MinSum::threshold;

MinSum::MinSum(const string& tupleValueFileName, const char* dimensionSeparatorChars, const char* elementSeparatorChars, const vector<unordered_map<string, unsigned int>>& labels2Ids, const vector<unsigned int>& dimensionOrder, const vector<unsigned int>& cardinalities, const float densityThreshold, const float thresholdParam): TupleMeasure(dimensionOrder.size()), minSum(0)
{
  ifstream tupleValueFile(tupleValueFileName.c_str());
  if (!tupleValueFile)
    {
      throw NoInputException(tupleValueFileName.c_str());
    }
  // Parsing the dimension ids of the tuples
  unsigned int lineNb = 0;
  vector<unsigned int> internalDimensionIds;
  internalDimensionIds.reserve(dimensionOrder.size());
  while (internalDimensionIds.empty() && !tupleValueFile.eof())
    {
      ++lineNb;
      string dataString;
      getline(tupleValueFile, dataString);
#ifdef VERBOSE_PARSER
      cout << tupleValueFileName << ':' << lineNb << ": " << dataString << '\n';
#endif
      unsigned int dimensionId;
      istringstream ss(dataString);
      while (ss >> dimensionId)
	{
	  if (dimensionId >= dimensionOrder.size())
	    {
	      throw DataFormatException(tupleValueFileName.c_str(), lineNb, ("dimension " + lexical_cast<string>(dimensionId) + " does not exist! (at most " + lexical_cast<string>(dimensionOrder.size() - 1) + " expected)").c_str());
	    }
	  internalDimensionIds.push_back(dimensionOrder[dimensionId]);
	}
    }
  if (tupleValueFile.eof())
    {
      throw DataFormatException(tupleValueFileName.c_str(), lineNb, "no dimension ids found!");
    }
  dimensionIds = internalDimensionIds;
  sort(dimensionIds.begin(), dimensionIds.end());
  vector<unsigned int>::const_iterator dimensionIdIt = dimensionIds.begin();
  while (++dimensionIdIt != dimensionIds.end() && *dimensionIdIt != *(dimensionIdIt - 1))
    {
    }
  if (dimensionIdIt != dimensionIds.end())
    {
      throw DataFormatException(tupleValueFileName.c_str(), lineNb, "dimensions must all be different!");
    }
  TupleValueDenseTube::setSize(cardinalities[dimensionIds.back()]);
  TupleValueSparseTube::setSizeLimit(cardinalities[dimensionIds.back()] * densityThreshold * sizeof(double) / (sizeof(unsigned int) + sizeof(double)));
  // Compute the tuple dimension ids, indicate the dimensions as relevant to the measure, which is monotone (and anti-monotone if there are negative values)
  vector<unsigned int> tupleDimensionIds;
  tupleDimensionIds.reserve(internalDimensionIds.size());
  for (const unsigned int internalDimensionId : internalDimensionIds)
    {
      unsigned int tupleDimensionId = 0;
      for (vector<unsigned int>::const_iterator dimensionIdIt = dimensionIds.begin(); *dimensionIdIt != internalDimensionId; ++dimensionIdIt)
	{
	  ++tupleDimensionId;
	}
      tupleDimensionIds.push_back(tupleDimensionId);
    }
  // Parsing the tuple values
  const char_separator<char> elementSeparator(elementSeparatorChars);
  const char_separator<char> dimensionSeparator(dimensionSeparatorChars);
  while (!tupleValueFile.eof())
    {
      ++lineNb;
      string dataString;
      getline(tupleValueFile, dataString);
#ifdef VERBOSE_PARSER
      cout << tupleValueFileName << ':' << lineNb << ": " << dataString << '\n';
#endif
      tokenizer<char_separator<char>> tupleValue(dataString, dimensionSeparator);
      tokenizer<char_separator<char>>::const_iterator tupleValueIt = tupleValue.begin();
      if (tupleValueIt != tupleValue.end())
	{
	  // Parsing the n-set
	  vector<vector<unsigned int>> nSet(tupleDimensionIds.size());
	  bool isCoverEmpty = false;
	  vector<unsigned int>::const_iterator tupleDimensionIdIt = tupleDimensionIds.begin();
	  for (const unsigned int internalDimensionId : internalDimensionIds)
	    {
	      vector<unsigned int>& elements = nSet[*tupleDimensionIdIt++];
	      const unordered_map<string, unsigned int>& labels2IdsView = labels2Ids[internalDimensionId];
	      for (const string& element : tokenizer<char_separator<char>>(*tupleValueIt, elementSeparator))
		{
		  const unordered_map<string, unsigned int>::const_iterator labels2IdsViewIt = labels2IdsView.find(element);
		  if (labels2IdsViewIt == labels2IdsView.end())
		    {
		      cerr << "Warning: ignoring " << element << " at line " << lineNb << " of " << tupleValueFileName << " because it is absent from the corresponding dimension of the input data\n";
		    }
		  else
		    {
		      if (labels2IdsViewIt->second != numeric_limits<unsigned int>::max())
			{
			  elements.push_back(labels2IdsViewIt->second);
			}
		    }
		}
	      if (++tupleValueIt == tupleValue.end())
		{
		  delete positiveTupleValues;
		  delete negativeTupleValues;
		  throw DataFormatException(tupleValueFileName.c_str(), lineNb, ("less than the expected " + lexical_cast<string>(dimensionIds.size() + 2) + " dimensions (a " + lexical_cast<string>(dimensionIds.size()) + "-set followed by a value)!").c_str());
		}
	      if (elements.empty())
		{
		  isCoverEmpty = true;
		}
	    }
	  // Parsing the value
          double value;
	  try
	    {
	      value = lexical_cast<double>(*tupleValueIt);
	    }
	  catch (bad_lexical_cast &)
	    {
	      delete positiveTupleValues;
	      delete negativeTupleValues;
	      throw DataFormatException(tupleValueFileName.c_str(), lineNb, (*tupleValueIt + " should be a double!").c_str());
	    }
	  if (!isfinite(value))
	    {
	      delete positiveTupleValues;
	      delete negativeTupleValues;
	      throw DataFormatException(tupleValueFileName.c_str(), lineNb, "value must be finite!");
	    }
          if (++tupleValueIt != tupleValue.end())
	    {
	      delete positiveTupleValues;
	      delete negativeTupleValues;
	      throw DataFormatException(tupleValueFileName.c_str(), lineNb, ("more than the expected " + lexical_cast<string>(dimensionIds.size() + 2) + " dimensions (a " + lexical_cast<string>(dimensionIds.size()) + "-set followed by a value)!").c_str());
	    }
	  if (!isCoverEmpty && value != 0)
	    {
	      // Storing the tuple values
	      if (value < 0)
		{
		  if (isValuesAllPositive)
		    {
		      isValuesAllPositive = false;
		      negativeTupleValues = initializeTupleValues(cardinalities);
		    }
		  if (negativeTupleValues->isFullSparseTube(nSet.front()))
		    {
		      TupleValueDenseTube* newTupleValues = static_cast<TupleValueSparseTube*>(negativeTupleValues)->getDenseRepresentation();
		      delete negativeTupleValues;
		      negativeTupleValues = newTupleValues;
		    }
		  negativeTupleValues->setTupleValues(nSet.begin(), -value);
		}
	      else
		{
		  if (isValuesAllNegative)
		    {
		      isValuesAllNegative = false;
		      positiveTupleValues = initializeTupleValues(cardinalities);
		    }
		  if (positiveTupleValues->isFullSparseTube(nSet.front()))
		    {
		      TupleValueDenseTube* newTupleValues = static_cast<TupleValueSparseTube*>(positiveTupleValues)->getDenseRepresentation();
		      delete positiveTupleValues;
		      positiveTupleValues = newTupleValues;
		    }
		  positiveTupleValues->setTupleValues(nSet.begin(), value);
		}
	    }
	}
    }
  if (!isValuesAllPositive)
    {
      negativeTupleValues->sortTubes();
      for (const unsigned int internalDimensionId : internalDimensionIds)
	{
	  relevantDimensionsForAntiMonotoneMeasures[internalDimensionId] = true;
	}
    }
  if (!isValuesAllNegative)
    {
      positiveTupleValues->sortTubesAndSetSum(minSum);
      for (const unsigned int internalDimensionId : internalDimensionIds)
	{
	  relevantDimensionsForMonotoneMeasures[internalDimensionId] = true;
	}
    }
  threshold = thresholdParam;
}

MinSum* MinSum::clone() const
{
  return new MinSum(*this);
}

bool MinSum::monotone() const
{
  return isValuesAllPositive;
}

bool MinSum::violationAfterPresentIncreased(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent)
{
  if (isValuesAllPositive)
    {
      return false;
    }
  // Test whether dimensionIdOfElementsSetPresent is a dimension of the tuples
  vector<unsigned int>::const_iterator dimensionIdIt = dimensionIds.begin();
  for (; dimensionIdIt != dimensionIds.end() && *dimensionIdIt < dimensionIdOfElementsSetPresent; ++dimensionIdIt)
    {
    }
  if (dimensionIdIt == dimensionIds.end() || *dimensionIdIt != dimensionIdOfElementsSetPresent)
    {
      return false;
    }
  // dimensionIdOfElementsSetPresent is a dimension of the tuples, update the sum
  negativeTupleValues->decreaseSum(present(), elementsSetPresent, dimensionIds.begin(), dimensionIdIt, minSum);
#ifdef DEBUG
  if (minSum < threshold)
    {
      cout << threshold << "-minimal sum constraint cannot be satisfied -> Prune!\n";
    }
#endif
  return minSum < threshold;
}

bool MinSum::violationAfterPresentAndPotentialDecreased(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent)
{
  if (isValuesAllNegative)
    {
      return false;
    }
  // Test whether dimensionIdOfElementsSetAbsent is a dimension of the tuples
  vector<unsigned int>::const_iterator dimensionIdIt = dimensionIds.begin();
  for (; dimensionIdIt != dimensionIds.end() && *dimensionIdIt < dimensionIdOfElementsSetAbsent; ++dimensionIdIt)
    {
    }
  if (dimensionIdIt == dimensionIds.end() || *dimensionIdIt != dimensionIdOfElementsSetAbsent)
    {
      return false;
    }
  // dimensionIdOfElementsSetAbsent is a dimension of the tuples, update the sum
  positiveTupleValues->decreaseSum(presentAndPotential(), elementsSetAbsent, dimensionIds.begin(), dimensionIdIt, minSum);
#ifdef DEBUG
  if (minSum < threshold)
    {
      cout << threshold << "-minimal sum constraint cannot be satisfied -> Prune!\n";
    }
#endif
  return minSum < threshold;
}

float MinSum::optimisticValue() const
{
  return minSum;
}

void MinSum::deleteTupleValues()
{
  if (!dimensionIds.empty())
    {
      delete positiveTupleValues;
      delete negativeTupleValues;
    }
}

AbstractTupleValueData* MinSum::initializeTupleValues(const vector<unsigned int>& cardinalities)
{
  if (dimensionIds.size() == 1)
    {
      return new TupleValueSparseTube();
    }
  return new TupleValueTrie(dimensionIds.begin(), dimensionIds.end(), cardinalities);
}
