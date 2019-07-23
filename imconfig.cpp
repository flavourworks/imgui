#include "imconfig.h"
#include "imgui.h"

namespace ImGui
{
    bool BeginCorner(const char* name, int corner)
    {
	    const float DISTANCE = 40.0f;

        ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background

        return Begin(name, NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
    }

#if FW_TARGET_PS4
    uint32_t ColorConvertOFToU32(const ofColor& c)
    {
        return ColorConvertFloat4ToU32(ImVec4( c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f ));
    }
#endif
}
