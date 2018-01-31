%module config

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
    #include "libdnf/options.hpp"
    #include "libdnf/config.hpp"
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
//%include "libdnf/options.hpp"
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
    %extend {
        int getPriority() const { return static_cast<int>($self->getPriority()); }
        void set(int priority, const typename ParentOptionType::ValueType & value) { $self->set(static_cast<Option::Priority>(priority), value); }
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};

template <class ParentOptionType>
class OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type> : public Option {
public:
    const std::string & getValue() const;
    const std::string & getDefaultValue() const;
    std::string getValueString() const override;
    %extend {
        int getPriority() const { return static_cast<int>($self->getPriority()); }
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};

template <typename T>
class OptionNumber : public Option {
public:
    typedef T ValueType;

    OptionNumber(T defaultValue, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max());
    void test(T value) const;
    T fromString(const std::string & value) const;
    T getValue() const;
    T getDefaultValue() const;
    std::string toString(T value) const;
    std::string getValueString() const override;
    %extend {
        void set(int priority, T value) { $self->set(static_cast<Option::Priority>(priority), value); }
        void set(int priority, const std::string & value) { $self->set(static_cast<Option::Priority>(priority), value); }
    }
};

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

class OptionStringMap : public Option {
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
};


/* Instantiate templates */
%template(OptionNumberInt32) OptionNumber<std::int32_t>;
%template(OptionNumberUInt32) OptionNumber<std::uint32_t>;
%template(OptionEnumString) OptionEnum<std::string>;
%template(OptionChildBool) OptionChild<OptionBool>;
%template(OptionChildString) OptionChild<OptionString>;
%template(OptionChildStringList) OptionChild<OptionStringList>;
%template(OptionChildNumberUInt32) OptionChild< OptionNumber<std::uint32_t> >;
%template(OptionChildEnumString) OptionChild< OptionEnum<std::string> >;

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
%template(ConfigReposIterator) Iterator<ConfigRepos>;

// ============================
//%include "libdnf/config.hpp"
// ============================

class Config;

class Substitution : public std::pair<std::string, std::string>
{
public:
    Substitution(const std::string & key, const std::string & value);
};

%template(VectorSubstitution) std::vector<Substitution>;
%template(OptionListSubstitution) OptionList<Substitution>;

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

//    OptionStringMap & substitutions();
//    OptionString & arch();
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

    %extend {
        OptionNumber<std::int32_t> & debuglevel() { return $self->debugLevel(); }
        OptionNumber<std::int32_t> & errorlevel() { return $self->errorLevel(); }
        OptionString & installroot() { return $self->installRoot(); }
        OptionString & config_file_path() { return $self->configFilePath(); }
        OptionStringList & pluginpath() { return $self->pluginPath(); }
        OptionStringList & pluginconfpath() { return $self->pluginConfPath(); }
        OptionString & persistdir() { return $self->persistDir(); }
        OptionBool & transformdb() { return $self->transformDb(); }
        OptionBool & reset_nice() { return $self->resetNice(); }
        OptionString & system_cachedir() { return $self->systemCacheDir(); }
        OptionBool & cacheonly() { return $self->cacheOnly(); }
        OptionBool & keepcache() { return $self->keepCache(); }
        OptionString & logdir() { return $self->logDir(); }
        OptionStringList & reposdir() { return $self->reposDir(); }
        OptionBool & debug_solver() { return $self->debugSolver(); }
        OptionStringList & installonlypkgs() { return $self->installOnlyPkgs(); }
        OptionStringList & group_package_types() { return $self->groupPackageTypes(); }

        /*  NOTE: If you set this to 2, then because it keeps the current
        kernel it means if you ever install an "old" kernel it'll get rid
        of the newest one so you probably want to use 3 as a minimum
        ... if you turn it on. */
        OptionNumber<std::uint32_t> & installonly_limit() { return $self->installOnlyLimit(); }

        OptionStringList & tsflags() { return $self->tsFlags(); }
        OptionBool & assumeyes() { return $self->assumeYes(); }
        OptionBool & assumeno() { return $self->assumeNo(); }
        OptionBool & check_config_file_age() { return $self->checkConfigFileAge(); }
        OptionBool & defaultyes() { return $self->defaultYes(); }
        OptionBool & diskspacecheck() { return $self->diskSpaceCheck(); }
        OptionBool & localpkg_gpgcheck() { return $self->localPkgGpgCheck(); }
        OptionBool & showdupesfromrepos() { return $self->showDupesFromRepos(); }
        OptionBool & exit_on_lock() { return $self->exitOnLock(); }
        OptionNumber<std::int32_t> & metadata_timer_sync() { return $self->metadataTimerSync(); }
        OptionStringList & disable_excludes() { return $self->disableExcludes(); }
        OptionEnum<std::string> & multilib_policy() { return $self->multilibPolicy(); } // :api
        OptionBool & install_weak_deps() { return $self->installWeakDeps(); }
        OptionString & bugtracker_url() { return $self->bugtrackerUrl(); }
        OptionString & color_list_installed_older() { return $self->colorListInstalledOlder(); }
        OptionString & color_list_installed_newer() { return $self->colorListInstalledNewer(); }
        OptionString & color_list_installed_reinstall() { return $self->colorListInstalledReinstall(); }
        OptionString & color_list_installed_extra() { return $self->colorListInstalledExtra(); }
        OptionString & color_list_available_upgrade() { return $self->colorListAvailableUpgrade(); }
        OptionString & color_list_available_downgrade() { return $self->colorListAvailableDowngrade(); }
        OptionString & color_list_available_reinstall() { return $self->colorListAvailableReinstall(); }
        OptionString & color_list_available_install() { return $self->colorListAvailableInstall(); }
        OptionString & color_update_installed() { return $self->colorUpdateInstalled(); }
        OptionString & color_update_local() { return $self->colorUpdateLocal(); }
        OptionString & color_update_remote() { return $self->colorUpdateRemote(); }
        OptionString & color_search_match() { return $self->colorSearchMatch(); }
        OptionBool & history_record() { return $self->historyRecord(); }
        OptionStringList & history_record_packages() { return $self->historyRecordPackages(); }
        OptionString & rpmverbosity() { return $self->rpmVerbosity(); }
        OptionBool & skip_broken() { return $self->skipBroken(); } // :yum-compatibility
        OptionBool & autocheck_running_kernel() { return $self->autocheckRunningKernel(); } // :yum-compatibility
        OptionBool & clean_requirements_on_remove() { return $self->cleanRequirementsOnRemove(); }
        OptionEnum<std::string> & history_list_view() { return $self->historyListView(); }
        OptionBool & upgrade_group_objects_upgrade() { return $self->upgradeGroupObjectsUpgrade(); }
        OptionPath & destdir() { return $self->destDir(); }
        OptionBool & downloadonly() { return $self->downloadOnly(); }
        OptionBool & ignorearch() { return $self->ignoreArch(); }


        // Repo main config
        OptionString & cachedir() { return $self->cacheDir(); }
        OptionBool & fastestmirror() { return $self->fastestMirror(); }
        OptionStringList & excludepkgs() { return $self->excludePkgs(); }
        OptionStringList & exclude() { return $self->excludePkgs(); }
        OptionStringList & includepkgs() { return $self->includePkgs(); }
        OptionString & proxy_username() { return $self->proxyUsername(); }
        OptionString & proxy_password() { return $self->proxyPassword(); }
        OptionStringList & protected_packages() { return $self->protectedPackages(); }
        OptionBool & gpgcheck() { return $self->gpgCheck(); }
        OptionBool & repo_gpgcheck() { return $self->repoGpgCheck(); }
        OptionBool & enablegroups() { return $self->enableGroups(); }
        OptionNumber<std::uint32_t> & minrate() { return $self->minRate(); }
        OptionEnum<std::string> & ip_resolve() { return $self->ipResolve(); }
        OptionNumber<std::uint32_t> & max_parallel_downloads() { return $self->maxParallelDownloads(); }
        OptionNumber<std::uint32_t> & metadata_expire() { return $self->metadataExpire(); }
        OptionString & sslcacert() { return $self->sslCaCert(); }
        OptionBool & sslverify() { return $self->sslVerify(); }
        OptionString & sslclientcert() { return $self->sslClientCert(); }
        OptionString & sslclientkey() { return $self->sslClientKey(); }
        OptionBool & deltarpm() { return $self->deltaRpm(); }
        OptionNumber<std::uint32_t> & deltarpm_percentage() { return $self->deltaRpmPercentage(); }
    }
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

    %extend {
        OptionChild<OptionString> & basecachedir() { return $self->baseCacheDir(); }
        OptionStringList & baseurl() { return $self->baseUrl(); }
        OptionString & mirrorlist() { return $self->mirrorList(); }
        OptionString & metalink() { return $self->metaLink(); }
        OptionString & mediaid() { return $self->mediaId(); }
        OptionStringList & gpgkey() { return $self->gpgKey(); }
        OptionChild<OptionStringList> & excludepkgs() { return $self->excludePkgs(); }
        OptionChild<OptionStringList> & exclude() { return $self->excludePkgs(); }
        OptionChild<OptionStringList> & includepkgs() { return $self->includePkgs(); }
        OptionChild<OptionBool> & fastestmirror() { return $self->fastestMirror(); }
        OptionChild<OptionString> & proxy_username() { return $self->proxyUsername(); }
        OptionChild<OptionString> & proxy_password() { return $self->proxyPassword(); }
        OptionChild<OptionStringList> & protected_packages() { return $self->protectedPackages(); }
        OptionChild<OptionBool> & gpgcheck() { return $self->gpgCheck(); }
        OptionChild<OptionBool> & repo_gpgcheck() { return $self->repoGpgCheck(); }
        OptionChild<OptionBool> & enablegroups() { return $self->enableGroups(); }
        OptionChild<OptionNumber<std::uint32_t> > & minrate() { return $self->minRate(); }
        OptionChild<OptionEnum<std::string> > & ip_resolve() { return $self->ipResolve(); }
        OptionChild<OptionNumber<std::uint32_t> > & max_parallel_downloads() { return $self->maxParallelDownloads(); }
        OptionChild<OptionNumber<std::uint32_t> > & metadata_expire() { return $self->metadataExpire(); }
        OptionChild<OptionString> & sslcacert() { return $self->sslCaCert(); }
        OptionChild<OptionBool> & sslverify() { return $self->sslVerify(); }
        OptionChild<OptionString> & sslclientcert() { return $self->sslClientCert(); }
        OptionChild<OptionString> & sslclientkey() { return $self->sslClientKey(); }
        OptionChild<OptionBool> & deltarpm() { return $self->deltaRpm(); }
        OptionChild<OptionNumber<std::uint32_t> > & deltarpm_percentage() { return $self->deltaRpmPercentage(); }
        OptionBool & skip_if_unavailable() { return $self->skipIfUnavailable(); }
        // option recognized by other tools, e.g. gnome-software, but unused in dnf
        OptionString & enabled_metadata() { return $self->enabledMetadata(); }
        // yum compatibility options
        OptionEnum<std::string> & failovermethod() { return $self->failoverMethod(); }
    }
};

class ConfigRepos {
public:
    ConfigRepo & at(const std::string & id);
    const ConfigRepo & at(const std::string & id) const;
    bool empty() const noexcept;
    std::size_t size() const noexcept;
    ConfigMain & getMain();

    %extend {
        void add(const std::string & id) { $self->add(id); }
    }
};

class Configuration {
public:
    ConfigMain & main();
    ConfigRepos & repos();

    %extend {
        void setValue(int priority, const std::string & section, const std::string & key,
                      const std::string & value, bool addRepo = false)
        {
            $self->setValue(static_cast<Option::Priority>(priority), section, key, value, addRepo);
        }
        void readIniFile(int priority, const std::string & filePath, const std::map<std::string, std::string> & substitutions) { $self->readIniFile(static_cast<Option::Priority>(priority), filePath, substitutions); }
        void readRepoFiles(int priority, const std::string & dirPath, const std::map<std::string, std::string> & substitutions) { $self->readRepoFiles(static_cast<Option::Priority>(priority), dirPath, substitutions); }
    }
};


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

%template(PairStringConfigRepo) std::pair<std::string, ConfigRepo *>;
%extend Iterator<ConfigRepos> {
    std::pair<std::string, ConfigRepo *> __next__()
    {
        if ($self->cur != $self->end) {
            auto & id = $self->cur->first;
            auto pValue = &($self->cur++)->second;
            return {id, pValue};
        }
        throw StopIterator();
    }
    std::pair<std::string, ConfigRepo *> next()
    {
        if ($self->cur != $self->end) {
            auto & id = $self->cur->first;
            auto pValue = &($self->cur++)->second;
            return {id, pValue};
        }
        throw StopIterator();
    }
}

%extend ConfigRepos {
    Iterator<ConfigRepos> __iter__()
    {
        return Iterator<ConfigRepos>($self->begin(), $self->end());
    }
}
