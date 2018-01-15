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

#ifndef _OPTIONS_HPP
#define _OPTIONS_HPP

#include "utils/bgettext/bgettext-lib.h"
#include "utils/tinyformat/tinyformat.hpp"
#include "utils/regex/regex.hpp"

#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <sstream>

template <typename T>
bool fromString(T & out, const std::string & in, std::ios_base & (*manipulator)(std::ios_base &))
{
   std::istringstream iss(in);
   return !(iss >> manipulator >> out).fail();
}

constexpr const char * defTrueNames[]{"1", "yes", "true", "enabled", nullptr};
constexpr const char * defFalseNames[]{"0", "no", "false", "disabled", nullptr};

static bool strToBool(bool & out, const std::string & in, const char * const trueNames[] = defTrueNames,
               const char * const falseNames[] = defFalseNames)
{
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

class Option {
public:
    enum class Priority {
        PRIO_EMPTY = 0,
        PRIO_DEFAULT = 10,
        PRIO_MAINCONFIG = 20,
        PRIO_AUTOMATICCONFIG = 30,
        PRIO_REPOCONFIG = 40,
        PRIO_PLUGINDEFAULT = 50,
        PRIO_PLUGINCONFIG = 60,
        PRIO_COMMANDLINE = 70,
        PRIO_RUNTIME = 80
    };

    class Exception : public std::runtime_error {
    public:
        Exception(const std::string & msg) : runtime_error(msg) {}
        Exception(const char * msg) : runtime_error(msg) {}
    };

    Option(Priority priority = Priority::PRIO_EMPTY) : priority{priority} {}

    virtual Priority getPriority() const { return priority; }

    virtual void set(Priority priority, const std::string & value) = 0;
    virtual std::string getValueString() const = 0;

    virtual ~Option() = default;

protected:
    Priority priority;
};

template <class ParentOptionType, class Enable = void>
class OptionChild : public Option {
public:
    OptionChild(const ParentOptionType & parent)
    : parent(parent) {}

    Priority getPriority() const override { return priority != Priority::PRIO_EMPTY ? priority : parent.getPriority(); }

    void set(Priority priority, const typename ParentOptionType::ValueType & value)
    {
        if (priority >= this->priority) {
            parent.test(value);
            this->priority = priority;
            this->value = value;
        }
    }

    void set(Priority priority, const std::string & value) override
    {
        if (priority >= this->priority)
            set(priority, parent.fromString(value));
    }

    const typename ParentOptionType::ValueType & getValue() const { return priority != Priority::PRIO_EMPTY ? value : parent.getValue(); }

    std::string getValueString() const override
    {
        return priority != Priority::PRIO_EMPTY ? parent.toString(value) : parent.getValueString();
    }

private:
    const ParentOptionType & parent;
    typename ParentOptionType::ValueType value;
};

template <class ParentOptionType>
class OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type> : public Option {
public:
    OptionChild(const ParentOptionType & parent)
    : parent(parent) {}

    Priority getPriority() const override { return priority != Priority::PRIO_EMPTY ? priority : parent.getPriority(); }

    void set(Priority priority, const std::string & value) override
    {
        if (priority >= this->priority) {
            parent.test(value);
            this->priority = priority;
            this->value = value;
        }
    }

    const std::string & getValue() const { return priority != Priority::PRIO_EMPTY ? value : parent.getValue(); }

    std::string getValueString() const override
    {
        return priority != Priority::PRIO_EMPTY ? value : parent.getValue();
    }

private:
    const ParentOptionType & parent;
    std::string value;
};

template <typename T>
class OptionNumber : public Option {
//    """An option representing an value."""
public:
    typedef T ValueType;

    OptionNumber(T defaultValue, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
    : Option{Priority::PRIO_DEFAULT}, defaultValue{defaultValue}, min{min}, max{max}, value{defaultValue} { test(defaultValue); }

    void test(T value) const
    {
        if (value > max)
            throw Exception(tfm::format(_("given value [%d] should be less than "
                                            "allowed value [%d]."), value, max));
        else if (value < min)
            throw Exception(tfm::format(_("given value [%d] should be greater than "
                                            "allowed value [%d]."), value, min));
    }


    T fromString(const std::string & value) const
    {
        T val;
        if (::fromString<T>(val, value, std::dec))
            return val;
        throw Exception(_("invalid value"));
    }

    void set(Priority priority, T value)
    {
        if (priority >= this->priority) {
            test(value);
            this->value = value;
            this->priority = priority;
        }
    }

    void set(Priority priority, const std::string & value) override
    {
        set(priority, fromString(value));
    }

    T getValue() const { return value; }

    std::string toString(T value) const
    {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    std::string getValueString() const override { return toString(value); }

protected:
    T defaultValue;
    T min;
    T max;
    T value;
};

class OptionBool : public Option {
/*   """An option representing a boolean value.  The value can be one
    of 0, 1, yes, no, true, or false.
*/
public:
    typedef bool ValueType;

    OptionBool(bool defaultValue, const char * const trueVals[], const char * const falseVals[])
    : Option{Priority::PRIO_DEFAULT}, trueNames{trueVals}, falseNames{falseVals}, defaultValue{defaultValue}, value{defaultValue} {}

    OptionBool(bool defaultValue)
    : OptionBool{defaultValue, nullptr, nullptr} {}

    void test(bool) const {}

    bool fromString(const std::string & value) const
    {
        bool val;
        if (strToBool(val, value, getTrueNames(), getFalseNames()))
            return val;
        throw Exception(tfm::format(_("invalid boolean value '%s'"), value));
    }

    void set(Priority priority, bool value)
    {
        if (priority >= this->priority) {
            this->value = value;
            this->priority = priority;
        }
    }

    void set(Priority priority, const std::string & value) override
    {
        set(priority, fromString(value));
    }

    bool getValue() const { return value; }

    std::string toString(bool value) const
    {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    std::string getValueString() const override { return toString(value); }

    bool getDefaultValue() const noexcept { return defaultValue; }
    const char * const * getTrueNames() const noexcept { return trueNames ? trueNames : defTrueNames; }
    const char * const * getFalseNames() const noexcept { return falseNames ? falseNames : defFalseNames; }

protected:
    const char * const * const trueNames;
    const char * const * const falseNames;
    bool defaultValue;
    bool value;
};

class OptionString : public Option {
public:
    typedef std::string ValueType;

    OptionString(const std::string & defaultValue)
    : Option{Priority::PRIO_DEFAULT}, defaultValue{defaultValue}, value{defaultValue} {}

    OptionString(const char * defaultValue)
    {
        if (defaultValue) {
            this->value = this->defaultValue = defaultValue;
            this->priority = Priority::PRIO_DEFAULT;
        }
    }

    OptionString(const std::string & defaultValue, Regex && regex)
    : Option{Priority::PRIO_DEFAULT}, regex{std::unique_ptr<Regex>(new Regex(std::move(regex)))}, defaultValue{defaultValue}, value{defaultValue} { test(defaultValue); }

    OptionString(const char * defaultValue, Regex && regex)
    : regex{std::unique_ptr<Regex>(new Regex(std::move(regex)))}
    {
        if (defaultValue) {
            this->defaultValue = defaultValue;
            test(this->defaultValue);
            this->value = this->defaultValue;
            this->priority = Priority::PRIO_DEFAULT;
        }
    }

    void test(const std::string & value) const
    {
        if (regex && !regex->match(value.c_str()))
            throw Exception(tfm::format(_("'%s' is not an allowed value"), value));
    }

    void set(Priority priority, const std::string & value) override
    {
        if (priority >= this->priority) {
            test(value);
            this->value = value;
            this->priority = priority;
        }
    }

    const std::string & getValue() const
    {
        if (priority == Priority::PRIO_EMPTY)
            throw Exception(_("GetValue(): Value not set"));
        return value;
    }

    std::string getValueString() const override { return getValue(); }

protected:
    std::unique_ptr<Regex> regex;
    std::string defaultValue;
    std::string value;
};

template <typename T>
class OptionEnum : public Option {
public:
    typedef T ValueType;

    OptionEnum(T defaultValue, const std::vector<T> & enumVals)
    : Option{Priority::PRIO_DEFAULT}, enumVals{enumVals}, defaultValue{defaultValue}, value{defaultValue} { test(defaultValue); }

    OptionEnum(T defaultValue, std::vector<T> && enumVals)
    : Option{Priority::PRIO_DEFAULT}, enumVals{std::move(enumVals)}, defaultValue{defaultValue}, value{defaultValue} { test(defaultValue); }

    void test(T value) const
    {
        auto it = std::find(enumVals.begin(), enumVals.end(), value);
        if (it == enumVals.end())
            throw Exception(tfm::format(_("'%s' is not an allowed value"), value));
    }

    T fromString(const std::string & value) const
    {
        T val;
        if (::fromString<T>(val, value, std::dec))
            return val;
        throw Exception(_("invalid value"));
    }

    void set(Priority priority, T value)
    {
        if (priority >= this->priority) {
            test(value);
            this->value = value;
            this->priority = priority;
        }
    }

    void set(Priority priority, const std::string & value) override
    {
        set(priority, fromString(value));
    }

    T getValue() const { return value; }

    std::string toString(T value) const
    {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    std::string getValueString() const override { return toString(value); }

protected:
    std::vector<T> enumVals;
    T defaultValue;
    T value;
};

template <>
class OptionEnum<std::string> : public Option {
public:
    typedef std::string ValueType;

    OptionEnum(const std::string & defaultValue, const std::vector<std::string> & enumVals)
    : Option{Priority::PRIO_DEFAULT}, enumVals{enumVals}, defaultValue{defaultValue}, value{defaultValue} { test(defaultValue); }

    OptionEnum(const std::string & defaultValue, std::vector<std::string> && enumVals)
    : Option{Priority::PRIO_DEFAULT}, enumVals{std::move(enumVals)}, defaultValue{defaultValue}, value{defaultValue} { test(defaultValue); }

    void test(const std::string & value) const
    {
        auto it = std::find(enumVals.begin(), enumVals.end(), value);
        if (it == enumVals.end())
            throw Exception(tfm::format(_("'%s' is not an allowed value"), value));
    }

    void set(Priority priority, const std::string & value) override
    {
        if (priority >= this->priority) {
            test(value);
            this->value = value;
            this->priority = priority;
        }
    }

    std::string getValue() const { return value; }

    std::string getValueString() const override { return value; }

protected:
    std::vector<std::string> enumVals;
    std::string defaultValue;
    std::string value;
};

class OptionStringList : public Option {
public:
    typedef std::vector<std::string> ValueType;

    OptionStringList(const std::vector<std::string> & defaultValue)
    : Option{Priority::PRIO_DEFAULT}, defaultValue{defaultValue}, value{defaultValue} {}

    OptionStringList(const std::vector<std::string> & defaultValue, Regex && regex)
    : Option{Priority::PRIO_DEFAULT}, regex{std::unique_ptr<Regex>(new Regex(std::move(regex)))}, defaultValue{defaultValue}, value{defaultValue} { test(defaultValue); }

    OptionStringList(const std::string & defaultValue)
    : Option{Priority::PRIO_DEFAULT}, defaultValue{fromString(defaultValue)}, value{defaultValue} {
        this->value = this->defaultValue = fromString(defaultValue);
    }

    OptionStringList(const std::string & defaultValue, Regex && regex)
    : Option{Priority::PRIO_DEFAULT}, regex{std::unique_ptr<Regex>(new Regex(std::move(regex)))} {
        this->defaultValue = fromString(defaultValue);
        test(this->defaultValue);
        value = this->defaultValue;
    }

    void test(const std::vector<std::string> & value) const
    {
        if (regex) {
            for (const auto & val : value) {
                if (!regex->match(val.c_str()))
                    throw Exception(tfm::format(_("'%s' is not an allowed value"), val));
            }
        }
    }

    std::vector<std::string> fromString(const std::string & value) const
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

    void set(Priority priority, const std::vector<std::string> & value) {
        if (priority >= this->priority) {
            test(value);
            this->value = value;
            this->priority = priority;
        }
    }

    void set(Priority priority, const std::string & value) override
    {
        if (priority >= this->priority)
            set(priority, fromString(value));
    }

    const std::vector<std::string> & getValue() const { return value; }

    std::string toString(const std::vector<std::string> & value) const
    {
        std::ostringstream oss;
        oss << "[";
        bool next{false};
        for (auto & val : value) {
            if (next)
                oss << ", ";
            oss << val;
        }
        oss << "]";
        return oss.str();
    }

    std::string getValueString() const override { return toString(value); }

protected:
    std::unique_ptr<Regex> regex;
    std::vector<std::string> defaultValue;
    std::vector<std::string> value;
};

//Option for file path which can validate path existence.
class OptionPath : public OptionString {
public:
    OptionPath(const std::string & defaultValue, bool exists = false, bool absPath = false)
    : OptionString{defaultValue}, exists{exists}, absPath{absPath}
    {
        this->defaultValue = removeFileProt(this->defaultValue);
        test(this->defaultValue);
        this->value = this->defaultValue;
    }

    OptionPath(const char * defaultValue, bool exists = false, bool absPath = false)
    : OptionString{defaultValue}, exists{exists}, absPath{absPath}
    {
        if (defaultValue) {
            this->defaultValue = removeFileProt(this->defaultValue);
            test(this->defaultValue);
            this->value = this->defaultValue;
        }
    }

    OptionPath(const std::string & defaultValue, Regex && regex, bool exists = false, bool absPath = false)
    : OptionString{removeFileProt(defaultValue), std::move(regex)}, exists{exists}, absPath{absPath} 
    {
        this->defaultValue = removeFileProt(this->defaultValue);
        test(this->defaultValue);
        this->value = this->defaultValue;
    }

    OptionPath(const char * defaultValue, Regex && regex, bool exists = false, bool absPath = false)
    : OptionString{defaultValue, std::move(regex)}, exists{exists}, absPath{absPath}
    {
        if (defaultValue) {
            this->defaultValue = removeFileProt(this->defaultValue);
            test(this->defaultValue);
            this->value = this->defaultValue;
        }
    }

    void test(const std::string & value) const;

    void set(Priority priority, const std::string & value) override
    {
        if (priority >= this->priority) {
            OptionString::test(value);
            auto val = removeFileProt(value);
            test(val);
            this->value = val;
            this->priority = priority;
        }
    }

    std::string getValueString() const override { return getValue(); }

protected:
    static std::string removeFileProt(const std::string & value)
    {
        if (value.compare(0, 7, "file://") == 0)
            return value.substr(7);
        return value;
    }

    bool exists;
    bool absPath;
};

#endif
