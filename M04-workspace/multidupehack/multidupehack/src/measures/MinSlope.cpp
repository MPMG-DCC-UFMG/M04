// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "MinSlope.h"

vector<unsigned int> MinSlope::dimensionIds;
AbstractTuplePointData* MinSlope::tuplePoints;
float MinSlope::threshold;

MinSlope::MinSlope(const string& tuplePointFileName, const char* dimensionSeparatorChars, const char* elementSeparatorChars, const vector<unordered_map<string, unsigned int>>& labels2Ids, const vector<unsigned int>& dimensionOrder, const vector<unsigned int>& cardinalities, const float densityThreshold, const float thresholdParam): TupleMeasure(dimensionOrder.size()), minSums(), maxSums(), minSlope(numeric_limits<float>::infinity())
{
  ifstream tuplePointFile(tuplePointFileName.c_str());
  if (!tuplePointFile)
    {
      throw NoInputException(tuplePointFileName.c_str());
    }
  // Parsing the dimension ids of the tuples
  unsigned int lineNb = 0;
  vector<unsigned int> internalDimensionIds;
  internalDimensionIds.reserve(dimensionOrder.size());
  while (internalDimensionIds.empty() && !tuplePointFile.eof())
    {
      ++lineNb;
      string dataString;
      getline(tuplePointFile, dataString);
#ifdef VERBOSE_PARSER
      cout << tuplePointFileName << ':' << lineNb << ": " << dataString << '\n';
#endif
      unsigned int dimensionId;
      istringstream ss(dataString);
      while (ss >> dimensionId)
	{
	  if (dimensionId >= dimensionOrder.size())
	    {
	      throw DataFormatException(tuplePointFileName.c_str(), lineNb, ("dimension " + lexical_cast<string>(dimensionId) + " does not exist! (at most " + lexical_cast<string>(dimensionOrder.size() - 1) + " expected)").c_str());
	    }
	  internalDimensionIds.push_back(dimensionOrder[dimensionId]);
	}
    }
  if (tuplePointFile.eof())
    {
      throw DataFormatException(tuplePointFileName.c_str(), lineNb, "no dimension ids found!");
    }
  dimensionIds = internalDimensionIds;
  sort(dimensionIds.begin(), dimensionIds.end());
  vector<unsigned int>::const_iterator dimensionIdIt = dimensionIds.begin();
  while (++dimensionIdIt != dimensionIds.end() && *dimensionIdIt != *(dimensionIdIt - 1))
    {
    }
  if (dimensionIdIt != dimensionIds.end())
    {
      throw DataFormatException(tuplePointFileName.c_str(), lineNb, "dimensions must all be different!");
    }
  TuplePointDenseTube::setSize(cardinalities[dimensionIds.back()]);
  TuplePointSparseTube::setSizeLimit(cardinalities[dimensionIds.back()] * densityThreshold * 2 * sizeof(double) / (sizeof(unsigned int) + 2 * sizeof(double)));
  // Compute the tuple dimension ids, indicate the dimensions as relevant to the measure, which is both monotone and anti-monotone
  vector<unsigned int> tupleDimensionIds;
  tupleDimensionIds.reserve(internalDimensionIds.size());
  for (const unsigned int internalDimensionId : internalDimensionIds)
    {
      relevantDimensionsForMonotoneMeasures[internalDimensionId] = true;
      relevantDimensionsForAntiMonotoneMeasures[internalDimensionId] = true;
      unsigned int tupleDimensionId = 0;
      for (vector<unsigned int>::const_iterator dimensionIdIt = dimensionIds.begin(); *dimensionIdIt != internalDimensionId; ++dimensionIdIt)
	{
	  ++tupleDimensionId;
	}
      tupleDimensionIds.push_back(tupleDimensionId);
    }
  // Initialize tuplePoints
  if (dimensionIds.size() == 1)
    {
      tuplePoints = new TuplePointSparseTube();
    }
  else
    {
      tuplePoints = new TuplePointTrie(dimensionIds.begin(), dimensionIds.end(), cardinalities);
    }
  // Parsing the tuple points
  const char_separator<char> elementSeparator(elementSeparatorChars);
  const char_separator<char> dimensionSeparator(dimensionSeparatorChars);
  while (!tuplePointFile.eof())
    {
      ++lineNb;
      string dataString;
      getline(tuplePointFile, dataString);
#ifdef VERBOSE_PARSER
      cout << tuplePointFileName << ':' << lineNb << ": " << dataString << '\n';
#endif
      tokenizer<char_separator<char>> tuplePoint(dataString, dimensionSeparator);
      tokenizer<char_separator<char>>::const_iterator tuplePointIt = tuplePoint.begin();
      if (tuplePointIt != tuplePoint.end())
	{
	  // Parsing the n-set
	  vector<vector<unsigned int>> nSet(tupleDimensionIds.size());
	  bool isCoverEmpty = false;
	  vector<unsigned int>::const_iterator tupleDimensionIdIt = tupleDimensionIds.begin();
	  for (const unsigned int internalDimensionId : internalDimensionIds)
	    {
	      vector<unsigned int>& elements = nSet[*tupleDimensionIdIt++];
	      const unordered_map<string, unsigned int>& labels2IdsView = labels2Ids[internalDimensionId];
	      for (const string& element : tokenizer<char_separator<char>>(*tuplePointIt, elementSeparator))
		{
		  const unordered_map<string, unsigned int>::const_iterator labels2IdsViewIt = labels2IdsView.find(element);
		  if (labels2IdsViewIt == labels2IdsView.end())
		    {
		      cerr << "Warning: ignoring " << element << " at line " << lineNb << " of " << tuplePointFileName << " because it is absent from the corresponding dimension of the input data\n";
		    }
		  else
		    {
		      if (labels2IdsViewIt->second != numeric_limits<unsigned int>::max())
			{
			  elements.push_back(labels2IdsViewIt->second);
			}
		    }
		}
	      if (++tuplePointIt == tuplePoint.end())
		{
		  delete tuplePoints;
		  throw DataFormatException(tuplePointFileName.c_str(), lineNb, ("less than the expected " + lexical_cast<string>(dimensionIds.size() + 2) + " dimensions (a " + lexical_cast<string>(dimensionIds.size()) + "-set followed by a point)!").c_str());
		}
	      if (elements.empty())
		{
		  isCoverEmpty = true;
		}
	    }
	  // Parsing the point
          pair<double, double> point;
	  try
	    {
	      point.first = lexical_cast<double>(*tuplePointIt);
	    }
	  catch (bad_lexical_cast &)
	    {
	      delete tuplePoints;
	      throw DataFormatException(tuplePointFileName.c_str(), lineNb, (*tuplePointIt + " (abscissa of the point) should be a double!").c_str());
	    }
	  if (!isfinite(point.first))
	    {
	      delete tuplePoints;
	      throw DataFormatException(tuplePointFileName.c_str(), lineNb, "abscissa of the point must be finite!");
	    }
          if (++tuplePointIt == tuplePoint.end())
	    {
	      delete tuplePoints;
	      throw DataFormatException(tuplePointFileName.c_str(), lineNb, ("less than the expected " + lexical_cast<string>(dimensionIds.size() + 2) + " dimensions (a " + lexical_cast<string>(dimensionIds.size()) + "-set followed by a point)!").c_str());
	    }
	  try
	    {
	      point.second = lexical_cast<double>(*tuplePointIt);
	    }
	  catch (bad_lexical_cast &)
	    {
	      delete tuplePoints;
	      throw DataFormatException(tuplePointFileName.c_str(), lineNb, (*tuplePointIt + " (ordinate of the point) should be a double!").c_str());
	    }
	  if (!isfinite(point.second))
	    {
	      delete tuplePoints;
	      throw DataFormatException(tuplePointFileName.c_str(), lineNb, "ordinate of the point must be finite!");
	    }
          if (++tuplePointIt != tuplePoint.end())
	    {
	      delete tuplePoints;
	      throw DataFormatException(tuplePointFileName.c_str(), lineNb, ("more than the expected " + lexical_cast<string>(dimensionIds.size() + 2) + " dimensions (a " + lexical_cast<string>(dimensionIds.size()) + "-set followed by a point)!").c_str());
	    }
          if (!isCoverEmpty)
	    {
	      // Storing the tuple points
	      if (tuplePoints->isFullSparseTube(nSet.front()))
		{
		  TuplePointDenseTube* newTuplePoints = static_cast<TuplePointSparseTube*>(tuplePoints)->getDenseRepresentation();
		  delete tuplePoints;
		  tuplePoints = newTuplePoints;
		}
	      tuplePoints->setTuplePoints(nSet.begin(), point);
	    }
	}
    }
  tuplePoints->sortTubesAndTranslateToPositiveQuadrant();
  tuplePoints->setSlopeSums(maxSums);
  threshold = thresholdParam;
}

MinSlope* MinSlope::clone() const
{
  return new MinSlope(*this);
}

bool MinSlope::violationAfterPresentIncreased(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent)
{
  // Test whether dimensionIdOfElementsSetPresent is a dimension of the tuples
  vector<unsigned int>::const_iterator dimensionIdIt = dimensionIds.begin();
  for (; dimensionIdIt != dimensionIds.end() && *dimensionIdIt < dimensionIdOfElementsSetPresent; ++dimensionIdIt)
    {
    }
  if (dimensionIdIt == dimensionIds.end() || *dimensionIdIt != dimensionIdOfElementsSetPresent)
    {
      return false;
    }
  // dimensionIdOfElementsSetPresent is a dimension of the tuples, update the sums
  tuplePoints->increaseSlopeSums(present(), elementsSetPresent, dimensionIds.begin(), dimensionIdIt, minSums);
  computeOptimisticValue();
#ifdef DEBUG
  if (minSlope < threshold)
    {
      cout << threshold << "-minimal slope constraint cannot be satisfied -> Prune!\n";
    }
#endif
  return minSlope < threshold;
}

bool MinSlope::violationAfterPresentAndPotentialDecreased(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent)
{
  // Test whether dimensionIdOfElementsSetAbsent is a dimension of the tuples
  vector<unsigned int>::const_iterator dimensionIdIt = dimensionIds.begin();
  for (; dimensionIdIt != dimensionIds.end() && *dimensionIdIt < dimensionIdOfElementsSetAbsent; ++dimensionIdIt)
    {
    }
  if (dimensionIdIt == dimensionIds.end() || *dimensionIdIt != dimensionIdOfElementsSetAbsent)
    {
      return false;
    }
  // dimensionIdOfElementsSetAbsent is a dimension of the tuples, update the sums
  tuplePoints->decreaseSlopeSums(presentAndPotential(), elementsSetAbsent, dimensionIds.begin(), dimensionIdIt, maxSums);
  computeOptimisticValue();
#ifdef DEBUG
  if (minSlope < threshold)
    {
      cout << threshold << "-minimal slope constraint cannot be satisfied -> Prune!\n";
    }
#endif
  return minSlope < threshold;
}

float MinSlope::optimisticValue() const
{
  return minSlope;
}

void MinSlope::computeOptimisticValue()
{
  double denominator = minSums.sumX * minSums.sumX - maxSums.nbOfPoints * maxSums.sumXSquared;
  if (denominator > 0)
    {
      const double numerator = maxSums.sumX * maxSums.sumY - minSums.nbOfPoints * minSums.sumXY;
      if (numerator > 0)
	{
	  minSlope = numerator / denominator;
	  return;
	}
      minSlope = numerator / (maxSums.sumX * maxSums.sumX - minSums.nbOfPoints * minSums.sumXSquared);
      return;
    }
  denominator = maxSums.sumX * maxSums.sumX - minSums.nbOfPoints * minSums.sumXSquared;
  if (denominator < 0)
    {
      const double numerator = minSums.sumX * minSums.sumY - maxSums.nbOfPoints * maxSums.sumXY;
      if (numerator < 0)
	{
	  minSlope = numerator / denominator;
	  return;
	}
      minSlope = numerator / (minSums.sumX * minSums.sumX - maxSums.nbOfPoints * maxSums.sumXSquared);
    }
}

void MinSlope::deleteTuplePoints()
{
  if (!dimensionIds.empty())
    {
      delete tuplePoints;
    }
}
