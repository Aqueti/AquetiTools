/******************************************************************************
 *
 * \file IDGenerator.cpp
 * \author Andrew Ferg
 * \brief Class methods for IDGenerator
 *
 * Copyright Aqueti 2018
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *****************************************************************************/

#include "IDGenerator.hpp"

namespace atl {

std::string IDGenerator::genAlphanumericString(int len)
{
    string alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::uniform_int_distribution<int> d(0, alphabet.size() - 1);
    std::random_device rd;

    string str;
    int pos;
    while (str.size() <= len) {
        pos = d(rd);
        str += str.substr(pos,1);
    }

    return str;
}

}//end namespace atl
