/*
   MIT License

   Copyright (c) 2019 Berkay Yigit <berkay2578@gmail.com>
      Copyright holder detail: Nickname(s) used by the copyright holder: 'berkay2578', 'berkayylmao'.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#pragma once
#include "stdafx.h"
#include "Extensions\Extensions.h"
using namespace GameInternals::Data::GameTypes;

namespace Extensions {
   namespace InGameMenu {
      class CarConfigurator : public _BaseInGameMenuItem {
         bool             autoUpdate       = false;
         RigidBodyData*   rigidBodyData    = nullptr;
         Settings::CarConfigurationPreset  activeCarConfigPreset = { 0 };

         Settings::CarConfigurationPreset*  pActivePreset     = nullptr;
         const char*                        pActivePresetName = nullptr;
         RockportEdControls::PresetControls presetControls;

         void updateActiveCarConfigPreset() {
            activeCarConfigPreset.Mass   = rigidBodyData->Mass;
            activeCarConfigPreset.OOMass = rigidBodyData->OOMass;
         }

      public:
         bool createPreset(const char* presetName) {
            updateActiveCarConfigPreset();

            auto stlString = std::string(presetName);
            Settings::settingsType.carConfigPresets[stlString] = activeCarConfigPreset;
            Settings::saveSettings();

            // TODO: Improve preset name loading
            pActivePreset = &Settings::settingsType.carConfigPresets[stlString];
            for (auto& preset : Settings::settingsType.carConfigPresets) {
               if (&preset.second == pActivePreset)
                  pActivePresetName = preset.first.c_str();
            }
            return pActivePreset && pActivePresetName;
         }

         bool loadPreset(LPVOID* pListActivePreset, const char** /*pListActivePresetName*/) {
            auto pPreset = reinterpret_cast<Settings::CarConfigurationPreset*>(*pListActivePreset);
            rigidBodyData->Mass   = pPreset->Mass;
            rigidBodyData->OOMass = pPreset->OOMass;
            activeCarConfigPreset.PhysicsTuning = pPreset->PhysicsTuning;

            GameInternals::Gameplay::Player::Car::setCarPhysicsTuning(reinterpret_cast<CarPhysicsTuning*>(&activeCarConfigPreset.PhysicsTuning));
            rigidBodyData->LinearVelocity.z += 1.5f;
            return true;
         }

         bool updateActivePreset() {
            updateActiveCarConfigPreset();
            Settings::settingsType.carConfigPresets[std::string(pActivePresetName)] = activeCarConfigPreset;
            return true;
         }

         bool deletePreset(LPVOID* pListActivePreset, const char** pListActivePresetName) {
            Settings::settingsType.carConfigPresets.erase(std::string(*pListActivePresetName));
            Settings::saveSettings();

            if (!Settings::settingsType.colours.hudColourPresets.empty()) {
               pActivePreset     = &Settings::settingsType.carConfigPresets.begin()->second;
               pActivePresetName = Settings::settingsType.carConfigPresets.begin()->first.c_str();
               return true;
            } else {
               pActivePreset     = nullptr;
               pActivePresetName = nullptr;
               return false;
            }
         }

         bool listPresets(LPVOID* pListActivePreset, const char** pListActivePresetName) {
            if (!Settings::settingsType.carConfigPresets.empty()) {
               if (!*pListActivePreset || !*pListActivePresetName) {
                  *pListActivePreset     = &Settings::settingsType.carConfigPresets.begin()->second;
                  *pListActivePresetName = Settings::settingsType.carConfigPresets.begin()->first.c_str();
               }
               if (*pListActivePreset && *pListActivePresetName) {
                  if (ImGui::BeginCombo("##Presets", *pListActivePresetName, ImGuiComboFlags_HeightSmall)) {
                     for (auto& preset : Settings::settingsType.carConfigPresets) {
                        const char* presetName = preset.first.c_str();
                        bool isItemSelected = (presetName == *pListActivePresetName);
                        if (ImGui::Selectable(presetName, isItemSelected)) {
                           *pListActivePreset     = &preset.second;
                           *pListActivePresetName = presetName;
                        }
                        if (isItemSelected)
                           ImGui::SetItemDefaultFocus();
                     }
                     ImGui::EndCombo();
                  }
                  return true;
               }
            }
            return false;
         }

         const virtual void loadData() override {
            presetControls.Set([this](const char* _1) { return this->createPreset(_1); },
                               [this](LPVOID* _1, const char** _2) { return this->loadPreset(_1, _2); },
                               [this]() { return this->updateActivePreset(); },
                               [this](LPVOID* _1, const char** _2) { return this->deletePreset(_1, _2); },
                               [this](LPVOID* _1, const char** _2) { return this->listPresets(_1, _2); },
                               reinterpret_cast<LPVOID*>(&pActivePreset),
                               &pActivePresetName
            );
            hasLoadedData = true;
         }

         const virtual void onFrame() override {}

         const virtual bool displayMenuItem(const ImVec2& buttonSize) override {
            return ImGui::Button("Car Configurator", buttonSize);
         }
         const virtual bool displayMenu() override {
            if (GameInternals::Gameplay::Object::getRigidBodyData(rigidBodyData)) {
               presetControls.Draw();

               ImGui::TextWrapped("Object Data");
               ImGui::Indent(5.0f);
               {
                  ImGui::SliderFloat("Mass", &rigidBodyData->Mass, 0.1f, 3500.0f);
                  ImGui::SliderFloat("OOMass", &rigidBodyData->OOMass, 0.0001f, 0.003f, "%.6f");
               }
               ImGui::Unindent(5.0f);

               ImGui::TextWrapped("Physics Tuning");
               ImGui::Indent(5.0f);
               {
                  ImGui::SliderFloat("Steering", &activeCarConfigPreset.PhysicsTuning.Steering, -10.0f, 10.0f);
                  ImGui::SliderFloat("Handling", &activeCarConfigPreset.PhysicsTuning.Handling, -10.0f, 10.0f);
                  ImGui::SliderFloat("Brakes", &activeCarConfigPreset.PhysicsTuning.Brakes, -10.0f, 10.0f);
                  ImGui::SliderFloat("Ride Height", &activeCarConfigPreset.PhysicsTuning.RideHeight, -10.0f, 10.0f);
                  ImGui::SliderFloat("Aerodynamics", &activeCarConfigPreset.PhysicsTuning.Aerodynamics, -10.0f, 10.0f);
                  ImGui::SliderFloat("NOS", &activeCarConfigPreset.PhysicsTuning.NOS, -10.0f, 10.0f);
                  ImGui::SliderFloat("Turbo", &activeCarConfigPreset.PhysicsTuning.Turbo, -10.0f, 10.0f);
                  if (ImGui::Button("Apply") || autoUpdate) {
                     GameInternals::Gameplay::Player::Car::setCarPhysicsTuning(reinterpret_cast<CarPhysicsTuning*>(&activeCarConfigPreset.PhysicsTuning));
                     if (!autoUpdate)
                        rigidBodyData->LinearVelocity.z += 1.5f;
                  } ImGui::SameLine();
                  ImGui::Checkbox("Auto Update", &autoUpdate);
               }
               ImGui::Unindent(5.0f);
               return true;
            } else {
               return false;
            }
         }
      };
   }
}