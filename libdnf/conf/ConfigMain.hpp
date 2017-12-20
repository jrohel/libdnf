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

#ifndef _CONFIG_MAIN_HPP
#define _CONFIG_MAIN_HPP

#include "Config.hpp"
#include "OptionBool.hpp"
#include "OptionEnum.hpp"
#include "OptionNumber.hpp"
#include "OptionPath.hpp"
#include "OptionSeconds.hpp"
#include "OptionString.hpp"
#include "OptionStringList.hpp"
#include "OptionStringListAppend.hpp"

#include <memory>

namespace libdnf {

class ConfigMain : public Config {
public:
    ConfigMain();
    ~ConfigMain();

    OptionNumber<std::int32_t> & debugLevel();
    OptionNumber<std::int32_t> & errorLevel();
    OptionString & installRoot();
    OptionString & configFilePath();
    OptionBool & plugins();
    OptionStringList & pluginPath();
    OptionStringList & pluginConfPath();
    OptionString & persistDir();
    OptionBool & transformDb();
    OptionNumber<std::int32_t> & recent();
    OptionBool & resetNice();
    OptionString & systemCacheDir();
    OptionBool & cacheOnly();
    OptionBool & keepCache();
    OptionString & logDir();
    OptionStringList & reposDir();
    OptionBool & debugSolver();
    OptionStringListAppend & installOnlyPkgs();
    OptionStringList & groupPackageTypes();

    /*  NOTE: If you set this to 2, then because it keeps the current
    kernel it means if you ever install an "old" kernel it'll get rid
    of the newest one so you probably want to use 3 as a minimum
    ... if you turn it on. */
    OptionNumber<std::uint32_t> & installOnlyLimit();

    OptionStringListAppend & tsFlags();
    OptionBool & assumeYes();
    OptionBool & assumeNo();
    OptionBool & checkConfigFileAge();
    OptionBool & defaultYes();
    OptionBool & diskSpaceCheck();
    OptionBool & localPkgGpgCheck();
    OptionBool & obsoletes();
    OptionBool & showDupesFromRepos();
    OptionBool & exitOnLock();
    OptionSeconds & metadataTimerSync();
    OptionStringList & disableExcludes();
    OptionEnum<std::string> & multilibPolicy(); // :api
    OptionBool & best(); // :api
    OptionBool & installWeakDeps();
    OptionString & bugtrackerUrl();
    OptionEnum<std::string> & color();
    OptionString & colorListInstalledOlder();
    OptionString & colorListInstalledNewer();
    OptionString & colorListInstalledReinstall();
    OptionString & colorListInstalledExtra();
    OptionString & colorListAvailableUpgrade();
    OptionString & colorListAvailableDowngrade();
    OptionString & colorListAvailableReinstall();
    OptionString & colorListAvailableInstall();
    OptionString & colorUpdateInstalled();
    OptionString & colorUpdateLocal();
    OptionString & colorUpdateRemote();
    OptionString & colorSearchMatch();
    OptionBool & historyRecord();
    OptionStringList & historyRecordPackages();
    OptionString & rpmVerbosity();
    OptionBool & strict(); // :api
    OptionBool & skipBroken(); // :yum-compatibility
    OptionBool & autocheckRunningKernel(); // :yum-compatibility
    OptionBool & cleanRequirementsOnRemove();
    OptionEnum<std::string> & historyListView();
    OptionBool & upgradeGroupObjectsUpgrade();
    OptionPath & destDir();
    OptionString & comment();
    OptionBool & downloadOnly();
    OptionBool & ignoreArch();

    // Repo main config
    OptionNumber<std::uint32_t> & retries();
    OptionString & cacheDir();
    OptionBool & fastestMirror();
    OptionStringListAppend & excludePkgs();
    OptionStringListAppend & includePkgs();
    OptionString & proxy();
    OptionString & proxyUsername();
    OptionString & proxyPassword();
    OptionStringList & protectedPackages();
    OptionString & username();
    OptionString & password();
    OptionBool & gpgCheck();
    OptionBool & repoGpgCheck();
    OptionBool & enabled();
    OptionBool & enableGroups();
    OptionNumber<std::uint32_t> & bandwidth();
    OptionNumber<std::uint32_t> & minRate();
    OptionEnum<std::string> & ipResolve();
    OptionNumber<float> & throttle();
    OptionSeconds & timeout();
    OptionNumber<std::uint32_t> & maxParallelDownloads();
    OptionSeconds & metadataExpire();
    OptionString & sslCaCert();
    OptionBool & sslVerify();
    OptionString & sslClientCert();
    OptionString & sslClientKey();
    OptionBool & deltaRpm();
    OptionNumber<std::uint32_t> & deltaRpmPercentage();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}

#endif
