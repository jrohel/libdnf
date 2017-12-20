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

#include "ConfigMain.hpp"
#include "Const.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <istream>
#include <ostream>
#include <fstream>
#include <utility>

#include <dirent.h>
#include <glob.h>
#include <string.h>
#include <sys/types.h>

#include "bgettext/bgettext-lib.h"
#include "tinyformat/tinyformat.hpp"

namespace libdnf {

/*  Function converts a friendly bandwidth option to bytes.  The input
    should be a string containing a (possibly floating point)
    number followed by an optional single character unit. Valid
    units are 'k', 'M', 'G'. Case is ignored. The convention that
    1k = 1024 bytes is used.

    Valid inputs: 100, 123M, 45.6k, 12.4G, 100K, 786.3, 0.
    Invalid inputs: -10, -0.1, 45.6L, 123Mb.
*/
static int strToBytes(const std::string & str)
{
    if (str.empty())
        throw std::runtime_error(_("no value specified"));

    std::size_t idx;
    auto res = std::stod(str, &idx);
    if (res < 0)
        throw std::runtime_error(tfm::format(_("seconds value '%s' must not be negative"), str));

    if (idx < str.length()) {
        if (idx < str.length() - 1)
            throw std::runtime_error(tfm::format(_("could not convert '%s' to bytes"), str));
        switch (str.back()) {
            case 'k': case 'K':
                res *= 1024;
                break;
            case 'm': case 'M':
                res *= 1024 * 1024;
                break;
            case 'g': case 'G':
                res *= 1024 * 1024 * 1024;
                break;
            default:
                throw std::runtime_error(tfm::format(_("unknown unit '%s'"), str.back()));
        }
    }

    return res;
}

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

/* Replaces globs (like /etc/foo.d/\\*.foo) by content of matching files.
 * Ignores comment lines (start with '#') and blank lines in files.
 * Result:
 * Words delimited by spaces. Characters ',' and '\n' are replaced by spaces.
 * Extra spaces are removed.
 */
static std::string resolveGlobs(const std::string & strWithGlobs)
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


// =========== ConfigMain class ===============

class ConfigMain::Impl {
    friend class ConfigMain;

    Impl(Config & owner) : owner(owner) {}

    Config & owner;

    OptionNumber<std::int32_t> debugLevel{2, 0, 10};
    OptionBinding debugLevelBinding{owner, debugLevel, "debug_level"};

    OptionNumber<std::int32_t> errorLevel{2, 0, 10};
    OptionBinding errorLevelBinding{owner, errorLevel, "error_level"};

    OptionPath installRoot{"/"};
    OptionBinding installRootBinding{owner, installRoot, "installroot"};

    OptionPath configFilePath{CONF_FILENAME};
    OptionBinding configFilePathBinding{owner, configFilePath, "config_file_path"};

    OptionBool plugins{true};
    OptionBinding pluginsBinding{owner, plugins, "plugins"};

    OptionStringList pluginPath{std::vector<std::string>{}};
    OptionBinding pluginPathBinding{owner, pluginPath, "pluginpath"};

    OptionStringList pluginConfPath{std::vector<std::string>{}};
    OptionBinding pluginConfPathBinding{owner, pluginConfPath, "pluginconfpath"};

    OptionPath persistDir{PERSISTDIR};
    OptionBinding persistDirBinding{owner, persistDir, "persistdir"};

    OptionBool transformDb{true};
    OptionBinding transformDbBinding{owner, transformDb, "transformdb"};

    OptionNumber<std::int32_t> recent{7, 0};
    OptionBinding recentBinding{owner, recent, "recent"};

    OptionBool resetNice{true};
    OptionBinding resetNiceBinding{owner, resetNice, "reset_nice"};

    OptionPath systemCacheDir{SYSTEM_CACHEDIR};
    OptionBinding systemCacheDirBindings{owner, systemCacheDir, "system_cachedir"};

    OptionBool cacheOnly{false};
    OptionBinding cacheOnlyBinding{owner, cacheOnly, "cacheonly"};

    OptionBool keepCache{false};
    OptionBinding keepCacheBinding{owner, keepCache, "keepcache"};

    OptionString logDir{"/var/log"};
    OptionBinding logDirBinding{owner, logDir, "logdir"};

    OptionStringList reposDir{{"/etc/yum.repos.d", "/etc/yum/repos.d", "/etc/distro.repos.d"}};
    OptionBinding reposDirBinding{owner, reposDir, "reposdir"};

    OptionBool debugSolver{false};
    OptionBinding debugSolverBinding{owner, debugSolver, "debug_solver"};

    OptionStringListAppend installOnlyPkgs{INSTALLONLYPKGS};
    OptionBinding installOnlyPkgsBinding{owner, installOnlyPkgs, "installonlypkgs"};

    OptionStringList groupPackageTypes{GROUP_PACKAGE_TYPES};
    OptionBinding groupPackageTypesBinding{owner, groupPackageTypes, "group_package_types"};

    OptionNumber<std::uint32_t> installOnlyLimit{3, 0, 
        [](const std::string & value)->std::uint32_t{
            if (value == "<off>")
                return 0;
            try {
                return std::stoul(value);
            }
            catch (...) {
                return 0;
            }
        }
    };
    OptionBinding installOnlyLimitBinding{owner, installOnlyLimit, "installonly_limit"};

    OptionStringListAppend tsFlags{std::vector<std::string>{}};
    OptionBinding tsFlagsBinding{owner, tsFlags, "tsflags"};

    OptionBool assumeYes{false};
    OptionBinding assumeYesBinding{owner, assumeYes, "assumeyes"};

    OptionBool assumeNo{false};
    OptionBinding assumeNoBinding{owner, assumeNo, "assumeno"};

    OptionBool checkConfigFileAge{true};
    OptionBinding checkConfigFileAgeBinding{owner, checkConfigFileAge, "check_config_file_age"};

    OptionBool defaultYes{false};
    OptionBinding defaultYesBinding{owner, defaultYes, "defaultyes"};

    OptionBool diskSpaceCheck{true};
    OptionBinding diskSpaceCheckBinding{owner, diskSpaceCheck, "diskspacecheck"};

    OptionBool localPkgGpgCheck{false};
    OptionBinding localPkgGpgCheckBinding{owner, localPkgGpgCheck, "localpkg_gpgcheck"};

    OptionBool obsoletes{true};
    OptionBinding obsoletesBinding{owner, obsoletes, "obsoletes"};

    OptionBool showDupesFromRepos{false};
    OptionBinding showDupesFromReposBinding{owner, showDupesFromRepos, "showdupesfromrepos"};

    OptionBool exitOnLock{false};
    OptionBinding exitOnLockBinding{owner, exitOnLock, "exit_on_lock"};

    OptionSeconds metadataTimerSync{60 * 60 * 3}; // 3 hours
    OptionBinding metadataTimerSyncBinding{owner, metadataTimerSync, "metadata_timer_sync"};

    OptionStringList disableExcludes{std::vector<std::string>{}};
    OptionBinding disableExcludesBinding{owner, disableExcludes, "disable_excludes"};

    OptionEnum<std::string> multilibPolicy{"best", {"best", "all"}}; // :api
    OptionBinding multilibPolicyBinding{owner, multilibPolicy, "multilib_policy"};

    OptionBool best{false}; // :api
    OptionBinding bestBinding{owner, best, "best"};

    OptionBool installWeakDeps{true};
    OptionBinding installWeakDepsBinding{owner, installWeakDeps, "install_weak_deps"};

    OptionString bugtrackerUrl{BUGTRACKER};
    OptionBinding bugtrackerUrlBinding{owner, bugtrackerUrl, "bugtracker_url"};

    OptionEnum<std::string> color{"auto", {"auto", "never", "always"},
        [](const std::string & value){
            const std::array<const char *, 4> always{{"on", "yes", "1", "true"}};
            const std::array<const char *, 4> never{{"off", "no", "0", "false"}};
            const std::array<const char *, 2> aut{{"tty", "if-tty"}};
            std::string tmp;
            if (std::find(always.begin(), always.end(), value) != always.end())
                tmp = "always";
            else if (std::find(never.begin(), never.end(), value) != never.end())
                tmp = "never";
            else if (std::find(aut.begin(), aut.end(), value) != aut.end())
                tmp = "auto";
            else
                tmp = value;
            return tmp;
        }
    };
    OptionBinding colorBinding{owner, color, "color"};

    OptionString colorListInstalledOlder{"bold"};
    OptionBinding colorListInstalledOlderBinding{owner, colorListInstalledOlder, "color_list_installed_older"};

    OptionString colorListInstalledNewer{"bold,yellow"};
    OptionBinding colorListInstalledNewerBinding{owner, colorListInstalledNewer, "color_list_installed_newer"};

    OptionString colorListInstalledReinstall{"normal"};
    OptionBinding colorListInstalledReinstallBinding{owner, colorListInstalledReinstall, "color_list_installed_reinstall"};

    OptionString colorListInstalledExtra{"bold,red"};
    OptionBinding colorListInstalledExtraBinding{owner, colorListInstalledExtra, "color_list_installed_extra"};

    OptionString colorListAvailableUpgrade{"bold,blue"};
    OptionBinding colorListAvailableUpgradeBinding{owner, colorListAvailableUpgrade, "color_list_available_upgrade"};

    OptionString colorListAvailableDowngrade{"dim,cyan"};
    OptionBinding colorListAvailableDowngradeBinding{owner, colorListAvailableDowngrade, "color_list_available_downgrade"};

    OptionString colorListAvailableReinstall{"bold,underline,green"};
    OptionBinding colorListAvailableReinstallBinding{owner, colorListAvailableReinstall, "color_list_available_reinstall"};

    OptionString colorListAvailableInstall{"normal"};
    OptionBinding colorListAvailableInstallBinding{owner, colorListAvailableInstall, "color_list_available_install"};

    OptionString colorUpdateInstalled{"normal"};
    OptionBinding colorUpdateInstalledBinding{owner, colorUpdateInstalled, "color_update_installed"};

    OptionString colorUpdateLocal{"bold"};
    OptionBinding colorUpdateLocalBinding{owner, colorUpdateLocal, "color_update_local"};

    OptionString colorUpdateRemote{"normal"};
    OptionBinding colorUpdateRemoteBinding{owner, colorUpdateRemote, "color_update_remote"};

    OptionString colorSearchMatch{"bold"};
    OptionBinding colorSearchMatchBinding{owner, colorSearchMatch, "color_search_match"};

    OptionBool historyRecord{true};
    OptionBinding historyRecordBinding{owner, historyRecord, "history_record"};

    OptionStringList historyRecordPackages{std::vector<std::string>{"dnf", "rpm"}};
    OptionBinding historyRecordPackagesBinding{owner, historyRecordPackages, "history_record_packages"};

    OptionString rpmVerbosity{"info"};
    OptionBinding rpmverbosityBinding{owner, rpmVerbosity, "rpmverbosity"};

    OptionBool strict{true}; // :api
    OptionBinding strictBinding{owner, strict, "strict"};

    OptionBool skipBroken{false}; // :yum-compatibility
    OptionBinding skipBrokenBinding{owner, skipBroken, "skip_broken"};

    OptionBool autocheckRunningKernel{true}; // :yum-compatibility
    OptionBinding autocheckRunningKernelBinding{owner, autocheckRunningKernel, "autocheck_running_kernel"};

    OptionBool cleanRequirementsOnRemove{true};
    OptionBinding cleanRequirementsOnRemoveBinding{owner, cleanRequirementsOnRemove, "clean_requirements_on_remove"};

    OptionEnum<std::string> historyListView{"commands", {"single-user-commands", "users", "commands"},
        [](const std::string & value){
            if (value == "cmds" || value == "default")
                return std::string("commands");
            else
                return value;
        }
    };
    OptionBinding historyListViewBinding{owner, historyListView, "history_list_view"};

    OptionBool upgradeGroupObjectsUpgrade{true}; // :api
    OptionBinding upgradeGroupObjectsUpgradeBinding{owner, upgradeGroupObjectsUpgrade, "upgrade_group_objects_upgrade"};

    OptionPath destDir{nullptr};
    OptionBinding destDirBinding{owner, destDir, "destdir"};

    OptionString comment{nullptr};
    OptionBinding commentBinding{owner, comment, "comment"};

    // runtime only options
    OptionBool downloadOnly{false};

    OptionBool ignoreArch{false};
    OptionBinding ignoreArchBinding{owner, ignoreArch, "ignorearch"};


    // Repo main config

    OptionNumber<std::uint32_t> retries{10};
    OptionBinding retriesBinding{owner, retries, "retries"};

    OptionString cacheDir{nullptr};
    OptionBinding cacheDirBindings{owner, cacheDir, "cachedir"};

    OptionBool fastestMirror{false};
    OptionBinding fastestMirrorBinding{owner, fastestMirror, "fastestmirror"};

    OptionStringListAppend excludePkgs{std::vector<std::string>{}};
    OptionBinding excludePkgsBinding{owner, excludePkgs, "excludepkgs"};
    OptionBinding excludeBinding{owner, excludePkgs, "exclude"}; //compatibility with yum

    OptionStringListAppend includePkgs{std::vector<std::string>{}};
    OptionBinding includePkgsBinding{owner, includePkgs, "includepkgs"};

    OptionString proxy{"", PROXY_URL_REGEX, true};
    OptionBinding proxyBinding{owner, proxy, "proxy"};

    OptionString proxyUsername{nullptr};
    OptionBinding proxyUsernameBinding{owner, proxyUsername, "proxy_username"};

    OptionString proxyPassword{nullptr};
    OptionBinding proxyPasswordBinding{owner, proxyPassword, "proxy_password"};

    OptionStringList protectedPackages{resolveGlobs("dnf glob:/etc/yum/protected.d/*.conf " \
                                          "glob:/etc/dnf/protected.d/*.conf")};
    OptionBinding protectedPackagesBinding{owner, protectedPackages, "protected_packages",
        [&](Option::Priority priority, const std::string & value){
            if (priority >= protectedPackages.getPriority())
                protectedPackages.set(priority, resolveGlobs(value));
        }, nullptr
    };

    OptionString username{""};
    OptionBinding usernameBinding{owner, username, "username"};

    OptionString password{""};
    OptionBinding passwordBinding{owner, password, "password"};

    OptionBool gpgCheck{false};
    OptionBinding gpgCheckBinding{owner, gpgCheck, "gpgcheck"};

    OptionBool repoGpgCheck{false};
    OptionBinding repoGpgCheckBinding{owner, repoGpgCheck, "repo_gpgcheck"};

    OptionBool enabled{true};
    OptionBinding enabledBinding{owner, enabled, "enabled"};

    OptionBool enableGroups{true};
    OptionBinding enableGroupsBinding{owner, enableGroups, "enablegroups"};

    OptionNumber<std::uint32_t> bandwidth{0, strToBytes};
    OptionBinding bandwidthBinding{owner, bandwidth, "bandwidth"};

    OptionNumber<std::uint32_t> minRate{1000, strToBytes};
    OptionBinding minRateBinding{owner, minRate, "minrate"};

    OptionEnum<std::string> ipResolve{"whatever", {"ipv4", "ipv6", "whatever"},
        [](const std::string & value){
            auto tmp = value;
            if (value == "4") tmp = "ipv4";
            else if (value == "6") tmp = "ipv6";
            else std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
            return tmp;
        }
    };
    OptionBinding ipResolveBinding{owner, ipResolve, "ip_resolve"};

    OptionNumber<float> throttle{0, 0,
        [](const std::string & value)->float{
            if (!value.empty() && value.back()=='%') {
                std::size_t idx;
                auto res = std::stod(value, &idx);
                if (res < 0 || res > 100)
                    throw std::runtime_error(tfm::format(_("percentage '%s' is out of range"), value));
                return res/100;
            }
            return strToBytes(value);
        }
    };
    OptionBinding throttleBinding{owner, throttle, "throttle"};

    OptionSeconds timeout{30};
    OptionBinding timeoutBinding{owner, timeout, "timeout"};

    OptionNumber<std::uint32_t> maxParallelDownloads{3, 1};
    OptionBinding maxParallelDownloadsBinding{owner, maxParallelDownloads, "max_parallel_downloads"};

    OptionSeconds metadataExpire{60 * 60 * 48};
    OptionBinding metadataExpireBinding{owner, metadataExpire, "metadata_expire"};

    OptionString sslCaCert{""};
    OptionBinding sslCaCertBinding{owner, sslCaCert, "sslcacert"};

    OptionBool sslVerify{true};
    OptionBinding sslVerifyBinding{owner, sslVerify, "sslverify"};

    OptionString sslClientCert{""};
    OptionBinding sslClientCertBinding{owner, sslClientCert, "sslclientcert"};

    OptionString sslClientKey{""};
    OptionBinding sslClientKeyBinding{owner, sslClientKey, "sslclientkey"};

    OptionBool deltaRpm{true};
    OptionBinding deltaRpmBinding{owner, deltaRpm, "deltarpm"};

    OptionNumber<std::uint32_t> deltaRpmPercentage{75};
    OptionBinding deltaRpmPercentageBinding{owner, deltaRpmPercentage, "deltarpm_percentage"};
};

ConfigMain::ConfigMain() { pImpl = std::unique_ptr<Impl>(new Impl(*this)); }
ConfigMain::~ConfigMain() = default;

OptionNumber<std::int32_t> & ConfigMain::debugLevel() { return pImpl->debugLevel; }
OptionNumber<std::int32_t> & ConfigMain::errorLevel() { return pImpl->errorLevel; }
OptionString & ConfigMain::installRoot() { return pImpl->installRoot; }
OptionString & ConfigMain::configFilePath() { return pImpl->configFilePath; }
OptionBool & ConfigMain::plugins() { return pImpl->plugins; }
OptionStringList & ConfigMain::pluginPath() { return pImpl->pluginPath; }
OptionStringList & ConfigMain::pluginConfPath() { return pImpl->pluginConfPath; }
OptionString & ConfigMain::persistDir() { return pImpl->persistDir; }
OptionBool & ConfigMain::transformDb() { return pImpl->transformDb; }
OptionNumber<std::int32_t> & ConfigMain::recent() { return pImpl->recent; }
OptionBool & ConfigMain::resetNice() { return pImpl->resetNice; }
OptionString & ConfigMain::systemCacheDir() { return pImpl->systemCacheDir; }
OptionBool & ConfigMain::cacheOnly() { return pImpl->cacheOnly; }
OptionBool & ConfigMain::keepCache() { return pImpl->keepCache; }
OptionString & ConfigMain::logDir() { return pImpl->logDir; }
OptionStringList & ConfigMain::reposDir() { return pImpl->reposDir; }
OptionBool & ConfigMain::debugSolver() { return pImpl->debugSolver; }
OptionStringListAppend & ConfigMain::installOnlyPkgs() { return pImpl->installOnlyPkgs; }
OptionStringList & ConfigMain::groupPackageTypes() { return pImpl->groupPackageTypes; }
OptionNumber<std::uint32_t> & ConfigMain::installOnlyLimit() { return pImpl->installOnlyLimit; }
OptionStringListAppend & ConfigMain::tsFlags() { return pImpl->tsFlags; }
OptionBool & ConfigMain::assumeYes() { return pImpl->assumeYes; }
OptionBool & ConfigMain::assumeNo() { return pImpl->assumeNo; }
OptionBool & ConfigMain::checkConfigFileAge() { return pImpl->checkConfigFileAge; }
OptionBool & ConfigMain::defaultYes() { return pImpl->defaultYes; }
OptionBool & ConfigMain::diskSpaceCheck() { return pImpl->diskSpaceCheck; }
OptionBool & ConfigMain::localPkgGpgCheck() { return pImpl->localPkgGpgCheck; }
OptionBool & ConfigMain::obsoletes() { return pImpl->obsoletes; }
OptionBool & ConfigMain::showDupesFromRepos() { return pImpl->showDupesFromRepos; }
OptionBool & ConfigMain::exitOnLock() { return pImpl->exitOnLock; }
OptionSeconds & ConfigMain::metadataTimerSync() { return pImpl->metadataTimerSync; }
OptionStringList & ConfigMain::disableExcludes() { return pImpl->disableExcludes; }
OptionEnum<std::string> & ConfigMain::multilibPolicy() { return pImpl->multilibPolicy; }
OptionBool & ConfigMain::best() { return pImpl->best; }
OptionBool & ConfigMain::installWeakDeps() { return pImpl->installWeakDeps; }
OptionString & ConfigMain::bugtrackerUrl() { return pImpl->bugtrackerUrl; }
OptionEnum<std::string> & ConfigMain::color() { return pImpl->color; }
OptionString & ConfigMain::colorListInstalledOlder() { return pImpl->colorListInstalledOlder; }
OptionString & ConfigMain::colorListInstalledNewer() { return pImpl->colorListInstalledNewer; }
OptionString & ConfigMain::colorListInstalledReinstall() { return pImpl->colorListInstalledReinstall; }
OptionString & ConfigMain::colorListInstalledExtra() { return pImpl->colorListInstalledExtra; }
OptionString & ConfigMain::colorListAvailableUpgrade() { return pImpl->colorListAvailableUpgrade; }
OptionString & ConfigMain::colorListAvailableDowngrade() { return pImpl->colorListAvailableDowngrade; }
OptionString & ConfigMain::colorListAvailableReinstall() { return pImpl->colorListAvailableReinstall; }
OptionString & ConfigMain::colorListAvailableInstall() { return pImpl->colorListAvailableInstall; }
OptionString & ConfigMain::colorUpdateInstalled() { return pImpl->colorUpdateInstalled; }
OptionString & ConfigMain::colorUpdateLocal() { return pImpl->colorUpdateLocal; }
OptionString & ConfigMain::colorUpdateRemote() { return pImpl->colorUpdateRemote; }
OptionString & ConfigMain::colorSearchMatch() { return pImpl->colorSearchMatch; }
OptionBool & ConfigMain::historyRecord() { return pImpl->historyRecord; }
OptionStringList & ConfigMain::historyRecordPackages() { return pImpl->historyRecordPackages; }
OptionString & ConfigMain::rpmVerbosity() { return pImpl->rpmVerbosity; }
OptionBool & ConfigMain::strict() { return pImpl->strict; }
OptionBool & ConfigMain::skipBroken() { return pImpl->skipBroken; }
OptionBool & ConfigMain::autocheckRunningKernel() { return pImpl->autocheckRunningKernel; }
OptionBool & ConfigMain::cleanRequirementsOnRemove() { return pImpl->cleanRequirementsOnRemove; }
OptionEnum<std::string> & ConfigMain::historyListView() { return pImpl->historyListView; }
OptionBool & ConfigMain::upgradeGroupObjectsUpgrade() { return pImpl->upgradeGroupObjectsUpgrade; }
OptionPath & ConfigMain::destDir() { return pImpl->destDir; }
OptionString & ConfigMain::comment() { return pImpl->comment; }
OptionBool & ConfigMain::downloadOnly() { return pImpl->downloadOnly; }
OptionBool & ConfigMain::ignoreArch() { return pImpl->ignoreArch; }

// Repo main config
OptionNumber<std::uint32_t> & ConfigMain::retries() { return pImpl->retries; }
OptionString & ConfigMain::cacheDir() { return pImpl->cacheDir; }
OptionBool & ConfigMain::fastestMirror() { return pImpl->fastestMirror; }
OptionStringListAppend & ConfigMain::excludePkgs() { return pImpl->excludePkgs; }
OptionStringListAppend & ConfigMain::includePkgs() { return pImpl->includePkgs; }
OptionString & ConfigMain::proxy() { return pImpl->proxy; }
OptionString & ConfigMain::proxyUsername() { return pImpl->proxyUsername; }
OptionString & ConfigMain::proxyPassword() { return pImpl->proxyPassword; }
OptionStringList & ConfigMain::protectedPackages() { return pImpl->protectedPackages; }
OptionString & ConfigMain::username() { return pImpl->username; }
OptionString & ConfigMain::password() { return pImpl->password; }
OptionBool & ConfigMain::gpgCheck() { return pImpl->gpgCheck; }
OptionBool & ConfigMain::repoGpgCheck() { return pImpl->repoGpgCheck; }
OptionBool & ConfigMain::enabled() { return pImpl->enabled; }
OptionBool & ConfigMain::enableGroups() { return pImpl->enableGroups; }
OptionNumber<std::uint32_t> & ConfigMain::bandwidth() { return pImpl->bandwidth; }
OptionNumber<std::uint32_t> & ConfigMain::minRate() { return pImpl->minRate; }
OptionEnum<std::string> & ConfigMain::ipResolve() { return pImpl->ipResolve; }
OptionNumber<float> & ConfigMain::throttle() { return pImpl->throttle; }
OptionSeconds & ConfigMain::timeout() { return pImpl->timeout; }
OptionNumber<std::uint32_t> & ConfigMain::maxParallelDownloads() { return pImpl->maxParallelDownloads; }
OptionSeconds & ConfigMain::metadataExpire() { return pImpl->metadataExpire; }
OptionString & ConfigMain::sslCaCert() { return pImpl->sslCaCert; }
OptionBool & ConfigMain::sslVerify() { return pImpl->sslVerify; }
OptionString & ConfigMain::sslClientCert() { return pImpl->sslClientCert; }
OptionString & ConfigMain::sslClientKey() { return pImpl->sslClientKey; }
OptionBool & ConfigMain::deltaRpm() { return pImpl->deltaRpm; }
OptionNumber<std::uint32_t> & ConfigMain::deltaRpmPercentage() { return pImpl->deltaRpmPercentage; }

}
