#pragma once

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <string>
#include <map>

template<class T>
class Menu {
public:
    Menu(const Menu<T>&) = delete;

    Menu(T& MenuData);
    ~Menu();

    inline T& GetMenuData();

    bool IsShowing();
    void SetIsShowing();

private:
    bool isShowing = false;
    T Data;
};

namespace DebugMenus
{

    struct Splat2DMenuData {
        float angle = 0.0f;
        float l0 = 0.0f;
        float l1 = 0.0f;
        glm::vec3 splatPos{0.0,0.0,0.0};
        float color[3] = {0.0f, 0.0f, 0.0f};
    };

    struct Splat3DMenuData {
        glm::vec3 v0{0.0f};
        glm::vec3 v1{0.0f};
        float l0 = 0.0f;
        float l1 = 0.0f;
        float l2 = 0.0f;
        glm::vec3 splatPos{0.0, 0.0, 0.0};
        float color[3] = { 0.0f, 0.0f, 0.0f };
    };

    struct Menu02Data {
        std::string buffer{};
        std::vector<Shader*> shaders;
        int selectedShaderIdx = -1;
        int selectedSourceIdx = -1;
    };

    struct BlendOpt {
        size_t selected0 = GL_SRC_ALPHA;
        size_t selected1 = GL_ONE_MINUS_SRC_ALPHA;
        std::map < size_t, std::string> SelectOpt{
            { GL_ZERO, "GL_ZERO" }, 
            { GL_ONE, "GL_ONE" },
            { GL_SRC_COLOR, "GL_SRC_COLOR" },
            { GL_ONE_MINUS_SRC_COLOR, "GL_ONE_MINUS_SRC_COLOR" },
            { GL_DST_COLOR, "GL_DST_COLOR" },
            { GL_ONE_MINUS_DST_COLOR, "GL_ONE_MINUS_DST_COLOR" },
            { GL_SRC_ALPHA, "GL_SRC_ALPHA" },
            { GL_ONE_MINUS_SRC_ALPHA, "GL_ONE_MINUS_SRC_ALPHA" },
            { GL_DST_ALPHA, "GL_DST_ALPHA" },
            { GL_ONE_MINUS_DST_ALPHA, "GL_ONE_MINUS_DST_ALPHA" },
            { GL_CONSTANT_COLOR, "GL_CONSTANT_COLOR" },
            { GL_ONE_MINUS_CONSTANT_COLOR, "GL_ONE_MINUS_CONSTANT_COLOR" },
            { GL_CONSTANT_ALPHA, "GL_CONSTANT_ALPHA" },
            { GL_ONE_MINUS_CONSTANT_ALPHA, "GL_ONE_MINUS_CONSTANT_ALPHA" },
        };
    };

    struct CamInfoData {
        std::string camInfo = "";
    };

	void Splat2DMenu(Splat2DMenuData* data)
	{

        ImGui::Begin("2D Splats");

        ImGui::SliderFloat("Angle", &(data->angle), -180.0f, 180.0f);
        ImGui::SliderFloat("l0", &(data->l0), 0.0f, 10.0f);
        ImGui::SliderFloat("l1", &(data->l1), 0.0f, 10.0f);

        float x = data->splatPos.x, y = data->splatPos.y, z = data->splatPos.z;
        ImGui::SliderFloat("Pos_X", &x, -10.0f, 10.0f);
        ImGui::SliderFloat("Pos_Y", &y, -10.0f, 10.0f);
        ImGui::SliderFloat("Pos_Z", &z, -10.0f, 10.0f);

        data->splatPos = glm::vec3{x,y,z};

        ImGui::ColorPicker3("Splat Color", (data->color));


        ImGui::End();
	}

    void CamInfo(ImGuiIO& io, std::string& info)
    {
        ImGui::Begin("Cam Info");
        ImGui::Text("Running at: %.2f FPS | %.2f ms/Frame", io.Framerate, 1000.0f / io.Framerate);
        ImGui::Text(info.c_str());
        ImGui::End();
    }

    void Splat3DMenu(Splat3DMenuData* data) 
    {
        ImGui::Begin("3D Splats");

        float x0 = data->v0.x, x1 = data->v0.y, x2 = data->v0.z,
            y0 = data->v1.x, y1 = data->v1.y, y2 = data->v1.z;
        ImGui::SliderFloat("x0", &x0, -180.0f, 180.0f);
        ImGui::SliderFloat("x1", &x1, -180.0f, 180.0f);
        ImGui::SliderFloat("x2", &x2, -180.0f, 180.0f);
        ImGui::SliderFloat("y0", &y0, -180.0f, 180.0f);
        ImGui::SliderFloat("y1", &y1, -180.0f, 180.0f);
        ImGui::SliderFloat("y2", &y2, -180.0f, 180.0f);
        ImGui::SliderFloat("l0", &(data->l0), 0.0f, 10.0f);
        ImGui::SliderFloat("l1", &(data->l1), 0.0f, 10.0f);
        ImGui::SliderFloat("l2", &(data->l2), 0.0f, 10.0f);

        float x = data->splatPos.x, y = data->splatPos.y, z = data->splatPos.z;
        ImGui::SliderFloat("Pos_X", &x, -10.0f, 10.0f);
        ImGui::SliderFloat("Pos_Y", &y, -10.0f, 10.0f);
        ImGui::SliderFloat("Pos_Z", &z, -10.0f, 10.0f);

        data->splatPos = glm::vec3{ x,y,z };

        ImGui::ColorPicker3("Splat Color", (data->color));


        ImGui::End();
    }

    void BlendOptMenu(BlendOpt* data)
    {
        ImGui::Begin("BlendOptMenu");

        std::string prev0 = data->selected0 < 0 ? "<default>" : data->SelectOpt[data->selected0];
        if (ImGui::BeginCombo("sfactor", prev0.c_str()))
        {
            for (const auto &elem : data->SelectOpt)
            {
                const bool is_selected = (data->selected0 == elem.first);
                if (ImGui::Selectable(elem.second.c_str(), is_selected))
                {
                    data->selected0 = elem.first;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        std::string prev1 = data->selected1 < 0 ? "<default>" : data->SelectOpt[data->selected1];
        if (ImGui::BeginCombo("dfactor", prev1.c_str()))
        {
            for (const auto& elem : data->SelectOpt)
            {
                const bool is_selected = (data->selected1 == elem.first);
                if (ImGui::Selectable(elem.second.c_str(), is_selected))
                {
                    data->selected1 = elem.first;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::End();
    }

    void ShaderEditor(Menu02Data *data)
    {
        ImGui::Begin("Shader Editor");

        std::string prev = data->selectedShaderIdx != -1 && data->selectedSourceIdx != -1 ?
            data->shaders[data->selectedShaderIdx]->GetShourceByIdx(data->selectedSourceIdx)->path
            : "<Select>";
        if(ImGui::BeginCombo("Shader Selection", prev.c_str()))
        {
            for (size_t i = 0; i < data->shaders.size(); ++i) 
            {
                for (size_t j = 0; j < data->shaders[i]->GetShaderSourceSize(); ++j)
                {
                    const bool is_selected = (data->selectedShaderIdx == i && data->selectedSourceIdx == j);
                    if (ImGui::Selectable(data->shaders[i]->GetShourceByIdx(j)->path.c_str(), is_selected))
                    {
                        data->selectedShaderIdx = i;
                        data->selectedSourceIdx = j;

                        data->buffer = data->shaders[i]->GetShourceByIdx(j)->source;

                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
            }


            ImGui::EndCombo();
        }


        ImGui::InputTextMultiline("Shader", &(data->buffer), ImVec2(850,550));
        if (ImGui::Button("Try Compile")) 
        {
            if (Shader::TryCompile(data->buffer, data->shaders[data->selectedShaderIdx]->GetShourceByIdx(data->selectedSourceIdx)->type)) 
            {
                std::cout << "Shader compiled successfully\n";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Compile Shader")) 
        {
            if (data->selectedShaderIdx > -1 && data->selectedSourceIdx > -1) 
            {
                data->shaders[data->selectedShaderIdx]->Unbind();
                data->shaders[data->selectedShaderIdx]->GetShourceByIdx(data->selectedSourceIdx)->source = data->buffer;
                data->shaders[data->selectedShaderIdx]->RebuildShader();
            }
            else 
            {
                std::cout << "[WARNING]: No shader selected\n";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload Shader"))
            if (data->selectedShaderIdx > -1) 
            {
                data->shaders[data->selectedShaderIdx]->Unbind();
                data->shaders[data->selectedShaderIdx]->RebuildShader();
            }
            else
                std::cout << "[WARNING]: No shader selected\n";
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
            if (data->selectedShaderIdx > -1 && data->selectedSourceIdx > -1)
                data->buffer = data->shaders[data->selectedShaderIdx]->GetShourceByIdx(data->selectedSourceIdx)->source;
            else
                std::cout << "[WARNING]: No shader selected\n";
        ImGui::End();
    }

    void MainMenuStrip() 
    {
        ImGui::BeginMainMenuBar();

        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("New");
            ImGui::MenuItem("Create");
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

}