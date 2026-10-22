#pragma once

#include "json.hpp"
#include <filesystem>
#include <stdexcept>

namespace settings_io
{
    // Fill absent fields only: explicit invalid values must still be rejected.
    inline void fill_missing(nlohmann::json &value, const nlohmann::json &defaults)
    {
        if(!value.is_object())
            throw std::runtime_error("Settings must be a JSON object");
        for(auto it = defaults.begin(); it != defaults.end(); ++it)
        {
            if(!value.contains(it.key()))
                value[it.key()] = it.value();
            else if(value[it.key()].is_object() && it.value().is_object())
                fill_missing(value[it.key()], it.value());
        }
    }

    inline std::filesystem::path backup_invalid(const std::filesystem::path &path)
    {
        // Never replace an earlier backup, even after several failed startups.
        for(unsigned index = 0; ; ++index)
        {
            auto backup = path;
            backup += L".invalid" + (index ? L"." + std::to_wstring(index) : L"") + L".bak";
            std::error_code error;
            if(std::filesystem::copy_file(path, backup, std::filesystem::copy_options::none, error))
                return backup;
            if(error != std::errc::file_exists)
                throw std::filesystem::filesystem_error("Cannot back up invalid settings", path, backup, error);
        }
    }
}
