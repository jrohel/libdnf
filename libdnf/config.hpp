#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include "options.hpp"

#include "utils/bgettext/bgettext-lib.h"
#include "utils/tinyformat/tinyformat.hpp"
#include "utils/regex/regex.hpp"

#include <array>
#include <functional>
#include <map>
#include <string>

constexpr const char * SYSTEM_CACHEDIR = "/var/cache/dnf";

constexpr const char * URL_REGEX = "(https?|ftp|file):\\/\\/[\\w.-/?=&#;]+$";
constexpr const char * PROXY_URL_REGEX = "^((https?|ftp|socks5h?|socks4a?):\\/\\/[\\w.-/?=&#;]+)?$";

constexpr const char * CONF_FILENAME = "/etc/dnf/dnf.conf";

const std::vector<std::string> GROUP_PACKAGE_TYPES{"mandatory", "default", "conditional"}; // :api
const std::vector<std::string> INSTALLONLYPKGS{"kernel", "kernel-PAE",
                 "installonlypkg(kernel)",
                 "installonlypkg(kernel-module)",
                 "installonlypkg(vm)"};

constexpr const char * BUGTRACKER="https://bugzilla.redhat.com/enter_bug.cgi?product=Fedora&component=dnf";

class Config {
public:
    class OptionBinding {
    public:
        typedef std::function<void(Option::Priority, const std::string &)> NewStringFunc;
        typedef std::function<const std::string & ()> GetValueStringFunc;

        OptionBinding(std::map<std::string, OptionBinding &> & optBinds, Option & option,
                        const std::string & name, NewStringFunc && newString, GetValueStringFunc && getValueString)
        : option(option), newStr(std::move(newString)), getValueStr(std::move(getValueString)) { optBinds.insert({name, *this}); }

        OptionBinding(std::map<std::string, OptionBinding &> & optBinds, Option & option,
                        const std::string & name)
        : option(option) { optBinds.insert({name, *this}); }

        void newString(Option::Priority priority, const std::string & value)
        {
            if (newStr)
                newStr(priority, value);
            else
                option.set(priority, value);
        }

        const std::string getValueString()
        {
            if (getValueStr)
                return getValueStr();
            else
                return option.getValueString();
        }

    private:
        Option & option;
        NewStringFunc newStr;
        GetValueStringFunc getValueStr;
    };

    OptionBinding & getOptionBinding(const std::string & key)
    {
        auto item = optBinds.find(key);
        if (item == optBinds.end())
            throw std::runtime_error(tfm::format(_("Config: OptionBinding with key \"%s\" does not exist"), key));
        return item->second;
    }

    const OptionBinding & getOptionBinding(const std::string & key) const
    {
        auto item = optBinds.find(key);
        if (item == optBinds.end())
            throw std::runtime_error(tfm::format(_("Config: OptionBinding with key \"%s\" does not exist"), key));
        return item->second;
    }

    std::map<std::string, OptionBinding &> optBinds;
};

template<typename T>
static void addToList(T & option, Option::Priority priority, const std::string & value)
{
    if (priority < option.getPriority())
        return;
    auto tmp = option.getValue(); 
    option.set(priority, value);
    tmp.insert(tmp.end(), option.getValue().begin(), option.getValue().end());
    option.set(priority, tmp);
}

/*  Function converts a human readable variation specifying days, hours, minutes or seconds
    and returns an integer number of seconds.
    Note that due to historical president -1 means "never", so this accepts
    that and allows the word never, too.

    Valid inputs: 100, 1.5m, 90s, 1.2d, 1d, 0.1, -1, never.
    Invalid inputs: -10, -0.1, 45.6Z, 1d6h, 1day, 1y.

    Return value will always be an integer
*/
int strToSeconds(const std::string & str);

/*  Function converts a friendly bandwidth option to bytes.  The input
    should be a string containing a (possibly floating point)
    number followed by an optional single character unit. Valid
    units are 'k', 'M', 'G'. Case is ignored. The convention that
    1k = 1024 bytes is used.

    Valid inputs: 100, 123M, 45.6k, 12.4G, 100K, 786.3, 0.
    Invalid inputs: -10, -0.1, 45.6L, 123Mb.
*/
int strToBytes(const std::string & str);

/* Replaces globs (like /etc/foo.d/\\*.foo) by content of matching files.
 * Ignores comment lines (start with '#') and blank lines in files.
 * Result:
 * Words delimited by spaces. Characters ',' and '\n' are replaced by spaces.
 * Extra spaces are removed.
 */
std::string resolveGlobs(const std::string & strWithGlobs);

class ConfigMain : public Config {
public:
    OptionNumber<int> debugLevel{2, 0, 10};
    OptionBinding debugLevelBinding{optBinds, debugLevel, "debug_level"};

    OptionNumber<int> errorLevel{2, 0, 10};
    OptionBinding errorLevelBinding{optBinds, errorLevel, "error_level"};

    OptionString installRoot{"/"};
    OptionBinding installRootBinding{optBinds, installRoot, "install_root"};

    OptionString configFilePath{CONF_FILENAME};
    OptionBinding configFilePathBinding{optBinds, configFilePath, "config_file_path", nullptr, [&](){ return configFilePath.getValue(); }};

    OptionBool plugins{true};
    OptionBinding pluginsBinding{optBinds, plugins, "plugins"};

    OptionStringList pluginPath{std::vector<std::string>{}};
    OptionBinding pluginPathBinding{optBinds, pluginPath, "pluginpath"};

    OptionStringList pluginConfPath{std::vector<std::string>{}};
    OptionBinding pluginConfPathBinding{optBinds, pluginConfPath, "pluginconfpath"};

    OptionString persistDir{""};
    OptionBinding persistDirBinding{optBinds, persistDir, "persistdir"};

    OptionBool transformDb{true};
    OptionBinding transformDbBinding{optBinds, transformDb, "transformdb"};

    OptionNumber<int> recent{7, 0};
    OptionBinding recentBinding{optBinds, recent, "recent"};

    OptionBool resetNice{true};
    OptionBinding resetNiceBinding{optBinds, resetNice, "reset_nice"};

    OptionString systemCacheDir{SYSTEM_CACHEDIR};
    OptionBinding systemCacheDirBindings{optBinds, systemCacheDir, "system_cachedir"};

    OptionBool cacheOnly{false};
    OptionBinding cacheOnlyBinding{optBinds, cacheOnly, "cacheonly"};

    OptionBool keepCache{false};
    OptionBinding keepCacheBinding{optBinds, keepCache, "keepcache"};

    OptionString logDir{"/var/log"};
    OptionBinding logDirBinding{optBinds, logDir, "logdir"};

    OptionStringList reposDir{{"/etc/yum.repos.d", "/etc/yum/repos.d", "/etc/distro.repos.d"}};
    OptionBinding reposDirBinding{optBinds, reposDir, "reposdir"};

    OptionBool debugSolver{false};
    OptionBinding debugSolverBinding{optBinds, debugSolver, "debug_solver"};

    OptionStringList installOnlyPkgs{INSTALLONLYPKGS};
    OptionBinding installOnlyPkgsBinding{optBinds, installOnlyPkgs, "installonlypkgs",
        [&](Option::Priority priority, const std::string & value){
            addToList(installOnlyPkgs, priority, value);
        }, nullptr
    };

    OptionStringList groupPackageTypes{GROUP_PACKAGE_TYPES};
    OptionBinding groupPackageTypesBinding{optBinds, groupPackageTypes, "group_package_types"};

/*  NOTE: If you set this to 2, then because it keeps the current
    kernel it means if you ever install an "old" kernel it'll get rid
    of the newest one so you probably want to use 3 as a minimum
    ... if you turn it on. */
    OptionNumber<unsigned int> installOnlyLimit{3, 2};
    OptionBinding installOnlyLimitBinding{optBinds, installOnlyLimit, "installonly_limit"};

    OptionStringList tsFlags{std::vector<std::string>{}};
    OptionBinding tsFlagsBinding{optBinds, tsFlags, "tsflags",
        [&](Option::Priority priority, const std::string & value){
            addToList(tsFlags, priority, value);
        }, nullptr
    };

    OptionBool assumeYes{false};
    OptionBinding assumeYesBinding{optBinds, assumeYes, "assumeyes"};

    OptionBool assumeNo{false};
    OptionBinding assumeNoBinding{optBinds, assumeNo, "assumeno"};

    OptionBool checkConfigFileAge{true};
    OptionBinding checkConfigFileAgeBinding{optBinds, checkConfigFileAge, "check_config_file_age"};

    OptionBool defaultYes{false};
    OptionBinding defaultYesBinding{optBinds, defaultYes, "defaultyes"};

    OptionBool diskSpaceCheck{true};
    OptionBinding diskSpaceCheckBinding{optBinds, diskSpaceCheck, "diskspacecheck"};

    OptionBool localPkgGpgCheck{false};
    OptionBinding localPkgGpgCheckBinding{optBinds, localPkgGpgCheck, "localpkg_gpgcheck"};

    OptionBool obsoletes{true};
    OptionBinding obsoletesBinding{optBinds, obsoletes, "obsoletes"};

    OptionBool showDupesFromRepos{false};
    OptionBinding showDupesFromReposBinding{optBinds, showDupesFromRepos, "showdupesfromrepos"};

    OptionBool exitOnLock{false};
    OptionBinding exitOnLockBinding{optBinds, exitOnLock, "exit_on_lock"};

    OptionNumber<int> metadataTimerSync{60 * 60 * 3}; // 3 hours
    OptionBinding metadataTimerSyncBinding{optBinds, metadataTimerSync, "metadata_timer_sync",
        [&](Option::Priority priority, const std::string & value){
            if (priority >= metadataTimerSync.getPriority())
                metadataTimerSync.set(priority, strToSeconds(value));
        }, nullptr
    };

    OptionStringList disableExcludes{std::vector<std::string>{}};
    OptionBinding disableExcludesBinding{optBinds, disableExcludes, "disable_excludes"};

    OptionEnum<std::string> multilibPolicy{"best", {"best", "all"}}; // :api
    OptionBinding multilibPolicyBinding{optBinds, multilibPolicy, "multilib_policy"};

    OptionBool best{false}; // :api
    OptionBinding bestBinding{optBinds, best, "best"};

    OptionBool installWeakDeps{true};
    OptionBinding installWeakDepsBinding{optBinds, installWeakDeps, "install_weak_deps"};

    OptionString bugtrackerUrl{BUGTRACKER};
    OptionBinding bugtrackerUrlBinding{optBinds, bugtrackerUrl, "bugtracker_url"};

    OptionEnum<std::string> color{"auto", {"auto", "never", "always"}};
    OptionBinding colorBinding{optBinds, color, "color",
        [&](Option::Priority priority, const std::string & value){
            if (priority >= color.getPriority()) {
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
                color.set(priority, tmp);
            }
        }, nullptr
    };

    OptionString colorListInstalledOlder{"bold"};
    OptionBinding colorListInstalledOlderBinding{optBinds, colorListInstalledOlder, "color_list_installed_older"};

    OptionString colorListInstalledNewer{"bold,yellow"};
    OptionBinding colorListInstalledNewerBinding{optBinds, colorListInstalledNewer, "color_list_installed_newer"};

    OptionString colorListInstalledReinstall{"normal"};
    OptionBinding colorListInstalledReinstallBinding{optBinds, colorListInstalledReinstall, "color_list_installed_reinstall"};

    OptionString colorListInstalledExtra{"bold,red"};
    OptionBinding colorListInstalledExtraBinding{optBinds, colorListInstalledExtra, "color_list_installed_extra"};

    OptionString colorListAvailableUpgrade{"bold,blue"};
    OptionBinding colorListAvailableUpgradeBinding{optBinds, colorListAvailableUpgrade, "color_list_available_upgrade"};

    OptionString colorListAvailableDowngrade{"dim,cyan"};
    OptionBinding colorListAvailableDowngradeBinding{optBinds, colorListAvailableDowngrade, "color_list_available_downgrade"};

    OptionString colorListAvailableReinstall{"bold,underline,green"};
    OptionBinding colorListAvailableReinstallBinding{optBinds, colorListAvailableReinstall, "color_list_available_reinstall"};

    OptionString colorListAvailableInstall{"normal"};
    OptionBinding colorListAvailableInstallBinding{optBinds, colorListAvailableInstall, "color_list_available_install"};

    OptionString colorUpdateInstalled{"normal"};
    OptionBinding colorUpdateInstalledBinding{optBinds, colorUpdateInstalled, "color_update_installed"};

    OptionString colorUpdateLocal{"bold"};
    OptionBinding colorUpdateLocalBinding{optBinds, colorUpdateLocal, "color_update_local"};

    OptionString colorUpdateRemote{"normal"};
    OptionBinding colorUpdateRemoteBinding{optBinds, colorUpdateRemote, "color_update_remote"};

    OptionString colorSearchMatch{"bold"};
    OptionBinding colorSearchMatchBinding{optBinds, colorSearchMatch, "color_search_match"};

    OptionBool historyRecord{true};
    OptionBinding historyRecordBinding{optBinds, historyRecord, "history_record"};

    OptionStringList historyRecordPackages{std::vector<std::string>{"dnf", "rpm"}};
    OptionBinding historyRecordPackagesBinding{optBinds, historyRecordPackages, "history_record_packages"};

    OptionString rpmVerbosity{"info"};
    OptionBinding rpmverbosityBinding{optBinds, rpmVerbosity, "rpmverbosity"};

    OptionBool strict{true}; // :api
    OptionBinding strictBinding{optBinds, strict, "strict"};

    OptionBool skipBroken{false}; // :yum-compatibility
    OptionBinding skipBrokenBinding{optBinds, skipBroken, "skip_broken"};

    OptionBool autocheckRunningKernel{true}; // :yum-compatibility
    OptionBinding autocheckRunningKernelBinding{optBinds, autocheckRunningKernel, "autocheck_running_kernel"};

    OptionBool cleanRequirementsOnRemove{true};
    OptionBinding cleanRequirementsOnRemoveBinding{optBinds, cleanRequirementsOnRemove, "clean_requirements_on_remove"};

    OptionEnum<std::string> historyListView{"commands", {"single-user-commands", "users", "commands"}};
    OptionBinding historyListViewBinding{optBinds, historyListView, "history_list_view",
        [&](Option::Priority priority, const std::string & value){
            if (priority >= historyListView.getPriority()) {
                if (value == "cmds" || value == "default")
                    historyListView.set(priority, "commands");
                else
                    historyListView.set(priority, value);
            }
        }, nullptr
    };

    OptionBool upgradeGroupObjectsUpgrade{true}; // :api
    OptionBinding upgradeGroupObjectsUpgradeBinding{optBinds, upgradeGroupObjectsUpgrade, "upgrade_group_objects_upgrade"};

    OptionPath destDir{nullptr};
    OptionBinding destDirBinding{optBinds, destDir, "destdir"};

    OptionString comment{nullptr};
    OptionBinding commentBinding{optBinds, comment, "comment"};

    // runtime only options
    OptionBool downloadOnly{false};

    OptionBool ignoreArch{false};
    OptionBinding ignoreArchBinding{optBinds, ignoreArch, "ignorearch"};
};

template<typename T>
static void addToList(T & option, Option::Priority priority, const typename T::ValueType & value)
{
    if (priority < option.getPriority())
        return;
    auto tmp = option.getValue(); 
    option.set(priority, value);
    tmp.insert(tmp.end(), option.getValue().begin(), option.getValue().end());
    option.set(priority, tmp);
}

class ConfigRepoMain : public Config {
public:
    OptionNumber<unsigned int> retries{10};
    OptionBinding retriesBinding{optBinds, retries, "retries"};

    OptionString cacheDir{SYSTEM_CACHEDIR};
    OptionBinding cacheDirBindings{optBinds, cacheDir, "cachedir"};

    OptionBool fastestMirror{false};
    OptionBinding fastestMirrorBinding{optBinds, fastestMirror, "fastestmirror"};

    OptionStringList excludePkgs{std::vector<std::string>{}};
    OptionBinding excludePkgsBinding{optBinds, excludePkgs, "excludepkgs",
        [&](Option::Priority priority, const std::string & value){
            addToList(includePkgs, priority, value);
        }, nullptr
    };
    OptionBinding excludeBinding{optBinds, excludePkgs, "exclude", //compatibility with yum
        [&](Option::Priority priority, const std::string & value){
            addToList(includePkgs, priority, value);
        }, nullptr
    };

    OptionStringList includePkgs{std::vector<std::string>{}};
    OptionBinding includePkgsBinding{optBinds, includePkgs, "includepkgs", 
        [&](Option::Priority priority, const std::string & value){
            addToList(includePkgs, priority, value);
        }, nullptr
    };

    OptionString proxy{"", {PROXY_URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding proxyBinding{optBinds, proxy, "proxy"};

    OptionString proxyUsername{nullptr};
    OptionBinding proxyUsernameBinding{optBinds, proxyUsername, "proxy_username"};

    OptionString proxyPassword{nullptr};
    OptionBinding proxyPasswordBinding{optBinds, proxyPassword, "proxy_password"};

    OptionStringList protectedPackages{resolveGlobs("dnf glob:/etc/yum/protected.d/*.conf " \
                                          "glob:/etc/dnf/protected.d/*.conf")};
    OptionBinding protectedPackagesBinding{optBinds, protectedPackages, "protected_packages",
        [&](Option::Priority priority, const std::string & value){
            if (priority >= protectedPackages.getPriority())
                protectedPackages.set(priority, resolveGlobs(value));
        }, nullptr
    };

    OptionString username{""};
    OptionBinding usernameBinding{optBinds, username, "username"};

    OptionString password{""};
    OptionBinding passwordBinding{optBinds, password, "password"};

    OptionBool gpgCheck{false};
    OptionBinding gpgCheckBinding{optBinds, gpgCheck, "gpgcheck"};

    OptionBool repoGpgCheck{false};
    OptionBinding repoGpgCheckBinding{optBinds, repoGpgCheck, "repo_gpgcheck"};

    OptionBool enabled{true};
    OptionBinding enabledBinding{optBinds, enabled, "enabled"};

    OptionBool enableGroups{true};
    OptionBinding enableGroupsBinding{optBinds, enableGroups, "enablegroups"};

    OptionNumber<std::uint32_t> bandwidth{0};
    OptionBinding bandwidthBinding{optBinds, bandwidth, "bandwidth",
        [&](Option::Priority priority, const std::string & value){
            if (priority >= bandwidth.getPriority())
                bandwidth.set(priority, strToBytes(value));
        }, nullptr
    };

    OptionNumber<std::uint32_t> minRate{1000};
    OptionBinding minRateBinding{optBinds, minRate, "minrate",
        [&](Option::Priority priority, const std::string & value){
            if (priority >= minRate.getPriority())
                minRate.set(priority, strToBytes(value));
        }, nullptr
    };

    OptionEnum<std::string> ipResolve{"whatever", {"ipv4", "ipv6", "whatever"}};
    OptionBinding ipResolveBinding{optBinds, ipResolve, "ip_resolve",
        [&](Option::Priority priority, const std::string & value){
            if (priority >= ipResolve.getPriority()) {
                auto tmp = value;
                if (value == "4") tmp = "ipv4";
                else if (value == "6") tmp = "ipv6";
                else std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
                ipResolve.set(priority, tmp);
            }
        }, nullptr
    };

    OptionNumber<std::uint32_t> throttle{0};
    OptionBinding throttleBinding{optBinds, throttle, "throttle",
        [&](Option::Priority priority, const std::string & value){
            if (priority >= throttle.getPriority())
                throttle.set(priority, strToBytes(value));
        }, nullptr
    };

    OptionNumber<std::uint32_t> timeout{30};
    OptionBinding timeoutBinding{optBinds, timeout, "timeout",
        [&](Option::Priority priority, const std::string & value){
            if (priority >= timeout.getPriority())
                timeout.set(priority, strToSeconds(value));
        }, nullptr
    };

    OptionNumber<std::uint32_t> maxParallelDownloads{3, 1};
    OptionBinding maxParallelDownloadsBinding{optBinds, maxParallelDownloads, "max_parallel_downloads"};

    OptionNumber<std::uint32_t> metadataExpire{60 * 60 * 48};
    OptionBinding metadataExpireBinding{optBinds, metadataExpire, "metadata_expire",
        [&](Option::Priority priority, const std::string & value){
            if (priority >= metadataExpire.getPriority())
                metadataExpire.set(priority, strToSeconds(value));
        }, nullptr
    };

    OptionString sslCaCert{""};
    OptionBinding sslCaCertBinding{optBinds, sslCaCert, "sslcacert"};

    OptionBool sslVerify{true};
    OptionBinding sslVerifyBinding{optBinds, sslVerify, "sslverify"};

    OptionString sslClientCert{""};
    OptionBinding sslClientCertBinding{optBinds, sslClientCert, "sslclientcert"};

    OptionString sslClientKey{""};
    OptionBinding sslClientKeyBinding{optBinds, sslClientKey, "sslclientkey"};

    OptionBool deltaRpm{true};
    OptionBinding deltaRpmBinding{optBinds, deltaRpm, "deltarpm"};

    OptionNumber<unsigned int> deltaRpmPercentage{75};
    OptionBinding deltaRpmPercentageBinding{optBinds, deltaRpmPercentage, "deltarpm_percentage"};
};

class ConfigRepo : public Config {
    ConfigRepoMain & parent;

public:
    ConfigRepo(ConfigRepoMain & parent) : parent(parent) {}

    OptionString name{""};
    OptionBinding nameBinding{optBinds, name, "name"};

    OptionChild<OptionBool> enabled{parent.enabled};
    OptionBinding enabledBinding{optBinds, enabled, "enabled"};

    OptionChild<OptionString> baseCacheDir{parent.cacheDir};
    OptionBinding baseCacheDirBindings{optBinds, baseCacheDir, "cachedir"};

    OptionStringList baseUrl{std::vector<std::string>{}, {URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding baseUrlBinding{optBinds, baseUrl, "baseurl"};

    OptionString mirrorList{nullptr, {URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding mirrorListBinding{optBinds, mirrorList, "mirrorlist"};

    OptionString metaLink{nullptr, {URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding metaLinkBinding{optBinds, metaLink, "metalink"};

    OptionString type{""};
    OptionBinding typeBinding{optBinds, type, "type"};

    OptionString mediaId{""};
    OptionBinding mediaIdBinding{optBinds, mediaId, "mediaid"};

    OptionStringList gpgKey{std::vector<std::string>{}, {URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding gpgKeyBinding{optBinds, gpgKey, "gpgkey"};

    OptionChild<OptionStringList> excludePkgs{parent.excludePkgs};
    OptionBinding excludePkgsBinding{optBinds, excludePkgs, "excludepkgs"};
    OptionBinding excludeBinding{optBinds, excludePkgs, "exclude"}; //compatibility with yum

    OptionChild<OptionStringList> includePkgs{parent.includePkgs};
    OptionBinding includePkgsBinding{optBinds, includePkgs, "includepkgs"};

    OptionChild<OptionBool> fastestMirror{parent.fastestMirror};
    OptionBinding fastestMirrorBinding{optBinds, fastestMirror, "fastestmirror"};

    OptionChild<OptionString> proxy{parent.proxy};
    OptionBinding proxyBinding{optBinds, proxy, "proxy"};

    OptionChild<OptionString> proxyUsername{parent.proxyUsername};
    OptionBinding proxyUsernameBinding{optBinds, proxyUsername, "proxy_username"};

    OptionChild<OptionString> proxyPassword{parent.proxyPassword};
    OptionBinding proxyPasswordBinding{optBinds, proxyPassword, "proxy_password"};

    OptionChild<OptionString> username{parent.username};
    OptionBinding usernameBinding{optBinds, username, "username"};

    OptionChild<OptionString> password{parent.password};
    OptionBinding passwordBinding{optBinds, password, "password"};

    OptionChild<OptionStringList> protectedPackages{parent.protectedPackages};
    OptionBinding protectedPackagesBinding{optBinds, protectedPackages, "protected_packages"};

    OptionChild<OptionBool> gpgCheck{parent.gpgCheck};
    OptionBinding gpgCheckBinding{optBinds, gpgCheck, "gpgcheck"};

    OptionChild<OptionBool> repoGpgCheck{parent.repoGpgCheck};
    OptionBinding repoGpgCheckBinding{optBinds, repoGpgCheck, "repo_gpgcheck"};

    OptionChild<OptionBool> enableGroups{parent.enableGroups};
    OptionBinding enableGroupsBinding{optBinds, enableGroups, "enablegroups"};

    OptionChild<OptionNumber<unsigned int> > retries{parent.retries};
    OptionBinding retriesBinding{optBinds, retries, "retries"};

    OptionChild<OptionNumber<std::uint32_t> > bandwidth{parent.bandwidth};
    OptionBinding bandwidthBinding{optBinds, bandwidth, "bandwidth"};

    OptionChild<OptionNumber<std::uint32_t> > minRate{parent.minRate};
    OptionBinding minRateBinding{optBinds, minRate, "minrate"};

    OptionChild<OptionEnum<std::string> > ipResolve{parent.ipResolve};
    OptionBinding ipResolveBinding{optBinds, ipResolve, "ip_resolve"};

    OptionChild<OptionNumber<std::uint32_t> > throttle{parent.throttle};
    OptionBinding throttleBinding{optBinds, throttle, "throttle"};

    OptionChild<OptionNumber<std::uint32_t> > timeout{parent.timeout};
    OptionBinding timeoutBinding{optBinds, timeout, "timeout"};

    OptionChild<OptionNumber<std::uint32_t> >  maxParallelDownloads{parent.maxParallelDownloads};
    OptionBinding maxParallelDownloadsBinding{optBinds, maxParallelDownloads, "max_parallel_downloads"};

    OptionChild<OptionNumber<std::uint32_t> > metadataExpire{parent.metadataExpire};
    OptionBinding metadataExpireBinding{optBinds, metadataExpire, "metadata_expire"};

    OptionNumber<int> cost{1000};
    OptionBinding costBinding{optBinds, cost, "cost"};

    OptionNumber<int> priority{99};
    OptionBinding priorityBinding{optBinds, priority, "priority"};

    OptionChild<OptionString> sslCaCert{parent.sslCaCert};
    OptionBinding sslCaCertBinding{optBinds, sslCaCert, "sslcacert"};

    OptionChild<OptionBool> sslVerify{parent.sslVerify};
    OptionBinding sslVerifyBinding{optBinds, sslVerify, "sslverify"};

    OptionChild<OptionString> sslClientCert{parent.sslClientCert};
    OptionBinding sslClientCertBinding{optBinds, sslClientCert, "sslclientcert"};

    OptionChild<OptionString> sslClientKey{parent.sslClientKey};
    OptionBinding sslClientKeyBinding{optBinds, sslClientKey, "sslclientkey"};

    OptionChild<OptionBool> deltaRpm{parent.deltaRpm};
    OptionBinding deltaRpmBinding{optBinds, deltaRpm, "deltarpm"};

    OptionChild<OptionNumber<unsigned int> > deltaRpmPercentage{parent.deltaRpmPercentage};
    OptionBinding deltaRpmPercentageBinding{optBinds, deltaRpmPercentage, "deltarpm_percentage"};

    OptionBool skipIfUnavailable{true};
    OptionBinding skipIfUnavailableBinding{optBinds, skipIfUnavailable, "skip_if_unavailable"};

    // option recognized by other tools, e.g. gnome-software, but unused in dnf
    OptionString enabledMetadata{""};
    OptionBinding enabledMetadataBinding{optBinds, enabledMetadata, "enabled_metadata"};

    // yum compatibility options
    OptionEnum<std::string> failoverMethod{"priority", {"priority", "roundrobin"}};

private:
};

class Configuration {
public:
    void setValue(Option::Priority priority, const std::string & section, const std::string & key, const std::string & value, bool addRepo = false);
    void setValue(Option::Priority priority, const std::string & section, const std::string & key, std::string && value, bool addRepo = false);

    void readIniFile(const std::string & filePath, Option::Priority priority);
    void readRepoFiles(const std::string & dirPath, Option::Priority priority);

    ConfigMain main;
    ConfigRepoMain repoMain;

    std::map<std::string, ConfigRepo> repos;
};

inline void Configuration::setValue(Option::Priority priority, const std::string& section, const std::string & key, const std::string & value, bool addRepo)
{
    setValue(priority, section, key, std::string(value), addRepo);
}

inline void Configuration::setValue(Option::Priority priority, const std::string& section, const std::string & key, std::string && value, bool addRepo)
{
    if (section == "main") {
        auto item = main.optBinds.find(key);
        if (item != main.optBinds.end())
            item->second.newString(priority, std::move(value));
        else
            repoMain.getOptionBinding(key).newString(priority, std::move(value));
    } else {
        auto item = repos.find(section);
        if (item == repos.end()) {
            if (addRepo) {
                auto res = repos.emplace(section, repoMain);
                item = res.first;
            } else
                throw std::runtime_error(tfm::format(_("Configuration: Repository with id \"%s\" does not exist"), section));
        }
        item->second.getOptionBinding(key).newString(priority, std::move(value));
    }
}

#endif
