// Copyright (C) 2013-2021 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "Tree.h"

vector<unsigned int> Tree::external2InternalAttributeOrder;
vector<unordered_map<string, unsigned int>> Tree::labels2Ids;
Trie* Tree::data;
unsigned int Tree::firstSymmetricAttributeId;
unsigned int Tree::secondSymmetricAttributeId;

ofstream Tree::outputFile;
string Tree::outputDimensionSeparator;
string Tree::patternSizeSeparator;
string Tree::sizeSeparator;
string Tree::sizeAreaSeparator;
bool Tree::isSizePrinted;
bool Tree::isAreaPrinted;

#ifdef NB_OF_CLOSED_N_SETS
unsigned int Tree::nbOfClosedNSets = 0;
#endif
#ifdef TIME
steady_clock::time_point Tree::overallBeginning;
#endif
#ifdef DETAILED_TIME
steady_clock::time_point Tree::startingPoint;
double Tree::parsingDuration;
double Tree::preProcessingDuration;
#endif
#ifdef ELEMENT_CHOICE_TIME
double Tree::valueChoiceDuration = 0;
#endif
#ifdef MIN_SIZE_ELEMENT_PRUNING_TIME
double Tree::minSizeElementPruningDuration = 0;
#endif
#ifdef NB_OF_LEFT_NODES
unsigned int Tree::nbOfLeftNodes = 0;
#endif

Tree::Tree(const char* dataFileName, const float densityThreshold, const vector<double>& epsilonVectorParam, const vector<unsigned int>& cliqueDimensions, const vector<double>& tauVectorParam, const vector<unsigned int>& minSizesParam, const unsigned int minAreaParam, const bool isReductionOnly, const vector<unsigned int>& unclosedDimensions, const char* inputElementSeparator, const char* inputDimensionSeparator, const char* outputFileName, const char* outputDimensionSeparatorParam, const char* patternSizeSeparatorParam, const char* sizeSeparatorParam, const char* sizeAreaSeparatorParam, const bool isSizePrintedParam, const bool isAreaPrintedParam): attributes(), mereConstraints(), isEnumeratedElementPotentiallyPreventingClosedness(false)
{
#ifdef TIME
  overallBeginning = steady_clock::now();
#endif
  vector<unsigned int> numDimensionIds;
  const vector<unsigned int>::const_iterator cliqueDimensionEnd = cliqueDimensions.end();
  vector<unsigned int>::const_iterator cliqueDimensionIt = cliqueDimensions.begin();
  unsigned int dimensionId = 0;
  for (const double tau : tauVectorParam)
    {
      if (tau != 0)
	{
	  for (; cliqueDimensionIt != cliqueDimensionEnd && *cliqueDimensionIt < dimensionId; ++cliqueDimensionIt)
	    {
	    }
	  if (cliqueDimensionIt != cliqueDimensionEnd && *cliqueDimensionIt == dimensionId)
	    {
	      throw UsageException(("clique and tau options indicate that attribute " + lexical_cast<string>(*cliqueDimensionIt) + " is both symmetric and almost-contiguous. This is not supported yet. Would you implement that feature?").c_str());
	    }
	  numDimensionIds.push_back(dimensionId);
	}
      ++dimensionId;
    }
#ifdef DETAILED_TIME
  startingPoint = steady_clock::now();
#endif
  NoisyTupleFileReader noisyTupleFileReader(dataFileName, epsilonVectorParam, cliqueDimensions, numDimensionIds, inputDimensionSeparator, inputElementSeparator);
  pair<vector<unsigned int>, double> noisyTuple = noisyTupleFileReader.next();
  const unsigned int n = noisyTuple.first.size();
  if (tauVectorParam.size() > n)
    {
      throw UsageException(("tau option should provide at most " + lexical_cast<string>(n) + " coefficients!").c_str());
    }
  if (minSizesParam.size() > n)
    {
      throw UsageException(("sizes option should provide at most " + lexical_cast<string>(n) + " sizes!").c_str());
    }
  // Initialize minSizeVector and find out whether the pre-process is wanted/useful
  vector<unsigned int> minSizeVector = minSizesParam;
  if (minAreaParam)
    {
      minSizeVector.resize(n, 1);
    }
  else
    {
      minSizeVector.resize(n);
    }
  if (!cliqueDimensions.empty())
    {
      if (minSizeVector[cliqueDimensions.front()] < minSizeVector[cliqueDimensions.back()])
	{
	  minSizeVector[cliqueDimensions.front()] = minSizeVector[cliqueDimensions.back()];
	}
      else
	{
	  minSizeVector[cliqueDimensions.back()] = minSizeVector[cliqueDimensions.front()];
	}
    }
  // Parse
  vector<vector<NoisyTuples*>> hyperplanes(n);
  bool isCrisp = true;
  for (; noisyTuple.second; noisyTuple = noisyTupleFileReader.next())
    {
      isCrisp = isCrisp && noisyTuple.second == 1;
      dimensionId = 0;
      vector<unsigned int>::const_iterator cliqueDimensionIt = cliqueDimensions.begin();
      vector<unsigned int> projectedTuple(++(noisyTuple.first.begin()), noisyTuple.first.end());
      vector<unsigned int>::iterator projectedTupleIt = projectedTuple.begin();
      vector<unsigned int>::const_iterator elementIt = noisyTuple.first.begin();
      for (vector<vector<NoisyTuples*>>::iterator hyperplanesInDimensionIt = hyperplanes.begin(); ; ++hyperplanesInDimensionIt)
	{
	  if (cliqueDimensionIt != cliqueDimensionEnd && *cliqueDimensionIt == dimensionId++)
	    {
	      ++cliqueDimensionIt;
	      while (*elementIt >= hyperplanesInDimensionIt->size())
		{
		  for (const unsigned int cliqueDimension2 : cliqueDimensions)
		    {
		      hyperplanes[cliqueDimension2].push_back(new NoisyTuples());
		    }
		}
	    }
	  else
	    {
	      while (*elementIt >= hyperplanesInDimensionIt->size())
		{
		  hyperplanesInDimensionIt->push_back(new NoisyTuples());
		}
	    }
	  (*hyperplanesInDimensionIt)[*elementIt]->insert(projectedTuple, noisyTuple.second);
	  if (projectedTupleIt == projectedTuple.end())
	    {
	      break;
	    }
	  *projectedTupleIt++ = *elementIt++;
	}
    }
  vector<unsigned int> cardinalities = noisyTupleFileReader.getCardinalities();
  // If noise-tolerance is such that an hyperplane of an n-set can be empty, those hyperplanes are needed
  vector<vector<NoisyTuples*>>::iterator hyperplanesInDimensionIt = hyperplanes.begin();
  for (const unsigned int cardinality : cardinalities)
    {
      while (hyperplanesInDimensionIt->size() != cardinality)
	{
	  hyperplanesInDimensionIt->push_back(new NoisyTuples());
	}
      ++hyperplanesInDimensionIt;
    }
  vector<double> epsilonVector = epsilonVectorParam;
  epsilonVector.resize(n);
#ifdef DETAILED_TIME
  parsingDuration = duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
  startingPoint = steady_clock::now();
#endif
  vector<Dimension*> dimensions = NoisyTuples::preProcess(epsilonVector, cliqueDimensions, minSizeVector, !minAreaParam, cardinalities, hyperplanes);
#ifdef DETAILED_TIME
  preProcessingDuration = duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
  const vector<Dimension*>::const_iterator dimensionEnd = dimensions.end();
  vector<Dimension*>::const_iterator dimensionIt = dimensions.begin();
  while ((*dimensionIt)->getCardinality() && ++dimensionIt != dimensionEnd)
    {
    }
  outputFile.open(outputFileName);
  if (!outputFile)
    {
      throw NoOutputException(outputFileName);
    }
  if (dimensionIt != dimensionEnd)
    {
      // The pre-process erased all tuples
      for (Dimension* dimension : dimensions)
	{
	  delete dimension;
	}
      for (vector<NoisyTuples*>& hyperplanesInDimension : hyperplanes)
	{
	  for (NoisyTuples* hyperplane : hyperplanesInDimension)
	    {
	      delete hyperplane;
	    }
	}
      return;
    }
  if (isReductionOnly)
    {
      // Delete the n - 1 last copies of the reduced data
      for (vector<vector<NoisyTuples*>>::iterator hyperplanesInDimensionIt = hyperplanes.begin(); ++hyperplanesInDimensionIt != hyperplanes.end(); )
	{
	  for (NoisyTuples* hyperplane : *hyperplanesInDimensionIt)
	    {
	      delete hyperplane;
	    }
	}
      // Output the reduced data
      unsigned int firstDimensionHyperplaneId = 0;
      bool isFirst = true;
      vector<NoisyTuples*>& hyperplanesInFirstDimension = hyperplanes.front();
      for (NoisyTuples* hyperplane : hyperplanesInFirstDimension)
	{
	  if (!hyperplane->empty())
	    {
	      if (isFirst)
		{
		  isFirst = false;
		}
	      else
		{
		  outputFile << '\n';
		}
	      noisyTupleFileReader.printTuplesInFirstDimensionHyperplane(outputFile, firstDimensionHyperplaneId, hyperplane->begin(), hyperplane->end(), outputDimensionSeparatorParam);
	    }
	  ++firstDimensionHyperplaneId;
	  delete hyperplane;
	}
      for (Dimension* dimension : dimensions)
	{
	  delete dimension;
	}
      outputFile << '\n';
      return;
    }
  // Init some static variables
  outputDimensionSeparator = outputDimensionSeparatorParam;
  patternSizeSeparator = patternSizeSeparatorParam;
  sizeSeparator = sizeSeparatorParam;
  sizeAreaSeparator = sizeAreaSeparatorParam;
  isSizePrinted = isSizePrintedParam;
  isAreaPrinted = isAreaPrintedParam;
  // Order the dimensions by increasing cardinality
  sort(dimensions.begin(), dimensions.end(), Dimension::smallerDimension);
  external2InternalAttributeOrder.resize(n);
  unsigned int attributeId = 0;
  vector<unsigned int> internal2ExternalAttributeOrder;
  internal2ExternalAttributeOrder.reserve(n);
  for (const Dimension* dimension : dimensions)
    {
      dimensionId = dimension->getId();
      internal2ExternalAttributeOrder.push_back(dimensionId);
      external2InternalAttributeOrder[dimensionId] = attributeId++;
    }
#if defined DEBUG || defined VERBOSE_ELEMENT_CHOICE || defined ASSERT
  Attribute::setInternal2ExternalAttributeOrder(internal2ExternalAttributeOrder);
#endif
#ifdef DEBUG
  Measure::setInternal2ExternalAttributeOrder(internal2ExternalAttributeOrder);
#endif
  // Define symmetric attributes ids accordingly
  firstSymmetricAttributeId = numeric_limits<unsigned int>::max();
  secondSymmetricAttributeId = 0;
  for (const unsigned int cliqueDimension : cliqueDimensions)
    {
      const unsigned int symmetricAttributeId = external2InternalAttributeOrder[cliqueDimension];
      if (symmetricAttributeId < firstSymmetricAttributeId)
	{
	  firstSymmetricAttributeId = symmetricAttributeId;
	}
      if (symmetricAttributeId > secondSymmetricAttributeId)
	{
	  secondSymmetricAttributeId = symmetricAttributeId;
	}
    }
#ifdef MIN_SIZE_ELEMENT_PRUNING
  // Initialize parameters to compute presentAndPotentialIrrelevancyThresholds given the sky-patterns
  IndistinctSkyPatterns::setParametersToComputePresentAndPotentialIrrelevancyThresholds(firstSymmetricAttributeId, secondSymmetricAttributeId);
#endif
  // Initialize cardinalities and oldIds2NewIds according to the new attribute order, delete the elements in dimensions and the hyperplanes not in the first internal dimension
  vector<unsigned int>::iterator cardinalityIt = cardinalities.begin();
  double nbOfTuples = 1;
  vector<vector<unsigned int>> oldIds2NewIds;
  oldIds2NewIds.reserve(n);
  vector<unsigned int>::const_iterator externalAttributeIdIt = internal2ExternalAttributeOrder.begin();
  dimensionIt = dimensions.begin();
  for (dimensionId = 0; dimensionId != n; ++dimensionId)
    {
      const unsigned int cardinality = (*dimensionIt)->getCardinality();
      delete *dimensionIt++;
      if (!secondSymmetricAttributeId || dimensionId != secondSymmetricAttributeId)
	{
	  if (dimensionId == firstSymmetricAttributeId)
	    {
	      oldIds2NewIds.insert(oldIds2NewIds.end(), cliqueDimensions.size(), NoisyTuples::createNewIds(cliqueDimensions, hyperplanes, cardinality));
	    }
	  else
	    {
	      if (*externalAttributeIdIt < tauVectorParam.size() && tauVectorParam[*externalAttributeIdIt] != 0)
		{
		  oldIds2NewIds.push_back(NoisyTuples::createNewIds(hyperplanes[*externalAttributeIdIt], cardinality, noisyTupleFileReader.getIds2Labels(*externalAttributeIdIt)));
		}
	      else
		{
		  oldIds2NewIds.push_back(NoisyTuples::createNewIds(hyperplanes[*externalAttributeIdIt], cardinality));
		}
	    }
	}
      if (dimensionId)
	{
	  for (NoisyTuples* hyperplane : hyperplanes[*externalAttributeIdIt])
	    {
	      delete hyperplane;
	    }
	}
      nbOfTuples *= cardinality;
      *cardinalityIt++ = cardinality;
      ++externalAttributeIdIt;
    }
  // Redefine epsilons exceeding the maximal number of tuples an element can cover to that number and compute the noise per unit so that no min-size noise irrelevancy threshold can exceed numeric_limits<unsigned int>::max()
  Attribute::noisePerUnit = numeric_limits<unsigned int>::max();
  externalAttributeIdIt = internal2ExternalAttributeOrder.begin();
  for (const unsigned int cardinality : cardinalities)
    {
      nbOfTuples /= cardinality;
      if (epsilonVector[*externalAttributeIdIt] > nbOfTuples)
	{
	  epsilonVector[*externalAttributeIdIt] = nbOfTuples;
	}
      if (numeric_limits<unsigned int>::max() / (nbOfTuples + epsilonVector[*externalAttributeIdIt]) < Attribute::noisePerUnit)
	{
	  Attribute::noisePerUnit = numeric_limits<unsigned int>::max() / (nbOfTuples + epsilonVector[*externalAttributeIdIt]);
	}
      nbOfTuples *= cardinality;
      ++externalAttributeIdIt;
    }
  if (isCrisp)
    {
#ifdef NUMERIC_PRECISION
#ifdef GNUPLOT
      cout << '0';
#else
      cout << "Numeric precision: 0\n";
#endif
#endif
      Attribute::noisePerUnit = 1;
      SparseCrispTube::setSizeLimit(densityThreshold * cardinalities.back() / sizeof(unsigned int) / 8);
      DenseCrispTube::setSize(cardinalities.back());
    }
  else
    {
#ifdef NUMERIC_PRECISION
#ifdef GNUPLOT
      cout << 1. / Attribute::noisePerUnit;
#else
      cout << "Numeric precision: " << 1. / Attribute::noisePerUnit << '\n';
#endif
#endif
      SparseFuzzyTube::setSizeLimit(densityThreshold * cardinalities.back() / 2);
      DenseFuzzyTube::setSize(cardinalities.back());
    }
  // Initialize attributes and labels2Ids
  labels2Ids.reserve(n);
  cardinalityIt = cardinalities.begin();
  vector<vector<unsigned int>>::const_iterator oldIds2NewIdsIt = oldIds2NewIds.begin();
  externalAttributeIdIt = internal2ExternalAttributeOrder.begin();
  attributes.reserve(n);
  for (dimensionId = 0; dimensionId != n; ++dimensionId)
    {
      if (dimensionId == firstSymmetricAttributeId)
	{
	  const vector<string> symmetricLabels = noisyTupleFileReader.setNewIdsAndGetSymmetricLabels(*oldIds2NewIdsIt, *cardinalityIt);
	  cardinalityIt += cliqueDimensions.size();
	  oldIds2NewIdsIt += cliqueDimensions.size();
	  SymmetricAttribute* firstSymmetricAttribute = new SymmetricAttribute(cardinalities, epsilonVector[*externalAttributeIdIt], minSizeVector[*externalAttributeIdIt++], symmetricLabels);
	  SymmetricAttribute* secondSymmetricAttribute = new SymmetricAttribute(cardinalities, epsilonVector[*externalAttributeIdIt], minSizeVector[*externalAttributeIdIt++], symmetricLabels);
	  firstSymmetricAttribute->setSymmetricAttribute(secondSymmetricAttribute);
	  secondSymmetricAttribute->setSymmetricAttribute(firstSymmetricAttribute);
	  attributes.push_back(firstSymmetricAttribute);
	  attributes.push_back(secondSymmetricAttribute);
	  labels2Ids.insert(labels2Ids.end(), cliqueDimensions.size(), noisyTupleFileReader.captureSymmetricLabels2Ids());
	  ++dimensionId;
	}
      else
	{
	  if (*externalAttributeIdIt < tauVectorParam.size() && tauVectorParam[*externalAttributeIdIt] != 0)
	    {
	      attributes.push_back(new MetricAttribute(cardinalities, epsilonVector[*externalAttributeIdIt], minSizeVector[*externalAttributeIdIt], noisyTupleFileReader.setNewIdsAndGetLabels(*externalAttributeIdIt, *oldIds2NewIdsIt++, *cardinalityIt++), tauVectorParam[*externalAttributeIdIt]));
	    }
	  else
	    {
	      attributes.push_back(new Attribute(cardinalities, epsilonVector[*externalAttributeIdIt], minSizeVector[*externalAttributeIdIt], noisyTupleFileReader.setNewIdsAndGetLabels(*externalAttributeIdIt, *oldIds2NewIdsIt++, *cardinalityIt++)));
	    }
	  labels2Ids.emplace_back(noisyTupleFileReader.captureLabels2Ids(*externalAttributeIdIt));
	  ++externalAttributeIdIt;
	}
    }
  // Initialize isClosedVector
  vector<bool> isClosedVector(n, true);
  unsigned int nbOfUnclosedSymmetricAttribute = 0;
  for (const unsigned int unclosedAttributeId : unclosedDimensions)
    {
      if (unclosedAttributeId >= n)
	{
	  throw UsageException(("dimension " + lexical_cast<string>(unclosedAttributeId) + " in argument of unclosed option does not exist! (at most " + lexical_cast<string>(n - 1) + " expected)").c_str());
	}
      const unsigned int internalAttributeId = external2InternalAttributeOrder[unclosedAttributeId];
      if (internalAttributeId < firstSymmetricAttributeId || internalAttributeId > secondSymmetricAttributeId)
	{
	  isClosedVector[internalAttributeId] = false;
	}
      else
	{
	  ++nbOfUnclosedSymmetricAttribute;
	}
    }
  if (nbOfUnclosedSymmetricAttribute && nbOfUnclosedSymmetricAttribute == cliqueDimensions.size())
    {
      const vector<bool>::iterator end = isClosedVector.begin() + secondSymmetricAttributeId + 1;
      for (vector<bool>::iterator isClosedVectorIt = isClosedVector.begin() + firstSymmetricAttributeId; isClosedVectorIt != end; ++isClosedVectorIt)
	{
	  *isClosedVectorIt = false;
	}
    }
  Attribute::setMinAreaIsClosedVectorAndIsStorageAllDense(minAreaParam, isClosedVector, !densityThreshold);
#if defined PRE_PROCESS && defined MIN_SIZE_ELEMENT_PRUNING
  // Initialize present and potential irrelevant thresholds
  for (Attribute* attribute : attributes)
    {
      attribute->setPresentAndPotentialIrrelevancyThreshold(attributes.begin(), attributes.end());
    }
#endif
  // Initialize data
  data = new Trie(cardinalities.begin(), cardinalities.end());
  if (secondSymmetricAttributeId)
    {
      Attribute::setIsAnAttributeSymmetric();
      // Insert every self loop
      data->setSelfLoops(firstSymmetricAttributeId, secondSymmetricAttributeId); // WARNING: start with the self loops (no code to insert self loops in dense tubes)
      unsigned int totalMembershipDueToSelfLoopsOnASymmetricValue = Attribute::noisePerUnit;
      for (Attribute* attribute : attributes)
	{
	  if (!attribute->symmetric())
	    {
	      totalMembershipDueToSelfLoopsOnASymmetricValue *= attribute->sizeOfPresentAndPotential();
	    }
	}
      const unsigned int nbOfSymmetricElements = attributes[firstSymmetricAttributeId]->sizeOfPresentAndPotential();
      for (Attribute* attribute : attributes)
	{
	  attribute->subtractSelfLoopsFromPotentialNoise(totalMembershipDueToSelfLoopsOnASymmetricValue, nbOfSymmetricElements);
	}
    }
  // Compute order in which to access the tuples in hyperplanes of the first attribute; delete hyperplanes not in the first attribute
  vector<unsigned int> attributeOrderForTuplesInFirstAtributeHyperplanes;
  attributeOrderForTuplesInFirstAtributeHyperplanes.reserve(n - 1);
  const unsigned int firstExternalAttributeId = internal2ExternalAttributeOrder.front();
  const vector<unsigned int>::const_iterator externalAttributeIdEnd = internal2ExternalAttributeOrder.end();
  for (externalAttributeIdIt = internal2ExternalAttributeOrder.begin(); ++externalAttributeIdIt != externalAttributeIdEnd; )
    {
      if (*externalAttributeIdIt < firstExternalAttributeId)
	{
	  attributeOrderForTuplesInFirstAtributeHyperplanes.push_back(*externalAttributeIdIt);
	}
      else
	{
	  if (*externalAttributeIdIt > firstExternalAttributeId)
	    {
	      attributeOrderForTuplesInFirstAtributeHyperplanes.push_back(*externalAttributeIdIt - 1);
	    }
	}
    }
  // Insert tuples but self loops
  unsigned int hyperplaneOldId = 0;
  vector<NoisyTuples*>& hyperplanesInFirstAttribute = hyperplanes[firstExternalAttributeId];
  for (NoisyTuples* hyperplane : hyperplanesInFirstAttribute)
    {
      if (!hyperplane->empty())
	{
	  data->setHyperplane(hyperplaneOldId, hyperplane->begin(), hyperplane->end(), attributeOrderForTuplesInFirstAtributeHyperplanes, oldIds2NewIds, attributes);
	}
      delete hyperplane;
      ++hyperplaneOldId;
    }
#ifdef DETAILED_TIME
  startingPoint = steady_clock::now();
#endif
}

// Constructor of a left child
Tree::Tree(const Tree& parent, const unsigned int presentAttributeId, const vector<Measure*>& mereConstraintsParam): attributes(), mereConstraints(std::move(mereConstraintsParam)), isEnumeratedElementPotentiallyPreventingClosedness(false)
{
#ifdef NB_OF_LEFT_NODES
  ++nbOfLeftNodes;
#endif
  // Deep copy of the attributes
  const vector<Attribute*>& parentAttributes = parent.attributes;
  attributes.reserve(parentAttributes.size());
  for (const Attribute* parentAttribute : parentAttributes)
    {
      attributes.push_back(parentAttribute->clone());
    }
  if (secondSymmetricAttributeId)
    {
      SymmetricAttribute* firstSymmetricAttribute = static_cast<SymmetricAttribute*>(attributes[firstSymmetricAttributeId]);
      SymmetricAttribute* secondSymmetricAttribute = static_cast<SymmetricAttribute*>(attributes[secondSymmetricAttributeId]);
      firstSymmetricAttribute->setSymmetricAttribute(secondSymmetricAttribute);
      secondSymmetricAttribute->setSymmetricAttribute(firstSymmetricAttribute);
    }
}

Tree::~Tree()
{
  for (Attribute* attribute : attributes)
    {
      delete attribute;
    }
  deleteMeasures(mereConstraints);
}

void Tree::initMeasures(const vector<unsigned int>& maxSizesParam, const int maxArea, const vector<string>& groupFileNames, const vector<unsigned int>& groupMinSizesParam, const vector<unsigned int>& groupMaxSizes, const vector<vector<float>>& groupMinRatios, const vector<vector<float>>& groupMinPiatetskyShapiros, const vector<vector<float>>& groupMinLeverages, const vector<vector<float>>& groupMinForces, const vector<vector<float>>& groupMinYulesQs, const vector<vector<float>>& groupMinYulesYs, const char* groupElementSeparator, const char* groupDimensionElementsSeparator, const char* sumValueFileName, const float minSum, const char* valueElementSeparator, const char* valueDimensionSeparator, const char* slopePointFileName, const float minSlope, const char* pointElementSeparator, const char* pointDimensionSeparator, const float densityThreshold)
{
  const unsigned int n = attributes.size();
  if (n)
    {
      vector<unsigned int> cardinalities;
      cardinalities.reserve(n);
      for (const Attribute* attribute : attributes)
	{
	  cardinalities.push_back(attribute->sizeOfPresentAndPotential());
	}
      try
	{
	  // Get the maximal sizes in the internal order of the attributes
	  vector<unsigned int> maxSizes(cardinalities);
	  if (!maxSizesParam.empty())
	    {
	      if (maxSizesParam.size() > n)
		{
		  throw UsageException(("Sizes option should provide at most " + lexical_cast<string>(n) + " sizes!").c_str());
		}
	      vector<unsigned int>::const_iterator external2InternalAttributeOrderIt = external2InternalAttributeOrder.begin();
	      for (const unsigned int maxSize : maxSizesParam)
		{
		  maxSizes[*external2InternalAttributeOrderIt] = maxSize;
		  ++external2InternalAttributeOrderIt;
		}
	      setMinParametersInClique(maxSizes);
	    }
	  // Initializing measures in increasing cost to update them
	  // Initializing MinSize measures
	  for (unsigned int attributeId = 0; attributeId != n; ++attributeId)
	    {
	      const unsigned int minSize = Attribute::getMinSize(attributeId);
	      if (minSize)
		{
		  mereConstraints.push_back(new MinSize(attributeId, cardinalities[attributeId], minSize));
		}
	    }
	  // Initializing minArea measure
	  if (Attribute::getMinArea())
	    {
	      mereConstraints.push_back(new MinArea(cardinalities, Attribute::getMinArea()));
	    }
	  // Initializing MaxSize measures
	  for (unsigned int attributeId = 0; attributeId != n; ++attributeId)
	    {
	      const unsigned int maxSize = maxSizes[attributeId];
	      if (maxSize < cardinalities[attributeId])
		{
		  mereConstraints.push_back(new MaxSize(attributeId, maxSize));
		}
	    }
	  // Initializing maxArea measure
	  if (maxArea != -1)
	    {
	      mereConstraints.push_back(new MaxArea(n, maxArea));
	    }
	  if (!groupFileNames.empty())
	    {
	      // Initializing groups
	      GroupMeasure::initGroups(groupFileNames, groupElementSeparator, groupDimensionElementsSeparator, cardinalities, labels2Ids, external2InternalAttributeOrder);
	      if (groupMinSizesParam.empty() && groupMaxSizes.empty() && groupMinRatios.empty() && groupMinPiatetskyShapiros.empty() && groupMinLeverages.empty() && groupMinForces.empty() && groupMinYulesQs.empty() && groupMinYulesYs.empty())
		{
		  // All groups must be entirely covered
		  for (unsigned int groupId = 0; groupId != groupFileNames.size(); ++groupId)
		    {
		      mereConstraints.push_back(new MinGroupCover(groupId, GroupMeasure::maxCoverOfGroup(groupId)));
		    }
		}
	      else
		{
		  // groupMinSizes is to be modified according to the diagonals of MinGroupCoverRatios, MinGroupCoverPiatetskyShapiros, MinGroupCoverLeverages, MinGroupCoverForces, MinGroupCoverYulesQs and MinGroupCoverYulesYs
		  vector<unsigned int> groupMinSizes = groupMinSizesParam;
		  groupMinSizes.resize(groupFileNames.size());
		  // Initializing MinGroupCoverRatio measures
		  if (!groupMinRatios.empty())
		    {
		      vector<unsigned int>::iterator groupMinSizeIt = groupMinSizes.begin();
		      unsigned int rowId = 0;
		      for (const vector<float>& row : groupMinRatios)
			{
			  if (row.size() > groupFileNames.size())
			    {
			      throw UsageException(("row " + lexical_cast<string>(rowId) + " of file set with gr option provides " + lexical_cast<string>(row.size()) + " ratios but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
			    }
			  unsigned int columnId = 0;
			  for (const float ratio : row)
			    {
			      if (ratio > 0)
				{
				  if (rowId == columnId)
				    {
				      const unsigned int groupMinSizeAccordingToMatrix = ratio;
				      if (groupMinSizeAccordingToMatrix > *groupMinSizeIt)
					{
					  *groupMinSizeIt = groupMinSizeAccordingToMatrix;
					}
				    }
				  else
				    {
				      mereConstraints.push_back(new MinGroupCoverRatio(rowId, columnId, ratio));
				    }
				}
			      ++columnId;
			    }
			  ++groupMinSizeIt;
			  ++rowId;
			}
		    }
		  // Initializing MinGroupCoverPiatetskyShapiro measures
		  if (!groupMinPiatetskyShapiros.empty())
		    {
		      vector<unsigned int>::iterator groupMinSizeIt = groupMinSizes.begin();
		      unsigned int rowId = 0;
		      for (const vector<float>& row : groupMinPiatetskyShapiros)
			{
			  if (row.size() > groupFileNames.size())
			    {
			      throw UsageException(("row " + lexical_cast<string>(rowId) + " of file set with gps option provides " + lexical_cast<string>(row.size()) + " Piatetsky-Shapiro's measures but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
			    }
			  unsigned int columnId = 0;
			  for (const float piatetskyShapiro : row)
			    {
			      if (rowId == columnId)
				{
				  const int groupMinSizeAccordingToMatrix = piatetskyShapiro;
				  if (groupMinSizeAccordingToMatrix > static_cast<int>(*groupMinSizeIt))
				    {
				      *groupMinSizeIt = groupMinSizeAccordingToMatrix;
				    }
				}
			      else
				{
				  if (-GroupMeasure::maxCoverOfGroup(rowId) < piatetskyShapiro * GroupMeasure::maxCoverOfGroup(columnId))
				    {
				      mereConstraints.push_back(new MinGroupCoverPiatetskyShapiro(rowId, columnId, piatetskyShapiro));
				    }
				}
			      ++columnId;
			    }
			  ++groupMinSizeIt;
			  ++rowId;
			}
		    }
		  // Initializing MinGroupCoverLeverage measures
		  if (!groupMinLeverages.empty())
		    {
		      vector<unsigned int>::iterator groupMinSizeIt = groupMinSizes.begin();
		      unsigned int rowId = 0;
		      for (const vector<float>& row : groupMinLeverages)
			{
			  if (row.size() > groupFileNames.size())
			    {
			      throw UsageException(("row " + lexical_cast<string>(rowId) + " of file set with gl option provides " + lexical_cast<string>(row.size()) + " leverages but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
			    }
			  unsigned int columnId = 0;
			  for (const float leverage : row)
			    {
			      if (rowId == columnId)
				{
				  const int groupMinSizeAccordingToMatrix = leverage;
				  if (groupMinSizeAccordingToMatrix > static_cast<int>(*groupMinSizeIt))
				    {
				      *groupMinSizeIt = groupMinSizeAccordingToMatrix;
				    }
				}
			      else
				{
				  if (-GroupMeasure::maxCoverOfGroup(rowId) < leverage * GroupMeasure::maxCoverOfGroup(columnId))
				    {
				      mereConstraints.push_back(new MinGroupCoverLeverage(rowId, columnId, leverage));
				    }
				}
			      ++columnId;
			    }
			  ++groupMinSizeIt;
			  ++rowId;
			}
		    }
		  // Initializing MinGroupCoverForce measures
		  if (!groupMinForces.empty())
		    {
		      vector<unsigned int>::iterator groupMinSizeIt = groupMinSizes.begin();
		      unsigned int rowId = 0;
		      for (const vector<float>& row : groupMinForces)
			{
			  if (row.size() > groupFileNames.size())
			    {
			      throw UsageException(("row " + lexical_cast<string>(rowId) + " of file set with gf option provides " + lexical_cast<string>(row.size()) + " forces but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
			    }
			  unsigned int columnId = 0;
			  for (const float force : row)
			    {
			      if (force > 0)
				{
				  if (rowId == columnId)
				    {
				      const unsigned int groupMinSizeAccordingToMatrix = force;
				      if (groupMinSizeAccordingToMatrix > *groupMinSizeIt)
					{
					  *groupMinSizeIt = groupMinSizeAccordingToMatrix;
					}
				    }
				  else
				    {
				      mereConstraints.push_back(new MinGroupCoverForce(rowId, columnId, force));
				    }
				}
			      ++columnId;
			    }
			  ++groupMinSizeIt;
			  ++rowId;
			}
		    }
		  // Initializing MinGroupCoverYulesQ measures
		  if (!groupMinYulesQs.empty())
		    {
		      vector<unsigned int>::iterator groupMinSizeIt = groupMinSizes.begin();
		      unsigned int rowId = 0;
		      for (const vector<float>& row : groupMinYulesQs)
			{
			  if (row.size() > groupFileNames.size())
			    {
			      throw UsageException(("row " + lexical_cast<string>(rowId) + " of file set with gyq option provides " + lexical_cast<string>(row.size()) + " Yule's Q measures but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
			    }
			  unsigned int columnId = 0;
			  for (const float yulesQ : row)
			    {
			      if (rowId == columnId)
				{
				  if (yulesQ > 0)
				    {
				      const unsigned int groupMinSizeAccordingToMatrix = yulesQ;
				      if (groupMinSizeAccordingToMatrix > *groupMinSizeIt)
					{
					  *groupMinSizeIt = groupMinSizeAccordingToMatrix;
					}
				    }
				}
			      else
				{
				  if (yulesQ > -1)
				    {
				      mereConstraints.push_back(new MinGroupCoverYulesQ(rowId, columnId, yulesQ));
				    }
				}
			      ++columnId;
			    }
			  ++groupMinSizeIt;
			  ++rowId;
			}
		    }
		  // Initializing MinGroupCoverYulesY measures
		  if (!groupMinYulesYs.empty())
		    {
		      vector<unsigned int>::iterator groupMinSizeIt = groupMinSizes.begin();
		      unsigned int rowId = 0;
		      for (const vector<float>& row : groupMinYulesYs)
			{
			  if (row.size() > groupFileNames.size())
			    {
			      throw UsageException(("row " + lexical_cast<string>(rowId) + " of file set with gyy option provides " + lexical_cast<string>(row.size()) + " Yule's Y measures but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
			    }
			  unsigned int columnId = 0;
			  for (const float yulesY : row)
			    {
			      if (rowId == columnId)
				{
				  if (yulesY > 0)
				    {
				      const unsigned int groupMinSizeAccordingToMatrix = yulesY;
				      if (groupMinSizeAccordingToMatrix > *groupMinSizeIt)
					{
					  *groupMinSizeIt = groupMinSizeAccordingToMatrix;
					}
				    }
				}
			      else
				{
				  if (yulesY > -1)
				    {
				      mereConstraints.push_back(new MinGroupCoverYulesY(rowId, columnId, yulesY));
				    }
				}
			      ++columnId;
			    }
			  ++groupMinSizeIt;
			  ++rowId;
			}
		    }
		  // Initializing MinGroupCover measures
		  const vector<unsigned int>::const_iterator end = groupMinSizes.end();
		  vector<unsigned int>::const_iterator groupMinSizeIt = groupMinSizes.begin();
		  for (unsigned int groupId = 0; groupMinSizeIt != end; ++groupId)
		    {
		      if (*groupMinSizeIt)
			{
			  mereConstraints.push_back(new MinGroupCover(groupId, *groupMinSizeIt));
			}
		      ++groupMinSizeIt;
		    }
		  // Initializing MaxGroupCover measures
		  if (!groupMaxSizes.empty())
		    {
		      unsigned int groupId = 0;
		      for (const unsigned int groupMaxSize : groupMaxSizes)
			{
			  if (groupMaxSize < GroupMeasure::maxCoverOfGroup(groupId))
			    {
			      mereConstraints.push_back(new MaxGroupCover(groupId, groupMaxSize));
			    }
			  ++groupId;
			}
		    }
		}
	    }
	  // Initializing min sum measure
	  const string sumValueFileNameString(sumValueFileName);
	  if (!sumValueFileNameString.empty())
	    {
	      mereConstraints.push_back(new MinSum(sumValueFileNameString, valueDimensionSeparator, valueElementSeparator, labels2Ids, external2InternalAttributeOrder, cardinalities, densityThreshold, minSum));
	    }
	  // Initializing min slope measure
	  const string slopePointFileNameString(slopePointFileName);
	  if (!slopePointFileNameString.empty())
	    {
	      mereConstraints.push_back(new MinSlope(slopePointFileNameString, pointDimensionSeparator, pointElementSeparator, labels2Ids, external2InternalAttributeOrder, cardinalities, densityThreshold, minSlope));
	    }
	}
      catch (std::exception& e)
	{
	  outputFile.close();
	  delete data;
	  rethrow_exception(current_exception());
	}
      labels2Ids.clear();
      stable_partition(mereConstraints.begin(), mereConstraints.end(), monotone);
      GroupMeasure::allMeasuresSet(mereConstraints);
      TupleMeasure::allMeasuresSet(cardinalities, mereConstraints);
    }
}

void Tree::mine()
{
  if (attributes.empty())
    {
#ifdef NUMERIC_PRECISION
#ifdef GNUPLOT
      cout << numeric_limits<double>::epsilon();
#else
      cout << "Numeric precision: " << numeric_limits<double>::epsilon() << '\n';
#endif
#endif
      return;
    }
  peel();
}

void Tree::terminate()
{
  outputFile.close();
  MinSum::deleteTupleValues();
  MinSlope::deleteTuplePoints();
  delete data;
#ifdef GNUPLOT
#ifdef NB_OF_CLOSED_N_SETS
#ifdef NUMERIC_PRECISION
  cout << '\t';
#endif
  cout << nbOfClosedNSets;
#endif
#ifdef NB_OF_LEFT_NODES
#if defined NUMERIC_PRECISION || defined NB_OF_CLOSED_N_SETS
  cout << '\t';
#endif
  cout << nbOfLeftNodes;
#endif
#ifdef TIME
#if defined NUMERIC_PRECISION || defined NB_OF_CLOSED_N_SETS || defined NB_OF_LEFT_NODES
  cout << '\t';
#endif
  cout << duration_cast<duration<double>>(steady_clock::now() - overallBeginning).count();
#endif
#ifdef DETAILED_TIME
#if defined NUMERIC_PRECISION || defined NB_OF_CLOSED_N_SETS || defined NB_OF_LEFT_NODES || defined TIME
  cout << '\t';
#endif
  cout << parsingDuration << '\t' << preProcessingDuration << '\t' << duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
#ifdef ELEMENT_CHOICE_TIME
#if defined NUMERIC_PRECISION || defined NB_OF_CLOSED_N_SETS || defined NB_OF_LEFT_NODES || defined TIME || defined DETAILED_TIME
  cout << '\t';
#endif
  cout << valueChoiceDuration;
#endif
#ifdef MIN_SIZE_ELEMENT_PRUNING_TIME
#if defined NUMERIC_PRECISION || defined NB_OF_CLOSED_N_SETS || defined NB_OF_LEFT_NODES || defined TIME || defined DETAILED_TIME || defined ELEMENT_CHOICE_TIME
  cout << '\t';
#endif
  cout << minSizeElementPruningDuration;
#endif
#if defined NUMERIC_PRECISION || defined NB_OF_CLOSED_N_SETS || defined NB_OF_LEFT_NODES || defined TIME || defined DETAILED_TIME || defined ELEMENT_CHOICE_TIME || defined MIN_SIZE_ELEMENT_PRUNING_TIME
  cout << '\n';
#endif
#else
#ifdef NB_OF_CLOSED_N_SETS
  cout << "Nb of closed ET-" << attributes.size() << "-sets: " << nbOfClosedNSets << '\n';
#endif
#ifdef NB_OF_LEFT_NODES
  cout << "Nb of considered " << attributes.size() << "-sets: " << nbOfLeftNodes << '\n';
#endif
#ifdef TIME
  cout << "Total time: " << duration_cast<duration<double>>(steady_clock::now() - overallBeginning).count() << "s\n";
#endif
#ifdef DETAILED_TIME
  cout << "Parsing time: " << parsingDuration << "s\nPre-processing time: " << preProcessingDuration << "s\nMining time: " << duration_cast<duration<double>>(steady_clock::now() - startingPoint).count() << "s\n";
#endif
#ifdef ELEMENT_CHOICE_TIME
  cout << "Time spent choosing the values to enumerate: " << valueChoiceDuration << "s\n";
#endif
#ifdef MIN_SIZE_ELEMENT_PRUNING_TIME
  cout << "Time spent identifying min-size irrelevant elements: " << minSizeElementPruningDuration << "s\n";
#endif
#endif
}

bool Tree::leftSubtree(const Attribute& presentAttribute)
{
  const unsigned int presentAttributeId = presentAttribute.getId();
  vector<Measure*> childMereConstraints = childMeasures(mereConstraints, presentAttributeId, presentAttribute.getChosenValueDataId());
  if (childMereConstraints.size() == mereConstraints.size())
    {
      Tree leftChild(*this, presentAttributeId, childMereConstraints);
      leftChild.setPresent(presentAttributeId, attributes);
      return leftChild.isEnumeratedElementPotentiallyPreventingClosedness;
    }
  return true;
}

vector<Measure*> Tree::childMeasures(const vector<Measure*>& parentMeasures, const unsigned int presentAttributeId, const unsigned int presentValueId)
{
  vector<Measure*> childMeasures;
  childMeasures.reserve(parentMeasures.size());
  const vector<unsigned int> elementSetPresent {presentValueId};
  for (const Measure* measure : parentMeasures)
    {
      Measure* childMeasure = measure->clone();
      if (childMeasure->violationAfterAdding(presentAttributeId, elementSetPresent))
	{
	  delete childMeasure;
	  deleteMeasures(childMeasures);
	  return childMeasures;
	}
      childMeasures.push_back(childMeasure);
    }
  // If attribute is symmetric, it is always the first one (given SymmetricAttribute::getAppealAndIndexOfValueToChoose)
  if (presentAttributeId == firstSymmetricAttributeId)
    {
      for (Measure* childMeasure : childMeasures)
	{
	  if (childMeasure->violationAfterAdding(secondSymmetricAttributeId, elementSetPresent))
	    {
	      deleteMeasures(childMeasures);
	      childMeasures.clear();
	      return childMeasures;
	    }
	}
    }
  return childMeasures;
}

void Tree::deleteMeasures(vector<Measure*>& measures)
{
  for (Measure* measure : measures)
    {
      delete measure;
    }
}

bool Tree::monotone(const Measure* measure)
{
  return measure->monotone();
}

void Tree::setPresent(const unsigned int presentAttributeId, const vector<Attribute*>& parentAttributes)
{
  parentAttributes.back()->sortPotentialAndAbsentButChosenPresentValueIfNecessary(presentAttributeId);
  const vector<Attribute*>::const_iterator parentAttributeEnd = parentAttributes.end();
  vector<Attribute*>::const_iterator parentAttributeIt = parentAttributes.begin();
  for (Attribute* attribute : attributes)
    {
      attribute->setPresentIntersections(parentAttributeIt++, parentAttributeEnd, presentAttributeId);
    }
  const vector<Attribute*>::iterator attributeBegin = attributes.begin();
  const vector<Attribute*>::iterator presentAttributeIt = attributeBegin + presentAttributeId;
  // If attribute is symmetric, it is always the first one (given SymmetricAttribute::getAppealAndIndexOfValueToChoose)
  if (presentAttributeId == firstSymmetricAttributeId)
    {
      data->setSymmetricPresent(presentAttributeIt, attributeBegin);
    }
  else
    {
      data->setPresent(presentAttributeIt, attributeBegin);
    }
  (*presentAttributeIt)->setChosenValuePresent();
  const vector<Attribute*>::iterator attributeEnd = attributes.end();
  vector<Attribute*>::iterator attributeIt = attributeBegin;
  for (; attributeIt != attributeEnd && !(*attributeIt)->findIrrelevantValuesAndCheckTauContiguity(attributeBegin, attributeEnd) && ((*attributeIt)->irrelevantEmpty() || !violationAfterRemoving((*attributeIt)->getId(), (*attributeIt)->getIrrelevantDataIds())); ++attributeIt)
    {
    }
  if (attributeIt == attributeEnd && !dominated())
    {
      parentAttributeIt = parentAttributes.begin();
      for (Attribute* attribute : attributes)
	{
	  attribute->cleanAndSortAbsent(attributeBegin, attributeEnd);
	  attribute->sortPotential();
	  (*parentAttributeIt)->sortPotentialAndAbsentButChosenValue(presentAttributeId);
	  ++parentAttributeIt;
	}
      parentAttributeIt = parentAttributes.begin();
      for (attributeIt = attributeBegin; attributeIt != attributeEnd; ++attributeIt)
	{
	  (*attributeIt)->setPresentAndPotentialIntersections(parentAttributeIt++, attributeIt, attributeEnd);
	}
      (*presentAttributeIt)->repositionChosenPresentValue();
      if (setIrrelevant())
	{
	  peel();
	}
    }
}

void Tree::rightSubtree(Attribute& absentAttribute, const bool isLastEnumeratedElementPotentiallyPreventingClosedness)
{
  const unsigned int absentAttributeId = absentAttribute.getId();
#ifdef MIN_SIZE_ELEMENT_PRUNING_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  const vector<unsigned int> absentValueDataIds = absentAttribute.setChosenValueIrrelevant(attributes.begin(), attributes.end());
#ifdef MIN_SIZE_ELEMENT_PRUNING_TIME
  minSizeElementPruningDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
  if (!(absentValueDataIds.empty() || violationAfterRemoving(absentAttributeId, absentValueDataIds) || dominated()) && (absentAttributeId != firstSymmetricAttributeId || !(violationAfterRemoving(absentAttributeId + 1, absentValueDataIds) || dominated())))
    {
      const vector<Attribute*>::iterator absentAttributeIt = attributes.begin() + absentAttributeId;
      if (isLastEnumeratedElementPotentiallyPreventingClosedness)
	{
	  unsigned int intersectionIndex = absentAttributeId;
	  for (vector<Attribute*>::iterator attributeIt = attributes.begin(); intersectionIndex--; ++attributeIt)
	    {
	      (*attributeIt)->setNullPresentAndPotentialIntersectionsWithNewAbsentValue(intersectionIndex);
	    }
	  absentAttribute.setLastIrrelevantValueChosen(absentAttributeIt, attributes.end());
	  attributes.back()->sortPotentialIrrelevantAndAbsentButChosenAbsentValueIfNecessary(absentAttributeId);
	  if (absentAttributeId == firstSymmetricAttributeId)
	    {
	      intersectionIndex = absentAttributeId + 1;
	      for (vector<Attribute*>::iterator attributeIt = attributes.begin(); intersectionIndex--; ++attributeIt)
		{
		  (*attributeIt)->setNullPresentAndPotentialIntersectionsWithNewAbsentValue(intersectionIndex);
		}
	      data->setSymmetricAbsent(absentAttributeIt, attributes.begin());
#ifdef MIN_SIZE_ELEMENT_PRUNING
	      for (Attribute* attribute : attributes)
		{
		  attribute->resetPresentAndPotentialIrrelevancyThreshold();
		}
#endif
	    }
	  else
	    {
	      data->setAbsent(absentAttributeIt, attributes.begin());
#ifdef MIN_SIZE_ELEMENT_PRUNING
	      resetPresentAndPotentialIrrelevancyThresholdsExceptOn(absentAttributeIt);
#endif
	    }
	  if ((*absentAttributeIt)->metric())
	    {
	      static_cast<MetricAttribute*>(*absentAttributeIt)->repositionChosenAbsentValue();
	    }
	}
      else
	{
	  attributes.back()->sortPotentialIrrelevantAndAbsentIfNecessary(absentAttributeId);
	  data->setIrrelevant(absentAttributeIt, (*absentAttributeIt)->eraseIrrelevantValues(), attributes.begin());
#ifdef MIN_SIZE_ELEMENT_PRUNING
	  resetPresentAndPotentialIrrelevancyThresholdsExceptOn(absentAttributeIt);
#else
	  // No Trie::setSymmetricRelevant, which would be only used here, #ifndef MIN_SIZE_ELEMENT_PRUNING
	  if (absentAttributeId == firstSymmetricAttributeId)
	    {
	      data->setIrrelevant(absentAttributeIt + 1, (*(absentAttributeIt + 1))->eraseIrrelevantValues(), attributes.begin());
	    }
#endif
	}
#ifdef MIN_SIZE_ELEMENT_PRUNING
      if (findMinSizeIrrelevantValuesAndCheckConstraints() && setIrrelevant())
	{
	  peel();
	}
#else
      if (!absentAttribute.irrelevantEmpty())
	{
	  data->setIrrelevant(absentAttributeIt, (*absentAttributeIt)->eraseIrrelevantValues(), attributes.begin());
	}
      peel();
#endif
    }
}

bool Tree::setIrrelevant()
{
  const vector<Attribute*>::iterator attributeBegin = attributes.begin();
  const vector<Attribute*>::iterator attributeEnd = attributes.end();
  vector<Attribute*>::iterator attributeToPurgeIt = attributeBegin;
  for (; attributeToPurgeIt != attributeEnd && (*attributeToPurgeIt)->irrelevantEmpty(); ++attributeToPurgeIt)
    {
    }
  if (attributeToPurgeIt == attributeEnd)
    {
      return true;
    }
  // PERF: symmetric attributes are usually more appealing and the irrelevant elements in both symmetric attributes should be set absent at once
  attributeToPurgeIt = max_element(attributeToPurgeIt, attributeEnd, Attribute::lessAppealingIrrelevant);
  attributes.back()->sortPotentialIrrelevantAndAbsentIfNecessary((*attributeToPurgeIt)->getId());
  data->setIrrelevant(attributeToPurgeIt, (*attributeToPurgeIt)->eraseIrrelevantValues(), attributeBegin);
#ifdef MIN_SIZE_ELEMENT_PRUNING
  resetPresentAndPotentialIrrelevancyThresholdsExceptOn(attributeToPurgeIt);
  return findMinSizeIrrelevantValuesAndCheckConstraints() && setIrrelevant();
#endif
  return setIrrelevant();
}

#ifdef MIN_SIZE_ELEMENT_PRUNING
void Tree::resetPresentAndPotentialIrrelevancyThresholdsExceptOn(const vector<Attribute*>::iterator absentAttributeIt)
{
  vector<Attribute*>::iterator attributeIt = attributes.begin();
  for (; attributeIt != absentAttributeIt; ++attributeIt)
    {
      (*attributeIt)->resetPresentAndPotentialIrrelevancyThreshold();
    }
  const vector<Attribute*>::iterator attributeEnd = attributes.end();
  while (++attributeIt != attributeEnd)
    {
      (*attributeIt)->resetPresentAndPotentialIrrelevancyThreshold();
    }
}

bool Tree::findMinSizeIrrelevantValuesAndCheckConstraints()
{
#ifdef MIN_SIZE_ELEMENT_PRUNING_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  vector<bool> possiblePresentAndPotentialIrrelevancyVector(attributes.size(), false);
  for (bool isPotentialReduced = true; isPotentialReduced; )
    {
      isPotentialReduced = false;
      for (Attribute* attribute : attributes)
	{
	  if (attribute->possiblePresentAndPotentialIrrelevancy(attributes.begin(), attributes.end()))
	    {
	      if (attribute->presentAndPotentialIrrelevant())
		{
#ifdef MIN_SIZE_ELEMENT_PRUNING_TIME
	          minSizeElementPruningDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
		  return false;
	        }
	      const pair<bool, vector<unsigned int>> isNotTauContiguousAndNewIrrelevantValues = attribute->findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity();
	      if (isNotTauContiguousAndNewIrrelevantValues.first)
		{
#ifdef MIN_SIZE_ELEMENT_PRUNING_TIME
		  minSizeElementPruningDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
		  return false;
		}
	      if (!isNotTauContiguousAndNewIrrelevantValues.second.empty())
		{
		  if (violationAfterRemoving(attribute->getId(), isNotTauContiguousAndNewIrrelevantValues.second) || (attribute->symmetric() && violationAfterRemoving(static_cast<SymmetricAttribute*>(attribute)->getSymmetricAttributeId(), isNotTauContiguousAndNewIrrelevantValues.second)))
		    {
#ifdef MIN_SIZE_ELEMENT_PRUNING_TIME
		      minSizeElementPruningDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
		      return false;
		    }
		  isPotentialReduced = true;
		}
	      possiblePresentAndPotentialIrrelevancyVector[attribute->getId()] = true;
	    }
	}
    }
  if (dominated())
    {
#ifdef MIN_SIZE_ELEMENT_PRUNING_TIME
      minSizeElementPruningDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
      return false;
    }
  vector<bool>::const_iterator possiblePresentAndPotentialIrrelevancyIt = possiblePresentAndPotentialIrrelevancyVector.begin();
  for (Attribute* attribute : attributes)
    {
      if (*possiblePresentAndPotentialIrrelevancyIt++)
	{
	  if (attribute->symmetric())
	    {
	      static_cast<SymmetricAttribute*>(attribute)->presentAndPotentialCleanAbsent(attributes.begin(), attributes.end());
	    }
	  else
	    {
	      attribute->presentAndPotentialCleanAbsent();
	      if (attribute->metric())
		{
		  static_cast<MetricAttribute*>(attribute)->sortAbsent();
		}
	    }
	}
    }
#ifdef MIN_SIZE_ELEMENT_PRUNING_TIME
  minSizeElementPruningDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
  return true;
}
#endif

bool Tree::violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent)
{
  for (Measure* measure : mereConstraints)
    {
      if (measure->violationAfterAdding(dimensionIdOfElementsSetPresent, elementsSetPresent))
	{
	  isEnumeratedElementPotentiallyPreventingClosedness = true;
	  return true;
	}
    }
  return false;
}

bool Tree::violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent)
{
  for (Measure* measure : mereConstraints)
    {
      if (measure->violationAfterRemoving(dimensionIdOfElementsSetAbsent, elementsSetAbsent))
	{
	  if (!measure->monotone())
	    {
	      isEnumeratedElementPotentiallyPreventingClosedness = true;
	    }
	  return true;
	}
    }
  return false;
}

bool Tree::dominated()
{
  return false;
}

void Tree::validPattern() const
{
#ifdef NB_OF_CLOSED_N_SETS
  ++nbOfClosedNSets;
#endif
  outputFile << *this;
  if (isSizePrinted)
    {
      outputFile << patternSizeSeparator;
      bool isFirstSize = true;
      for (const unsigned int internalAttributeId : external2InternalAttributeOrder)
	{
	  if (isFirstSize)
	    {
	      isFirstSize = false;
	    }
	  else
	    {
	      outputFile << sizeSeparator;
	    }
	  outputFile << attributes[internalAttributeId]->sizeOfPresentAndPotential();
	}
    }
  if (isAreaPrinted)
    {
      unsigned int area = 1;
      for (const Attribute* attribute : attributes)
	{
	  area *= attribute->sizeOfPresentAndPotential();
	}
      outputFile << sizeAreaSeparator << area;
    }
  outputFile << '\n';
}

void Tree::setMinParametersInClique(vector<unsigned int>& parameterVector)
{
  if (secondSymmetricAttributeId)
    {
      const vector<unsigned int>::iterator parameterIt = parameterVector.begin() + firstSymmetricAttributeId;
      const unsigned int minParameter = min(*parameterIt, *(parameterIt + 1));
      *parameterIt = minParameter;
      *(parameterIt + 1) = minParameter;
    }
}

void Tree::setMaxParametersInClique(vector<unsigned int>& parameterVector)
{
  if (secondSymmetricAttributeId)
    {
      const vector<unsigned int>::iterator parameterIt = parameterVector.begin() + firstSymmetricAttributeId;
      const unsigned int maxParameter = max(*parameterIt, *(parameterIt + 1));
      *parameterIt = maxParameter;
      *(parameterIt + 1) = maxParameter;
    }
}

void Tree::peel()
{
#ifdef DEBUG
  cout << "Node after:\n";
  printNode(cout);
  cout << '\n';
#endif
  const vector<Attribute*>::iterator attributeBegin = attributes.begin();
  const vector<Attribute*>::iterator attributeEnd = attributes.end();
#ifdef ASSERT
  for (vector<Attribute*>::const_iterator attributeIt = attributeBegin; attributeIt != attributeEnd; ++attributeIt)
    {
      vector<Value*>::const_iterator end = (*attributeIt)->presentEnd();
      vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin();
      for (; valueIt != end; ++valueIt)
  	{
  	  assertNoiseOnValue(attributeIt, **valueIt);
  	  assertNoiseAtIntersectionsWithPotentialAndAbsent(attributeIt, **valueIt);
  	}
      end = (*attributeIt)->potentialEnd();
      for (; valueIt != end; ++valueIt)
  	{
  	  assertNoiseOnValue(attributeIt, **valueIt);
  	}
      end = (*attributeIt)->absentEnd();
      for (; valueIt != end; ++valueIt)
  	{
  	  assertNoiseOnValue(attributeIt, **valueIt);
  	  assertNoiseAtIntersectionsWithPresentAndPotential(attributeIt, **valueIt);
  	}
    }
#endif
  bool unclosedInMetricAttribute = false;
  for (const Attribute* attribute : attributes)
    {
      if (attribute->unclosed(attributeBegin, attributeEnd))
	{
	  if (!attribute->metric())
	    {
	      return;
	    }
	  unclosedInMetricAttribute = true;
	}
    }
  if (unclosedInMetricAttribute)
    {
      isEnumeratedElementPotentiallyPreventingClosedness = true;
      return;
    }
  // Check wether all potential elements can be set present at once
  vector<Attribute*>::iterator attributeIt = attributeBegin;
  for (; attributeIt != attributeEnd && (*attributeIt)->finalizable(); ++attributeIt)
    {
    }
  if (attributeIt == attributeEnd)
    {
      // Leaf
#ifdef DEBUG
      cout << "Every remaining potential value is present!\n";
#endif
      for (Attribute* attribute : attributes)
	{
	  if (attribute->metric() && static_cast<const MetricAttribute*>(attribute)->globallyUnclosed(attributeBegin, attributeEnd))
	    {
	      return;
	    }
	  const vector<unsigned int> elementsSetPresent = attribute->finalize();
	  if (!elementsSetPresent.empty() && violationAfterAdding(attribute->getId(), elementsSetPresent))
	    {
	      return;
	    }
	}
      if (!dominated())
	{
#ifdef DEBUG
	  cout << "*********************** closed ET-" << attributes.size() << "-set ************************\n" << *this << "\n****************************************************************\n";
#endif
	  validPattern();
	  isEnumeratedElementPotentiallyPreventingClosedness = true;
	}
      return;
    }
  // Not a leaf: choose the value to enumerate
#ifdef ELEMENT_CHOICE_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  Attribute* attributeToPeel = Attribute::chooseValue(attributes);
#ifdef ELEMENT_CHOICE_TIME
  valueChoiceDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
  // Construct the subtrees
#ifdef DEBUG
  cout << "Left child: ";
  attributeToPeel->printChosenValue(cout);
  cout << " set present\nNode before:\n";
  printNode(cout);
  cout << '\n';
#endif
  const bool isLastEnumeratedElementPotentiallyPreventingClosedness = leftSubtree(*attributeToPeel);
#ifdef DEBUG
  cout << "Right child: ";
  attributeToPeel->printChosenValue(cout);
  cout << " set absent";
  if (!(isLastEnumeratedElementPotentiallyPreventingClosedness && attributeToPeel->closedAttribute()))
    {
      cout << " and cannot prevent the closedness of any future pattern";
    }
  cout << "\nNode before:\n";
  printNode(cout);
  cout << '\n';
#endif
#ifdef NON_EXTENSION_ELEMENT_DETECTION
  rightSubtree(*attributeToPeel, isLastEnumeratedElementPotentiallyPreventingClosedness && attributeToPeel->closedAttribute());
#else
  rightSubtree(*attributeToPeel, attributeToPeel->closedAttribute());
#endif
  if (isLastEnumeratedElementPotentiallyPreventingClosedness)
    {
      isEnumeratedElementPotentiallyPreventingClosedness = true;
    }
}

ostream& operator<<(ostream& out, const Tree& node)
{
  bool isFirstAttribute = true;
  for (const unsigned int internalAttributeId : node.external2InternalAttributeOrder)
    {
      if (isFirstAttribute)
	{
	  isFirstAttribute = false;
	}
      else
	{
	  out << Tree::outputDimensionSeparator;
	}
      out << *(node.attributes[internalAttributeId]);
    }
  return out;
}

#ifdef DEBUG
void Tree::printNode(ostream& out) const
{
  out << "  present: ";
  bool isFirstAttribute = true;
  for (const unsigned int internalAttributeId : external2InternalAttributeOrder)
    {
      if (isFirstAttribute)
	{
	  isFirstAttribute = false;
	}
      else
	{
	  out << outputDimensionSeparator;
	}
      attributes[internalAttributeId]->printPresent(out);
    }
  out << "\n  potential: ";
  isFirstAttribute = true;
  for (const unsigned int internalAttributeId : external2InternalAttributeOrder)
    {
      if (isFirstAttribute)
	{
	  isFirstAttribute = false;
	}
      else
	{
	  out << outputDimensionSeparator;
	}
      attributes[internalAttributeId]->printPotential(out);
    }
  out << "\n  possible extensions: ";
  isFirstAttribute = true;
  for (const unsigned int internalAttributeId : external2InternalAttributeOrder)
    {
      if (isFirstAttribute)
	{
	  isFirstAttribute = false;
	}
      else
	{
	  out << outputDimensionSeparator;
	}
      attributes[internalAttributeId]->printAbsent(out);
    }
}
#endif

#ifdef ASSERT
void Tree::assertNoiseOnValue(const vector<Attribute*>::const_iterator attributeIt, const Value& value) const
{
  const vector<Attribute*>::const_iterator attributeBegin = attributes.begin();
  unsigned int actualNoise = data->noiseSumOnPresent(attributeIt, value, attributeBegin);
  unsigned int pretendedNoise = value.getPresentNoise();
  if (actualNoise != pretendedNoise)
    {
      (*attributeIt)->printValue(value, cout);
      cout << " has " << static_cast<double>(actualNoise) / Attribute::noisePerUnit << " present noise but pretends to have " << static_cast<double>(pretendedNoise) / Attribute::noisePerUnit << '\n';
    }
  actualNoise = data->noiseSumOnPresentAndPotential(attributeIt, value, attributeBegin);
  pretendedNoise = value.getPresentAndPotentialNoise();
  if (actualNoise != pretendedNoise)
    {
      (*attributeIt)->printValue(value, cout);
      cout << " has " << static_cast<double>(actualNoise) / Attribute::noisePerUnit << " present and potential noise but pretends to have " << static_cast<double>(pretendedNoise) / Attribute::noisePerUnit << '\n';
    }
}

void Tree::assertPresentNoiseAtIntersection(const vector<Attribute*>::const_iterator firstAttributeIt, const Value& firstValue, const vector<Attribute*>::const_iterator secondAttributeIt, const Value& secondValue) const
{
  const unsigned int actualNoise = data->noiseSumOnPresent(firstAttributeIt, firstValue, secondAttributeIt, secondValue, attributes.begin());
  const unsigned int pretendedNoise = firstValue.presentNoiseAtIntersectionWith(secondValue, (*secondAttributeIt)->getId() - (*firstAttributeIt)->getId() - 1);
  if (actualNoise != pretendedNoise)
    {
      (*firstAttributeIt)->printValue(firstValue, cout);
      cout << " and ";
      (*secondAttributeIt)->printValue(secondValue, cout);
      cout << " have " << static_cast<double>(actualNoise) / Attribute::noisePerUnit << " present noise but pretend to have " << static_cast<double>(pretendedNoise) / Attribute::noisePerUnit << '\n';
    }
}

void Tree::assertPresentAndPotentialNoiseAtIntersection(const vector<Attribute*>::const_iterator firstAttributeIt, const Value& firstValue, const vector<Attribute*>::const_iterator secondAttributeIt, const Value& secondValue) const
{
  const unsigned int actualNoise = data->noiseSumOnPresentAndPotential(firstAttributeIt, firstValue, secondAttributeIt, secondValue, attributes.begin());
  const unsigned int pretendedNoise = firstValue.presentAndPotentialNoiseAtIntersectionWith(secondValue, (*secondAttributeIt)->getId() - (*firstAttributeIt)->getId() - 1);
  if (actualNoise != pretendedNoise)
    {
      (*firstAttributeIt)->printValue(firstValue, cout);
      cout << " and ";
      (*secondAttributeIt)->printValue(secondValue, cout);
      cout << " have " << static_cast<double>(actualNoise) / Attribute::noisePerUnit << " present and potential noise but pretend to have " << static_cast<double>(pretendedNoise) / Attribute::noisePerUnit << '\n';
    }
}

void Tree::assertNoiseAtIntersectionsWithPotentialAndAbsent(const vector<Attribute*>::const_iterator presentAttributeIt, const Value& presentValue) const
{
  vector<Attribute*>::const_iterator attributeIt = attributes.begin();
  for (; attributeIt != presentAttributeIt; ++attributeIt)
    {
      const vector<Value*>::const_iterator end = (*attributeIt)->absentEnd();
      for (vector<Value*>::const_iterator potentialOrAbsentValueIt = (*attributeIt)->potentialBegin(); potentialOrAbsentValueIt != end; ++potentialOrAbsentValueIt)
	{
	  assertPresentNoiseAtIntersection(attributeIt, **potentialOrAbsentValueIt, presentAttributeIt, presentValue);
	}
    }
  const vector<Attribute*>::const_iterator attributeEnd = attributes.end();
  while (++attributeIt != attributeEnd)
    {
      const vector<Value*>::const_iterator end = (*attributeIt)->absentEnd();
      for (vector<Value*>::const_iterator potentialOrAbsentValueIt = (*attributeIt)->potentialBegin(); potentialOrAbsentValueIt != end; ++potentialOrAbsentValueIt)
	{
	  assertPresentNoiseAtIntersection(presentAttributeIt, presentValue, attributeIt, **potentialOrAbsentValueIt);
	}
    }
}

void Tree::assertNoiseAtIntersectionsWithPresentAndPotential(const vector<Attribute*>::const_iterator absentAttributeIt, const Value& absentValue) const
{
  vector<Attribute*>::const_iterator attributeIt = attributes.begin();
  for (; attributeIt != absentAttributeIt; ++attributeIt)
    {
      const vector<Value*>::const_iterator end = (*attributeIt)->potentialEnd();
      for (vector<Value*>::const_iterator presentOrPotentialValueIt = (*attributeIt)->presentBegin(); presentOrPotentialValueIt != end; ++presentOrPotentialValueIt)
	{
	  assertPresentAndPotentialNoiseAtIntersection(attributeIt, **presentOrPotentialValueIt, absentAttributeIt, absentValue);
	}
    }
  const vector<Attribute*>::const_iterator attributeEnd = attributes.end();
  while (++attributeIt != attributeEnd)
    {
      const vector<Value*>::const_iterator end = (*attributeIt)->potentialEnd();
      for (vector<Value*>::const_iterator presentOrPotentialValueIt = (*attributeIt)->presentBegin(); presentOrPotentialValueIt != end; ++presentOrPotentialValueIt)
	{
	  assertPresentAndPotentialNoiseAtIntersection(absentAttributeIt, absentValue, attributeIt, **presentOrPotentialValueIt);
	}
    }
}
#endif
