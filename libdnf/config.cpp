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

#include "config.hpp"

#include <exception>
#include <fstream>

void Configuration::readIniFile(const std::string& filePath, Option::Priority priority)
{
    std::ifstream ifs(filePath);
    if (!ifs)
        throw std::runtime_error("parseIni(): Can't open file");
    ifs.exceptions(std::ifstream::badbit);

    std::string section;
    std::string line;
    while (!ifs.eof()) {
        std::getline(ifs, line);
        auto start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos)
            continue;
        if (line[start] == '#')
            continue;
        auto end = line.find_last_not_of(" \t\r");

        if (line[start] == '[') {
            if (line[end] != ']')
                throw std::runtime_error("parseIni(): Missing ']'");
            section = line.substr(start + 1,end - start - 1);
            continue;
        }
        if (line[start] == '=')
            throw std::runtime_error("parseIni(): Missing key");
        auto eqlpos = line.find_first_of("=");
        if (eqlpos == std::string::npos)
            throw std::runtime_error("parseIni(): Missing '='");
        auto endkeypos = line.find_last_not_of(" \t", eqlpos - 1);
        auto valuepos = line.find_first_not_of(" \t", eqlpos + 1);
        auto key = line.substr(start, endkeypos - start + 1);
        auto value = line.substr(valuepos, end - valuepos + 1);

        try {
            setValue(priority, section, key, std::move(value), true);
        } catch (std::exception &) {
        }
    }
}
