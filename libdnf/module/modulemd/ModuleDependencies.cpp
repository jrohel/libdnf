#include <modulemd/modulemd-simpleset.h>
#include <sstream>

#include "libdnf/utils/utils.hpp"
#include "ModuleDependencies.hpp"

ModuleDependencies::ModuleDependencies(ModulemdDependencies *dependencies)
        : dependencies(dependencies)
{}

std::vector <std::map<std::string, std::vector<std::string> > > ModuleDependencies::getBuildRequires()
{
    auto cBuildRequires = modulemd_dependencies_peek_buildrequires(dependencies);
    return getRequirements(cBuildRequires);

}

std::vector <std::map<std::string, std::vector<std::string> > > ModuleDependencies::getRequires()
{
    auto cRequires = modulemd_dependencies_peek_requires(dependencies);
    return getRequirements(cRequires);
}

std::map<std::string, std::vector<std::string>> ModuleDependencies::wrapModuleDependencies(const void *moduleName, const void *streams) const
{
    std::map<std::string, std::vector<std::string> > moduleRequirements;

    auto name = static_cast<const char *>(moduleName);
    auto streamSet = modulemd_simpleset_dup((ModulemdSimpleSet *) streams);
    while (streamSet != nullptr) {
        moduleRequirements[name].emplace_back(*streamSet);
        streamSet++;
    }

    return moduleRequirements;
}

std::vector<std::map<std::string, std::vector<std::string> > > ModuleDependencies::getRequirements(GHashTable *requirements) const
{
    std::vector <std::map<std::string, std::vector<std::string> > > requires;
    GHashTableIter iterator{};
    gpointer key, value;

    g_hash_table_iter_init (&iterator, requirements);
    while (g_hash_table_iter_next(&iterator, &key, &value)) {
        auto moduleRequirements = wrapModuleDependencies(key, value);

        requires.push_back(moduleRequirements);
    }

    return requires;
}
