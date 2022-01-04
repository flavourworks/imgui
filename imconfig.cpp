#include "imconfig.h"
#include "imgui.h"

namespace
{
    ImVec4 colorToBlackAndWhite(const ImVec4& color)
    {
        ImVec4 ret;
        f32 average = (color.x + color.y + color.y) / 3.0f;
        ret.x = ret.y = ret.z = average;
        ret.w = color.w;
        return ret;
    }
}

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
    
    bool SmallCheckbox(const char* label, bool* v)
    {
        PushStyleVar(ImGuiStyleVar_FramePadding, {0,0});
        bool ret = ImGui::Checkbox(label, v);
        PopStyleVar();
        return ret;
    }
    
    bool ComboTrueFalse(const char* label, bool* b)
    {
        int i = *b ? 0 : 1;
        bool ret = Combo(label, &i, "True\0False\0");
        *b = i == 0;
        return ret;
    }

    void PushStyleReadOnly()
    {
        vec4f frameColor = colorToBlackAndWhite(ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, frameColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, frameColor + vec4f(0.2f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, frameColor + vec4f(0.1f));

        vec4f headerColor = colorToBlackAndWhite(ImGui::GetStyleColorVec4(ImGuiCol_Header));
        ImGui::PushStyleColor(ImGuiCol_Header, headerColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, headerColor + vec4f(0.1f));

        vec4f headerActive = colorToBlackAndWhite(ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, headerActive);

        vec4f buttonColor = colorToBlackAndWhite(ImGui::GetStyleColorVec4(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonColor + vec4f(0.2f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor + vec4f(0.1f));
    }

    void PopStyleReadOnly()
    {
        ImGui::PopStyleColor(9);
    }

    void HelpMarker(const char* desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}
