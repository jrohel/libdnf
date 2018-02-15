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

#include <functional>
#include <string>
#include <vector>
#include <sstream>

namespace libdnf {

constexpr const char * defTrueNames[]{"1", "yes", "true", "enabled", nullptr};
constexpr const char * defFalseNames[]{"0", "no", "false", "disabled", nullptr};

template <typename T>
bool fromString(T & out, const std::string & in, std::ios_base & (*manipulator)(std::ios_base &))
{
   std::istringstream iss(in);
   return !(iss >> manipulator >> out).fail();
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

    Option(Priority priority = Priority::PRIO_EMPTY);
    virtual Priority getPriority() const;
    virtual void set(Priority priority, const std::string & value) = 0;
    virtual std::string getValueString() const = 0;
    virtual ~Option() = default;

protected:
    Priority priority;
};


template <class ParentOptionType, class Enable = void>
class OptionChild : public Option {
public:
    OptionChild(const ParentOptionType & parent);
    Priority getPriority() const override;
    void set(Priority priority, const typename ParentOptionType::ValueType & value);
    void set(Priority priority, const std::string & value) override;
    const typename ParentOptionType::ValueType getValue() const;
    const typename ParentOptionType::ValueType getDefaultValue() const;
    std::string getValueString() const override;

private:
    const ParentOptionType & parent;
    typename ParentOptionType::ValueType value;
};


template <class ParentOptionType>
class OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type> : public Option {
public:
    OptionChild(const ParentOptionType & parent);
    Priority getPriority() const override;
    void set(Priority priority, const std::string & value) override;
    const std::string & getValue() const;
    const std::string & getDefaultValue() const;
    std::string getValueString() const override;

private:
    const ParentOptionType & parent;
    std::string value;
};

template <typename T>
class OptionNumber : public Option {
//    """An option representing an value."""
public:
    typedef T ValueType;
    typedef std::function<ValueType (const std::string &)> FromStringFunc;

    OptionNumber(T defaultValue, T min, T max);
    OptionNumber(T defaultValue, T min);
    OptionNumber(T defaultValue);
    OptionNumber(T defaultValue, T min, T max, FromStringFunc && fromStringFunc);
    OptionNumber(T defaultValue, T min, FromStringFunc && fromStringFunc);
    OptionNumber(T defaultValue, FromStringFunc && fromStringFunc);
    void test(ValueType value) const;
    T fromString(const std::string & value) const;
    void set(Priority priority, ValueType value);
    void set(Priority priority, const std::string & value) override;
    T getValue() const;
    T getDefaultValue() const;
    std::string toString(ValueType value) const;
    std::string getValueString() const override;

protected:
    FromStringFunc fromStringUser;
    ValueType defaultValue;
    ValueType min;
    ValueType max;
    ValueType value;
};


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


class OptionString : public Option {
public:
    typedef std::string ValueType;

    OptionString(const std::string & defaultValue);
    OptionString(const char * defaultValue);
    OptionString(const std::string & defaultValue, const std::string & regex, bool icase);
    OptionString(const char * defaultValue, const std::string & regex, bool icase);
    void test(const std::string & value) const;
    void set(Priority priority, const std::string & value) override;
    const std::string & getValue() const;
    const std::string & getDefaultValue() const noexcept;
    std::string getValueString() const override;

protected:
    std::string regex;
    bool icase;
    std::string defaultValue;
    std::string value;
};


template <typename T>
class OptionEnum : public Option {
public:
    typedef T ValueType;
    typedef std::function<ValueType (const std::string &)> FromStringFunc;

    OptionEnum(ValueType defaultValue, const std::vector<ValueType> & enumVals);
    OptionEnum(ValueType defaultValue, std::vector<ValueType> && enumVals);
    OptionEnum(ValueType defaultValue, const std::vector<ValueType> & enumVals, FromStringFunc && fromStringFunc);
    OptionEnum(ValueType defaultValue, std::vector<ValueType> && enumVals, FromStringFunc && fromStringFunc);
    void test(ValueType value) const;
    ValueType fromString(const std::string & value) const;
    void set(Priority priority, ValueType value);
    void set(Priority priority, const std::string & value) override;
    T getValue() const;
    T getDefaultValue() const;
    std::string toString(ValueType value) const;
    std::string getValueString() const override;

protected:
    FromStringFunc fromStringUser;
    std::vector<ValueType> enumVals;
    ValueType defaultValue;
    ValueType value;
};


template <>
class OptionEnum<std::string> : public Option {
public:
    typedef std::string ValueType;
    typedef std::function<ValueType (const std::string &)> FromStringFunc;

    OptionEnum(const std::string & defaultValue, const std::vector<ValueType> & enumVals);
    OptionEnum(const std::string & defaultValue, std::vector<ValueType> && enumVals);
    OptionEnum(const std::string & defaultValue, const std::vector<ValueType> & enumVals, FromStringFunc && fromStringFunc);
    OptionEnum(const std::string & defaultValue, std::vector<ValueType> && enumVals, FromStringFunc && fromStringFunc);
    void test(const std::string & value) const;
    std::string fromString(const std::string & value) const;
    void set(Priority priority, const std::string & value) override;
    const std::string & getValue() const;
    const std::string & getDefaultValue() const;
    std::string getValueString() const override;

protected:
    FromStringFunc fromStringUser;
    std::vector<ValueType> enumVals;
    ValueType defaultValue;
    ValueType value;
};


class OptionStringList : public Option {
public:
    typedef std::vector<std::string> ValueType;

    OptionStringList(const ValueType & defaultValue);
    OptionStringList(const ValueType & defaultValue, const std::string & regex, bool icase);
    OptionStringList(const std::string & defaultValue);
    OptionStringList(const std::string & defaultValue, const std::string & regex, bool icase);
    void test(const std::vector<std::string> & value) const;
    ValueType fromString(const std::string & value) const;
    void set(Priority priority, const ValueType & value);
    void set(Priority priority, const std::string & value) override;
    const ValueType & getValue() const;
    const ValueType & getDefaultValue() const;
    std::string toString(const ValueType & value) const;
    std::string getValueString() const override;

protected:
    std::string regex;
    bool icase;
    ValueType defaultValue;
    ValueType value;
};

/* An option representing an integer value of seconds.

   Valid inputs: 100, 1.5m, 90s, 1.2d, 1d, 0xF, 0.1, -1, never.
   Invalid inputs: -10, -0.1, 45.6Z, 1d6h, 1day, 1y.

   Return value will always be an integer
*/
class OptionSeconds : public OptionNumber<std::int32_t> {
public:
    OptionSeconds(ValueType defaultValue, ValueType min, ValueType max);
    OptionSeconds(ValueType defaultValue, ValueType min);
    OptionSeconds(ValueType defaultValue);
    ValueType fromString(const std::string & value) const;
    using OptionNumber::set;
    void set(Priority priority, const std::string & value) override;
};

//Option for file path which can validate path existence.
class OptionPath : public OptionString {
public:
    OptionPath(const std::string & defaultValue, bool exists = false, bool absPath = false);
    OptionPath(const char * defaultValue, bool exists = false, bool absPath = false);
    OptionPath(const std::string & defaultValue, const std::string & regex, bool icase, bool exists = false, bool absPath = false);
    OptionPath(const char * defaultValue, const std::string & regex, bool icase, bool exists = false, bool absPath = false);
    void test(const std::string & value) const;
    void set(Priority priority, const std::string & value) override;

protected:
    bool exists;
    bool absPath;
};


// implementation of inline methods
template <class ParentOptionType, class Enable>
inline OptionChild<ParentOptionType, Enable>::OptionChild(const ParentOptionType & parent)
: parent(parent) {}

template <class ParentOptionType, class Enable>
inline Option::Priority OptionChild<ParentOptionType, Enable>::getPriority() const
{
    return priority != Priority::PRIO_EMPTY ? priority : parent.getPriority(); 
}

template <class ParentOptionType, class Enable>
inline void OptionChild<ParentOptionType, Enable>::set(Priority priority, const typename ParentOptionType::ValueType & value)
{
    if (priority >= this->priority) {
        parent.test(value);
        this->priority = priority;
        this->value = value;
    }
}

template <class ParentOptionType, class Enable>
inline void OptionChild<ParentOptionType, Enable>::set(Priority priority, const std::string & value)
{
    if (priority >= this->priority)
        set(priority, parent.fromString(value));
}

template <class ParentOptionType, class Enable>
inline const typename ParentOptionType::ValueType OptionChild<ParentOptionType, Enable>::getValue() const
{
    return priority != Priority::PRIO_EMPTY ? value : parent.getValue(); 
}

template <class ParentOptionType, class Enable>
inline const typename ParentOptionType::ValueType OptionChild<ParentOptionType, Enable>::getDefaultValue() const { return parent.getDefaultValue(); }

template <class ParentOptionType, class Enable>
inline std::string OptionChild<ParentOptionType, Enable>::getValueString() const
{
    return priority != Priority::PRIO_EMPTY ? parent.toString(value) : parent.getValueString();
}


template <class ParentOptionType>
inline OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::OptionChild(const ParentOptionType & parent)
: parent(parent) {}

template <class ParentOptionType>
inline Option::Priority OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::getPriority() const
{
    return priority != Priority::PRIO_EMPTY ? priority : parent.getPriority(); 
}

template <class ParentOptionType>
inline void OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::set(Priority priority, const std::string & value)
{
    if (priority >= this->priority) {
        parent.test(value);
        this->priority = priority;
        this->value = value;
    }
}

template <class ParentOptionType>
inline const std::string & OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::getValue() const
{
    return priority != Priority::PRIO_EMPTY ? value : parent.getValue();
}

template <class ParentOptionType>
inline const std::string & OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::getDefaultValue() const
{
    return parent.getDefaultValue();
}

template <class ParentOptionType>
inline std::string OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::getValueString() const
{
    return priority != Priority::PRIO_EMPTY ? value : parent.getValue();
}

inline Option::Option(Priority priority)
: priority(priority) {}

inline Option::Priority Option::getPriority() const
{
    return priority;
}

template <typename T>
inline T OptionNumber<T>::getValue() const
{
    return value;
}

template <typename T>
inline T OptionNumber<T>::getDefaultValue() const
{
    return defaultValue;
}

template <typename T>
inline std::string OptionNumber<T>::getValueString() const
{
    return toString(value); 
}

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

inline const std::string & OptionString::getDefaultValue() const noexcept
{
    return defaultValue;
}

inline std::string OptionString::getValueString() const
{
    return getValue();
}


inline const std::string & OptionEnum<std::string>::getValue() const
{
    return value;
}

inline const std::string & OptionEnum<std::string>::getDefaultValue() const
{
    return defaultValue;
}

inline std::string OptionEnum<std::string>::getValueString() const
{
    return value;
}

inline const OptionStringList::ValueType & OptionStringList::getValue() const
{
    return value;
}

inline const OptionStringList::ValueType & OptionStringList::getDefaultValue() const
{
    return defaultValue;
}

inline std::string OptionStringList::getValueString() const
{
    return toString(value); 
}

extern template class OptionNumber<std::int32_t>;
extern template class OptionNumber<std::uint32_t>;
extern template class OptionNumber<float>;

}

#endif
