#ifndef LIBDNF_MODULEDEPENDENCIES_HPP
#define LIBDNF_MODULEDEPENDENCIES_HPP


#include <map>
#include <memory>
#include <string>
#include <vector>
#include <modulemd/modulemd-dependencies.h>

class ModuleDependencies
{
public:
    explicit ModuleDependencies(ModulemdDependencies *dependencies);

    std::vector<std::map<std::string, std::vector<std::string> > > getBuildRequires();
    std::vector<std::map<std::string, std::vector<std::string> > > getRequires();

private:
    static std::map<std::string, std::vector<std::string>> wrapModuleDependencies(const char *moduleName, ModulemdSimpleSet *streams);
    static std::vector<std::map<std::string, std::vector<std::string> > > getRequirements(GHashTable *requirements);

    ModulemdDependencies *dependencies;
};


#endif //LIBDNF_MODULEDEPENDENCIES_HPP
