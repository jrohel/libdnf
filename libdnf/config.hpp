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

        OptionBinding(std::map<std::string, OptionBinding &> & options, Option & option,
                        const std::string & name, NewStringFunc && newString, GetValueStringFunc && getValueString)
        : option{option}, newStr{newString}, getValueStr{getValueString} { options.insert({name, *this}); }

        OptionBinding(std::map<std::string, OptionBinding &> & options, Option & option,
                        const std::string & name)
        : option{option} { options.insert({name, *this}); }

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

    std::map<std::string, OptionBinding &> options;
};

class ConfigMain : public Config {
public:
    OptionNumber<int> debugLevel{2, 0, 10};
    OptionBinding debugLevelBinding{options, debugLevel, "debug_level"};

    OptionNumber<int> errorLevel{2, 0, 10};
    OptionBinding errorLevelBinding{options, errorLevel, "error_level"};

    OptionString installRoot{"/"};
    OptionBinding installRootBinding{options, installRoot, "install_root"};

    OptionString configFilePath{CONF_FILENAME};
    OptionBinding configFilePathBinding{options, configFilePath, "config_file_path", nullptr, [&](){ return configFilePath.getValue(); }};

    OptionBool plugins{true};
    OptionBinding pluginsBinding{options, plugins, "plugins"};

    OptionStringList pluginPath{{}};
    OptionBinding pluginPathBinding{options, pluginPath, "pluginpath"};

    OptionStringList pluginConfPath{{}};
    OptionBinding pluginConfPathBinding{options, pluginConfPath, "pluginconfpath"};

    OptionString persistDir{""};
    OptionBinding persistDirBinding{options, persistDir, "persistdir"};

    OptionBool transformDb{true};
    OptionBinding transformDbBinding{options, transformDb, "transformdb"};

    OptionNumber<int> recent{7, 0};
    OptionBinding recentBinding{options, recent, "recent"};

    OptionBool resetNice{true};
    OptionBinding resetNiceBinding{options, resetNice, "reset_nice"};

    OptionString systemCachedir{SYSTEM_CACHEDIR};
    OptionBinding systemCachedirBindings{options, systemCachedir, "system_cachedir"};

    OptionBool cacheOnly{false};
    OptionBinding cacheOnlyBinding{options, cacheOnly, "cacheonly"};

    OptionBool keepCache{false};
    OptionBinding keepCacheBinding{options, keepCache, "keepcache"};

    OptionString logDir{"/var/log"};
    OptionBinding logDirBinding{options, logDir, "logdir"};

    OptionStringList reposDir{{"/etc/yum.repos.d", "/etc/yum/repos.d", "/etc/distro.repos.d"}};
    OptionBinding reposDirBinding{options, reposDir, "reposdir"};

    OptionBool debugSolver{false};
    OptionBinding debugSolverBinding{options, debugSolver, "debug_solver"};

    OptionStringList installOnlyPkgs{{}};
    OptionBinding installOnlyPkgsBinding{options, installOnlyPkgs, "installonlypkgs"};

    OptionStringList groupPackageTypes{{}};
    OptionBinding groupPackageTypesBinding{options, groupPackageTypes, "group_package_types"};

    OptionNumber<unsigned int> installOnlyLimit{3, 2};
    OptionBinding installOnlyLimitBinding{options, installOnlyLimit, "installonly_limit"};

    OptionStringList tsFlags{{}};
    OptionBinding tsFlagsBinding{options, tsFlags, "tsflags"};

    OptionBool assumeYes{false};
    OptionBinding assumeYesBinding{options, assumeYes, "assumeyes"};

    OptionBool assumeNo{false};
    OptionBinding assumeNoBinding{options, assumeNo, "assumeno"};
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
    OptionBinding retriesBinding{options, retries, "retries"};

    OptionString cachedir{SYSTEM_CACHEDIR};
    OptionBinding cachedirBindings{options, cachedir, "cachedir"};

    OptionBool fastestMirror{false};
    OptionBinding fastestMirrorBinding{options, fastestMirror, "fastestmirror"};

    OptionStringList excludePkgs{{}};
    OptionBinding excludePkgsBinding{options, excludePkgs, "excludepkgs"};
    OptionBinding excludeBinding{options, excludePkgs, "exclude"}; //compatibility with yum

    OptionStringList includePkgs{{}};
    OptionBinding includePkgsBinding{options, includePkgs, "includepkgs"};

    OptionString proxy{"", {PROXY_URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding proxyBinding{options, proxy, "proxy"};

    OptionString proxyUsername{""};
    OptionBinding proxyUsernameBinding{options, proxyUsername, "proxy_username"};

    OptionString proxyPassword{""};
    OptionBinding proxyPasswordBinding{options, proxyPassword, "proxy_password"};

    OptionStringList protectedPackages{{}};
    OptionBinding protectedPackagesBinding{options, protectedPackages, "protected_packages"};

    OptionString username{""};
    OptionBinding usernameBinding{options, username, "username"};

    OptionString password{""};
    OptionBinding passwordBinding{options, password, "password"};

    OptionBool gpgCheck{false};
    OptionBinding gpgCheckBinding{options, gpgCheck, "gpgcheck"};

    OptionBool repoGpgCheck{false};
    OptionBinding repoGpgCheckBinding{options, repoGpgCheck, "repo_gpgcheck"};

    OptionBool enabled{true};
    OptionBinding enabledBinding{options, enabled, "enabled"};

    OptionBool enableGroups{true};
    OptionBinding enableGroupsBinding{options, enableGroups, "enablegroups"};

    OptionNumber<std::uint32_t> bandwidth{0};
    OptionBinding bandwidthBinding{options, bandwidth, "bandwidth"};

    OptionNumber<std::uint32_t> minRate{1000};
    OptionBinding minRateBinding{options, minRate, "minrate"};

    OptionEnum<std::string> ipResolve{"whatever", {"ipv4", "ipv6", "whatever"}};
    OptionBinding ipResolveBinding{options, ipResolve, "ip_resolve"};

    OptionNumber<std::uint32_t> throttle{0};
    OptionBinding throttleBinding{options, throttle, "throttle"};

    OptionNumber<std::uint32_t> timeout{30};
    OptionBinding timeoutBinding{options, timeout, "timeout"};

    OptionNumber<std::uint32_t> maxParallelDownloads{3, 1};
    OptionBinding maxParallelDownloadsBinding{options, maxParallelDownloads, "max_parallel_downloads"};

    OptionNumber<std::uint32_t> metadataExpire{60 * 60 * 48};
    OptionBinding metadataExpireBinding{options, metadataExpire, "metadata_expire"};

    OptionString sslCaCert{""};
    OptionBinding sslCaCertBinding{options, sslCaCert, "sslcacert"};

    OptionBool sslVerify{true};
    OptionBinding sslVerifyBinding{options, sslVerify, "sslverify"};

    OptionString sslClientCert{""};
    OptionBinding sslClientCertBinding{options, sslClientCert, "sslclientcert"};

    OptionString sslClientKey{""};
    OptionBinding sslClientKeyBinding{options, sslClientKey, "sslclientkey"};

    OptionBool deltaRpm{true};
    OptionBinding deltaRpmBinding{options, deltaRpm, "deltarpm"};

    OptionNumber<unsigned int> deltaRpmPercentage{75};
    OptionBinding deltaRpmPercentageBinding{options, deltaRpmPercentage, "deltarpm_percentage"};
};

class ConfigRepo : Config {
    ConfigRepoMain & parent;

public:
    ConfigRepo(ConfigRepoMain & parent) : parent{parent} {}

    OptionString name{""};
    OptionBinding nameBinding{options, name, "name"};

    OptionChild<OptionBool> enabled{parent.enabled};
    OptionBinding enabledBinding{options, enabled, "enabled"};

    OptionChild<OptionString> baseCachedir{parent.cachedir};
    OptionBinding baseCachedirBindings{options, baseCachedir, "cachedir"};

    OptionStringList baseUrl{{}, {URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding baseUrlBinding{options, baseUrl, "baseurl"};

    OptionString mirrorList{nullptr, {URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding mirrorListBinding{options, mirrorList, "mirrorlist"};

    OptionString metaLink{nullptr, {URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding metaLinkBinding{options, metaLink, "metalink"};

    OptionString type{""};
    OptionBinding typeBinding{options, type, "type"};

    OptionString mediaId{""};
    OptionBinding mediaIdBinding{options, mediaId, "mediaid"};

    OptionStringList gpgKey{{}, {URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB}};
    OptionBinding gpgKeyBinding{options, gpgKey, "gpgkey"};

    OptionChild<OptionStringList> excludePkgs{parent.excludePkgs};
    OptionBinding excludePkgsBinding{options, excludePkgs, "excludepkgs"};
    OptionBinding excludeBinding{options, excludePkgs, "exclude"}; //compatibility with yum

    OptionChild<OptionStringList> includePkgs{parent.includePkgs};
    OptionBinding includePkgsBinding{options, includePkgs, "includepkgs"};

    OptionChild<OptionBool> fastestMirror{parent.fastestMirror};
    OptionBinding fastestMirrorBinding{options, fastestMirror, "fastestmirror"};

    OptionChild<OptionString> proxy{parent.proxy};
    OptionBinding proxyBinding{options, proxy, "proxy"};

    OptionChild<OptionString> proxyUsername{parent.proxyUsername};
    OptionBinding proxyUsernameBinding{options, proxyUsername, "proxy_username"};

    OptionChild<OptionString> proxyPassword{parent.proxyPassword};
    OptionBinding proxyPasswordBinding{options, proxyPassword, "proxy_password"};

    OptionChild<OptionString> username{parent.username};
    OptionBinding usernameBinding{options, username, "username"};

    OptionChild<OptionString> password{parent.password};
    OptionBinding passwordBinding{options, password, "password"};

    OptionChild<OptionStringList> protectedPackages{parent.protectedPackages};
    OptionBinding protectedPackagesBinding{options, protectedPackages, "protected_packages"};

    OptionChild<OptionBool> gpgCheck{parent.gpgCheck};
    OptionBinding gpgCheckBinding{options, gpgCheck, "gpgcheck"};

    OptionChild<OptionBool> repoGpgCheck{parent.repoGpgCheck};
    OptionBinding repoGpgCheckBinding{options, repoGpgCheck, "repo_gpgcheck"};

    OptionChild<OptionBool> enableGroups{parent.enableGroups};
    OptionBinding enableGroupsBinding{options, enableGroups, "enablegroups"};

    OptionChild<OptionNumber<unsigned int> > retries{parent.retries};
    OptionBinding retriesBinding{options, retries, "retries"};

    OptionChild<OptionNumber<std::uint32_t> > bandwidth{parent.bandwidth};
    OptionBinding bandwidthBinding{options, bandwidth, "bandwidth"};

    OptionChild<OptionNumber<std::uint32_t> > minRate{parent.minRate};
    OptionBinding minRateBinding{options, minRate, "minrate"};

    OptionChild<OptionEnum<std::string> > ipResolve{parent.ipResolve};
    OptionBinding ipResolveBinding{options, ipResolve, "ip_resolve"};

    OptionChild<OptionNumber<std::uint32_t> > throttle{parent.throttle};
    OptionBinding throttleBinding{options, throttle, "throttle"};

    OptionChild<OptionNumber<std::uint32_t> > timeout{parent.timeout};
    OptionBinding timeoutBinding{options, timeout, "timeout"};

    OptionChild<OptionNumber<std::uint32_t> >  maxParallelDownloads{parent.maxParallelDownloads};
    OptionBinding maxParallelDownloadsBinding{options, maxParallelDownloads, "max_parallel_downloads"};

    OptionChild<OptionNumber<std::uint32_t> > metadataExpire{parent.metadataExpire};
    OptionBinding metadataExpireBinding{options, metadataExpire, "metadata_expire"};

    OptionNumber<int> cost{1000};
    OptionBinding costBinding{options, cost, "cost"};

    OptionNumber<int> priority{99};
    OptionBinding priorityBinding{options, priority, "priority"};

    OptionChild<OptionString> sslCaCert{parent.sslCaCert};
    OptionBinding sslCaCertBinding{options, sslCaCert, "sslcacert"};

    OptionChild<OptionBool> sslVerify{parent.sslVerify};
    OptionBinding sslVerifyBinding{options, sslVerify, "sslverify"};

    OptionChild<OptionString> sslClientCert{parent.sslClientCert};
    OptionBinding sslClientCertBinding{options, sslClientCert, "sslclientcert"};

    OptionChild<OptionString> sslClientKey{parent.sslClientKey};
    OptionBinding sslClientKeyBinding{options, sslClientKey, "sslclientkey"};

    OptionChild<OptionBool> deltaRpm{parent.deltaRpm};
    OptionBinding deltaRpmBinding{options, deltaRpm, "deltarpm"};

    OptionChild<OptionNumber<unsigned int> > deltaRpmPercentage{parent.deltaRpmPercentage};
    OptionBinding deltaRpmPercentageBinding{options, deltaRpmPercentage, "deltarpm_percentage"};

    OptionBool skipIfUnavailable{true};
    OptionBinding skipIfUnavailableBinding{options, skipIfUnavailable, "skip_if_unavailable"};

    // options recognized by other tools, e.g. gnome-software, but unused in dnf
    OptionString enabledMetadata{""};
    OptionBinding enabledMetadataBinding{options, enabledMetadata, "enabled_metadata"};

    // yum compatibility options
    OptionEnum<std::string> failoverMethod{"priority", {"priority", "roundrobin"}};

private:
};

#endif
