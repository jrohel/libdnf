#include <regex>

#include "ModuleProfile.hpp"

#include <modulemd/modulemd.h>


ModuleProfile::ModuleProfile(ModulemdProfile *profile)
        : profile(profile)
{
}

std::string ModuleProfile::getName() const
{
    return modulemd_profile_peek_name(profile);
}

std::string ModuleProfile::getDescription() const
{
    return modulemd_profile_peek_description(profile);
}

std::vector<std::string> ModuleProfile::getContent() const
{
    std::vector<std::string> rpms;

    ModulemdSimpleSet *profileRpms = modulemd_profile_peek_rpms(profile);
    gchar **cRpms = modulemd_simpleset_get(profileRpms);

    for (int i = 0; cRpms[i] != nullptr; i++) {
        std::string rpm = cRpms[i];
        rpms.push_back(rpm);
    }

    return rpms;
}

bool ModuleProfile::hasRpm(const std::string &rpm) const
{
    for (auto &&item : getContent()) {
        if (item.find('*') != std::string::npos) {
            std::regex regexp(rpm);
            if (std::regex_search(item, regexp)) {
                return true;
            }
        } else if (item == rpm) {
            return true;
        }
    }

    return false;
}
