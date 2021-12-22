// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "DataFormatException.h"

DataFormatException::DataFormatException(const char* fileName, const unsigned int lineNb, const char* message): message(string(fileName) + ':' + boost::lexical_cast<string>(lineNb) + ": " + string(message))
{
}

const char* DataFormatException::what() const noexcept
{
  return message.c_str();
}
