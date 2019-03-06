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

std::string IDGenerator::genAlphanumericString(unsigned int len)
{
    std::string alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    //std::uniform_int_distribution<int> d(0, alphabet.size() - 1);
    //std::random_device rd;
    std::srand(atl::getUsecTime()); //use current time as seed for RNG

    std::string str;
    int pos;
    while (str.size() <= len) {
        //pos = d(rd);
        int randomInt = std::rand();
        pos = randomInt % alphabet.length();
        str += alphabet.substr(pos,1);
    }

    return str;
}

std::string IDGenerator::genNumericString(unsigned int len)
{
    std::string alphabet = "0123456789";
    //std::uniform_int_distribution<int> d(0, alphabet.size() - 1);
    //std::random_device rd;
    std::srand(atl::getUsecTime()); //use current time as seed for RNG

    std::string str;
    int pos;
    while (str.size() <= len) {
        //pos = d(rd);
        int randomInt = std::rand();
        pos = randomInt % alphabet.length();
        str += alphabet.substr(pos,1);
    }

    return str;
}

uint64_t IDGenerator::genUint64()
{
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<uint64_t> distribution(0, UINT64_MAX);
    return distribution(generator);
}

}//end namespace atl
