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

#include "regex.hpp"
#include "bgettext/bgettext-lib.h"

Regex::Regex(const char * regex, int flags)
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

Regex & Regex::operator=(Regex && src) noexcept
{
    free();
    exp = src.exp;
    src.freed = true;
    freed = false;
    return *this;
}

bool Regex::match(const char *str)
{
    if (freed)
        throw Exception(_("Error: Regex object unusable. Its value moved/swaped to another Regex object."));

    return regexec(&exp, str, 0, nullptr, 0) == 0;
}

bool Regex::match(const char *str, std::vector<regmatch_t> & matches)
{
    if (freed)
        throw Exception(_("Error: Regex object unusable. Its value moved/swaped to another Regex object."));

    if (regexec(&exp, str, matches.size(), matches.data(), 0) == 0) {
        int i{0};
        while (matches[i].rm_so) ++i;
        matches.resize(i);
        return true;
    } else {
        return false;
    }
}
