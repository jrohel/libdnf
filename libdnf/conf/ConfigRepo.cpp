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

#include "ConfigRepo.hpp"
#include "Const.hpp"

namespace libdnf {

class ConfigRepo::Impl {
    friend class ConfigRepo;

    Impl(Config & owner, ConfigMain & masterConfig) : owner(owner), masterConfig(masterConfig) {}

    Config & owner;
    ConfigMain & masterConfig;

    OptionString name{""};
    OptionBinding nameBinding{owner, name, "name"};

    OptionChild<OptionBool> enabled{masterConfig.enabled()};
    OptionBinding enabledBinding{owner, enabled, "enabled"};

    OptionChild<OptionString> baseCacheDir{masterConfig.cacheDir()};
    OptionBinding baseCacheDirBindings{owner, baseCacheDir, "cachedir"};

    OptionStringList baseUrl{std::vector<std::string>{}, URL_REGEX, true};
    OptionBinding baseUrlBinding{owner, baseUrl, "baseurl"};

    OptionString mirrorList{nullptr, URL_REGEX, true};
    OptionBinding mirrorListBinding{owner, mirrorList, "mirrorlist"};

    OptionString metaLink{nullptr, URL_REGEX, true};
    OptionBinding metaLinkBinding{owner, metaLink, "metalink"};

    OptionString type{""};
    OptionBinding typeBinding{owner, type, "type"};

    OptionString mediaId{""};
    OptionBinding mediaIdBinding{owner, mediaId, "mediaid"};

    OptionStringList gpgKey{std::vector<std::string>{}, URL_REGEX, true};
    OptionBinding gpgKeyBinding{owner, gpgKey, "gpgkey"};

    OptionStringListAppend excludePkgs{std::vector<std::string>{}};
    OptionBinding excludePkgsBinding{owner, excludePkgs, "excludepkgs"};
    OptionBinding excludeBinding{owner, excludePkgs, "exclude"}; //compatibility with yum

    OptionStringListAppend includePkgs{std::vector<std::string>{}};
    OptionBinding includePkgsBinding{owner, includePkgs, "includepkgs"};

    OptionChild<OptionBool> fastestMirror{masterConfig.fastestMirror()};
    OptionBinding fastestMirrorBinding{owner, fastestMirror, "fastestmirror"};

    OptionChild<OptionString> proxy{masterConfig.proxy()};
    OptionBinding proxyBinding{owner, proxy, "proxy"};

    OptionChild<OptionString> proxyUsername{masterConfig.proxyUsername()};
    OptionBinding proxyUsernameBinding{owner, proxyUsername, "proxy_username"};

    OptionChild<OptionString> proxyPassword{masterConfig.proxyPassword()};
    OptionBinding proxyPasswordBinding{owner, proxyPassword, "proxy_password"};

    OptionChild<OptionString> username{masterConfig.username()};
    OptionBinding usernameBinding{owner, username, "username"};

    OptionChild<OptionString> password{masterConfig.password()};
    OptionBinding passwordBinding{owner, password, "password"};

    OptionChild<OptionStringList> protectedPackages{masterConfig.protectedPackages()};
    OptionBinding protectedPackagesBinding{owner, protectedPackages, "protected_packages"};

    OptionChild<OptionBool> gpgCheck{masterConfig.gpgCheck()};
    OptionBinding gpgCheckBinding{owner, gpgCheck, "gpgcheck"};

    OptionChild<OptionBool> repoGpgCheck{masterConfig.repoGpgCheck()};
    OptionBinding repoGpgCheckBinding{owner, repoGpgCheck, "repo_gpgcheck"};

    OptionChild<OptionBool> enableGroups{masterConfig.enableGroups()};
    OptionBinding enableGroupsBinding{owner, enableGroups, "enablegroups"};

    OptionChild<OptionNumber<std::uint32_t> > retries{masterConfig.retries()};
    OptionBinding retriesBinding{owner, retries, "retries"};

    OptionChild<OptionNumber<std::uint32_t> > bandwidth{masterConfig.bandwidth()};
    OptionBinding bandwidthBinding{owner, bandwidth, "bandwidth"};

    OptionChild<OptionNumber<std::uint32_t> > minRate{masterConfig.minRate()};
    OptionBinding minRateBinding{owner, minRate, "minrate"};

    OptionChild<OptionEnum<std::string> > ipResolve{masterConfig.ipResolve()};
    OptionBinding ipResolveBinding{owner, ipResolve, "ip_resolve"};

    OptionChild<OptionNumber<float> > throttle{masterConfig.throttle()};
    OptionBinding throttleBinding{owner, throttle, "throttle"};

    OptionChild<OptionSeconds> timeout{masterConfig.timeout()};
    OptionBinding timeoutBinding{owner, timeout, "timeout"};

    OptionChild<OptionNumber<std::uint32_t> >  maxParallelDownloads{masterConfig.maxParallelDownloads()};
    OptionBinding maxParallelDownloadsBinding{owner, maxParallelDownloads, "max_parallel_downloads"};

    OptionChild<OptionSeconds> metadataExpire{masterConfig.metadataExpire()};
    OptionBinding metadataExpireBinding{owner, metadataExpire, "metadata_expire"};

    OptionNumber<std::int32_t> cost{1000};
    OptionBinding costBinding{owner, cost, "cost"};

    OptionNumber<std::int32_t> priority{99};
    OptionBinding priorityBinding{owner, priority, "priority"};

    OptionChild<OptionString> sslCaCert{masterConfig.sslCaCert()};
    OptionBinding sslCaCertBinding{owner, sslCaCert, "sslcacert"};

    OptionChild<OptionBool> sslVerify{masterConfig.sslVerify()};
    OptionBinding sslVerifyBinding{owner, sslVerify, "sslverify"};

    OptionChild<OptionString> sslClientCert{masterConfig.sslClientCert()};
    OptionBinding sslClientCertBinding{owner, sslClientCert, "sslclientcert"};

    OptionChild<OptionString> sslClientKey{masterConfig.sslClientKey()};
    OptionBinding sslClientKeyBinding{owner, sslClientKey, "sslclientkey"};

    OptionChild<OptionBool> deltaRpm{masterConfig.deltaRpm()};
    OptionBinding deltaRpmBinding{owner, deltaRpm, "deltarpm"};

    OptionChild<OptionNumber<std::uint32_t> > deltaRpmPercentage{masterConfig.deltaRpmPercentage()};
    OptionBinding deltaRpmPercentageBinding{owner, deltaRpmPercentage, "deltarpm_percentage"};

    OptionBool skipIfUnavailable{true};
    OptionBinding skipIfUnavailableBinding{owner, skipIfUnavailable, "skip_if_unavailable"};

    OptionString enabledMetadata{""};
    OptionBinding enabledMetadataBinding{owner, enabledMetadata, "enabled_metadata"};

    OptionEnum<std::string> failoverMethod{"priority", {"priority", "roundrobin"}};
};

ConfigRepo::ConfigRepo(ConfigMain & masterConfig) : pImpl(new Impl(*this, masterConfig)) {}
ConfigRepo::~ConfigRepo() = default;
ConfigRepo::ConfigRepo(ConfigRepo && src) : pImpl(std::move(src.pImpl)) {}

OptionString & ConfigRepo::name() { return pImpl->name; }
OptionChild<OptionBool> & ConfigRepo::enabled() { return pImpl->enabled; }
OptionChild<OptionString> & ConfigRepo::baseCacheDir() { return pImpl->baseCacheDir; }
OptionStringList & ConfigRepo::baseUrl() { return pImpl->baseUrl; }
OptionString & ConfigRepo::mirrorList() { return pImpl->mirrorList; }
OptionString & ConfigRepo::metaLink() { return pImpl->metaLink; }
OptionString & ConfigRepo::type() { return pImpl->type; }
OptionString & ConfigRepo::mediaId() { return pImpl->mediaId; }
OptionStringList & ConfigRepo::gpgKey() { return pImpl->gpgKey; }
OptionStringListAppend & ConfigRepo::excludePkgs() { return pImpl->excludePkgs; }
OptionStringListAppend & ConfigRepo::includePkgs() { return pImpl->includePkgs; }
OptionChild<OptionBool> & ConfigRepo::fastestMirror() { return pImpl->fastestMirror; }
OptionChild<OptionString> & ConfigRepo::proxy() { return pImpl->proxy; }
OptionChild<OptionString> & ConfigRepo::proxyUsername() { return pImpl->proxyUsername; }
OptionChild<OptionString> & ConfigRepo::proxyPassword() { return pImpl->proxyPassword; }
OptionChild<OptionString> & ConfigRepo::username() { return pImpl->username; }
OptionChild<OptionString> & ConfigRepo::password() { return pImpl->password; }
OptionChild<OptionStringList> & ConfigRepo::protectedPackages() { return pImpl->protectedPackages; }
OptionChild<OptionBool> & ConfigRepo::gpgCheck() { return pImpl->gpgCheck; }
OptionChild<OptionBool> & ConfigRepo::repoGpgCheck() { return pImpl->repoGpgCheck; }
OptionChild<OptionBool> & ConfigRepo::enableGroups() { return pImpl->enableGroups; }
OptionChild<OptionNumber<std::uint32_t> > & ConfigRepo::retries() { return pImpl->retries; }
OptionChild<OptionNumber<std::uint32_t> > & ConfigRepo::bandwidth() { return pImpl->bandwidth; }
OptionChild<OptionNumber<std::uint32_t> > & ConfigRepo::minRate() { return pImpl->minRate; }
OptionChild<OptionEnum<std::string> > & ConfigRepo::ipResolve() { return pImpl->ipResolve; }
OptionChild<OptionNumber<float> > & ConfigRepo::throttle() { return pImpl->throttle; }
OptionChild<OptionSeconds> & ConfigRepo::timeout() { return pImpl->timeout; }
OptionChild<OptionNumber<std::uint32_t> > & ConfigRepo::maxParallelDownloads() { return pImpl->maxParallelDownloads; }
OptionChild<OptionSeconds> & ConfigRepo::metadataExpire() { return pImpl->metadataExpire; }
OptionNumber<std::int32_t> & ConfigRepo::cost() { return pImpl->cost; }
OptionNumber<std::int32_t> & ConfigRepo::priority() { return pImpl->priority; }
OptionChild<OptionString> & ConfigRepo::sslCaCert() { return pImpl->sslCaCert; }
OptionChild<OptionBool> & ConfigRepo::sslVerify() { return pImpl->sslVerify; }
OptionChild<OptionString> & ConfigRepo::sslClientCert() { return pImpl->sslClientCert; }
OptionChild<OptionString> & ConfigRepo::sslClientKey() { return pImpl->sslClientKey; }
OptionChild<OptionBool> & ConfigRepo::deltaRpm() { return pImpl->deltaRpm; }
OptionChild<OptionNumber<std::uint32_t> > & ConfigRepo::deltaRpmPercentage() { return pImpl->deltaRpmPercentage; }
OptionBool & ConfigRepo::skipIfUnavailable() { return pImpl->skipIfUnavailable; }
OptionString & ConfigRepo::enabledMetadata() { return pImpl->enabledMetadata; }
OptionEnum<std::string> & ConfigRepo::failoverMethod() { return pImpl->failoverMethod; }

}
