#include "stdafx.h"
#include "Settings.h"
// IO
#include <iostream>
#include <fstream>
#include <filesystem>
// (De)Serialization
#include <cereal\archives\xml.hpp>

namespace Settings
{
    const std::string mainFolder = std::filesystem::current_path().u8string() + std::string("\\RockportEd");
    const std::string settingsFile = mainFolder + std::string("\\RockportEd.xml");

    SettingsType settingsType = {};

    bool loadSettings()
    {
        try
        {
            if (!std::filesystem::exists(settingsFile))
            {
                saveSettings();
                return true;
            }
            std::ifstream ifs(settingsFile);
            cereal::XMLInputArchive iarchive(ifs);
            iarchive(cereal::make_nvp("Settings", settingsType));
        }
        catch (std::runtime_error e)
        {
            MessageBoxA(NULL, e.what(), "Error during loading settings.", MB_ICONERROR | MB_OK);
            return false;
        }
        return true;
    }

    bool saveSettings()
    {
        try
        {
            std::filesystem::create_directories(mainFolder);

            std::ofstream ofs(settingsFile);
            cereal::XMLOutputArchive oarchive(ofs);
            oarchive(cereal::make_nvp("Settings", settingsType));
        }
        catch (std::runtime_error e)
        {
            MessageBoxA(NULL, e.what(), "Error during saving settings.", MB_ICONERROR | MB_OK);
            return false;
        }
        return true;
    }
}
