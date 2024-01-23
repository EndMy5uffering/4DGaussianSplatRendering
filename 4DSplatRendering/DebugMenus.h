#pragma once

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <string>
#include <map>
#include <cstdlib>
#include <ctime>


#define ArrayFromV4(v) { v.x, v.y, v.z, v.w }
#define V4FromArray(a) { a[0], a[1], a[2], a[3] }
#define ArrayFromV3(v) { v.x, v.y, v.z }
#define V3FromArray(a) { a[0], a[1], a[2] }
#define RNG(lower, upper) lower + ((upper - lower) * (std::rand()/RAND_MAX))

constexpr float RAD_RANGE = 2.0f * 3.1415926535f;

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
        bool show_4DSplat = false;
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

    void Splat3DMenu(Splat3D* splat, bool* is_Showing)
    {
        if (!*is_Showing) return;
        
        ImGui::Begin("3D Splats", is_Showing);
        //glm::quatLookAt(glm::normalize(splat3DMenuData.rot), glm::vec3{0.0f, 1.0f, 0.0f})
        float rot[] = ArrayFromV3(glm::eulerAngles(splat->GetQuaternion()));
        float scale[] = ArrayFromV3(splat->GetScale());
        float pos[] = ArrayFromV4(splat->GetPosition());
        ImGui::SliderFloat("x0", &rot[0], 0.0f, RAD_RANGE);
        ImGui::SliderFloat("x1", &rot[1], 0.0f, RAD_RANGE);
        ImGui::SliderFloat("x2", &rot[2], 0.0f, RAD_RANGE);
        ImGui::SliderFloat("l0", &scale[0], 0.0f, 100.0f);
        ImGui::SliderFloat("l1", &scale[1], 0.0f, 100.0f);
        ImGui::SliderFloat("l2", &scale[2], 0.0f, 100.0f);
        ImGui::SliderFloat("pos_0", &pos[0], -250.0f, 250.0f);
        ImGui::SliderFloat("pos_1", &pos[1], -250.0f, 250.0f);
        ImGui::SliderFloat("pos_2", &pos[2], -250.0f, 250.0f);

        splat->SetQuaternion(glm::quat(glm::highp_vec3(rot[0], rot[1], rot[2])));
        splat->SetLambas(V3FromArray(scale));
        splat->SetPosition(V4FromArray(pos));

        float c[] = ArrayFromV4(splat->GetColor());
        ImGui::ColorEdit4("Splat Color", c, ImGuiColorEditFlags_AlphaBar);
        splat->SetColor(V4FromArray(c));

        if (ImGui::Button("Reset Pos")) 
        {
            splat->SetPosition({ 0.0, 0.0, 0.0, 1.0 });
        }

        if (ImGui::Button("Reset Rot"))
        {
            splat->SetQuaternion(glm::quatLookAt(glm::vec3{ 0.0, 0.0, -1.0 }, glm::vec3{0.0f, 1.0f, 0.0f}));
        }

        if (ImGui::Button("Reset Lambdas"))
        {
            splat->SetLambas({ 1.0, 1.0, 1.0 });
        }

        ImGui::End();
    }

    void Splat4DMenu(Splat4D* splat, Camera* cam, bool* is_Showing)
    {
        if (!*is_Showing) return;

        ImGui::Begin("4D Splats", is_Showing);

        float time = splat->GetTime();
        ImGui::SliderFloat("Time:", &time, -1000.0f, 1000.0f);
        splat->SetTime(time);

        ImGui::NewLine();

        float pos[] = ArrayFromV4(splat->GetPosititon());
        ImGui::SliderFloat("Pos_x", &pos[0], -100.0f, 100.0f);
        ImGui::SliderFloat("Pos_y", &pos[1], -100.0f, 100.0f);
        ImGui::SliderFloat("Pos_z", &pos[2], -100.0f, 100.0f);
        ImGui::SliderFloat("Pos_w", &pos[3], -10.0f, 10.0f);
        splat->SetPosition(V4FromArray(pos));

        ImGui::NewLine();

        float scale[] = ArrayFromV4(splat->GetScale());
        ImGui::SliderFloat("Scale_x", &scale[0], 0.0f, 25.0f);
        ImGui::SliderFloat("Scale_y", &scale[1], 0.0f, 25.0f);
        ImGui::SliderFloat("Scale_z", &scale[2], 0.0f, 25.0f);
        ImGui::SliderFloat("Scale_w", &scale[3], 0.0f, 25.0f);
        splat->SetScale(V4FromArray(scale));

        ImGui::NewLine();

        if (ImGui::Button("Rand Q0"))
        {
            std::srand(std::time(nullptr));
            splat->SetQuat0(glm::quatLookAt(glm::vec3{RNG(-1.0f, 1.0f), RNG(-1.0f, 1.0f), RNG(-1.0f, 1.0f)}, glm::vec3{0.0, 1.0f, 0.0f}));
        }

        float quat0[] = ArrayFromV4(splat->GetQuat0());
        ImGui::SliderFloat("Q0_x", &quat0[0], -RAD_RANGE, RAD_RANGE);
        ImGui::SliderFloat("Q0_y", &quat0[1], -RAD_RANGE, RAD_RANGE);
        ImGui::SliderFloat("Q0_z", &quat0[2], -RAD_RANGE, RAD_RANGE);
        ImGui::SliderFloat("Q0_w", &quat0[3], -RAD_RANGE, RAD_RANGE);
        splat->SetQuat0(glm::quat{quat0[3], quat0[0], quat0[1], quat0[2]});


        ImGui::NewLine();

        if (ImGui::Button("Rand Q1"))
        {
            std::srand(std::time(nullptr));
            splat->SetQuat1(glm::quatLookAt(glm::vec3{RNG(-1.0f, 1.0f), RNG(-1.0f, 1.0f), RNG(-1.0f, 1.0f)}, glm::vec3{0.0, 1.0f, 0.0f}));
        }

        float quat1[] = ArrayFromV4(splat->GetQuat1());
        ImGui::SliderFloat("Q1_x", &quat1[0], 0.0f, RAD_RANGE);
        ImGui::SliderFloat("Q1_y", &quat1[1], 0.0f, RAD_RANGE);
        ImGui::SliderFloat("Q1_z", &quat1[2], 0.0f, RAD_RANGE);
        ImGui::SliderFloat("Q1_w", &quat1[3], 0.0f, RAD_RANGE);
        splat->SetQuat1(glm::quat{quat1[3], quat1[0], quat1[1], quat1[2]});

        ImGui::NewLine();

        float c[] = ArrayFromV4(splat->GetColor());
        ImGui::ColorEdit4("Splat Color", c, ImGuiColorEditFlags_AlphaBar);
        splat->SetColor(V4FromArray(c));

        ImGui::NewLine();

        bool isCamOnSplat = cam->IsViewFixed();
        bool isPosFixed = cam->IsPositionFixed();
            
        /*ImGui::Checkbox("Keep cam on time splat", &isCamOnSplat);
        cam->SetIsViewFixedOnPoint(isCamOnSplat, glm::vec4(splat->GetTimePos(), 1.0f));

        ImGui::Checkbox("Keep distence to point", &isPosFixed);
        cam->SetIsPositionFixed(isPosFixed);
        if (isPosFixed)
        {
            float dist = glm::length(cam->position - splat->GetTimePos());
            ImGui::SliderFloat("Dist:", &dist, 0.0f, 100.0f);
            glm::vec3 dirSplatToCam = glm::normalize(cam->position - splat->GetTimePos());
            cam->SetPosition(splat->GetTimePos() + (dirSplatToCam * dist));
        }*/

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
                if (ImGui::Button("Splat 4D Menu")) data->show_4DSplat = !data->show_4DSplat;
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