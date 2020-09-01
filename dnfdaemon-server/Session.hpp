/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-server: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-server is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-server is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-server.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNFDAEMON_SERVER_SESSION_HPP
#define DNFDAEMON_SERVER_SESSION_HPP

#include "types.hpp"

#include <libdnf/base/base.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <memory>
#include <string>
#include <vector>

class Session;

class IDbusSessionService {
public:
    IDbusSessionService(Session &session) : session(session) {};
    virtual ~IDbusSessionService() {}
    virtual void dbus_register(const std::string object_path) = 0;
    virtual void dbus_deregister() = 0;
protected:
    Session &session;
};

class Session {
public:
    Session(sdbus::IConnection &connection, KeyValueMap session_configuration, std::string object_path);
    ~Session();
    sdbus::IConnection &connection;
    libdnf::Base base;
    template <typename ItemType>
    ItemType session_configuration_value(const std::string key, const ItemType &default_value);
private:
    KeyValueMap session_configuration;
    std::string object_path;
    std::vector<std::unique_ptr<IDbusSessionService>> services{};
};

#endif