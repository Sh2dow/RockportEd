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
#include "Helpers\imgui\imgui.h"
// (De)Serialization
#include <cereal\archives\xml.hpp>

namespace Settings {
   struct HUDColourPreset {
      bool DrawGradients          = false;
      bool ClearBackgroundColours = true;

      ImColor RPMGauge_BackColour                                = ImColor(IM_COL32(0, 0, 0, 0)); // not used
      ImColor RPMGauge_NormalRange_Background_Colour             = ImColor(IM_COL32(150, 150, 150, 100));
      ImColor RPMGauge_NormalRange_Background_Colour_GradientEnd = ImColor(IM_COL32(150, 150, 150, 100));
      ImColor RPMGauge_NormalRange_Foreground_Colour             = ImColor(IM_COL32(255, 255, 255, 200));
      ImColor RPMGauge_NormalRange_Foreground_Colour_GradientEnd = ImColor(IM_COL32(255, 255, 255, 200));
      ImColor RPMGauge_Redline_Background_Colour                 = ImColor(IM_COL32(182, 22, 54, 100));
      ImColor RPMGauge_Redline_Background_Colour_GradientEnd     = ImColor(IM_COL32(182, 22, 54, 100));
      ImColor RPMGauge_Redline_Foreground_Colour                 = ImColor(IM_COL32(217, 26, 64, 200));
      ImColor RPMGauge_Redline_Foreground_Colour_GradientEnd     = ImColor(IM_COL32(217, 26, 64, 200));
      ImColor RPMGauge_Needle_Foreground_Colour                  = ImColor(IM_COL32(255, 255, 255, 255));

      ImColor SpeedDisplay_FillerText_Colour   = ImColor(ImVec4(0.7f, 0.7f, 0.7f, 0.6f));
      ImColor SpeedDisplay_ValueText_Colour    = ImColor(ImVec4(1.0f, 1.0f, 1.0f, 0.8f));
      ImColor SpeedDisplay_UnitTypeText_Colour = ImColor(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

      ImColor NOSBar_Background_Colour       = ImColor(IM_COL32(0, 0, 0, 0)); // not used
      ImColor NOSBar_Value_Background_Colour = ImColor(IM_COL32(22, 27, 10, 255));
      ImColor NOSBar_Value_Foreground_Colour = ImColor(IM_COL32(161, 193, 79, 255));

      template <class Archive>
      void serialize(Archive& archive) {
         archive(
            CEREAL_NVP(DrawGradients),
            CEREAL_NVP(ClearBackgroundColours),
            CEREAL_NVP(RPMGauge_BackColour),
            CEREAL_NVP(RPMGauge_NormalRange_Background_Colour),
            CEREAL_NVP(RPMGauge_NormalRange_Background_Colour_GradientEnd),
            CEREAL_NVP(RPMGauge_NormalRange_Foreground_Colour),
            CEREAL_NVP(RPMGauge_NormalRange_Foreground_Colour_GradientEnd),
            CEREAL_NVP(RPMGauge_Redline_Background_Colour),
            CEREAL_NVP(RPMGauge_Redline_Background_Colour_GradientEnd),
            CEREAL_NVP(RPMGauge_Redline_Foreground_Colour),
            CEREAL_NVP(RPMGauge_Redline_Foreground_Colour),
            CEREAL_NVP(RPMGauge_Needle_Foreground_Colour),

            CEREAL_NVP(SpeedDisplay_FillerText_Colour),
            CEREAL_NVP(SpeedDisplay_ValueText_Colour),
            CEREAL_NVP(SpeedDisplay_UnitTypeText_Colour),

            CEREAL_NVP(NOSBar_Background_Colour),
            CEREAL_NVP(NOSBar_Value_Background_Colour),
            CEREAL_NVP(NOSBar_Value_Foreground_Colour)
         );
      }
   };

   struct Colours {
      std::map<std::string, HUDColourPreset> hudColourPresets = {};

      template <class Archive>
      void serialize(Archive& archive) {
         archive(
            cereal::make_nvp("HUDColourPresets", hudColourPresets)
         );
      }
   };
}

// True std::map (de)serialization
namespace cereal {
   template <class Archive, class C, class A,
      traits::EnableIf<traits::is_text_archive<Archive>::value> = traits::sfinae> inline
      void save(Archive& ar, const std::map<std::string, Settings::HUDColourPreset, C, A>& map) {
      for (const auto& item : map)
         ar(cereal::make_nvp(item.first, item.second));
   }

   template <class Archive, class C, class A,
      traits::EnableIf<traits::is_text_archive<Archive>::value> = traits::sfinae> inline
      void load(Archive& ar, std::map<std::string, Settings::HUDColourPreset, C, A>& map) {
      map.clear();

      auto hint = map.begin();
      while (true) {
         const auto namePtr = ar.getNodeName();
         if (!namePtr)
            break;

         Settings::HUDColourPreset value; ar(value);
         hint = map.emplace_hint(hint, std::move(std::string(namePtr)), std::move(value));
      }
   }
}