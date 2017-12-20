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

#ifndef _CONFIG_REPO_HPP
#define _CONFIG_REPO_HPP

#include "ConfigMain.hpp"
#include "OptionChild.hpp"

#include <memory>

namespace libdnf {

class ConfigRepo : public Config {
public:
    ConfigRepo(ConfigMain & masterConfig);
    ~ConfigRepo();
    ConfigRepo(ConfigRepo && src);

    OptionString & name();
    OptionChild<OptionBool> & enabled();
    OptionChild<OptionString> & baseCacheDir();
    OptionStringList & baseUrl();
    OptionString & mirrorList();
    OptionString & metaLink();
    OptionString & type();
    OptionString & mediaId();
    OptionStringList & gpgKey();
    OptionStringListAppend & excludePkgs();
    OptionStringListAppend & includePkgs();
    OptionChild<OptionBool> & fastestMirror();
    OptionChild<OptionString> & proxy();
    OptionChild<OptionString> & proxyUsername();
    OptionChild<OptionString> & proxyPassword();
    OptionChild<OptionString> & username();
    OptionChild<OptionString> & password();
    OptionChild<OptionStringList> & protectedPackages();
    OptionChild<OptionBool> & gpgCheck();
    OptionChild<OptionBool> & repoGpgCheck();
    OptionChild<OptionBool> & enableGroups();
    OptionChild<OptionNumber<std::uint32_t> > & retries();
    OptionChild<OptionNumber<std::uint32_t> > & bandwidth();
    OptionChild<OptionNumber<std::uint32_t> > & minRate();
    OptionChild<OptionEnum<std::string> > & ipResolve();
    OptionChild<OptionNumber<float> > & throttle();
    OptionChild<OptionSeconds> & timeout();
    OptionChild<OptionNumber<std::uint32_t> > & maxParallelDownloads();
    OptionChild<OptionSeconds> & metadataExpire();
    OptionNumber<std::int32_t> & cost();
    OptionNumber<std::int32_t> & priority();
    OptionChild<OptionString> & sslCaCert();
    OptionChild<OptionBool> & sslVerify();
    OptionChild<OptionString> & sslClientCert();
    OptionChild<OptionString> & sslClientKey();
    OptionChild<OptionBool> & deltaRpm();
    OptionChild<OptionNumber<std::uint32_t> > & deltaRpmPercentage();
    OptionBool & skipIfUnavailable();
    // option recognized by other tools, e.g. gnome-software, but unused in dnf
    OptionString & enabledMetadata();
    // yum compatibility options
    OptionEnum<std::string> & failoverMethod();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}

#endif
