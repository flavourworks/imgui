#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#if FW_TARGET_PS4

#include "imgui.h"
#include "imgui_impl_of.h"
#include "ofMain.h"

#ifdef PS4BUILD
#include "commonutil.h"
#endif

namespace DevUI
{
    //gl stuff
    static GLuint       g_FontTexture = 0;
    static int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
    static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
    static int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;

    static const int k_numBuffers = 10;
    static unsigned int g_VboHandles[k_numBuffers] = { 0 };
    static unsigned int g_ElementsHandles[k_numBuffers] = { 0 };
   
    //genera globals
    static double       g_Time = 0.0f;
    static bool         g_initialised = false;
    static bool         g_enabled = false;

    bool createFontsTexture()
    {
        // Build texture atlas
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        GLint last_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGenTextures(1, &g_FontTexture);
        glBindTexture(GL_TEXTURE_2D, g_FontTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        // Store our identifier
        io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

        // Restore state
        glBindTexture(GL_TEXTURE_2D, last_texture);

        return true;
    }

    bool createDeviceObjects()
    {
#ifndef FW_TARGET_ANDROID // FWTECH_TODO android cant support vao
        // Backup GL state
        GLint last_texture, last_array_buffer, last_vertex_array;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

        // Store GL version string so we can refer to it later in case we recreate shaders.
        //g_ShaderHandle = CreateProgramFromFile("data/shaders/imgui.vert.sb", "data/shaders/imgui.frag.sb", true);
        
        static ofShader imguiShader;
        imguiShader.load("data/shaders/imgui");
        
        g_ShaderHandle = imguiShader.getProgram();

        g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
        g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
        g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
        g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
        g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

        glGenBuffers(k_numBuffers, g_VboHandles);
        glGenBuffers(k_numBuffers, g_ElementsHandles);

        createFontsTexture();

        // Restore modified GL state
        glBindTexture(GL_TEXTURE_2D, last_texture);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        glBindVertexArray(last_vertex_array);
#endif
        return true;
    }

    void destroyDeviceObjects()
    {
        if (g_VboHandles[0]) 
            glDeleteBuffers(k_numBuffers, g_VboHandles);

        if (g_ElementsHandles[0]) 
            glDeleteBuffers(k_numBuffers, g_ElementsHandles);

        if (g_ShaderHandle && g_VertHandle) glDetachShader(g_ShaderHandle, g_VertHandle);
        if (g_VertHandle) glDeleteShader(g_VertHandle);
        g_VertHandle = 0;

        if (g_ShaderHandle && g_FragHandle) glDetachShader(g_ShaderHandle, g_FragHandle);
        if (g_FragHandle) glDeleteShader(g_FragHandle);
        g_FragHandle = 0;

        if (g_ShaderHandle) glDeleteProgram(g_ShaderHandle);
        g_ShaderHandle = 0;

        if (g_FontTexture)
        {
            glDeleteTextures(1, &g_FontTexture);
            ImGui::GetIO().Fonts->TexID = 0;
            g_FontTexture = 0;
        }
    }

    void init()
    {
        ImGui::CreateContext();
        g_initialised = true;
    }

    void shutdown()
    {
        // Destroy OpenGL objects
        destroyDeviceObjects();
    }

    static GamePadState g_GamePadState;

    const char* gamepadButtonNames[]
    {
        "None",
        "L3",
        "R3",
        "Options",
        "Up",
        "Right",
        "Down",
        "Left",
        "L2",
        "R2",
        "L1",
        "R1",
        "Triangle",
        "Circle",
        "Cross",
        "Square",
        "Touchpad",
        "Intercepted"
    };


    void newFrame()
    {
        if (!g_FontTexture)
            createDeviceObjects();

        ImGuiIO& io = ImGui::GetIO();

        // Setup display size (every frame to accommodate for window resizing)
        int w, h;
        int display_w, display_h;

        w = 1920; h = 1080;
        display_w = 1920; display_h = 1080;

        io.DisplaySize = ImVec2((float)w, (float)h);
        io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

        // Setup time step
        double current_time = ofGetSystemTime();
        io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
        g_Time = current_time;

        if (g_GamePadState.m_buttons && DevUI::focused())
        {
#define MAP_BUTTON(NAV_NO, BUTTON_NO) if( g_GamePadState.m_buttons[BUTTON_NO] ) { io.NavInputs[NAV_NO] = 1.0f; };
#define MAP_ANALOG(NAV_NO, AXIS_NO) io.NavInputs[NAV_NO] = g_GamePadState.m_axis[AXIS_NO];
            MAP_BUTTON(ImGuiNavInput_Activate, GP_BUTTON_CROSS);
            MAP_BUTTON(ImGuiNavInput_Cancel, GP_BUTTON_CIRCLE);
            MAP_BUTTON(ImGuiNavInput_Menu, GP_BUTTON_SQUARE);
            MAP_BUTTON(ImGuiNavInput_Input, GP_BUTTON_TRIANGLE);
            MAP_BUTTON(ImGuiNavInput_DpadLeft, GP_BUTTON_LEFT);
            MAP_BUTTON(ImGuiNavInput_DpadRight, GP_BUTTON_RIGHT);
            MAP_BUTTON(ImGuiNavInput_DpadUp, GP_BUTTON_UP);
            MAP_BUTTON(ImGuiNavInput_DpadDown, GP_BUTTON_DOWN);
            //MAP_BUTTON(ImGuiNavInput_FocusPrev, GP_BUTTON_L1); // @note tom: Don't do this as we are using L1 to toggle focused state?
            MAP_BUTTON(ImGuiNavInput_FocusNext, GP_BUTTON_R1);
            MAP_BUTTON(ImGuiNavInput_TweakSlow, GP_BUTTON_L2);         // L1 / LB
            MAP_BUTTON(ImGuiNavInput_TweakFast, GP_BUTTON_R2);         // R1 / RB

            // @note:
            //      This behaviour doesn't appear to match that described in the imgui comments
            //      perhaps the gamepad navigation is still buggy? 
            //
            //      The comments seemed to sugest we should also be using ImGuiNavInput_LStickDown and ImGuiNavInput_LStickRight
            //              ~ tom @ imgui v1.61

            io.NavInputs[ImGuiNavInput_LStickLeft] = -g_GamePadState.m_axis[0];
            io.NavInputs[ImGuiNavInput_LStickUp] = -g_GamePadState.m_axis[1];

            io.NavInputs[ImGuiNavInput_LStickLeft] /= (float)w;
            io.NavInputs[ImGuiNavInput_LStickRight] /= (float)w;
            io.NavInputs[ImGuiNavInput_LStickUp] /= (float)h;
            io.NavInputs[ImGuiNavInput_LStickRight] /= (float)h;

            io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        }

        io.MouseDrawCursor = true;

        // Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
        ImGui::NewFrame();
    }

    void setGamePadState(const GamePadState& state)
    {
        if (!g_initialised || !g_enabled)
            return;

        g_GamePadState = state;
    }

    void renderDrawData(ImDrawData* draw_data)
    {
#ifndef FW_TARGET_ANDROID // FWTECH_TODO, android cant use vao
        // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
        ImGuiIO& io = ImGui::GetIO();
        int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
        int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
        if (fb_width == 0 || fb_height == 0)
            return;
        draw_data->ScaleClipRects(io.DisplayFramebufferScale);

        // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_SCISSOR_TEST);

        // Setup viewport, orthographic projection matrix
        glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
        const float ortho_projection[4][4] =
        {
            { 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
            { 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
            { 0.0f,                  0.0f,                  -1.0f, 0.0f },
            { -1.0f,                  1.0f,                   0.0f, 1.0f },
        };
        glUseProgram(g_ShaderHandle);
        glUniform1i(g_AttribLocationTex, 0);
        glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
        glBindSampler(0, 0); // Rely on combined texture/sampler state.

        // Recreate the VAO every time 
        // (This is to easily allow multiple GL contexts. VAO are not shared among GL contexts, and we don't track creation/deletion of windows so we don't have an obvious key to use to cache them.)

        // Draw
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            GLuint vao_handle = 0;
            glGenVertexArrays(1, &vao_handle);
            glBindVertexArray(vao_handle);
            glBindBuffer(GL_ARRAY_BUFFER, g_VboHandles[n]);
            glEnableVertexAttribArray(g_AttribLocationPosition);
            glEnableVertexAttribArray(g_AttribLocationUV);
            glEnableVertexAttribArray(g_AttribLocationColor);
            glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
            glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
            glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            const ImDrawIdx* idx_buffer_offset = 0;

            glBindBuffer(GL_ARRAY_BUFFER, g_VboHandles[n]);
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandles[n]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback)
                {
                    pcmd->UserCallback(cmd_list, pcmd);
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                    glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                    glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
                }
                idx_buffer_offset += pcmd->ElemCount;
            }

            glDeleteVertexArrays(1, &vao_handle);
        }

        // Restore modified GL state
        glUseProgram(0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindSampler(0, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);

        glViewport(0, 0, fb_width, fb_height);
        glScissor(0, 0, fb_width, fb_height);
#endif
    }

    void render()
    {
        //if (g_enabled)
        {
            ImGui::Render();
            renderDrawData(ImGui::GetDrawData());

        }
        
        ImGui::EndFrame();
    }

    void toggleEnabled()
    {
        g_enabled = !g_enabled;
    }

    bool isEnabled()
    {
        return g_enabled;
    }

    bool s_focused = true;

    void setFocus(bool val)
    {
        s_focused = val;
    }

    bool focused()
    {
        return s_focused;
    }

    void StyleColorsFocused()
    {
        ImGui::StyleColorsDark();
    }

    void StyleColorsUnfocused()
    {
        ImGui::StyleColorsDark();

        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;

        for(int i = 0; i < ImGuiCol_COUNT; ++i)
        {
            float grey = colors[i].x + colors[i].y + colors[i].z;
            grey *= 0.3f;

            colors[i] = ImVec4(grey, grey, grey, colors[i].w);
        }

        colors[ImGuiCol_WindowBg].w = 0.6f;
    }

    void dbg()
    {
        if (!g_GamePadState.m_buttons)
            return;

        bool opened = true;
        ImGui::Begin("Gamepad", &opened, ImGuiWindowFlags_AlwaysAutoResize);

        for (int i = 0; i < g_GamePadState.m_buttons_count; ++i)
        {
            bool val = g_GamePadState.m_buttons[i];
            ImGui::Checkbox(gamepadButtonNames[i], &val);
        }

        ImGui::InputFloat2("Left Stick", &g_GamePadState.m_axis[0]);
        ImGui::InputFloat2("Right Stick", &g_GamePadState.m_axis[2]);

        ImGui::End();

        bool otherOpened = true;
        ImGui::Begin("Another Window", &otherOpened, ImGuiWindowFlags_AlwaysAutoResize);

        static int  ii = 0;
        ImGui::InputInt("Test", &ii);

        static int  i2 = 0;
        ImGui::InputInt("Test 2", &i2);

        ImGui::End();
    }

    void demo()
    {
        static bool show_demo_window = true;
        static bool show_another_window = true;
        static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


        // 1. Show a simple window.
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
        {
            static float f = 0.0f;
            static int counter = 0;
            ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        // 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
        if (show_demo_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&show_demo_window);
        }
    }
}

#endif
