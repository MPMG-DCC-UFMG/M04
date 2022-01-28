// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef TUPLE_VALUE_TRIE_H
#define TUPLE_VALUE_TRIE_H

#include "TupleValueSparseTube.h"

class TupleValueTrie final : public AbstractTupleValueData
{
 public:
  TupleValueTrie(const vector<unsigned int>::const_iterator dimensionIdIt, const vector<unsigned int>::const_iterator dimensionIdsEnd, const vector<unsigned int>& cardinalities);
  TupleValueTrie(const TupleValueTrie& otherTuplevalueTrie);
  TupleValueTrie(TupleValueTrie&& otherTupleValueTrie);

  ~TupleValueTrie();

  TupleValueTrie& operator=(const TupleValueTrie& otherTupleValueTrie);
  TupleValueTrie& operator=(TupleValueTrie&& otherTupleValueTrie);

  void setTupleValues(const vector<vector<unsigned int>>::const_iterator dimensionIt, const double value);
  void sortTubes();
  void sortTubesAndSetSum(double& sum);

  void decreaseSum(const vector<vector<unsigned int>>& pattern, const vector<unsigned int>& elements, const vector<unsigned int>::const_iterator dimensionIdIt, const vector<unsigned int>::const_iterator absentDimensionIdIt, double& sum) const;

 private:
  vector<AbstractTupleValueData*> hyperplanes;

  void copy(const TupleValueTrie& otherTupleValueTrie);
  TupleValueTrie* clone() const;

  void decreaseSum(const vector<vector<unsigned int>>& pattern, const vector<unsigned int>::const_iterator dimensionIdIt, double& sum) const;
};
#endif
