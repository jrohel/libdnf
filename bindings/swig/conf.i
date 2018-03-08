%module conf

%include <stdint.i>
%include <std_map.i>
%include <std_pair.i>
%include <std_vector.i>
%include <std_string.i>

%begin %{
    #define SWIG_PYTHON_2_UNICODE
%}

%{
    // make SWIG wrap following headers
    #include <iterator>
    #include "libdnf/conf/ConfigRepo.hpp"
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

// for SWIG < 3.0.0
//#define override
//#define noexcept

// make SWIG look into following headers
%include "libdnf/conf/Option.hpp"
%include "libdnf/conf/OptionChild.hpp"
%include "libdnf/conf/OptionBool.hpp"
%include "libdnf/conf/OptionEnum.hpp"
%template(OptionEnumString) libdnf::OptionEnum<std::string>;
%include "libdnf/conf/OptionNumber.hpp"
%template(OptionNumberInt32) libdnf::OptionNumber<std::int32_t>;
%template(OptionNumberUInt32) libdnf::OptionNumber<std::uint32_t>;
%template(OptionNumberFloat) libdnf::OptionNumber<float>;
%include "libdnf/conf/OptionSeconds.hpp"
%include "libdnf/conf/OptionString.hpp"
%include "libdnf/conf/OptionStringList.hpp"
%include "libdnf/conf/OptionStringListAppend.hpp"
%include "libdnf/conf/OptionPath.hpp"

%template(OptionChildBool) libdnf::OptionChild<OptionBool>;
%template(OptionChildString) libdnf::OptionChild<OptionString>;
%template(OptionChildStringList) libdnf::OptionChild<OptionStringList>;
%template(OptionChildNumberInt32) libdnf::OptionChild< OptionNumber<std::int32_t> >;
%template(OptionChildNumberUInt32) libdnf::OptionChild< OptionNumber<std::uint32_t> >;
%template(OptionChildNumberFloat) libdnf::OptionChild< OptionNumber<float> >;
%template(OptionChildEnumString) libdnf::OptionChild< OptionEnum<std::string> >;
%template(OptionChildSeconds) libdnf::OptionChild<OptionSeconds>;
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

/*%ignore OptionBinding::OptionBinding();
%ignore OptionBinding::OptionBinding(const OptionBinding & src);
%ignore OptionBinding::operator=(const OptionBinding & src);
%ignore OptionBinds::OptionBinds();
%ignore OptionBinds::OptionBinds(const OptionBinds & src);
%ignore OptionBinds::operator=(const OptionBinds & src);
*/

%include "libdnf/conf/Config.hpp"
%include "libdnf/conf/ConfigMain.hpp"
%include "libdnf/conf/ConfigRepo.hpp"

%template(OptionBindsIterator) Iterator<libdnf::OptionBinds>;

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

%template(PairStringOptionBinding) std::pair<std::string, libdnf::OptionBinding *>;
%extend Iterator<libdnf::OptionBinds> {
    std::pair<std::string, libdnf::OptionBinding *> __next__()
    {
        if ($self->cur != $self->end) {
            auto & id = $self->cur->first;
            auto pValue = &($self->cur++)->second;
            return {id, pValue};
        }
        throw StopIterator();
    }
    std::pair<std::string, libdnf::OptionBinding *> next()
    {
        if ($self->cur != $self->end) {
            auto & id = $self->cur->first;
            auto pValue = &($self->cur++)->second;
            return {id, pValue};
        }
        throw StopIterator();
    }
}

%extend libdnf::OptionBinds {
    Iterator<libdnf::OptionBinds> __iter__()
    {
        return Iterator<libdnf::OptionBinds>($self->begin(), $self->end());
    }
}

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
