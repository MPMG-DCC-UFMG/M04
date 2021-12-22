// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef VECTOR_HASH_H_
#define VECTOR_HASH_H_

#include <vector>
#include <boost/functional/hash.hpp>

template<typename T>
struct vector_hash
{
  size_t operator()(const std::vector<T>& v) const
  {
    return boost::hash_range(v.begin(), v.end());
  }
};

#endif /*VECTOR_HASH_H_*/
