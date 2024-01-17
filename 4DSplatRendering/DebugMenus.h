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
        bool is_Showing = true;
    };

    struct Splat3DMenuData {
        glm::vec3 rot{0.0f};
        glm::vec4 pos{0.0f};
        glm::vec3 lambdas{0.0f};
        glm::vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };
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

    struct MenueStripData {
        bool show_2DSplat = false;
        bool show_3DSplat = false;
        bool show_CamInfo = false;
        bool show_BlendOpt = false; 
        bool show_ShaderEditor = false;
        Camera* cam;
    };

	void Splat2DMenu(Splat2DMenuData* data, bool* is_Showing)
	{
        if (!*is_Showing) return;

        ImGui::Begin("2D Splats", is_Showing);

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

    void CamInfo(ImGuiIO& io, std::string& info, bool* is_Showing)
    {
        if (!*is_Showing) return;

        ImGui::Begin("Cam Info", is_Showing);
        ImGui::Text("Running at: %.2f FPS | %.2f ms/Frame", io.Framerate, 1000.0f / io.Framerate);
        ImGui::Text(info.c_str());
        ImGui::End();
    }

    void TextScreen(const std::string &name, std::string& info)
    {
        ImGui::Begin(name.c_str());
        ImGui::Text(info.c_str());
        ImGui::End();
    }

    void Splat3DMenu(Splat3DMenuData* data, bool* is_Showing)
    {
        if (!*is_Showing) return;

        ImGui::Begin("3D Splats", is_Showing);

        float x0 = data->rot.x, x1 = data->rot.y, x2 = data->rot.z,
            y0 = data->pos.x, y1 = data->pos.y, y2 = data->pos.z;
        ImGui::SliderFloat("x0", &x0, -1.0f, 1.0f);
        ImGui::SliderFloat("x1", &x1, -1.0f, 1.0f);
        ImGui::SliderFloat("x2", &x2, -1.0f, 1.0f);
        ImGui::SliderFloat("l0", &(data->lambdas[0]), 0.0f, 100.0f);
        ImGui::SliderFloat("l1", &(data->lambdas[1]), 0.0f, 100.0f);
        ImGui::SliderFloat("l2", &(data->lambdas[2]), 0.0f, 100.0f);
        ImGui::SliderFloat("pos_0", &y0, -250.0f, 250.0f);
        ImGui::SliderFloat("pos_1", &y1, -250.0f, 250.0f);
        ImGui::SliderFloat("pos_2", &y2, -250.0f, 250.0f);

        data->rot = { x0, x1, x2 };
        data->pos = { y0, y1, y2, 1.0f };

        float c[] = {
            data->color.r,
            data->color.g,
            data->color.b,
            data->color.a
        };
        ImGui::ColorEdit4("Splat Color", c, ImGuiColorEditFlags_AlphaBar);
        data->color = { c[0], c[1], c[2], c[3] };

        if (ImGui::Button("Reset Pos")) 
        {
            data->pos = { 0.0, 0.0, 0.0, 1.0 };
        }

        if (ImGui::Button("Reset Rot"))
        {
            data->rot = {0.0, 0.0, -1.0};
        }

        if (ImGui::Button("Reset Lambdas"))
        {
            data->lambdas = { 1.0, 1.0, 1.0 };
        }

        ImGui::End();
    }

    void BlendOptMenu(BlendOpt* data, bool* is_Showing)
    {
        if (!*is_Showing) return;

        ImGui::Begin("BlendOptMenu", is_Showing);

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

    void ShaderEditor(Menu02Data *data, bool* is_Showing)
    {
        if (!*is_Showing) return;

        ImGui::Begin("Shader Editor", is_Showing);

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

    void MainMenuStrip(MenueStripData* data) 
    {
        ImGui::BeginMainMenuBar();

        if (ImGui::BeginMenu("File"))
        {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::BeginMenu("Splats"))
            {
                if (ImGui::Button("Splat 2D Menu")) data->show_2DSplat = !data->show_2DSplat;
                if (ImGui::Button("Splat 3D Menu")) data->show_3DSplat = !data->show_3DSplat;
                if (ImGui::Button("Splat 4D Menu")) {}
                ImGui::EndMenu();
            }
            if (ImGui::Button("Blend functions")) data->show_BlendOpt = !data->show_BlendOpt;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::Button("Cam info")) data->show_CamInfo = !data->show_CamInfo;
            if (ImGui::Button("Shader editor")) data->show_ShaderEditor = !data->show_ShaderEditor;

            if (data->cam->IsLockX()) 
            {
                if (ImGui::Button("Unlock Cam X")) data->cam->SetLockX(false);
            }
            else 
            {
                if (ImGui::Button("Lock Cam X")) data->cam->SetLockX(true);
            }
                
            if (data->cam->IsLockY())
            {
                if (ImGui::Button("Unlock Cam Y")) data->cam->SetLockY(false);
            }
            else
            {
                if (ImGui::Button("Lock Cam Y")) data->cam->SetLockY(true);
            }

            if (ImGui::Button("Reset Cam Pos"))
            {
                data->cam->SetPosition({ 0.0, 0.0, 5.0 });
            }

            if (ImGui::Button("Reset Cam Orientation"))
            {
                data->cam->SetOrientation({ 0.0f, 0.0f, -1.0f });
                data->cam->SetUp({ 0.0f, 1.0f, 0.0f });
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    

}