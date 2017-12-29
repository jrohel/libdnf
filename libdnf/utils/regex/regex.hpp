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

#ifndef _REGEX_HPP
#define _REGEX_HPP

#include "bgettext/bgettext-lib.h"

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
        LibException(int code, const std::string & msg) : Exception(msg), ecode{code} {}
        LibException(int code, const char * msg) : Exception(msg), ecode{code} {}
        int code() const noexcept { return ecode; }
    protected:
        int ecode;
    };

    Regex(const Regex & src) = delete;
    Regex(Regex && src) noexcept : exp{src.exp} { src.freed = true; }
    Regex(const char * regex, int flags);

    Regex & operator=(const Regex & src) = delete;
    Regex & operator=(Regex && src) noexcept;

    void swap(Regex & x) { std::swap(*this, x); }

    bool match(const char *str);
    bool match(const char *str, std::vector<regmatch_t> & matches);

    ~Regex() { free(); }

private:
    void free() noexcept { if (!freed) regfree(&exp); }

    bool freed{false};
    regex_t exp;
};

inline Regex::Regex(const char * regex, int flags)
{
    auto errCode = regcomp(&exp, regex, flags);
    if (errCode != 0) {
        auto size = regerror(errCode, &exp, nullptr, 0);
        if (size) {
            std::string msg(size, '\0');
            regerror(errCode, &exp, &msg.front(), size);
            throw LibException(errCode, msg);
        }
        throw LibException(errCode, "");
    }
}

inline Regex & Regex::operator=(Regex && src) noexcept
{
    free();
    exp = src.exp;
    src.freed = true;
    freed = false;
    return *this;
}

inline bool Regex::match(const char *str)
{
    if (freed)
        throw Exception(_("Error: Regex object unusable. Its value moved/swaped to another Regex object."));

    //regexec() returns 0 on match, otherwise REG_NOMATCH
    return regexec(&exp, str, 0, nullptr, 0) == 0;
}

inline bool Regex::match(const char *str, std::vector<regmatch_t> & matches)
{
    if (freed)
        throw Exception(_("Error: Regex object unusable. Its value moved/swaped to another Regex object."));

    //regexec() returns 0 on match, otherwise REG_NOMATCH
    if (regexec(&exp, str, matches.size(), matches.data(), 0) == 0) {
        int i{0};
        while (matches[i].rm_so) ++i;
        matches.resize(i);
        return true;
    } else {
        return false;
    }
}

#endif
