/*
 * Copyright (C) 2017 Red Hat, Inc.
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

#include "options.hpp"

#include "bgettext/bgettext-lib.h"
#include "tinyformat/tinyformat.hpp"
#include "regex/regex.hpp"

#include <limits>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace libdnf {

template <typename T>
OptionNumber<T>::OptionNumber(T defaultValue, T min, T max)
: Option{Priority::PRIO_DEFAULT}, defaultValue{defaultValue}, min{min}, max{max}, value{defaultValue} { test(defaultValue); }

template <typename T>
OptionNumber<T>::OptionNumber(T defaultValue, T min)
: OptionNumber(defaultValue, min, std::numeric_limits<T>::max()) {}

template <typename T>
OptionNumber<T>::OptionNumber(T defaultValue)
: OptionNumber(defaultValue, std::numeric_limits<T>::min(), std::numeric_limits<T>::max()) {}

template <typename T>
OptionNumber<T>::OptionNumber(T defaultValue, T min, T max, FromStringFunc && fromStringFunc)
: Option{Priority::PRIO_DEFAULT}
, fromStringUser{fromStringFunc}
, defaultValue{defaultValue}, min{min}, max{max}, value{defaultValue} { test(defaultValue); }

template <typename T>
OptionNumber<T>::OptionNumber(T defaultValue, T min, FromStringFunc && fromStringFunc)
: OptionNumber(defaultValue, min, std::numeric_limits<T>::max(), std::move(fromStringFunc)) {}

template <typename T>
OptionNumber<T>::OptionNumber(T defaultValue, FromStringFunc && fromStringFunc)
: OptionNumber(defaultValue, std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), std::move(fromStringFunc)) {}

template <typename T>
void OptionNumber<T>::test(ValueType value) const
{
    if (value > max)
        throw Exception(tfm::format(_("given value [%d] should be less than "
                                        "allowed value [%d]."), value, max));
    else if (value < min)
        throw Exception(tfm::format(_("given value [%d] should be greater than "
                                        "allowed value [%d]."), value, min));
}

template <typename T>
T OptionNumber<T>::fromString(const std::string & value) const
{
    if (fromStringUser)
        return fromStringUser(value);
    ValueType val;
    if (libdnf::fromString<ValueType>(val, value, std::dec))
        return val;
    throw Exception(_("invalid value"));
}

template <typename T>
void OptionNumber<T>::set(Priority priority, ValueType value)
{
    if (priority >= this->priority) {
        test(value);
        this->value = value;
        this->priority = priority;
    }
}

template <typename T>
void OptionNumber<T>::set(Option::Priority priority, const std::string & value)
{
    set(priority, fromString(value));
}

template <typename T>
std::string OptionNumber<T>::toString(ValueType value) const
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

static bool strToBool(bool & out, std::string in, const char * const trueNames[] = defTrueNames,
               const char * const falseNames[] = defFalseNames)
{
    for (auto & ch : in)
        ch = std::tolower(ch);
    for (auto it = falseNames; *it; ++it) {
        if (in == *it) {
            out = false;
            return true;
        }
    }
    for (auto it = trueNames; *it; ++it) {
        if (in == *it) {
            out = true;
            return true;
        }
    }
    return false;
}

OptionBool::OptionBool(bool defaultValue, const char * const trueVals[], const char * const falseVals[])
: Option(Priority::PRIO_DEFAULT), trueNames(trueVals), falseNames(falseVals)
, defaultValue(defaultValue), value(defaultValue) {}

OptionBool::OptionBool(bool defaultValue)
: OptionBool(defaultValue, nullptr, nullptr) {}

bool OptionBool::fromString(const std::string & value) const
{
    bool val;
    if (strToBool(val, value, getTrueNames(), getFalseNames()))
        return val;
    throw Exception(tfm::format(_("invalid boolean value '%s'"), value));
}

void OptionBool::set(Priority priority, bool value)
{
    if (priority >= this->priority) {
        this->value = value;
        this->priority = priority;
    }
}

void OptionBool::set(Priority priority, const std::string & value)
{
    set(priority, fromString(value));
}


std::string OptionBool::toString(bool value) const
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

OptionString::OptionString(const std::string & defaultValue)
: Option(Priority::PRIO_DEFAULT), defaultValue(defaultValue), value(defaultValue) {}

OptionString::OptionString(const char * defaultValue)
{
    if (defaultValue) {
        this->value = this->defaultValue = defaultValue;
        this->priority = Priority::PRIO_DEFAULT;
    }
}

OptionString::OptionString(const std::string & defaultValue, const std::string & regex, bool icase)
: Option{Priority::PRIO_DEFAULT}, regex{regex}, icase{icase}, defaultValue(defaultValue), value(defaultValue) { test(defaultValue); }

OptionString::OptionString(const char * defaultValue, const std::string & regex, bool icase)
: regex{regex}, icase{icase}
{
    if (defaultValue) {
        this->defaultValue = defaultValue;
        test(this->defaultValue);
        this->value = this->defaultValue;
        this->priority = Priority::PRIO_DEFAULT;
    }
}

void OptionString::test(const std::string & value) const
{
    if (regex.empty())
        return;
    if (!Regex(regex.c_str(), (icase ? REG_ICASE : 0) | REG_EXTENDED | REG_NOSUB).match(value.c_str()))
        throw Exception(tfm::format(_("'%s' is not an allowed value"), value));
}

void OptionString::set(Priority priority, const std::string & value)
{
    if (priority >= this->priority) {
        test(value);
        this->value = value;
        this->priority = priority;
    }
}

const std::string & OptionString::getValue() const
{
    if (priority == Priority::PRIO_EMPTY)
        throw Exception(_("GetValue(): Value not set"));
    return value;
}

template <typename T>
OptionEnum<T>::OptionEnum(ValueType defaultValue, const std::vector<ValueType> & enumVals)
: Option{Priority::PRIO_DEFAULT}, enumVals{enumVals}, defaultValue{defaultValue}, value{defaultValue}
{
    test(defaultValue);
}

template <typename T>
OptionEnum<T>::OptionEnum(ValueType defaultValue, std::vector<ValueType> && enumVals)
: Option{Priority::PRIO_DEFAULT}, enumVals{std::move(enumVals)}, defaultValue{defaultValue}, value{defaultValue}
{
    test(defaultValue);
}

template <typename T>
OptionEnum<T>::OptionEnum(ValueType defaultValue, const std::vector<ValueType> & enumVals, FromStringFunc && fromStringFunc)
: Option{Priority::PRIO_DEFAULT}, fromStringUser{std::move(fromStringFunc)}
, enumVals{enumVals}, defaultValue{defaultValue}, value{defaultValue}
{
    test(defaultValue);
}

template <typename T>
OptionEnum<T>::OptionEnum(ValueType defaultValue, std::vector<ValueType> && enumVals, FromStringFunc && fromStringFunc)
: Option{Priority::PRIO_DEFAULT}, fromStringUser{std::move(fromStringFunc)}
, enumVals{std::move(enumVals)}, defaultValue{defaultValue}, value{defaultValue}
{
    test(defaultValue);
}

template <typename T>
void OptionEnum<T>::test(ValueType value) const
{
    auto it = std::find(enumVals.begin(), enumVals.end(), value);
    if (it == enumVals.end())
        throw Exception(tfm::format(_("'%s' is not an allowed value"), value));
}

template <typename T>
T OptionEnum<T>::fromString(const std::string & value) const
{
    if (fromStringUser)
        return fromStringUser(value);
    T val;
    if (libdnf::fromString<ValueType>(val, value, std::dec))
        return val;
    throw Exception(_("invalid value"));
}

template <typename T>
void OptionEnum<T>::set(Priority priority, ValueType value)
{
    if (priority >= this->priority) {
        test(value);
        this->value = value;
        this->priority = priority;
    }
}

template <typename T>
void OptionEnum<T>::set(Priority priority, const std::string & value)
{
    set(priority, fromString(value));
}

template <typename T>
T OptionEnum<T>::getValue() const
{
    return value;
}

template <typename T>
T OptionEnum<T>::getDefaultValue() const
{
    return defaultValue;
}

template <typename T>
std::string OptionEnum<T>::toString(ValueType value) const
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template <typename T>
std::string OptionEnum<T>::getValueString() const 
{
    return toString(value);
}

OptionEnum<std::string>::OptionEnum(const std::string & defaultValue, const std::vector<ValueType> & enumVals)
: Option{Priority::PRIO_DEFAULT}, enumVals{enumVals}, defaultValue{defaultValue}, value{defaultValue}
{
    test(defaultValue);
}

OptionEnum<std::string>::OptionEnum(const std::string & defaultValue, std::vector<ValueType> && enumVals)
: Option{Priority::PRIO_DEFAULT}, enumVals{std::move(enumVals)}, defaultValue{defaultValue}, value{defaultValue}
{
    test(defaultValue);
}

OptionEnum<std::string>::OptionEnum(const std::string & defaultValue, const std::vector<ValueType> & enumVals, FromStringFunc && fromStringFunc)
: Option{Priority::PRIO_DEFAULT}, fromStringUser{std::move(fromStringFunc)}
, enumVals{enumVals}, defaultValue{defaultValue}, value{defaultValue}
{
    test(defaultValue);
}

OptionEnum<std::string>::OptionEnum(const std::string & defaultValue, std::vector<ValueType> && enumVals, FromStringFunc && fromStringFunc)
: Option{Priority::PRIO_DEFAULT}, fromStringUser{std::move(fromStringFunc)}
, enumVals{std::move(enumVals)}, defaultValue{defaultValue}, value{defaultValue}
{
    test(defaultValue);
}

void OptionEnum<std::string>::test(const std::string & value) const
{
    auto it = std::find(enumVals.begin(), enumVals.end(), value);
    if (it == enumVals.end())
        throw Exception(tfm::format(_("'%s' is not an allowed value"), value));
}

std::string OptionEnum<std::string>::fromString(const std::string & value) const
{
    if (fromStringUser)
        return fromStringUser(value);
    return value;
}

void OptionEnum<std::string>::set(Priority priority, const std::string & value)
{
    auto val = fromString(value);
    if (priority >= this->priority) {
        test(val);
        this->value = val;
        this->priority = priority;
    }
}

OptionStringList::OptionStringList(const ValueType & defaultValue)
: Option{Priority::PRIO_DEFAULT}, defaultValue{defaultValue}, value{defaultValue} {}

OptionStringList::OptionStringList(const ValueType & defaultValue, const std::string & regex, bool icase)
: Option{Priority::PRIO_DEFAULT}, regex{regex}, icase{icase}, defaultValue{defaultValue}, value{defaultValue} { test(defaultValue); }

OptionStringList::OptionStringList(const std::string & defaultValue)
: Option{Priority::PRIO_DEFAULT} {
    this->value = this->defaultValue = fromString(defaultValue);
}

OptionStringList::OptionStringList(const std::string & defaultValue, const std::string & regex, bool icase)
: Option{Priority::PRIO_DEFAULT}, regex{regex}, icase{icase}
{
    this->defaultValue = fromString(defaultValue);
    test(this->defaultValue);
    value = this->defaultValue;
}

void OptionStringList::test(const std::vector<std::string> & value) const
{
    if (regex.empty())
        return;
    Regex regexObj(regex.c_str(), (icase ? REG_ICASE : 0) | REG_EXTENDED | REG_NOSUB);
    for (const auto & val : value) {
        if (!regexObj.match(val.c_str()))
            throw Exception(tfm::format(_("'%s' is not an allowed value"), val));
    }
}

OptionStringList::ValueType OptionStringList::fromString(const std::string & value) const
{
    std::vector<std::string> tmp;
    std::string::size_type start{0};
    while (start < value.length()) {
        auto end = value.find_first_of(" ,\n", start);
        if (end == std::string::npos) {
            tmp.push_back(value.substr(start));
            break;
        }
        if (end-start != 0)
            tmp.push_back(value.substr(start, end - start));
        start = end + 1;
    }
    return tmp;
}

void OptionStringList::set(Priority priority, const ValueType & value)
{
    if (priority >= this->priority) {
        test(value);
        this->value = value;
        this->priority = priority;
    }
}

void OptionStringList::set(Priority priority, const std::string & value)
{
    if (priority >= this->priority)
        set(priority, fromString(value));
}

std::string OptionStringList::toString(const ValueType & value) const
{
    std::ostringstream oss;
    oss << "[";
    bool next{false};
    for (auto & val : value) {
        if (next)
            oss << ", ";
        else
            next = true;
        oss << val;
    }
    oss << "]";
    return oss.str();
}

OptionSeconds::OptionSeconds(ValueType defaultValue, ValueType min, ValueType max)
: OptionNumber(defaultValue, min, max) {}

OptionSeconds::OptionSeconds(ValueType defaultValue, ValueType min)
: OptionNumber(defaultValue, min) {}

OptionSeconds::OptionSeconds(ValueType defaultValue)
: OptionNumber(defaultValue) {}

OptionSeconds::ValueType OptionSeconds::fromString(const std::string & value) const
{
    if (value.empty())
        throw std::runtime_error(_("no value specified"));

    if (value == "-1" || value == "never") // Special cache timeout, meaning never
        return -1;

    std::size_t idx;
    auto res = std::stod(value, &idx);
    if (res < 0)
        throw std::runtime_error(tfm::format(_("seconds value '%s' must not be negative"), value));

    if (idx < value.length()) {
        if (idx < value.length() - 1)
            throw std::runtime_error(tfm::format(_("could not convert '%s' to seconds"), value));
        switch (value.back()) {
            case 's': case 'S':
                break;
            case 'm': case 'M':
                res *= 60;
                break;
            case 'h': case 'H':
                res *= 60 * 60;
                break;
            case 'd': case 'D':
                res *= 60 * 60 * 24;
                break;
            default:
                throw std::runtime_error(tfm::format(_("unknown unit '%s'"), value.back()));
        }
    }

    return res;
}

void OptionSeconds::set(Priority priority, const std::string & value)
{
    set(priority, fromString(value));
}


static std::string removeFileProt(const std::string & value)
{
    if (value.compare(0, 7, "file://") == 0)
        return value.substr(7);
    return value;
}


OptionPath::OptionPath(const std::string & defaultValue, bool exists, bool absPath)
: OptionString{defaultValue}, exists{exists}, absPath{absPath}
{
    this->defaultValue = removeFileProt(this->defaultValue);
    test(this->defaultValue);
    this->value = this->defaultValue;
}

OptionPath::OptionPath(const char * defaultValue, bool exists, bool absPath)
: OptionString{defaultValue}, exists{exists}, absPath{absPath}
{
    if (defaultValue) {
        this->defaultValue = removeFileProt(this->defaultValue);
        test(this->defaultValue);
        this->value = this->defaultValue;
    }
}

OptionPath::OptionPath(const std::string & defaultValue, const std::string & regex, bool icase, bool exists, bool absPath)
: OptionString{removeFileProt(defaultValue), regex, icase}, exists{exists}, absPath{absPath} 
{
    this->defaultValue = removeFileProt(this->defaultValue);
    test(this->defaultValue);
    this->value = this->defaultValue;
}

OptionPath::OptionPath(const char * defaultValue, const std::string & regex, bool icase, bool exists, bool absPath)
: OptionString{defaultValue, regex, icase}, exists{exists}, absPath{absPath}
{
    if (defaultValue) {
        this->defaultValue = removeFileProt(this->defaultValue);
        test(this->defaultValue);
        this->value = this->defaultValue;
    }
}

void OptionPath::test(const std::string & value) const
{
    if (absPath && value[0] != '/')
        throw Exception(tfm::format(_("given path '%s' is not absolute."), value));

    struct stat buffer; 
    if (exists && stat(value.c_str(), &buffer))
        throw Exception(tfm::format(_("given path '%s' does not exist."), value));
}

void OptionPath::set(Priority priority, const std::string & value)
{
    if (priority >= this->priority) {
        OptionString::test(value);
        auto val = removeFileProt(value);
        test(val);
        this->value = val;
        this->priority = priority;
    }
}

template class OptionNumber<std::int32_t>;
template class OptionNumber<std::uint32_t>;
template class OptionNumber<float>;

}
