#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include "options.hpp"

#include <exception>
#include <functional>
#include <istream>
#include <ostream>
#include <map>
#include <memory>
#include <string>

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

class Config;

class Substitution : public std::pair<std::string, std::string>
{
public:
    Substitution() = default;
    Substitution(const std::string & key, const std::string & value) : pair(key, value) {}
};

std::ostream & operator<<(std::ostream & os, const Substitution & subst);
std::istream & operator>>(std::istream & is, Substitution & subst);

class OptionBinding {
public:
    typedef std::function<void(Option::Priority, const std::string &)> NewStringFunc;
    typedef std::function<const std::string & ()> GetValueStringFunc;

    OptionBinding(Config & config, Option & option, const std::string & name,
                  NewStringFunc && newString, GetValueStringFunc && getValueString);
    OptionBinding(Config & config, Option & option, const std::string & name);
    OptionBinding(const OptionBinding & src) = delete;

    OptionBinding & operator=(const OptionBinding & src) = delete;

    Option::Priority getPriority() const;
    void newString(Option::Priority priority, const std::string & value);
    std::string getValueString() const;

private:
    Option & option;
    NewStringFunc newStr;
    GetValueStringFunc getValueStr;
};

class OptionBinds {
public:
    typedef std::map<std::string, OptionBinding &> Container;
    typedef Container::iterator iterator;
    typedef Container::const_iterator const_iterator;

    OptionBinds(const OptionBinds &) = delete;
    OptionBinds & operator=(const OptionBinds &) = delete;

    OptionBinding & at(const std::string & id);
    const OptionBinding & at(const std::string & id) const;
    bool empty() const noexcept { return items.empty(); }
    std::size_t size() const noexcept { return items.size(); }
    iterator begin() noexcept { return items.begin(); }
    const_iterator begin() const noexcept { return items.begin(); }
    const_iterator cbegin() const noexcept { return items.cbegin(); }
    iterator end() noexcept { return items.end(); }
    const_iterator end() const noexcept { return items.end(); }
    const_iterator cend() const noexcept { return items.cend(); }
    iterator find(const std::string & id) { return items.find(id); }
    const_iterator find(const std::string & id) const { return items.find(id); }

private:
    friend class Config;
    friend class OptionBinding;
    OptionBinds() = default;
    iterator add(const std::string & id, OptionBinding & optBind);
    Container items;
};

class Config {
public:
    OptionBinds & optBinds() { return binds; }

private:
    OptionBinds binds;
};

class ConfigMain : public Config {
public:
    ConfigMain();
    ~ConfigMain();

    OptionStringMap & substitutions();
    OptionString & arch();
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
    OptionStringList & installOnlyPkgs();
    OptionStringList & groupPackageTypes();

    /*  NOTE: If you set this to 2, then because it keeps the current
    kernel it means if you ever install an "old" kernel it'll get rid
    of the newest one so you probably want to use 3 as a minimum
    ... if you turn it on. */
    OptionNumber<std::uint32_t> & installOnlyLimit();

    OptionStringList & tsFlags();
    OptionBool & assumeYes();
    OptionBool & assumeNo();
    OptionBool & checkConfigFileAge();
    OptionBool & defaultYes();
    OptionBool & diskSpaceCheck();
    OptionBool & localPkgGpgCheck();
    OptionBool & obsoletes();
    OptionBool & showDupesFromRepos();
    OptionBool & exitOnLock();
    OptionNumber<std::int32_t> & metadataTimerSync();
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
    OptionStringList & excludePkgs();
    OptionStringList & includePkgs();
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
    OptionNumber<std::uint32_t> & throttle();
    OptionNumber<std::uint32_t> & timeout();
    OptionNumber<std::uint32_t> & maxParallelDownloads();
    OptionNumber<std::uint32_t> & metadataExpire();
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
    OptionChild<OptionStringList> & excludePkgs();
    OptionChild<OptionStringList> & includePkgs();
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
    OptionChild<OptionNumber<std::uint32_t> > & throttle();
    OptionChild<OptionNumber<std::uint32_t> > & timeout();
    OptionChild<OptionNumber<std::uint32_t> > & maxParallelDownloads();
    OptionChild<OptionNumber<std::uint32_t> > & metadataExpire();
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

class ConfigRepos {
public:
    typedef std::map<std::string, ConfigRepo> Container;
    typedef Container::iterator iterator;
    typedef Container::const_iterator const_iterator;

    iterator add(const std::string & id);
    ConfigRepo & at(const std::string & id);
    const ConfigRepo & at(const std::string & id) const;
    bool empty() const noexcept { return items.empty(); }
    std::size_t size() const noexcept { return items.size(); }
    iterator begin() noexcept { return items.begin(); }
    const_iterator begin() const noexcept { return items.begin(); }
    const_iterator cbegin() const noexcept { return items.cbegin(); }
    iterator end() noexcept { return items.end(); }
    const_iterator end() const noexcept { return items.end(); }
    const_iterator cend() const noexcept { return items.cend(); }
    iterator find(const std::string & id) { return items.find(id); }
    const_iterator find(const std::string & id) const { return items.find(id); }
    ConfigMain & getMain() { return repoMain; }

private:
    ConfigMain repoMain;
    Container items;
};

class Configuration {
public:
    void setValue(Option::Priority priority, const std::string & section, const std::string & key,
                  const std::string & value, bool addRepo = false);
    void setValue(Option::Priority priority, const std::string & section, const std::string & key,
                  std::string && value, bool addRepo = false);

    void readIniFile(Option::Priority priority, const std::string & filePath, const std::map<std::string, std::string> & substitutions);
    void readRepoFiles(Option::Priority priority, const std::string & dirPath, const std::map<std::string, std::string> & substitutions);

    ConfigMain & main() { return cfgMain; }
    ConfigRepos & repos() { return cfgRepos; }

private:
    ConfigMain cfgMain;
    ConfigRepos cfgRepos;
};

#endif
