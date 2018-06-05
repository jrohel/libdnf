#include <modulemd/modulemd-simpleset.h>
#include "ContextTest.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(ContextTest);

#include "libdnf/dnf-context.hpp"
#include "libdnf/dnf-repo-loader.h"
#include "libdnf/sack/query.hpp"
#include "libdnf/nevra.hpp"
#include "libdnf/utils/File.hpp"
#include "libdnf/sack/packageset.hpp"


void ContextTest::setUp()
{
    context = dnf_context_new();
}

void ContextTest::tearDown()
{
    g_object_unref(context);
    g_object_unref(repo);
}

void ContextTest::testLoadModules()
{
    GError *error = nullptr;

    /* set up local context */
    constexpr auto repos_dir = TESTDATADIR "/modules/yum.repos.d/";
    dnf_context_set_repo_dir(context, repos_dir);
    dnf_context_set_solv_dir(context, "/tmp");
    auto ret = dnf_context_setup(context, nullptr, &error);
    g_assert_no_error(error);
    g_assert(ret);

    /* use this as a throw-away */

    g_autoptr(DnfRepoLoader) repo_loader = dnf_repo_loader_new(context);

    /* load local metadata repo */
    repo = dnf_repo_loader_get_repo_by_id(repo_loader, "test", &error);
    g_assert_no_error(error);
    g_assert(repo != nullptr);
    g_assert_cmpint(dnf_repo_get_enabled(repo), ==, DNF_REPO_ENABLED_METADATA | DNF_REPO_ENABLED_PACKAGES);
    g_assert_cmpint(dnf_repo_get_kind(repo), ==, DNF_REPO_KIND_LOCAL);

    DnfState *state = dnf_state_new();
    dnf_repo_check(repo, G_MAXUINT, state, &error);

    state = dnf_context_get_state(context);
    dnf_context_setup_sack(context, state, &error);
    g_assert_no_error(error);

    auto sack = dnf_context_get_sack(context);
    auto moduleExcludes = dnf_sack_get_module_excludes(sack);

    auto modules_fn = dnf_repo_get_filename_md(repo, "modules");

    auto yaml = libdnf::File::newFile(modules_fn);
    yaml->open("r");
    const auto &yamlContent = yaml->getContent();
    yaml->close();

    auto modules = ModuleMetadata::metadataFromString(yamlContent);
    for (const auto &module : modules) {

        // default module:stream
        if (module->getName() == "httpd" && module->getStream() == "2.4")
            sackHas(sack, module);

        // disabled stream
        if (module->getName() == "httpd" && module->getStream() == "2.2")
            sackHasNot(sack, module);
    }
}

void ContextTest::sackHas(DnfSack *sack, const std::shared_ptr<ModuleMetadata> &module) const
{
    libdnf::Nevra nevra{};
    libdnf::Query query{sack};
    auto artifacts = module->getArtifacts();
    for (auto artifact : artifacts) {
            nevra.parse(artifact.c_str(), HY_FORM_NEVRA);
            query.addFilter(&nevra, false);

            artifact = artifact.replace(artifact.find("-0:"), 3, "-");

            auto packageSet = const_cast<libdnf::PackageSet *>(query.runSet());
            CPPUNIT_ASSERT(dnf_packageset_count(packageSet) == 1);
            auto package = dnf_package_new(sack, packageSet->operator[](0));
            CPPUNIT_ASSERT(dnf_package_get_nevra(package) == artifact);

            nevra.clear();
            query.clear();
        }
}

void ContextTest::sackHasNot(DnfSack *sack, const std::shared_ptr<ModuleMetadata> &module) const
{
    libdnf::Nevra nevra{};
    libdnf::Query query{sack};
    auto artifacts = module->getArtifacts();
    for (auto artifact : artifacts) {
        nevra.parse(artifact.c_str(), HY_FORM_NEVRA);
        query.addFilter(&nevra, false);

        artifact = artifact.replace(artifact.find("-0:"), 3, "-");

        auto packageSet = const_cast<libdnf::PackageSet *>(query.runSet());
        CPPUNIT_ASSERT(dnf_packageset_count(packageSet) == 0);

        nevra.clear();
        query.clear();
    }
}
