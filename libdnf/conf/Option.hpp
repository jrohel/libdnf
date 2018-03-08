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

#ifndef _OPTION_HPP
#define _OPTION_HPP

#include <stdexcept>
#include <string>

namespace libdnf {

class Option {
public:
    enum class Priority {
        EMPTY = 0,
        DEFAULT = 10,
        MAINCONFIG = 20,
        AUTOMATICCONFIG = 30,
        REPOCONFIG = 40,
        PLUGINDEFAULT = 50,
        PLUGINCONFIG = 60,
        COMMANDLINE = 70,
        RUNTIME = 80
    };

    class Exception : public std::runtime_error {
    public:
        Exception(const std::string & msg) : runtime_error(msg) {}
        Exception(const char * msg) : runtime_error(msg) {}
    };

    Option(Priority priority = Priority::EMPTY);
    virtual Priority getPriority() const;
    virtual void set(Priority priority, const std::string & value) = 0;
    virtual std::string getValueString() const = 0;
    virtual ~Option() = default;

protected:
    Priority priority;
};

inline Option::Option(Priority priority)
: priority(priority) {}

inline Option::Priority Option::getPriority() const
{
    return priority;
}

}

#endif
