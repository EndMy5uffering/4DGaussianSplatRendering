#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace DebugMenu 
{

    struct Menu01Data {
        float v0;
        float v1;
        float l0;
        float l1;
        float color[3] = {0.0f, 0.0f, 0.0f};
    };

	void Menu_01(ImGuiIO& io, Menu01Data* data)
	{

        ImGui::Begin("4D Splats :D");
        ImGui::Text("Running at: %.2f FPS | %.2f ms/Frame", io.Framerate, 1000.0f / io.Framerate);

        ImGui::SliderFloat("V0", &(data->v0), -10.0f, 10.0f);
        ImGui::SliderFloat("V1", &(data->v1), -10.0f, 10.0f);
        ImGui::SliderFloat("l0", &(data->l0), 0.0f, 10.0f);
        ImGui::SliderFloat("l1", &(data->l1), 0.0f, 10.0f);

        ImGui::ColorPicker3("Splat Color", (data->color));
        ImGui::End();
	}

    void Menu_02(char* buf, size_t bufSize)
    {
        ImGui::Begin("Win 2");

        ImGui::InputTextMultiline("Shader", buf, bufSize);

        ImGui::End();
    }

}