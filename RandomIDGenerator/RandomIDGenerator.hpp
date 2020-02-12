/******************************************************************************
 *
 * \file RandomIDGenerator.hpp
 * \author Andrew Ferg
 * \brief Class of static functions that generate random strings and integers
 *
 * Copyright Aqueti 2018
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *****************************************************************************/
#pragma once

#include <string>

namespace acl {

class RandomIDGenerator
{
    public:
        static std::string genAlphanumericString(unsigned int len=8);
        static std::string genNumericString(unsigned int len=8);
        static uint64_t genUint64();
};

}//end namespace acl
