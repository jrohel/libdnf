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

#include "Config.hpp"

#include "bgettext/bgettext-lib.h"
#include "tinyformat/tinyformat.hpp"

#include <utility>

namespace libdnf {

// ========== OptionBinding class ===============

OptionBinding::OptionBinding(Config & config, Option & option, const std::string & name,
                             NewStringFunc && newString, GetValueStringFunc && getValueString)
: option(option), newStr(std::move(newString)), getValueStr(std::move(getValueString))
{
    config.optBinds().add(name, *this);
}

OptionBinding::OptionBinding(Config & config, Option & option, const std::string & name)
: option(option)
{
    config.optBinds().add(name, *this);
}

Option::Priority OptionBinding::getPriority() const
{
    return option.getPriority();
}

void OptionBinding::newString(Option::Priority priority, const std::string & value)
{
    if (newStr)
        newStr(priority, value);
    else
        option.set(priority, value);
}

std::string OptionBinding::getValueString() const
{
    if (getValueStr) {
        return getValueStr();
    }else
        return option.getValueString();
}


// =========== OptionBinds class ===============

OptionBinding & OptionBinds::at(const std::string & id)
{
    auto item = items.find(id);
    if (item == items.end())
        throw std::runtime_error(tfm::format(_("Configuration: OptionBinding with id \"%s\" does not exist"), id));
    return item->second;
}

const OptionBinding & OptionBinds::at(const std::string & id) const 
{
    auto item = items.find(id);
    if (item == items.end())
        throw std::runtime_error(tfm::format(_("Configuration: OptionBinding with id \"%s\" does not exist"), id));
    return item->second;
}

OptionBinds::iterator OptionBinds::add(const std::string & id, OptionBinding & optBind)
{
    auto item = items.find(id);
    if (item != items.end())
        throw std::runtime_error(tfm::format(_("Configuration: OptionBinding with id \"%s\" exist"), id));
    auto res = items.insert({id, optBind});
    return res.first;
}

}
