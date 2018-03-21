#include "ProfileMaker.hpp"
#include "ModuleProfile.hpp"
#include "NullProfile.hpp"

std::shared_ptr<Profile> ProfileMaker::getProfile(const std::string &profileName, std::shared_ptr<ModulemdModule> modulemd)
{
    GHashTable *profiles = modulemd_module_get_profiles(modulemd.get());

    GHashTableIter iterator{};
    gpointer key, value;

    g_hash_table_iter_init(&iterator, profiles);
    while (g_hash_table_iter_next(&iterator, &key, &value)) {
        std::string key_str = (char *) key;
        if (profileName == key_str) {
            auto *profile = (ModulemdProfile *) value;
            return std::make_shared<ModuleProfile>(profile);
        }
    }

    return std::make_shared<NullProfile>();
}
