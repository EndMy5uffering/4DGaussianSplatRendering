#pragma once

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <string>
#include <map>
#include <cstdlib>
#include <ctime>
#include <sstream>


#define ArrayFromV4(v) { v.x, v.y, v.z, v.w }
#define V4FromArray(a) { a[0], a[1], a[2], a[3] }
#define ArrayFromV3(v) { v.x, v.y, v.z }
#define V3FromArray(a) { a[0], a[1], a[2] }
#define RNG(lower, upper) lower + ((upper - lower) * (std::rand()/RAND_MAX))

#define SwapScene(SType) { if (data->u_scene_ptr) \
                {\
                    data->u_scene_ptr->unload();\
                    data->u_scene_ptr.reset();\
                }\
                data->u_scene_ptr = std::make_unique<SType>(renderer, cam);\
                data->u_scene_ptr->init(); }

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
        std::unique_ptr<Scene> u_scene_ptr;
    };

	void Splat2DMenu(Splat2D* data, bool* is_Showing)
	{
        if (!*is_Showing) return;

        ImGui::Begin("2D Splats", is_Showing);

        glm::vec2 v0 = data->GetVector();
        ImGui::SliderFloat("v_x", &v0.x, -1.0f, 1.0f);
        ImGui::SliderFloat("v_y", &v0.y, -1.0f, 1.0f);
        data->SetVectors(v0);
        float l0 = data->GetLambda0();
        float l1 = data->GetLambda1();
        ImGui::SliderFloat("l0", &l0, 0.0f, 10.0f);
        ImGui::SliderFloat("l1", &l1, 0.0f, 10.0f);

        float p[] = ArrayFromV3(data->GetPosition());
        ImGui::SliderFloat("Pos_X", &p[0], -10.0f, 10.0f);
        ImGui::SliderFloat("Pos_Y", &p[0], -10.0f, 10.0f);
        ImGui::SliderFloat("Pos_Z", &p[0], -10.0f, 10.0f);

        data->SetPosition(V3FromArray(p));

        float c[] = ArrayFromV4(data->GetColor());
        ImGui::ColorPicker3("Splat Color", c);
        data->SetColor(V4FromArray(c));


        ImGui::End();
	}

    void CamInfo(ImGuiIO& io, Camera& cam, bool* is_Showing)
    {
        if (!*is_Showing) return;
        
        glm::vec3 ps = cam.GetPosition();
        glm::vec3 dir = cam.orientation;

        ImGui::Begin("Cam Info", is_Showing);
        ImGui::Text("Running at: %.2f FPS | %.2f ms/Frame", io.Framerate, 1000.0f / io.Framerate);
        ImGui::Text("POS: { %.2F, %.2F, %.2F }", ps.x, ps.y, ps.z);
        ImGui::Text("DIR: { %.2F, %.2F, %.2F }", dir.x, dir.y, dir.z);
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
        if (ImGui::Button("Reset Time")) 
        {
            time = 0.0f;
        }
        ImGui::SliderFloat("Time:", &time, -25.0f, 25.0f);
        splat->SetTime(time);

        ImGui::NewLine();

        float pos[] = ArrayFromV4(splat->GetPosititon());
        if (ImGui::Button("Reset Position")) 
        {
            pos[0] = 0.0f;
            pos[1] = 0.0f;
            pos[2] = 0.0f;
            pos[3] = 0.0f;
        }
        ImGui::SliderFloat("Pos_x", &pos[0], -100.0f, 100.0f);
        ImGui::SliderFloat("Pos_y", &pos[1], -100.0f, 100.0f);
        ImGui::SliderFloat("Pos_z", &pos[2], -100.0f, 100.0f);
        ImGui::SliderFloat("Pos_w", &pos[3], -10.0f, 10.0f);
        splat->SetPosition(V4FromArray(pos));

        ImGui::NewLine();

        float c[] = ArrayFromV4(splat->GetColor());
        ImGui::ColorEdit4("Splat Color", c, ImGuiColorEditFlags_AlphaBar);
        splat->SetColor(V4FromArray(c));

        ImGui::NewLine();

        bool isCamOnSplat = cam->IsViewFixed();
        bool isPosFixed = cam->IsPositionFixed();
            
        ImGui::Checkbox("Helper Lines", &splat->drawHelperAxis);

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

        if (ImGui::Button("Next Selection"))
        {
            if (data->selected0 != (--data->SelectOpt.end())->first && 
                data->selected1 != (--data->SelectOpt.end())->first)
            {
                if (data->selected1 == (--data->SelectOpt.end())->first)
                {
                    data->selected1 = 0;
                    for (auto i = data->SelectOpt.begin(); i != data->SelectOpt.end(); ++i)
                    {
                        if (i->first == data->selected0) data->selected0 = (++i)->first;
                    }
                }
                else 
                {

                    for (auto i = data->SelectOpt.begin(); i != data->SelectOpt.end(); ++i)
                    {
                        if (i->first == data->selected1) data->selected1 = (++i)->first;
                    }
                }
            }
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

    void MainMenuStrip(MenueStripData* data, Renderer& renderer, Camera& cam) 
    {
        ImGui::BeginMainMenuBar();

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

        if (ImGui::BeginMenu("Scenes"))
        {
            if (ImGui::Button("Linear Motion")){ SwapScene(Scenes::LinearMotion) }
                
            if (ImGui::Button("Non Linear Motion")){ SwapScene(Scenes::NonLinearMotion) }
                
            if (ImGui::Button("Rotation")){ SwapScene(Scenes::RotationMotion) }
                
            if (ImGui::Button("Combined Motion")){ SwapScene(Scenes::CombinedMotion) }
                
            ImGui::NewLine();


            if (ImGui::Button("Cloud")) { SwapScene(Scenes::Clouds) }
            if (ImGui::Button("2D Gaussians")) { SwapScene(Scenes::Gaussians2D) }
            if (ImGui::Button("3D Gaussians")) { SwapScene(Scenes::Gaussians3D) }
            if (ImGui::Button("4D Gaussians")) { SwapScene(Scenes::Gaussians4D) }
            if (ImGui::Button("Modulo Motion")) { SwapScene(Scenes::BrokenMotion) }
            if (ImGui::Button("Square Motion")) { SwapScene(Scenes::SquareMotion) }
            if (ImGui::Button("Object Display")) { SwapScene(Scenes::ObjectDisplay) }
                

            ImGui::NewLine();
            if (ImGui::Button("> Empty <"))
                SwapScene(Scenes::Empty)

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    

}