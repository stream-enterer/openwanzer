// Simplified rlImGui implementation compatible with ImGui 1.90.1 and Raylib
#include "raylib.h"
#include "rlgl.h"
#include "imgui.h"
#include <cstring>
#include <cstdint>

static double g_Time = 0.0;
static Texture2D g_FontTexture = { 0 };

extern "C" {

void rlImGuiSetup(bool darkTheme)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Setup keyboard mapping
    io.KeyMap[ImGuiKey_Tab] = KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = KEY_HOME;
    io.KeyMap[ImGuiKey_End] = KEY_END;
    io.KeyMap[ImGuiKey_Insert] = KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = KEY_A;
    io.KeyMap[ImGuiKey_C] = KEY_C;
    io.KeyMap[ImGuiKey_V] = KEY_V;
    io.KeyMap[ImGuiKey_X] = KEY_X;
    io.KeyMap[ImGuiKey_Y] = KEY_Y;
    io.KeyMap[ImGuiKey_Z] = KEY_Z;

    // Load font texture
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    Image image = {
        .data = pixels,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    g_FontTexture = LoadTextureFromImage(image);
    io.Fonts->TexID = (ImTextureID)(intptr_t)g_FontTexture.id;

    // Setup style
    if (darkTheme)
        ImGui::StyleColorsDark();
    else
        ImGui::StyleColorsLight();
}

void rlImGuiShutdown()
{
    if (g_FontTexture.id != 0)
    {
        UnloadTexture(g_FontTexture);
        g_FontTexture.id = 0;
    }
    ImGui::DestroyContext();
}

static void UpdateMouseInput()
{
    ImGuiIO& io = ImGui::GetIO();

    Vector2 mousePos = GetMousePosition();
    io.MousePos = ImVec2(mousePos.x, mousePos.y);

    io.MouseDown[0] = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    io.MouseDown[1] = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
    io.MouseDown[2] = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON);

    io.MouseWheel = GetMouseWheelMove();
}

static void UpdateKeyboardInput()
{
    ImGuiIO& io = ImGui::GetIO();

    io.KeyCtrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    io.KeyShift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    io.KeyAlt = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
    io.KeySuper = IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);

    // Get all keys
    for (int i = 0; i < 512; i++)
        io.KeysDown[i] = IsKeyDown(i);

    // Get text input
    int key = GetCharPressed();
    while (key > 0)
    {
        io.AddInputCharacter((unsigned int)key);
        key = GetCharPressed();
    }
}

void rlImGuiBegin()
{
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)GetScreenWidth(), (float)GetScreenHeight());

    double currentTime = GetTime();
    io.DeltaTime = g_Time > 0.0 ? (float)(currentTime - g_Time) : (float)(1.0f/60.0f);
    g_Time = currentTime;

    UpdateMouseInput();
    UpdateKeyboardInput();

    ImGui::NewFrame();
}

static void RenderDrawLists(ImDrawData* draw_data)
{
    rlDrawRenderBatchActive();
    rlDisableBackfaceCulling();

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                ImVec2 pos = draw_data->DisplayPos;
                int rectX = (int)(pcmd->ClipRect.x - pos.x);
                int rectY = (int)(pcmd->ClipRect.y - pos.y);
                int rectW = (int)(pcmd->ClipRect.z - rectX);
                int rectH = (int)(pcmd->ClipRect.w - rectY);

                BeginScissorMode(rectX, rectY, rectW, rectH);

                unsigned int texId = (unsigned int)(intptr_t)pcmd->TextureId;

                rlBegin(RL_TRIANGLES);
                rlSetTexture(texId);

                for (unsigned int i = 0; i <= (pcmd->ElemCount - 3); i += 3)
                {
                    ImDrawIdx index;
                    ImDrawVert* vertex;

                    index = idx_buffer[i];
                    vertex = (ImDrawVert*)&vtx_buffer[index];
                    rlColor4ub(vertex->col & 0xFF, (vertex->col >> 8) & 0xFF, (vertex->col >> 16) & 0xFF, (vertex->col >> 24) & 0xFF);
                    rlTexCoord2f(vertex->uv.x, vertex->uv.y);
                    rlVertex2f(vertex->pos.x, vertex->pos.y);

                    index = idx_buffer[i + 1];
                    vertex = (ImDrawVert*)&vtx_buffer[index];
                    rlColor4ub(vertex->col & 0xFF, (vertex->col >> 8) & 0xFF, (vertex->col >> 16) & 0xFF, (vertex->col >> 24) & 0xFF);
                    rlTexCoord2f(vertex->uv.x, vertex->uv.y);
                    rlVertex2f(vertex->pos.x, vertex->pos.y);

                    index = idx_buffer[i + 2];
                    vertex = (ImDrawVert*)&vtx_buffer[index];
                    rlColor4ub(vertex->col & 0xFF, (vertex->col >> 8) & 0xFF, (vertex->col >> 16) & 0xFF, (vertex->col >> 24) & 0xFF);
                    rlTexCoord2f(vertex->uv.x, vertex->uv.y);
                    rlVertex2f(vertex->pos.x, vertex->pos.y);
                }

                rlEnd();
                EndScissorMode();
            }

            idx_buffer += pcmd->ElemCount;
        }
    }

    rlSetTexture(0);
    rlEnableBackfaceCulling();
}

void rlImGuiEnd()
{
    ImGui::Render();
    RenderDrawLists(ImGui::GetDrawData());
}

} // extern "C"
