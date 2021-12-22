// Copyright 2018,2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef NO_OUTPUT_EXCEPTION_H_
#define NO_OUTPUT_EXCEPTION_H_

#include <exception>
#include <string>

using namespace std;

class NoOutputException final : public std::exception
{
 public:
  NoOutputException(const char* fileName);
  const char* what() const noexcept;

 private:
  const string message;
};

#endif /*NO_OUTPUT_EXCEPTION_H_*/
