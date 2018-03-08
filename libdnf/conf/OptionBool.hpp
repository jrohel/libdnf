/*
 * Copyright (C) 2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _OPTION_BOOL_HPP
#define _OPTION_BOOL_HPP

#include "Option.hpp"

namespace libdnf {

constexpr const char * defTrueNames[]{"1", "yes", "true", "enabled", nullptr};
constexpr const char * defFalseNames[]{"0", "no", "false", "disabled", nullptr};

class OptionBool : public Option {
/*   """An option representing a boolean value.  The value can be one
    of 0, 1, yes, no, true, or false.
*/
public:
    typedef bool ValueType;

    OptionBool(bool defaultValue, const char * const trueVals[], const char * const falseVals[]);
    OptionBool(bool defaultValue);
    void test(bool) const;
    bool fromString(const std::string & value) const;
    void set(Priority priority, bool value);
    void set(Priority priority, const std::string & value) override;
    bool getValue() const noexcept;
    bool getDefaultValue() const noexcept;
    std::string toString(bool value) const;
    std::string getValueString() const override;
    const char * const * getTrueNames() const noexcept;
    const char * const * getFalseNames() const noexcept;

protected:
    const char * const * const trueNames;
    const char * const * const falseNames;
    bool defaultValue;
    bool value;
};

inline void OptionBool::test(bool) const {}

inline bool OptionBool::getValue() const noexcept
{
    return value;
}

inline bool OptionBool::getDefaultValue() const noexcept
{
    return defaultValue;
}

inline std::string OptionBool::getValueString() const
{
    return toString(value);
}

inline const char * const * OptionBool::getTrueNames() const noexcept
{
    return trueNames ? trueNames : defTrueNames;
}

inline const char * const * OptionBool::getFalseNames() const noexcept
{
    return falseNames ? falseNames : defFalseNames; 
}

}

#endif
