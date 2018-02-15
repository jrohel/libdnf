%module conf

%include <stdint.i>
%include <std_map.i>
%include <std_pair.i>
%include <std_vector.i>
%include <std_shared_ptr.i>
%include <std_string.i>

%begin %{
    #define SWIG_PYTHON_2_UNICODE
%}

%shared_ptr(Configuration)

%{
    // make SWIG wrap following headers
    #include <iterator>
    #include "libdnf/utils/regex/regex.hpp"
    #include "libdnf/conf/options.hpp"
    #include "libdnf/conf/config.hpp"
    using namespace libdnf;
%}

%include <exception.i>
%exception {
    try {
        $action
    }
    catch (const std::exception & e)
    {
       SWIG_exception(SWIG_RuntimeError, (std::string("C++ std::exception: ") + e.what()).c_str());
    }
    catch (...)
    {
       SWIG_exception(SWIG_UnknownError, "C++ anonymous exception");
    }
}

// make SWIG look into following headers
// ============================
//%include "libdnf/conf/options.hpp"
// ============================

// for SWIG < 3.0.0
#define override
#define noexcept

%ignore Priority::Priority();
%ignore Priority::~Priority();
%inline %{
    struct OptionPriority {
        enum {
            EMPTY = 0,
            DEFAULT = 10,
            MAINCONFIG = 20,
            AUTOMATICCONFIG = 30,
            REPOCONFIG = 40,
            PLUGINDEFAULT = 50,
            PLUGINCONFIG = 60,
            COMMANDLINE = 70,
            RUNTIME = 80
        };
    };
%}

%ignore Option::Option();
class Option {
public:
    virtual std::string getValueString() const = 0;
    %extend {
        int getPriority() const { return static_cast<int>($self->getPriority()); }
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};

%ignore OptionChild();
template <class ParentOptionType, class Enable = void>
class OptionChild : public Option {
public:
    const typename ParentOptionType::ValueType getValue() const;
    const typename ParentOptionType::ValueType getDefaultValue() const;
    std::string getValueString() const override;
};
%extend OptionChild {
    int getPriority() const { return static_cast<int>($self->getPriority()); }
    void set(int priority, const typename ParentOptionType::ValueType & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
}

template <class ParentOptionType>
class OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type> : public Option {
public:
    const std::string & getValue() const;
    const std::string & getDefaultValue() const;
    std::string getValueString() const override;
};
/*%extend OptionChild<ParentOptionType> {
    int getPriority() const { return static_cast<int>($self->getPriority()); }
    void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
}*/

template <typename T>
class OptionNumber : public Option {
public:
    typedef T ValueType;

    OptionNumber(T defaultValue, T min, T max);
    OptionNumber(T defaultValue, T min);
    OptionNumber(T defaultValue);
    void test(T value) const;
    T fromString(const std::string & value) const;
    T getValue() const;
    T getDefaultValue() const;
    std::string toString(T value) const;
    std::string getValueString() const override;
};
%extend OptionNumber {
    void set(int priority, T value) { $self->set(static_cast<Option::Priority>(priority), value); }
    void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
}

class OptionBool : public Option {
public:
    typedef bool ValueType;

    OptionBool(bool defaultValue, const char * const trueVals[], const char * const falseVals[]);
    OptionBool(bool defaultValue);
    void test(bool) const;
    bool fromString(const std::string & value) const;
    bool getValue() const;
    std::string toString(bool value) const;
    std::string getValueString() const override;

    bool getDefaultValue() const noexcept;
    const char * const * getTrueNames() const noexcept;
    const char * const * getFalseNames() const noexcept;

    %extend {
        void set(int priority, bool value) { $self->set(static_cast<Option::Priority>(priority), value); }
        void set(int priority, int value) { $self->set(static_cast<Option::Priority>(priority), value); }
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};

class OptionString : public Option {
public:
    typedef std::string ValueType;

    OptionString(const std::string & defaultValue);
    OptionString(const char * defaultValue);
    void test(const std::string & value) const;
    const std::string & getValue() const;
    const std::string & getDefaultValue() const noexcept;
    std::string getValueString() const override;
    %extend {
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};

template <typename T>
class OptionEnum : public Option {
public:
    typedef T ValueType;

    OptionEnum(T defaultValue, const std::vector<T> & enumVals);
    void test(T value) const;
    T fromString(const std::string & value) const;
    T getValue() const;
    std::string toString(T value) const;
    std::string getValueString() const override;
    %extend {
        void set(int priority, T value) { $self->set(static_cast<Option::Priority>(priority), value); }
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};

template <>
class OptionEnum<std::string> : public Option {
public:
    typedef std::string ValueType;

    OptionEnum(const std::string & defaultValue, const std::vector<std::string> & enumVals);
    void test(const std::string & value) const;
    const std::string & getValue() const;
    const std::string & getDefaultValue() const;
    std::string getValueString() const override;
    %extend {
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};

template<typename T>
class OptionList : public Option {
public:
    typedef std::vector<T> ValueType;

    OptionList(const std::vector<T> & defaultValue);
    OptionList(const std::string & defaultValue);
    void test(const std::vector<T> &) const;
    std::vector<T> fromString(const std::string & value) const;
    const std::vector<T> & getValue() const;
    const std::vector<T> & getDefaultValue() const;
    std::string toString(const std::vector<T> & value) const;
    std::string getValueString() const override;
    %extend {
        void set(int priority, const std::vector<T> & value) { $self->set(static_cast<Option::Priority>(priority), value); }
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};

class OptionStringList : public Option {
public:
    typedef std::vector<std::string> ValueType;

    OptionStringList(const std::vector<std::string> & defaultValue);
    OptionStringList(const std::string & defaultValue);
    void test(const std::vector<std::string> & value) const;
    std::vector<std::string> fromString(const std::string & value) const;
    const std::vector<std::string> & getValue() const;
    const std::vector<std::string> & getDefaultValue() const;
    std::string toString(const std::vector<std::string> & value) const;
    std::string getValueString() const override;
    %extend {
        void set(int priority, const std::vector<std::string> & value) { $self->set(static_cast<Option::Priority>(priority), value); }
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};

%template(OptionNumberInt32) OptionNumber<std::int32_t>;
class OptionSeconds : public OptionNumber<std::int32_t> {
public:
    typedef int ValueType;
    OptionSeconds(ValueType defaultValue, ValueType min, ValueType max);
    OptionSeconds(ValueType defaultValue, ValueType min);
    OptionSeconds(ValueType defaultValue);
    ValueType getValue() const;
    ValueType getDefaultValue() const;
    std::string toString(ValueType value) const;
    std::string getValueString() const override;
    %extend {
        void set(int priority, ValueType value) { $self->set(static_cast<Option::Priority>(priority), value); }
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};

class OptionPath : public OptionString {
public:
    OptionPath(const std::string & defaultValue, bool exists = false, bool absPath = false);
    OptionPath(const char * defaultValue, bool exists = false, bool absPath = false);
    void test(const std::string & value) const;
    std::string getValueString() const override;
    %extend {
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};

/*class OptionStringMap : public Option {
public:
    typedef std::map<std::string, std::string> ValueType;

    OptionStringMap(const ValueType & defaultValue);
    OptionStringMap(const std::string & defaultValue);
    void test(const ValueType &) const;
    ValueType fromString(const std::string & value) const;
    const ValueType & getValue() const;
    const ValueType & getDefaultValue() const;
    std::string toString(const ValueType & value) const;
    std::string getValueString() const override;
    %extend {
        void set(int priority, const ValueType & value) { $self->set(static_cast<Option::Priority>(priority), value); }
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};*/


/* Instantiate templates */
%template(OptionNumberUInt32) OptionNumber<std::uint32_t>;
%template(OptionNumberFloat) OptionNumber<float>;
%template(OptionEnumString) OptionEnum<std::string>;
%template(OptionChildBool) OptionChild<OptionBool>;
%template(OptionChildString) OptionChild<OptionString>;
%template(OptionChildStringList) OptionChild<OptionStringList>;
%template(OptionChildNumberUInt32) OptionChild< OptionNumber<std::uint32_t> >;
%template(OptionChildNumberFloat) OptionChild< OptionNumber<float> >;
%template(OptionChildEnumString) OptionChild< OptionEnum<std::string> >;
%template(OptionChildSeconds) OptionChild<OptionSeconds>;

%template(VectorString) std::vector<std::string>;
%template(PairStringString) std::pair<std::string, std::string>;
%template(MapStringString) std::map<std::string, std::string>;


%inline %{
class StopIterator {};

template<class T>
class Iterator {
public:
    Iterator(typename T::iterator _cur, typename T::iterator _end) : cur(_cur), end(_end) {}
    Iterator* __iter__()
    {
      return this;
    }

    typename T::iterator cur;
    typename T::iterator end;
  };
%}

%template(OptionBindsIterator) Iterator<OptionBinds>;

// ============================
//%include "libdnf/conf/config.hpp"
// ============================

class Config;

%ignore OptionBinding::OptionBinding();
%ignore OptionBinding::OptionBinding(const OptionBinding & src);
%ignore OptionBinding::operator=(const OptionBinding & src);
class OptionBinding {
public:
    Option::Priority getPriority() const;
//    void newString(Option::Priority priority, const std::string & value);
    std::string getValueString() const;
};

%ignore OptionBinds::OptionBinds();
%ignore OptionBinds::OptionBinds(const OptionBinds & src);
%ignore OptionBinds::operator=(const OptionBinds & src);
class OptionBinds {
public:
    OptionBinding & at(const std::string & id);
    const OptionBinding & at(const std::string & id) const;
    bool empty() const noexcept;
    std::size_t size() const noexcept;
};

class Config {
public:
    OptionBinds & optBinds();
};

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
    OptionStringList & installOnlyPkgs();
    OptionStringList & groupPackageTypes();

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

};

class ConfigRepo : public Config {
public:
    ConfigRepo(ConfigMain & masterConfig);
    ~ConfigRepo();

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
};

%pythoncode %{
# Compatible name aliases

ConfigMain.debuglevel = ConfigMain.debugLevel
ConfigMain.errorlevel = ConfigMain.errorLevel
ConfigMain.installroot = ConfigMain.installRoot
ConfigMain.config_file_path = ConfigMain.configFilePath
ConfigMain.pluginpath = ConfigMain.pluginPath
ConfigMain.pluginconfpath = ConfigMain.pluginConfPath
ConfigMain.persistdir = ConfigMain.persistDir
ConfigMain.transformdb = ConfigMain.transformDb
ConfigMain.reset_nice = ConfigMain.resetNice
ConfigMain.system_cachedir = ConfigMain.systemCacheDir
ConfigMain.cacheonly = ConfigMain.cacheOnly
ConfigMain.keepcache = ConfigMain.keepCache
ConfigMain.logdir = ConfigMain.logDir
ConfigMain.reposdir = ConfigMain.reposDir
ConfigMain.debug_solver = ConfigMain.debugSolver
ConfigMain.installonlypkgs = ConfigMain.installOnlyPkgs
ConfigMain.group_package_types = ConfigMain.groupPackageTypes
ConfigMain.installonly_limit = ConfigMain.installOnlyLimit

ConfigMain.tsflags = ConfigMain.tsFlags
ConfigMain.assumeyes = ConfigMain.assumeYes
ConfigMain.assumeno = ConfigMain.assumeNo
ConfigMain.check_config_file_age = ConfigMain.checkConfigFileAge
ConfigMain.defaultyes = ConfigMain.defaultYes
ConfigMain.diskspacecheck = ConfigMain.diskSpaceCheck
ConfigMain.localpkg_gpgcheck = ConfigMain.localPkgGpgCheck
ConfigMain.showdupesfromrepos = ConfigMain.showDupesFromRepos
ConfigMain.exit_on_lock = ConfigMain.exitOnLock
ConfigMain.metadata_timer_sync = ConfigMain.metadataTimerSync
ConfigMain.disable_excludes = ConfigMain.disableExcludes
ConfigMain.multilib_policy = ConfigMain.multilibPolicy  # :api
ConfigMain.install_weak_deps = ConfigMain.installWeakDeps
ConfigMain.bugtracker_url = ConfigMain.bugtrackerUrl
ConfigMain.color_list_installed_older = ConfigMain.colorListInstalledOlder
ConfigMain.color_list_installed_newer = ConfigMain.colorListInstalledNewer
ConfigMain.color_list_installed_reinstall = ConfigMain.colorListInstalledReinstall
ConfigMain.color_list_installed_extra = ConfigMain.colorListInstalledExtra
ConfigMain.color_list_available_upgrade = ConfigMain.colorListAvailableUpgrade
ConfigMain.color_list_available_downgrade = ConfigMain.colorListAvailableDowngrade
ConfigMain.color_list_available_reinstall = ConfigMain.colorListAvailableReinstall
ConfigMain.color_list_available_install = ConfigMain.colorListAvailableInstall
ConfigMain.color_update_installed = ConfigMain.colorUpdateInstalled
ConfigMain.color_update_local = ConfigMain.colorUpdateLocal
ConfigMain.color_update_remote = ConfigMain.colorUpdateRemote
ConfigMain.color_search_match = ConfigMain.colorSearchMatch
ConfigMain.history_record = ConfigMain.historyRecord
ConfigMain.history_record_packages = ConfigMain.historyRecordPackages
ConfigMain.rpmverbosity = ConfigMain.rpmVerbosity
ConfigMain.skip_broken = ConfigMain.skipBroken  # :yum-compatibility
ConfigMain.autocheck_running_kernel = ConfigMain.autocheckRunningKernel  # :yum-compatibility
ConfigMain.clean_requirements_on_remove = ConfigMain.cleanRequirementsOnRemove
ConfigMain.history_list_view = ConfigMain.historyListView
ConfigMain.upgrade_group_objects_upgrade = ConfigMain.upgradeGroupObjectsUpgrade
ConfigMain.destdir = ConfigMain.destDir
ConfigMain.downloadonly = ConfigMain.downloadOnly
ConfigMain.ignorearch = ConfigMain.ignoreArch


# Repo main config
ConfigMain.cachedir = ConfigMain.cacheDir
ConfigMain.fastestmirror = ConfigMain.fastestMirror
ConfigMain.excludepkgs = ConfigMain.excludePkgs
ConfigMain.exclude = ConfigMain.excludePkgs
ConfigMain.includepkgs = ConfigMain.includePkgs
ConfigMain.proxy_username = ConfigMain.proxyUsername
ConfigMain.proxy_password = ConfigMain.proxyPassword
ConfigMain.protected_packages = ConfigMain.protectedPackages
ConfigMain.gpgcheck = ConfigMain.gpgCheck
ConfigMain.repo_gpgcheck = ConfigMain.repoGpgCheck
ConfigMain.enablegroups = ConfigMain.enableGroups
ConfigMain.minrate = ConfigMain.minRate
ConfigMain.ip_resolve = ConfigMain.ipResolve
ConfigMain.max_parallel_downloads = ConfigMain.maxParallelDownloads
ConfigMain.metadata_expire = ConfigMain.metadataExpire
ConfigMain.sslcacert = ConfigMain.sslCaCert
ConfigMain.sslverify = ConfigMain.sslVerify
ConfigMain.sslclientcert = ConfigMain.sslClientCert
ConfigMain.sslclientkey = ConfigMain.sslClientKey
ConfigMain.deltarpm = ConfigMain.deltaRpm
ConfigMain.deltarpm_percentage = ConfigMain.deltaRpmPercentage

ConfigRepo.basecachedir = ConfigRepo.baseCacheDir
ConfigRepo.baseurl = ConfigRepo.baseUrl
ConfigRepo.mirrorlist = ConfigRepo.mirrorList
ConfigRepo.metalink = ConfigRepo.metaLink
ConfigRepo.mediaid = ConfigRepo.mediaId
ConfigRepo.gpgkey = ConfigRepo.gpgKey
ConfigRepo.excludepkgs = ConfigRepo.excludePkgs
ConfigRepo.exclude = ConfigRepo.excludePkgs
ConfigRepo.includepkgs = ConfigRepo.includePkgs
ConfigRepo.fastestmirror = ConfigRepo.fastestMirror
ConfigRepo.proxy_username = ConfigRepo.proxyUsername
ConfigRepo.proxy_password = ConfigRepo.proxyPassword
ConfigRepo.protected_packages = ConfigRepo.protectedPackages
ConfigRepo.gpgcheck = ConfigRepo.gpgCheck
ConfigRepo.repo_gpgcheck = ConfigRepo.repoGpgCheck
ConfigRepo.enablegroups = ConfigRepo.enableGroups
ConfigRepo.minrate = ConfigRepo.minRate
ConfigRepo.ip_resolve = ConfigRepo.ipResolve
ConfigRepo.max_parallel_downloads = ConfigRepo.maxParallelDownloads
ConfigRepo.metadata_expire = ConfigRepo.metadataExpire
ConfigRepo.sslcacert = ConfigRepo.sslCaCert
ConfigRepo.sslverify = ConfigRepo.sslVerify
ConfigRepo.sslclientcert = ConfigRepo.sslClientCert
ConfigRepo.sslclientkey = ConfigRepo.sslClientKey
ConfigRepo.deltarpm = ConfigRepo.deltaRpm
ConfigRepo.deltarpm_percentage = ConfigRepo.deltaRpmPercentage
ConfigRepo.skip_if_unavailable = ConfigRepo.skipIfUnavailable
ConfigRepo.enabled_metadata = ConfigRepo.enabledMetadata
ConfigRepo.failovermethod = ConfigRepo.failoverMethod
%}

%exception __next__() {
    try
    {
        $action  // calls %extend function next() below
    }
    catch (StopIterator)
    {
        PyErr_SetString(PyExc_StopIteration, "End of iterator");
        return NULL;
    }
}

// For old Python
%exception next() {
    try
    {
        $action  // calls %extend function next() below
    }
    catch (StopIterator)
    {
        PyErr_SetString(PyExc_StopIteration, "End of iterator");
        return NULL;
    }
}

%template(PairStringOptionBinding) std::pair<std::string, OptionBinding *>;
%extend Iterator<OptionBinds> {
    std::pair<std::string, OptionBinding *> __next__()
    {
        if ($self->cur != $self->end) {
            auto & id = $self->cur->first;
            auto pValue = &($self->cur++)->second;
            return {id, pValue};
        }
        throw StopIterator();
    }
    std::pair<std::string, OptionBinding *> next()
    {
        if ($self->cur != $self->end) {
            auto & id = $self->cur->first;
            auto pValue = &($self->cur++)->second;
            return {id, pValue};
        }
        throw StopIterator();
    }
}

%extend OptionBinds {
    Iterator<OptionBinds> __iter__()
    {
        return Iterator<OptionBinds>($self->begin(), $self->end());
    }
}
