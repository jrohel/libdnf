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

#ifndef _REGEX_HPP
#define _REGEX_HPP

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <regex.h>

class Regex {
public:
    class Exception : public std::runtime_error {
    public:
        Exception(const std::string & msg) : runtime_error(msg) {}
        Exception(const char * msg) : runtime_error(msg) {}
    };

    class LibException : public Exception {
    public:
        LibException(int code, const std::string & msg) : Exception(msg), ecode(code) {}
        LibException(int code, const char * msg) : Exception(msg), ecode(code) {}
        int code() const noexcept { return ecode; }
    protected:
        int ecode;
    };

    Regex(const Regex & src) = delete;
    Regex(Regex && src) noexcept;
    Regex(const char * regex, int flags);
    Regex & operator=(const Regex & src) = delete;
    Regex & operator=(Regex && src) noexcept;
    void swap(Regex & x);
    bool match(const char *str);
    bool match(const char *str, std::vector<regmatch_t> & matches);
    ~Regex();

private:
    void free() noexcept;

    bool freed{false};
    regex_t exp;
};

inline Regex::Regex(Regex && src) noexcept
: exp(src.exp)
{
    src.freed = true;
}

inline void Regex::swap(Regex & x)
{
    std::swap(*this, x);
}

inline Regex::~Regex()
{
    free();
}

inline void Regex::free() noexcept
{
    if (!freed)
        regfree(&exp);
}

#endif
