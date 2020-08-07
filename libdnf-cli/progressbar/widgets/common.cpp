/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "common.hpp"

#include <fmt/format.h>


namespace libdnf::cli::progressbar {


static const char * units[] = {
    "B",
    "kB",
    "MB",
    "GB",
    "TB",
    "PB",
    "EB",
    "ZB",
    "YB",
};


std::string format_size(int64_t num) {
    float i = static_cast<float>(num);
    int index = 0;
    while (i > 999) {
        i /= 1024;
        index++;
    }
    return fmt::format("{0:5.1f} {1:>2s}", i, units[index]);
}


/// Display time in [ -]MM:SS format.
/// The explicit 'negative' argument allows displaying '-00:00'.
std::string format_time_mmss(int64_t num, bool negative) {
    char negative_sign = negative ? '-' : ' ';
    int64_t seconds = std::abs(num) % 60;
    int64_t minutes = std::abs(num) / 60;
    if (minutes > 99) {
        minutes = 99;
    }
    return fmt::format("{0}{1:02d}:{2:02d}", negative_sign, minutes, seconds);
}


}  // namespace libdnf::cli::progressbar
