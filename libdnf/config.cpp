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

#include <dirent.h>
#include <glob.h>
#include <string.h>
#include <sys/types.h>

static void addFromFile(std::ostream & out, const std::string & filePath)
{
    std::ifstream ifs(filePath);
    if (!ifs)
        throw std::runtime_error("parseIniFile(): Can't open file");
    ifs.exceptions(std::ifstream::badbit);

    std::string line;
    while (!ifs.eof()) {
        std::getline(ifs, line);
        auto start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos)
            continue;
        if (line[start] == '#')
            continue;
        auto end = line.find_last_not_of(" \t\r");

        out.write(line.c_str()+start, end - start + 1);
        out.put(' ');
    }
}

static void addFromFiles(std::ostream & out, const std::string & globPath)
{
    glob_t globBuf;
    glob(globPath.c_str(), GLOB_MARK | GLOB_NOSORT, NULL, &globBuf);
    for (size_t i = 0; i < globBuf.gl_pathc; ++i) {
        auto path = globBuf.gl_pathv[i];
        if (path[strlen(path)-1] != '/')
            addFromFile(out, path);
    }
    globfree(&globBuf);
}

std::string resolveGlobs(const std::string & strWithGlobs)
{
    std::ostringstream res;
    std::string::size_type start{0};
    while (start < strWithGlobs.length()) {
        auto end = strWithGlobs.find_first_of(" ,\n", start);
        if (strWithGlobs.compare(start, 5, "glob:") == 0) {
            start += 5;
            if (start >= strWithGlobs.length())
                break;
            if (end == std::string::npos) {
                addFromFiles(res, strWithGlobs.substr(start));
                break;
            }
            if (end - start != 0)
                addFromFiles(res, strWithGlobs.substr(start, end - start));
        } else {
            if (end == std::string::npos) {
                res << strWithGlobs.substr(start);
                break;
            }
            if (end - start != 0)
                res << strWithGlobs.substr(start, end - start) << " ";
        }
        start = end + 1;
    }
    return res.str();
}

void Configuration::readIniFile(const std::string& filePath, Option::Priority priority)
{
    std::ifstream ifs(filePath);
    if (!ifs)
        throw std::runtime_error("parseIniFile(): Can't open file");
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
                throw std::runtime_error("parseIniFile(): Missing ']'");
            section = line.substr(start + 1,end - start - 1);
            continue;
        }
        if (line[start] == '=')
            throw std::runtime_error("parseIniFile(): Missing key");
        auto eqlpos = line.find_first_of("=");
        if (eqlpos == std::string::npos)
            throw std::runtime_error("parseIniFile(): Missing '='");
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

static bool endsWith(const std::string & str, const std::string & suffix)
{
    return str.size() >= suffix.size()
        && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

void Configuration::readRepoFiles(const std::string & dirPath, Option::Priority priority)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp = opendir(dirPath.c_str())) == NULL) {
        throw std::runtime_error("");
    }

    errno = 0;
    while ((dirp = readdir(dp)) != NULL) {
        auto fname = std::string(dirp->d_name);
        if (endsWith(fname, ".repo"))
            readIniFile(dirPath + fname, priority);
        errno = 0;
    }
    if (errno) {
        //log
    }
    closedir(dp);
}
