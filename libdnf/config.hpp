#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include "options.hpp"

#include "utils/bgettext/bgettext-lib.h"
#include "utils/tinyformat/tinyformat.hpp"
#include "utils/regex/regex.hpp"

#include <functional>
#include <map>
#include <string>

constexpr const char * SYSTEM_CACHEDIR = "/var/cache/dnf";

constexpr const char * URL_REGEX = "(https?|ftp|file):\\/\\/[\\w.-/?=&#;]+$";
constexpr const char * PROXY_URL_REGEX = "^((https?|ftp|socks5h?|socks4a?):\\/\\/[\\w.-/?=&#;]+)?$";

constexpr const char * CONF_FILENAME = "/etc/dnf/dnf.conf";

class Config {
public:
    class OptionBinding {
    public:
        typedef std::function<void(Option::Priority, const std::string &)> NewStringFunc;
        typedef std::function<const std::string & ()> GetValueStringFunc;

        OptionBinding(std::map<std::string, OptionBinding &> & optBinds, Option & option,
                        const std::string & name, NewStringFunc && newString, GetValueStringFunc && getValueString)
        : option{option}, newStr{newString}, getValueStr{getValueString} { optBinds.insert({name, *this}); }

        OptionBinding(std::map<std::string, OptionBinding &> & optBinds, Option & option,
                        const std::string & name)
        : option{option} { optBinds.insert({name, *this}); }

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
            throw (std::runtime_error(tfm::format(_("Config: OptionBinding with key \"%s\" does not exist"), key)));
        return item->second;
    }

    const OptionBinding & getOptionBinding(const std::string & key) const
    {
        auto item = optBinds.find(key);
        if (item == optBinds.end())
            throw (std::runtime_error(tfm::format(_("Config: OptionBinding with key \"%s\" does not exist"), key)));
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

    OptionStringList pluginPath{{}};
    OptionBinding pluginPathBinding{optBinds, pluginPath, "pluginpath"};

    OptionStringList pluginConfPath{{}};
    OptionBinding pluginConfPathBinding{optBinds, pluginConfPath, "pluginconfpath"};

    OptionString persistDir{""};
    OptionBinding persistDirBinding{optBinds, persistDir, "persistdir"};

    OptionBool transformDb{true};
    OptionBinding transformDbBinding{optBinds, transformDb, "transformdb"};

    OptionNumber<int> recent{7, 0};
    OptionBinding recentBinding{optBinds, recent, "recent"};

    OptionBool resetNice{true};
    OptionBinding resetNiceBinding{optBinds, resetNice, "reset_nice"};

    OptionString systemCachedir{SYSTEM_CACHEDIR};
    OptionBinding systemCachedirBindings{optBinds, systemCachedir, "system_cachedir"};

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

    OptionStringList installOnlyPkgs{{}};
    OptionBinding installOnlyPkgsBinding{optBinds, installOnlyPkgs, "installonlypkgs"};

    OptionStringList groupPackageTypes{{}};
    OptionBinding groupPackageTypesBinding{optBinds, groupPackageTypes, "group_package_types"};

    OptionNumber<unsigned int> installOnlyLimit{3, 2};
    OptionBinding installOnlyLimitBinding{optBinds, installOnlyLimit, "installonly_limit"};

    OptionStringList tsFlags{{}};
    OptionBinding tsFlagsBinding{optBinds, tsFlags, "tsflags"};

    OptionBool assumeYes{false};
    OptionBinding assumeYesBinding{optBinds, assumeYes, "assumeyes"};

    OptionBool assumeNo{false};
    OptionBinding assumeNoBinding{optBinds, assumeNo, "assumeno"};
/*
        self._add_option('check_config_file_age', BoolOption(True))
        self._add_option('defaultyes', BoolOption(False))
        self._add_option('diskspacecheck', BoolOption(True))
        self._add_option('localpkg_gpgcheck', BoolOption(False))
        self._add_option('obsoletes', BoolOption(True))
        self._add_option('showdupesfromrepos', BoolOption(False))
        self._add_option('exit_on_lock', BoolOption(False))

        self._add_option('metadata_timer_sync',
                         SecondsOption(60 * 60 * 3)) #  3 hours
        self._add_option('disable_excludes', ListOption())
        self._add_option('multilib_policy',
                         SelectionOption('best', choices=('best', 'all'))) # :api
        self._add_option('best', BoolOption(False)) # :api
        self._add_option('install_weak_deps', BoolOption(True))
        self._add_option('bugtracker_url', Option(dnf.const.BUGTRACKER))

        self._add_option('color',
                         SelectionOption('auto',
                                         choices=('auto', 'never', 'always'),
                                         mapper={'on': 'always', 'yes' : 'always',
                                                 '1' : 'always', 'true': 'always',
                                                 'off': 'never', 'no':   'never',
                                                 '0':   'never', 'false': 'never',
                                                 'tty': 'auto', 'if-tty': 'auto'})
                        )
        self._add_option('color_list_installed_older', Option('bold'))
        self._add_option('color_list_installed_newer', Option('bold,yellow'))
        self._add_option('color_list_installed_reinstall', Option('normal'))
        self._add_option('color_list_installed_extra', Option('bold,red'))
        self._add_option('color_list_available_upgrade', Option('bold,blue'))
        self._add_option('color_list_available_downgrade', Option('dim,cyan'))
        self._add_option('color_list_available_reinstall',
                         Option('bold,underline,green'))
        self._add_option('color_list_available_install', Option('normal'))
        self._add_option('color_update_installed', Option('normal'))
        self._add_option('color_update_local', Option('bold'))
        self._add_option('color_update_remote', Option('normal'))
        self._add_option('color_search_match', Option('bold'))

        self._add_option('history_record', BoolOption(True))
        self._add_option('history_record_packages', ListOption(['dnf', 'rpm']))

        self._add_option('rpmverbosity', Option('info'))
        self._add_option('strict', BoolOption(True)) # :api
        self._add_option('skip_broken', BoolOption(False))  # :yum-compatibility
        self._add_option('autocheck_running_kernel', BoolOption(True))  # :yum-compatibility
        self._add_option('clean_requirements_on_remove', BoolOption(True))
        self._add_option('history_list_view',
                         SelectionOption('commands',
                                         choices=('single-user-commands',
                                                  'users', 'commands'),
                                         mapper={'cmds': 'commands',
                                                 'default': 'commands'}))
        self._add_option('upgrade_group_objects_upgrade',
                         BoolOption(True))  # :api
        self._add_option('destdir', PathOption(None))
        self._add_option('comment', Option())
        # runtime only options
        self._add_option('downloadonly', BoolOption(False, runtimeonly=True))
        self._add_option('ignorearch', BoolOption(False))
*/
};

class ConfigRepoMain : public Config {
public:
    OptionNumber<unsigned int> retries{10};
    OptionBinding retriesBinding{optBinds, retries, "retries"};

    OptionString cachedir{SYSTEM_CACHEDIR};
    OptionBinding cachedirBindings{optBinds, cachedir, "cachedir"};

    OptionBool fastestMirror{false};
    OptionBinding fastestMirrorBinding{optBinds, fastestMirror, "fastestmirror"};

    OptionStringList excludePkgs{{}};
    OptionBinding excludePkgsBinding{optBinds, excludePkgs, "excludepkgs"};
    OptionBinding excludeBinding{optBinds, excludePkgs, "exclude"}; //compatibility with yum

    OptionStringList includePkgs{{}};
    OptionBinding includePkgsBinding{optBinds, includePkgs, "includepkgs"};

    OptionString proxy{"", {PROXY_URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding proxyBinding{optBinds, proxy, "proxy"};

    OptionString proxyUsername{""};
    OptionBinding proxyUsernameBinding{optBinds, proxyUsername, "proxy_username"};

    OptionString proxyPassword{""};
    OptionBinding proxyPasswordBinding{optBinds, proxyPassword, "proxy_password"};

    OptionStringList protectedPackages{{}};
    OptionBinding protectedPackagesBinding{optBinds, protectedPackages, "protected_packages"};

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
    OptionBinding bandwidthBinding{optBinds, bandwidth, "bandwidth"};

    OptionNumber<std::uint32_t> minRate{1000};
    OptionBinding minRateBinding{optBinds, minRate, "minrate"};

    OptionEnum<std::string> ipResolve{"whatever", {"ipv4", "ipv6", "whatever"}};
    OptionBinding ipResolveBinding{optBinds, ipResolve, "ip_resolve"};

    OptionNumber<std::uint32_t> throttle{0};
    OptionBinding throttleBinding{optBinds, throttle, "throttle"};

    OptionNumber<std::uint32_t> timeout{30};
    OptionBinding timeoutBinding{optBinds, timeout, "timeout"};

    OptionNumber<std::uint32_t> maxParallelDownloads{3, 1};
    OptionBinding maxParallelDownloadsBinding{optBinds, maxParallelDownloads, "max_parallel_downloads"};

    OptionNumber<std::uint32_t> metadataExpire{60 * 60 * 48};
    OptionBinding metadataExpireBinding{optBinds, metadataExpire, "metadata_expire"};

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
    ConfigRepo(ConfigRepoMain & parent) : parent{parent} {}

    OptionString name{""};
    OptionBinding nameBinding{optBinds, name, "name"};

    OptionChild<OptionBool> enabled{parent.enabled};
    OptionBinding enabledBinding{optBinds, enabled, "enabled"};

    OptionChild<OptionString> baseCachedir{parent.cachedir};
    OptionBinding baseCachedirBindings{optBinds, baseCachedir, "cachedir"};

    OptionStringList baseUrl{{}, {URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding baseUrlBinding{optBinds, baseUrl, "baseurl"};

    OptionString mirrorList{nullptr, {URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding mirrorListBinding{optBinds, mirrorList, "mirrorlist"};

    OptionString metaLink{nullptr, {URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding metaLinkBinding{optBinds, metaLink, "metalink"};

    OptionString type{""};
    OptionBinding typeBinding{optBinds, type, "type"};

    OptionString mediaId{""};
    OptionBinding mediaIdBinding{optBinds, mediaId, "mediaid"};

    OptionStringList gpgKey{{}, {URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
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
