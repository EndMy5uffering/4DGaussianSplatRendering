/*
    This file holds all the demo scenes that were used for the experiments and other showcases.
    I admind this file is a bit chaotic and long. If i had more time i would redo this.
*/
#pragma once
#include "Scene.h"
#include <cstdlib>
#include <ctime>
#include <math.h>
#include "Utils.h"
#include <tuple>

#define RANDOM (float(std::rand()) / float(RAND_MAX))

typedef std::tuple<glm::vec3, glm::vec3> ModelEdges;

namespace Scenes 
{
    /*
        Struct to hold the data that is send to the shader.
    */
    struct SplatData
    {
        glm::vec4 pos;
        glm::vec4 col;
        glm::mat4 sig;

        inline glm::vec4 GetMeanInTime(float ctime)
        {
            float _ctime = (ctime - pos.w);
            float x = pos.x + sig[3].x * _ctime;
            float y = pos.y + sig[3].y * _ctime;
            float z = pos.z + sig[3].z * _ctime;

            return glm::vec4{x, y, z, 1};
        }
    };

    //function from https://en.wikipedia.org/wiki/HSL_and_HSV
    static float hsl_f(float n, float h, float s, float l)
    {
        float k = fmod((n + (h / 30.0f)), 12.0f);
        return l - (s * Utils::minf(l, 1.0f - l)) * Utils::maxf(-1.0f, Utils::minf(k - 3.0f, Utils::minf(9.0f - k, 1.0f)));
    }

    static glm::vec3 GetHSLColor(float h, float s, float l)
    {
        return { hsl_f(0.0f, h, s, l), hsl_f(8.0f, h, s, l), hsl_f(4.0f, h, s, l) };
    }

    static glm::vec4 GetColor(glm::vec3 pos, ModelEdges medges, glm::vec3 normal, float mina = 0.65f, float maxa = 1.0f, float lower = 0.0f) 
    {
        float max_bright = Utils::mapf(-glm::dot({0,-1,0}, normal), -1, 1, mina, maxa);
        auto [minp, maxp] = medges;

        return glm::clamp(glm::vec4{
            Utils::lerp(lower, max_bright, ((pos.x - minp.x) / (maxp.x - minp.x))),
            Utils::lerp(lower, max_bright, ((pos.y - minp.y) / (maxp.y - minp.y))),
            Utils::lerp(lower, max_bright, ((pos.z - minp.z) / (maxp.z - minp.z))),
                1.0f }, { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 });
    }

    /**
        Finds the outer most points of a given model

        @param[in] model A vector of matrices containing the model information provided from a vData model
    */
    static ModelEdges GetModelExtrema(std::vector<glm::mat3>& model)
    {
        glm::vec3 maxpos{-INFINITY, -INFINITY, -INFINITY};
        glm::vec3 minpos{INFINITY, INFINITY, INFINITY};

        for (int i = 0; i < model.size(); ++i)
        {
            glm::vec3 pos = model[i][0];
            maxpos.x = Utils::maxf(pos.x, maxpos.x);
            maxpos.y = Utils::maxf(pos.y, maxpos.y);
            maxpos.z = Utils::maxf(pos.z, maxpos.z);
            minpos.x = Utils::minf(pos.x, minpos.x);
            minpos.y = Utils::minf(pos.y, minpos.y);
            minpos.z = Utils::minf(pos.z, minpos.z);
        }
        return { minpos, maxpos };
    }

    /**
        Finds the outer most points of a given model

        @param[in] model A vector of splat data containing the model information provided from a splat data model
    */
    static ModelEdges GetModelExtrema(std::vector<VData::VSplatData>& model)
    {
        glm::vec3 maxpos{-INFINITY, -INFINITY, -INFINITY};
        glm::vec3 minpos{INFINITY, INFINITY, INFINITY};

        for (int i = 0; i < model.size(); ++i)
        {
            glm::vec3 pos = model[i].pos;
            maxpos.x = Utils::maxf(pos.x, maxpos.x);
            maxpos.y = Utils::maxf(pos.y, maxpos.y);
            maxpos.z = Utils::maxf(pos.z, maxpos.z);
            minpos.x = Utils::minf(pos.x, minpos.x);
            minpos.y = Utils::minf(pos.y, minpos.y);
            minpos.z = Utils::minf(pos.z, minpos.z);
        }
        return { minpos, maxpos };
    }

    /*
        Empty demo scene to start the programm on.
    */
    class Empty : public Scene
    {

    public:
        Empty(Renderer& r, Camera& c) : Scene(r, c)
        {
        }
        ~Empty()
        {
        }

        void init() override
        {
            std::cout << "Init: Scenes::Empty\n";
            std::cout << "Done Init: Scenes::Empty\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scenes::Empty\n";
            std::cout << "Done unloading Scenes::Empty\n";
        }

        void Render() override
        {

            GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);

        }

        void Update(GLFWwindow* hwin) override
        {

        }

        void GUI() override {}
    };

    /*
        Linear motion experiment scene.
    */
    class LinearMotion : public Scene
    {
    private:

        std::vector<glm::mat3> m_vModelData; // Model (teapot)
        std::vector<SplatData> m_sdata; // Splat data genereted from the model data

        glm::vec3 m_first_pos{0}; // Used for the path
        glm::vec3 m_last_pos{0}; // Used for the path

        GLuint m_key_buf = 0; // ID of sort buffer
        GLuint m_values_buf = 0; // ID of sort buffer

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        std::vector<GLuint> m_key_buffer_data_pre;
        std::vector<GLfloat> m_val_buffer_data_pre;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf4DSpltas = 0;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_time_speed = 0.25f;
        float m_max_time = 50.0f;
        float m_min_opacity = 0.0f;
        float m_splat_speed = 1.0f;
        float m_splat_lifetime = 1.0f;
        float m_splat_fade_offset = 0.5f;
        float m_lin_time_multiplyer = 1.0f;
        int m_steps_in_time = 50;

        float m_object_scale = 5.0f;
        float m_splat_scale_x = 4.0f;
        float m_splat_scale_y = 4.0f;
        float m_splat_scale_z = 1.0f;

        bool m_loop = true;
        bool m_do_time = false;
        bool m_scene_menu = false;

        bool m_do_sort = false;

        bool m_show_grid = true;
        bool m_show_axis = true;
        bool m_show_unitlenght = true;
        bool m_show_path = false;

    public:
        LinearMotion(Renderer& r, Camera& c) : Scene(r, c)
        {
            // loading shader for the scene
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DFragShader.GLSL", GL_FRAGMENT_SHADER);
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DVertexShaderInstanced.GLSL", GL_VERTEX_SHADER);
            m_S4DShaderInstanced.BuildShader();
        }
        ~LinearMotion()
        {
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
        }

        void init() override
        {
            GetCamera().SetPosition({60, 90, 90});
            GetCamera().SetOrientation({ 0, -1.0, -1.0 });
            std::cout << "Initializing Scene: Linear Motion\n";
            m_vModelData = VData::parse("../Objects/teapot.vdata");
            m_numOf4DSpltas = m_vModelData.size() * m_steps_in_time;
            m_sdata.clear();
            m_sdata.reserve(m_numOf4DSpltas);

            m_key_buffer_data_pre.clear();
            m_key_buffer_data_pre.reserve(m_numOf4DSpltas);
            m_val_buffer_data_pre.clear();
            m_val_buffer_data_pre.reserve(m_numOf4DSpltas);

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

            std::vector<GLuint> keys_non_sorted(m_numOf4DSpltas);
            unsigned int s_idx = 0; //Sorting index for later

            ModelEdges medge = GetModelExtrema(m_vModelData); //Used for the color gradient

            glm::vec3 downVec{0, -1, 0};
            /*
                Init all splats of the object in all positions in time
            */
            for (int dt = 0; dt < m_steps_in_time; ++dt)
            {
                for (int i = 0; i < m_vModelData.size(); ++i)
                {
                    glm::vec3 pos = m_vModelData[i][0]; //Vertex position
                    glm::vec3 dir{1.0, 0.0, 0.0};
                    glm::vec3 timeOffset = dir * float(dt * m_lin_time_multiplyer);
                    Splat4D s4d{
                        glm::vec4{(m_object_scale * pos) + timeOffset, float(dt)}, //Linear position offset
                        glm::normalize(glm::quatLookAt(glm::normalize(m_vModelData[i][1]), glm::vec3(0,1,0))),
                        glm::vec3{m_splat_scale_x, m_splat_scale_y, m_splat_scale_z},
                        m_splat_lifetime,
                        m_splat_fade_offset,
                        glm::normalize(dir) * m_splat_speed,
                        GetColor(pos, medge, m_vModelData[i][1])
                    };

                    m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });
                    m_key_buffer_data_pre.push_back(s_idx);
                    m_val_buffer_data_pre.push_back(0.0f);
                    ++s_idx;
                }
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());


            m_sorter = std::make_unique< radix_sort::sorter >(m_numOf4DSpltas);
            m_ssbo_splat_data = std::make_unique< ShareStorageBuffer >(m_sdata.data(), m_numOf4DSpltas * sizeof(SplatData));
            std::cout << "Done initializing Scene: Linear Motion\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scene: Linear Motion\n";
            m_ssbo_splat_data.reset();
            m_sorter.reset();
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
            std::cout << "Done unloading Scene: Linear Motion\n";
        }

        void Render() override
        {
            if(m_show_grid) GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            if(m_show_axis) GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            if(m_show_unitlenght) GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);

            if (m_show_path)
            {
                GetRenderer().DrawLine(glm::vec3{0,0,0}, glm::vec3{50, 0, 0}, glm::vec4{1,0,0,1}, GetCamera(), 5);
            }

            if (m_do_sort) 
            {
                for (int i = 0; i < m_numOf4DSpltas; ++i)
                {
                    m_key_buffer_data_pre[i] = i;
                    glm::vec4 tmp = m_sdata[i].GetMeanInTime(m_time) - glm::vec4(GetCamera().GetPosition(), 1);
                    m_val_buffer_data_pre[i] = 1.0f / sqrtf(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLfloat), m_val_buffer_data_pre.data());

                m_sorter->sort(m_values_buf, m_key_buf, m_numOf4DSpltas);
            }

            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
            m_S4DShaderInstanced.SetUniform1f("uMinOpacity", m_min_opacity);
            m_S4DShaderInstanced.SetUniformMat4f("uView", GetCamera().GetViewMatrix());
            m_S4DShaderInstanced.SetUniformMat4f("uProj", GetCamera().GetProjMatrix());

            GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_key_buf));
            m_ssbo_splat_data->Bind(2);

            GetRenderer().Draw(quad.QuadVA, quad.QuadIdxBuffer, m_numOf4DSpltas);
        }

        void Update(GLFWwindow* hwin) override
        {
            if (glfwGetKey(hwin, GLFW_KEY_M) == GLFW_PRESS) m_scene_menu = true;

            if (m_do_time) m_time += m_time_speed;
            if (m_time > m_max_time && m_loop)
            {
                m_time = 0;
            }
            else if (m_time > m_max_time && !m_loop)
            {
                m_do_time = false;
                m_time = m_max_time;
            }
        }

        void GUI() override
        {
            if (!m_scene_menu) return;
            ImGui::Begin("Linear Scene Menu", &m_scene_menu);

            ImGui::SliderFloat("Time Speed", &m_time_speed, -2.0f, 2.0f);
            ImGui::InputFloat("Time Max", &m_max_time);
            ImGui::SliderFloat("Time", &m_time, 0.0f, m_max_time);

            ImGui::Checkbox("Loop", &m_loop);
            ImGui::Checkbox("Sort", &m_do_sort);

            ImGui::NewLine();

            ImGui::Checkbox("Grid", &m_show_grid);
            ImGui::Checkbox("Axis", &m_show_axis);
            ImGui::Checkbox("Unit length", &m_show_unitlenght);
            ImGui::Checkbox("Path", &m_show_path);

            ImGui::NewLine();


            if (ImGui::Button("Run"))
                m_do_time = true;
            if (ImGui::Button("Stop"))
                m_do_time = false;
            if (ImGui::Button("Reset"))
                m_time = 0;

            ImGui::NewLine();

            if (ImGui::Button("Cam_2")) 
            {
                GetCamera().SetPosition({12,40,40});
                GetCamera().SetOrientation({0,-1,-1});
            }

            ImGui::NewLine();

            ImGui::InputFloat("m_Splat_Speed", &m_splat_speed);
            ImGui::InputFloat("m_splat_lifetime", &m_splat_lifetime);
            ImGui::InputFloat("m_min_opacity", &m_min_opacity);
            ImGui::InputFloat("m_splat_fade_offset", &m_splat_fade_offset);
            ImGui::InputFloat("m_lin_time_multiplyer", &m_lin_time_multiplyer);
            ImGui::InputInt("m_steps_in_time", &m_steps_in_time);

            ImGui::NewLine();

            ImGui::InputFloat("m_object_scale", &m_object_scale);
            ImGui::InputFloat("m_splat_scale_x", &m_splat_scale_x);
            ImGui::InputFloat("m_splat_scale_y", &m_splat_scale_y);
            ImGui::InputFloat("m_splat_scale_z", &m_splat_scale_z);

            ImGui::NewLine();

            if (ImGui::Button("Reload Scene"))
            {
                unload();
                init();
            }


            ImGui::End();
        }

    };

    /*
        Non linear motion experiment scene.
    */
    class NonLinearMotion : public Scene
    {
    private:

        /*Similar setup to liniear motion*/
        std::vector<glm::mat3> m_vModelData;
        std::vector<SplatData> m_sdata;
        std::vector<glm::vec3> m_positions;

        GLuint m_key_buf = 0;
        GLuint m_values_buf = 0;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        std::vector<GLuint> m_key_buffer_data_pre;
        std::vector<GLfloat> m_val_buffer_data_pre;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf4DSpltas = 0;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_max_time = 90.0f;
        float m_time_speed = 0.25f;
        float m_min_opacity = 0.0f;
        float m_Splat_Speed = 20.0f;
        float m_splat_lifetime = 1.0f;
        float m_splat_fade_offset = 0.5f;
        float m_rotation_radius = 20.0f;
        float m_angle_multiplyer = 4.0f;
        int m_steps_in_time = 92;

        float m_object_scale = 5.0f;
        float m_splat_scale_x = 4.0f;
        float m_splat_scale_y = 4.0f;
        float m_splat_scale_z = 1.0f;

        bool m_loop = true;
        bool m_doTime = false;
        bool m_SceneMenu = false;

        bool m_DoSort = false;

        bool m_showGrid = true;
        bool m_showAxis = true;
        bool m_showUnitlenght = true;
        bool m_showPath = false;

    public:
        NonLinearMotion(Renderer& r, Camera& c) : Scene(r, c)
        {
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DFragShader.GLSL", GL_FRAGMENT_SHADER);
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DVertexShaderInstanced.GLSL", GL_VERTEX_SHADER);
            m_S4DShaderInstanced.BuildShader();
        }
        ~NonLinearMotion()
        {
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
        }

        void init() override
        {
            GetCamera().SetPosition({ 0, 60, 60 });
            GetCamera().SetOrientation({ 0.0, -1.0, -1.0 });
            std::cout << "Init: Scenes::NonLinearMotion\n";
            m_vModelData = VData::parse("../Objects/teapot.vdata");
            m_numOf4DSpltas = m_vModelData.size() * m_steps_in_time;
            m_sdata.clear();
            m_sdata.reserve(m_numOf4DSpltas);

            m_key_buffer_data_pre.clear();
            m_key_buffer_data_pre.reserve(m_numOf4DSpltas);
            m_val_buffer_data_pre.clear();
            m_val_buffer_data_pre.reserve(m_numOf4DSpltas);

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

            ModelEdges medge = GetModelExtrema(m_vModelData);

            unsigned int s_idx = 0;
            for (int dt = 0; dt < m_steps_in_time; ++dt)
            {
                m_positions.push_back(glm::rotate(glm::vec4{1.0, 0.0, 0.0, 0.0}, glm::radians(float(dt * m_angle_multiplyer)), { 0.0, 1.0, 0.0 }) * m_rotation_radius);
                for (int i = 0; i < m_vModelData.size(); ++i)
                {
                    /*
                        Position offset is calculated as a vector rotated arround the origin
                    */
                    glm::vec3 pos = m_vModelData[i][0];
                    glm::vec4 forward{ 1.0, 0.0, 0.0, 0.0 };
                    /* timeOffset_next is used for the direction of movement of the splats */
                    glm::vec3 timeOffset = glm::vec3{ glm::rotate(forward, glm::radians(float(dt * m_angle_multiplyer)), {0.0, 1.0, 0.0}) };
                    glm::vec3 timeOffset_next = glm::vec3{ glm::rotate(forward, glm::radians(float((dt+1)* m_angle_multiplyer)), {0.0, 1.0, 0.0}) };
                    Splat4D s4d{
                        glm::vec4{(m_object_scale * pos) + (timeOffset * m_rotation_radius), float(dt)},
                        glm::normalize(glm::quatLookAt(glm::normalize(m_vModelData[i][1]), glm::vec3(0,1,0))),
                        glm::vec3{m_splat_scale_x, m_splat_scale_y, m_splat_scale_z},
                        m_splat_lifetime,
                        m_splat_fade_offset,
                        (timeOffset_next - timeOffset) * m_Splat_Speed,
                        GetColor(pos, medge, m_vModelData[i][1])
                    };

                    m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });
                    m_key_buffer_data_pre.push_back(s_idx);
                    m_val_buffer_data_pre.push_back(0.0f);
                    ++s_idx;
                }
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

            m_sorter = std::make_unique< radix_sort::sorter >(m_numOf4DSpltas);
            m_ssbo_splat_data = std::make_unique< ShareStorageBuffer >(m_sdata.data(), m_numOf4DSpltas * sizeof(SplatData));
            std::cout << "Done Init: Scenes::NonLinearMotion\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scenes::NonLinearMotion\n";
            m_ssbo_splat_data.reset();
            m_sorter.reset();
            m_positions.clear();
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
            std::cout << "Done unloading Scenes::NonLinearMotion\n";
        }

        void Render() override
        {

            if(m_showGrid) GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            if(m_showAxis) GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            if(m_showUnitlenght) GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);

            if (m_showPath) 
            {
                GetRenderer().DrawLine(m_positions, {1.0,0.0,0.0,1.0}, GetCamera(), 5.0f);
            }

            if (m_DoSort) 
            {
                for (int i = 0; i < m_numOf4DSpltas; ++i)
                {
                    m_key_buffer_data_pre[i] = i;
                    glm::vec4 tmp = m_sdata[i].GetMeanInTime(m_time) - glm::vec4(GetCamera().GetPosition(), 1);
                    m_val_buffer_data_pre[i] = 1.0 / sqrtf(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLfloat), m_val_buffer_data_pre.data());

                m_sorter->sort(m_values_buf, m_key_buf, m_numOf4DSpltas);
            }

            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
            m_S4DShaderInstanced.SetUniform1f("uMinOpacity", m_min_opacity);
            m_S4DShaderInstanced.SetUniformMat4f("uView", GetCamera().GetViewMatrix());
            m_S4DShaderInstanced.SetUniformMat4f("uProj", GetCamera().GetProjMatrix());

            GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_key_buf));
            m_ssbo_splat_data->Bind(2);

            GetRenderer().Draw(quad.QuadVA, quad.QuadIdxBuffer, m_numOf4DSpltas);

        }

        void Update(GLFWwindow* hwin) override
        {
            if (glfwGetKey(hwin, GLFW_KEY_M) == GLFW_PRESS) m_SceneMenu = true;

            if (m_doTime) m_time += m_time_speed;
            if (m_time > m_max_time && m_loop) 
            {
                m_time = 0;
            }
            else if (m_time > m_max_time && !m_loop)
            {
                m_doTime = false;
                m_time = m_max_time;
            }
        }

        void GUI() override 
        {
            if (!m_SceneMenu) return;
            ImGui::Begin("Non Linear Scene Menu", &m_SceneMenu);

            ImGui::SliderFloat("Time Speed", &m_time_speed, -2.0f, 2.0f);
            ImGui::InputFloat("Time Max", &m_max_time);
            ImGui::SliderFloat("Time", &m_time, 0.0f, m_max_time);

            ImGui::Checkbox("Loop", &m_loop);
            ImGui::Checkbox("Sort", &m_DoSort);

            ImGui::NewLine();

            ImGui::Checkbox("Grid", &m_showGrid);
            ImGui::Checkbox("Axis", &m_showAxis);
            ImGui::Checkbox("Unit length", &m_showUnitlenght);
            ImGui::Checkbox("Path", &m_showPath);

            ImGui::NewLine();

            if (ImGui::Button("Run")) 
                m_doTime = true;
            if (ImGui::Button("Stop"))
                m_doTime = false;
            if (ImGui::Button("Reset"))
                m_time = 0;

            ImGui::NewLine();

            ImGui::InputFloat("m_Splat_Speed", &m_Splat_Speed);
            ImGui::InputFloat("m_splat_lifetime", &m_splat_lifetime);
            ImGui::InputFloat("m_min_opacity", &m_min_opacity);
            ImGui::InputFloat("m_splat_fade_offset", &m_splat_fade_offset);
            ImGui::InputFloat("m_rotation_radius", &m_rotation_radius);
            ImGui::InputFloat("m_angle_multiplyer", &m_angle_multiplyer);
            ImGui::InputInt("m_steps_in_time", &m_steps_in_time);

            ImGui::NewLine();

            ImGui::InputFloat("m_object_scale", &m_object_scale);
            ImGui::InputFloat("m_splat_scale_x", &m_splat_scale_x);
            ImGui::InputFloat("m_splat_scale_y", &m_splat_scale_y);
            ImGui::InputFloat("m_splat_scale_z", &m_splat_scale_z);

            ImGui::NewLine();

            if (ImGui::Button("Reload Scene")) 
            {
                unload();
                init();
            }


            ImGui::End();
        }

    };

    /*
        Rotational motion experiment scene.
    */
    class RotationMotion : public Scene
    {
    private:

        /* Same setup as the above */
        std::vector<glm::mat3> m_vModelData;
        std::vector<SplatData> m_sdata;

        GLuint m_key_buf = 0;
        GLuint m_values_buf = 0;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        std::vector<GLuint> m_key_buffer_data_pre;
        std::vector<GLfloat> m_val_buffer_data_pre;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf4DSpltas = 0;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_max_time = 90.0f;
        float m_time_speed = 0.25f;
        float m_min_opacity = 0.0f;
        float m_Splat_Speed = 5.0f;
        float m_splat_lifetime = 0.6f;
        float m_splat_fade_offset = 0.5f;
        float m_angle_multiplyer = 4.0f;
        int m_steps_in_time = 92;

        float m_object_scale = 5.0f;
        float m_splat_scale_x = 4.0f;
        float m_splat_scale_y = 4.0f;
        float m_splat_scale_z = 1.0f;

        bool m_loop = true;
        bool m_doTime = false;
        bool m_SceneMenu = false;
        bool m_DoSort = false;

        bool m_showGrid = true;
        bool m_showAxis = true;
        bool m_showUnitlenght = true;

    public:
        RotationMotion(Renderer& r, Camera& c) : Scene(r, c)
        {
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DFragShader.GLSL", GL_FRAGMENT_SHADER);
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DVertexShaderInstanced.GLSL", GL_VERTEX_SHADER);
            m_S4DShaderInstanced.BuildShader();
        }
        ~RotationMotion()
        {
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
        }

        void init() override
        {
            GetCamera().SetPosition({ 0, 60, 30 });
            GetCamera().SetOrientation({ 0.0, -1.0, -0.5 });
            std::cout << "Init: Scenes::RotationMotion\n";
            m_vModelData = VData::parse("../Objects/teapot.vdata");
            m_numOf4DSpltas = m_vModelData.size() * m_steps_in_time;
            m_sdata.clear();
            m_sdata.reserve(m_numOf4DSpltas);

            m_key_buffer_data_pre.clear();
            m_key_buffer_data_pre.reserve(m_numOf4DSpltas);
            m_val_buffer_data_pre.clear();
            m_val_buffer_data_pre.reserve(m_numOf4DSpltas);

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);
            
            glm::vec3 maxpos{-INFINITY, -INFINITY, -INFINITY};
            glm::vec3 minpos{INFINITY, INFINITY, INFINITY};

            ModelEdges medge = GetModelExtrema(m_vModelData);

            unsigned int s_idx = 0;
            for (int dt = 0; dt < m_steps_in_time; ++dt)
            {
                for (int i = 0; i < m_vModelData.size(); ++i)
                {
                    /* 
                        Each splats postion is rotated arround the y axis at the origin 
                        as well as its facing direction to avoid the object chanigin shpae as it rotates
                    */
                    glm::vec4 pos{ m_vModelData[i][0], 0.0 };
                    // Position of vertex is rotated arround the y axis arround the origin
                    glm::vec3 timeOffset = glm::vec3{ glm::rotate(pos, glm::radians(float(dt * m_angle_multiplyer)), {0.0, 1.0, 0.0}) };
                    glm::vec3 timeOffset_next = glm::vec3{ glm::rotate(pos, glm::radians(float((dt + 1) * m_angle_multiplyer)), {0.0, 1.0, 0.0}) };
                    glm::vec3 norm = glm::vec3{ glm::rotate(glm::vec4{m_vModelData[i][1], 0}, glm::radians(float(dt * m_angle_multiplyer)), {0.0, 1.0, 0.0}) };
                    Splat4D s4d{
                        glm::vec4{(m_object_scale * timeOffset), float(dt)},
                        glm::normalize(glm::quatLookAt(glm::normalize(norm), glm::vec3(0,1,0))),
                        glm::vec3{m_splat_scale_x, m_splat_scale_y, m_splat_scale_z},
                        m_splat_lifetime,
                        m_splat_fade_offset,
                        (timeOffset_next - timeOffset) * m_Splat_Speed,
                        GetColor(pos, medge, m_vModelData[i][1])
                    };

                    m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });
                    m_key_buffer_data_pre.push_back(s_idx);
                    m_val_buffer_data_pre.push_back(0.0f);
                    ++s_idx;
                }
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

            m_sorter = std::make_unique< radix_sort::sorter >(m_numOf4DSpltas);
            m_ssbo_splat_data = std::make_unique< ShareStorageBuffer >(m_sdata.data(), m_numOf4DSpltas * sizeof(SplatData));
            std::cout << "Done Init: Scenes::RotationMotion\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scenes::RotationMotion\n";
            m_ssbo_splat_data.reset();
            m_sorter.reset();
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
            std::cout << "Done unloading Scenes::RotationMotion\n";
        }

        void Render() override
        {

            if(m_showGrid) GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            if(m_showAxis) GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            if(m_showUnitlenght) GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);

            if(m_DoSort)
            {
                for (int i = 0; i < m_numOf4DSpltas; ++i)
                {
                    m_key_buffer_data_pre[i] = i;
                    glm::vec4 tmp = m_sdata[i].GetMeanInTime(m_time) - glm::vec4(GetCamera().GetPosition(), 1);
                    m_val_buffer_data_pre[i] = 1.0f / sqrtf(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLfloat), m_val_buffer_data_pre.data());

                m_sorter->sort(m_values_buf, m_key_buf, m_numOf4DSpltas);
            }

            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
            m_S4DShaderInstanced.SetUniform1f("uMinOpacity", m_min_opacity);
            m_S4DShaderInstanced.SetUniformMat4f("uView", GetCamera().GetViewMatrix());
            m_S4DShaderInstanced.SetUniformMat4f("uProj", GetCamera().GetProjMatrix());

            GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_key_buf));
            m_ssbo_splat_data->Bind(2);

            GetRenderer().Draw(quad.QuadVA, quad.QuadIdxBuffer, m_numOf4DSpltas);
        }

        void Update(GLFWwindow* hwin) override
        {
            if (glfwGetKey(hwin, GLFW_KEY_M) == GLFW_PRESS) m_SceneMenu = true;

            if (m_doTime) m_time += m_time_speed;
            if (m_time > m_max_time && m_loop)
            {
                m_time = 0;
            }
            else if (m_time > m_max_time && !m_loop)
            {
                m_doTime = false;
                m_time = m_max_time;
            }
        }

        void GUI() override
        {
            if (!m_SceneMenu) return;
            ImGui::Begin("Rotation Scene Menu", &m_SceneMenu);

            ImGui::SliderFloat("Time Speed", &m_time_speed, -2.0f, 2.0f);
            ImGui::InputFloat("Time Max", &m_max_time);
            ImGui::SliderFloat("Time", &m_time, 0.0f, m_max_time);

            ImGui::Checkbox("Loop", &m_loop);
            ImGui::Checkbox("Sort", &m_DoSort);

            ImGui::NewLine();

            ImGui::Checkbox("Grid", &m_showGrid);
            ImGui::Checkbox("Axis", &m_showAxis);
            ImGui::Checkbox("Unit length", &m_showUnitlenght);

            ImGui::NewLine();

            if (ImGui::Button("Run"))
                m_doTime = true;
            if (ImGui::Button("Stop"))
                m_doTime = false;
            if (ImGui::Button("Reset"))
                m_time = 0;

            ImGui::NewLine();

            ImGui::InputFloat("m_Splat_Speed", &m_Splat_Speed);
            ImGui::InputFloat("m_splat_lifetime", &m_splat_lifetime);
            ImGui::InputFloat("m_min_opacity", &m_min_opacity);
            ImGui::InputFloat("m_splat_fade_offset", &m_splat_fade_offset);
            ImGui::InputFloat("m_angle_multiplyer", &m_angle_multiplyer);
            ImGui::InputInt("m_steps_in_time", &m_steps_in_time);

            ImGui::NewLine();

            ImGui::InputFloat("m_object_scale", &m_object_scale);
            ImGui::InputFloat("m_splat_scale_x", &m_splat_scale_x);
            ImGui::InputFloat("m_splat_scale_y", &m_splat_scale_y);
            ImGui::InputFloat("m_splat_scale_z", &m_splat_scale_z);

            ImGui::NewLine();

            if (ImGui::Button("Reload Scene"))
            {
                unload();
                init();
            }


            ImGui::End();
        }

    };

    /*
        Combined motion experiment scene.
    */
    class CombinedMotion : public Scene
    {
    private:

        /*Same as above*/
        std::vector<glm::mat3> m_vModelData;
        std::vector<SplatData> m_sdata;
        std::vector<glm::vec3> m_positions;

        GLuint m_key_buf = 0;
        GLuint m_values_buf = 0;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        std::vector<GLuint> m_key_buffer_data_pre;
        std::vector<GLfloat> m_val_buffer_data_pre;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf4DSpltas = 0;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_max_time = 65.0f;
        float m_time_speed = 0.25f;
        float m_min_opacity = 0.0f;
        float m_Splat_Speed = 1.0f;
        float m_splat_lifetime = 1.0f;
        float m_splat_fade_offset = 0.5f;
        float m_angle_multiplyer = 8.0f;
        float m_lin_multiplyer = 8.0f;
        int m_steps_in_time = 65;

        float m_object_scale = 5.0f;
        float m_splat_scale_x = 4.0f;
        float m_splat_scale_y = 4.0f;
        float m_splat_scale_z = 0.0f;

        float m_amplitude = 1.0f;
        float m_frequency = 0.15f;

        bool m_loop = true;
        bool m_doTime = false;
        bool m_SceneMenu = false;
        bool m_DoSort = false;

        bool m_showGrid = true;
        bool m_showAxis = true;
        bool m_showUnitlenght = true;
        bool m_showPath = false;

    public:
        CombinedMotion(Renderer& r, Camera& c) : Scene(r, c)
        {
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DFragShader.GLSL", GL_FRAGMENT_SHADER);
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DVertexShaderInstanced.GLSL", GL_VERTEX_SHADER);
            m_S4DShaderInstanced.BuildShader();
        }
        ~CombinedMotion()
        {
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
        }

        void init() override
        {
            GetCamera().SetPosition({ 50, 90, 90 });
            GetCamera().SetOrientation({ 0.0, -1.0, -1.0 });
            std::cout << "Init: Scenes::RotationMotion\n";
            m_vModelData = VData::parse("../Objects/teapot.vdata");
            m_numOf4DSpltas = m_vModelData.size() * m_steps_in_time;
            m_sdata.clear();
            m_sdata.reserve(m_numOf4DSpltas);

            m_key_buffer_data_pre.clear();
            m_key_buffer_data_pre.reserve(m_numOf4DSpltas);
            m_val_buffer_data_pre.clear();
            m_val_buffer_data_pre.reserve(m_numOf4DSpltas);

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

            /*This is to find the tip of the pot*/
            glm::vec3 frontmostPoint{-INFINITY, -INFINITY, 0.0};
            for (int i = 0; i < m_vModelData.size(); ++i)
            {
                frontmostPoint.x = Utils::maxf(m_vModelData[i][0].x, frontmostPoint.x);
                if(frontmostPoint.x == m_vModelData[i][0].x) frontmostPoint.y = m_vModelData[i][0].y;
            }
            /*---------------------------------*/
            ModelEdges medge = GetModelExtrema(m_vModelData);

            unsigned int s_idx = 0;
            for (int dt = 0; dt < m_steps_in_time; ++dt)
            {
                m_positions.push_back(
                    glm::vec3{ 
                        glm::rotate(
                            glm::vec4{m_object_scale * frontmostPoint, 0.0},
                            glm::radians(float(dt* m_angle_multiplyer)), 
                            { 0.0, 1.0, 0.0 }
                        ) 
                    } + m_lin_multiplyer * glm::vec3{m_frequency * float(dt), m_amplitude * sinf(m_frequency * float(dt)), 0.0f}
                );
                for (int i = 0; i < m_vModelData.size(); ++i)
                {
                    glm::vec3 md = m_object_scale * m_vModelData[i][0];
                    glm::vec3 nd = m_vModelData[i][1];
                    glm::vec3 pos = glm::vec3{ glm::rotate(glm::vec4{md, 0}, glm::radians(float(dt * m_angle_multiplyer)), {0.0, 1.0, 0.0}) } + m_lin_multiplyer * glm::vec3{m_frequency * float(dt), m_amplitude * sinf(m_frequency * float(dt)), 0.0f};
                    glm::vec3 pos_next = glm::vec3{ glm::rotate(glm::vec4{md, 0}, glm::radians(float((dt+1) * m_angle_multiplyer)), {0.0, 1.0, 0.0}) } + m_lin_multiplyer * glm::vec3{m_frequency* float(dt+1), m_amplitude* sinf(m_frequency * float(dt+1)), 0.0f};
                    glm::vec3 norm = glm::vec3{ glm::rotate(glm::vec4{nd, 0}, glm::radians(float(dt * m_angle_multiplyer)), {0.0, 1.0, 0.0}) };
                    Splat4D s4d{
                        glm::vec4{pos, float(dt)},
                        glm::normalize(glm::quatLookAt(glm::normalize(norm), glm::vec3(0,1,0))),
                        glm::vec3{m_splat_scale_x, m_splat_scale_y, m_splat_scale_z},
                        m_splat_lifetime,
                        m_splat_fade_offset,
                        (pos_next - pos) * m_Splat_Speed,
                        GetColor(m_vModelData[i][0], medge, m_vModelData[i][1])
                    };

                    m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });
                    m_key_buffer_data_pre.push_back(s_idx);
                    m_val_buffer_data_pre.push_back(0.0f);
                    ++s_idx;
                }
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

            m_sorter = std::make_unique< radix_sort::sorter >(m_numOf4DSpltas);
            m_ssbo_splat_data = std::make_unique< ShareStorageBuffer >(m_sdata.data(), m_numOf4DSpltas * sizeof(SplatData));
            std::cout << "Done Init: Scenes::RotationMotion\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scenes::RotationMotion\n";
            m_ssbo_splat_data.reset();
            m_sorter.reset();
            m_positions.clear();
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
            std::cout << "Done unloading Scenes::RotationMotion\n";
        }

        void Render() override
        {
            if (m_showGrid) GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            if (m_showAxis) GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            if (m_showUnitlenght) GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);

            if (m_showPath)
            {
                GetRenderer().DrawLine(m_positions, { 1.0,0.0,0.0,1.0 }, GetCamera(), 5.0f);
            }

            if (m_DoSort) 
            {
                for (int i = 0; i < m_numOf4DSpltas; ++i)
                {
                    m_key_buffer_data_pre[i] = i;
                    glm::vec4 tmp = m_sdata[i].GetMeanInTime(m_time) - glm::vec4(GetCamera().GetPosition(), 1);
                    m_val_buffer_data_pre[i] = 1.0f / sqrtf(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLfloat), m_val_buffer_data_pre.data());

                m_sorter->sort(m_values_buf, m_key_buf, m_numOf4DSpltas);
            }

            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
            m_S4DShaderInstanced.SetUniform1f("uMinOpacity", m_min_opacity);
            m_S4DShaderInstanced.SetUniformMat4f("uView", GetCamera().GetViewMatrix());
            m_S4DShaderInstanced.SetUniformMat4f("uProj", GetCamera().GetProjMatrix());

            GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_key_buf));
            m_ssbo_splat_data->Bind(2);

            GetRenderer().Draw(quad.QuadVA, quad.QuadIdxBuffer, m_numOf4DSpltas);
        }

        void Update(GLFWwindow* hwin) override
        {
            if (glfwGetKey(hwin, GLFW_KEY_M) == GLFW_PRESS) m_SceneMenu = true;

            if (m_doTime) m_time += m_time_speed;
            if (m_time > m_max_time && m_loop)
            {
                m_time = 0;
            }
            else if (m_time > m_max_time && !m_loop)
            {
                m_doTime = false;
                m_time = m_max_time;
            }
        }

        void GUI() override 
        {
            if (!m_SceneMenu) return;
            ImGui::Begin("Combination Scene Menu", &m_SceneMenu);

            ImGui::SliderFloat("Time Speed", &m_time_speed, -2.0f, 2.0f);
            ImGui::InputFloat("Time Max", &m_max_time);
            ImGui::SliderFloat("Time", &m_time, 0.0f, m_max_time);

            ImGui::Checkbox("Loop", &m_loop);
            ImGui::Checkbox("Sort", &m_DoSort);

            ImGui::NewLine();

            ImGui::Checkbox("Grid", &m_showGrid);
            ImGui::Checkbox("Axis", &m_showAxis);
            ImGui::Checkbox("Unit length", &m_showUnitlenght);
            ImGui::Checkbox("Path", &m_showPath);

            ImGui::NewLine();

            if (ImGui::Button("Run"))
                m_doTime = true;
            if (ImGui::Button("Stop"))
                m_doTime = false;
            if (ImGui::Button("Reset"))
                m_time = 0;

            ImGui::NewLine();

            ImGui::InputFloat("m_Splat_Speed", &m_Splat_Speed);
            ImGui::InputFloat("m_splat_lifetime", &m_splat_lifetime);
            ImGui::InputFloat("m_min_opacity", &m_min_opacity);
            ImGui::InputFloat("m_splat_fade_offset", &m_splat_fade_offset);
            ImGui::InputFloat("m_angle_multiplyer", &m_angle_multiplyer);
            ImGui::InputFloat("m_lin_multiplyer", &m_lin_multiplyer);
            ImGui::InputInt("m_steps_in_time", &m_steps_in_time);

            ImGui::NewLine();

            ImGui::InputFloat("m_amplitude", &m_amplitude);
            ImGui::InputFloat("m_frequency", &m_frequency);

            ImGui::NewLine();

            ImGui::InputFloat("m_object_scale", &m_object_scale);
            ImGui::InputFloat("m_splat_scale_x", &m_splat_scale_x);
            ImGui::InputFloat("m_splat_scale_y", &m_splat_scale_y);
            ImGui::InputFloat("m_splat_scale_z", &m_splat_scale_z);

            ImGui::NewLine();

            if (ImGui::Button("Reload Scene"))
            {
                unload();
                init();
            }


            ImGui::End();
        }


    };

    /*
        A test scene as a test to model clouds.
    */
    class Clouds : public Scene
    {
    private:


        std::vector<SplatData> m_sdata;

        GLuint m_key_buf = 0;
        GLuint m_values_buf = 0;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        std::vector<GLuint> m_key_buffer_data_pre;
        std::vector<GLfloat> m_val_buffer_data_pre;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf4DSpltas = 0;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_max_time = 90.0f;
        float m_time_speed = 0.25f;
        float m_min_opacity = 0.0f;

        bool m_loop = true;
        bool m_doTime = false;
        bool m_SceneMenu = false;
        bool m_DoSort = false;

    public:
        Clouds(Renderer& r, Camera& c) : Scene(r, c)
        {
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DFragShader.GLSL", GL_FRAGMENT_SHADER);
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DVertexShaderInstanced.GLSL", GL_VERTEX_SHADER);
            m_S4DShaderInstanced.BuildShader();
        }
        ~Clouds()
        {
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
        }

        float p(float x, float mu, float sig) 
        {
            float e = ((x - mu) / sig);
            return expf(-0.5 * (e * e));
        }

        void MakeCloud(glm::vec3 c_pos, glm::vec3 scale, glm::vec3 dir, int splats) 
        {
            unsigned int s_idx = m_sdata.size();
            for (int i = 0; i < m_numOf4DSpltas; ++i)
            {

                float rngx = RANDOM;
                float rngy = RANDOM;
                float rngz = RANDOM;
                float rngc = RANDOM;
                float rnga = RANDOM;
                float rngsx = RANDOM;
                float rngsy = RANDOM;
                glm::vec3 pos{rngx * scale.x, rngy * scale.y, rngz * scale.z};

                float avr = (p(pos.x, c_pos.x, scale.x * scale.x) + p(pos.y, c_pos.y, scale.y * scale.y))/2.0f;
                float col = glm::clamp(1.0f - avr * rngc, 0.2f, 1.0f);
                Splat4D s4d{
                    glm::vec4{c_pos + pos, 0.0f},
                    glm::normalize(glm::quatLookAt(dir, glm::vec3(0,1,0))),
                    glm::vec3{glm::clamp(rngsx* scale.x, 10.0f, scale.x), 10, glm::clamp(rngsy* scale.z, 10.0f, scale.z)},
                    50,
                    0.5,
                    glm::vec3{1.0, 0.0, 0.0},
                    glm::vec4{col, col, col, glm::clamp(rnga+0.1f, 0.0f, 1.0f)}
                };

                m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });
                m_key_buffer_data_pre.push_back(s_idx);
                m_val_buffer_data_pre.push_back(0.0f);
                ++s_idx;
            }
        }

        void init() override
        {
            GetCamera().SetPosition({ 50, 90, 90 });
            GetCamera().SetOrientation({ 0.0, -1.0, -1.0 });
            std::cout << "Init: Scenes::Clouds\n";
            m_numOf4DSpltas = 150;
            m_sdata.clear();
            m_sdata.reserve(m_numOf4DSpltas);

            m_key_buffer_data_pre.clear();
            m_key_buffer_data_pre.reserve(m_numOf4DSpltas);
            m_val_buffer_data_pre.clear();
            m_val_buffer_data_pre.reserve(m_numOf4DSpltas);

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);


            /*CLOUDS*/

            MakeCloud({ 0.0,0.0,0.0 }, { 50,10,50 }, {1.0, 0.0, 0.0}, 50);

            /*CLOUDS*/


            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

            m_sorter = std::make_unique< radix_sort::sorter >(m_numOf4DSpltas);
            m_ssbo_splat_data = std::make_unique< ShareStorageBuffer >(m_sdata.data(), m_numOf4DSpltas * sizeof(SplatData));
            std::cout << "Done Init: Scenes::Clouds\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scenes::Clouds\n";
            m_ssbo_splat_data.reset();
            m_sorter.reset();
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
            std::cout << "Done unloading Scenes::Clouds\n";
        }

        void Render() override
        {

            GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);

            if (m_DoSort)
            {
                for (int i = 0; i < m_numOf4DSpltas; ++i)
                {
                    m_key_buffer_data_pre[i] = i;
                    glm::vec4 tmp = m_sdata[i].GetMeanInTime(0) - glm::vec4(GetCamera().GetPosition(), 1);
                    m_val_buffer_data_pre[i] = 1.0f / sqrtf(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLfloat), m_val_buffer_data_pre.data());

                m_sorter->sort(m_values_buf, m_key_buf, m_numOf4DSpltas);
            }

            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
            m_S4DShaderInstanced.SetUniform1f("uMinOpacity", m_min_opacity);
            m_S4DShaderInstanced.SetUniformMat4f("uView", GetCamera().GetViewMatrix());
            m_S4DShaderInstanced.SetUniformMat4f("uProj", GetCamera().GetProjMatrix());

            GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_key_buf));
            m_ssbo_splat_data->Bind(2);

            GetRenderer().Draw(quad.QuadVA, quad.QuadIdxBuffer, m_numOf4DSpltas);
        }

        void Update(GLFWwindow* hwin) override
        {
            if (glfwGetKey(hwin, GLFW_KEY_M) == GLFW_PRESS) m_SceneMenu = true;

            if (m_doTime) m_time += m_time_speed;
            if (m_time > m_max_time && m_loop)
            {
                m_time = 0;
            }
            else if (m_time > m_max_time && !m_loop)
            {
                m_doTime = false;
                m_time = m_max_time;
            }
        }

        void GUI() override
        {
            if (!m_SceneMenu) return;
            ImGui::Begin("Combination Scene Menu", &m_SceneMenu);

            ImGui::SliderFloat("Time Speed", &m_time_speed, -2.0f, 2.0f);
            ImGui::InputFloat("Time Max", &m_max_time);
            ImGui::SliderFloat("Time", &m_time, 0.0f, m_max_time);

            ImGui::Checkbox("Loop", &m_loop);
            ImGui::Checkbox("Sort", &m_DoSort);

            ImGui::NewLine();


            if (ImGui::Button("Run"))
                m_doTime = true;
            if (ImGui::Button("Stop"))
                m_doTime = false;
            if (ImGui::Button("Reset"))
                m_time = 0;

            ImGui::NewLine();

            ImGui::InputFloat("m_min_opacity", &m_min_opacity);

            ImGui::NewLine();

            if (ImGui::Button("Reload Scene"))
            {
                unload();
                init();
            }


            ImGui::End();
        }


    };

    /*
        2D Gaussians showcase scene.
    */
    class Gaussians2D : public Scene
    {
    private:

        struct Splat2DData 
        {
            glm::vec4 position;
            glm::vec4 color;
            glm::mat2 geoinfo;
        };

        size_t num_splats = 20;

        std::vector<Splat2DData> m_sdata;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;

        Shader m_Shader;

        const Geometry::Quad quad;

        bool m_SceneMenu = false;

        bool m_showGrid = true;
        bool m_showAxis = true;
        bool m_showUnitlenght = true;

    public:
        Gaussians2D(Renderer& r, Camera& c) : Scene(r, c)
        {
            m_Shader.AddShaderSource("../Shader/Splats2D/Splat2DFragShader.GLSL", GL_FRAGMENT_SHADER);
            m_Shader.AddShaderSource("../Shader/Splats2D/Splat2DVSI.GLSL", GL_VERTEX_SHADER);
            m_Shader.BuildShader();
        }
        ~Gaussians2D()
        {

        }

        float p(float x, float mu, float sig)
        {
            float e = ((x - mu) / sig);
            return expf(-0.5 * (e * e));
        }

        void init() override
        {
            GetCamera().SetPosition({ -10, 10, 0 });
            GetCamera().SetOrientation({ 1.0, -1.0, 0 });
            std::cout << "Init: Scenes::Gaussians2D\n";
            m_sdata.clear();

            /*2D*/
            for (int i = 0; i < num_splats; ++i)
            {
                float a = glm::radians(360.0f * RANDOM);
                glm::mat2 R{cosf(a), -sinf(a), sinf(a), cos(a)};
                glm::mat2 S{1.0f + (5.0f * RANDOM), 0.0f, 0.0f, 1.0f + (5.0f * RANDOM)};
                m_sdata.push_back({ {10.0f * (-0.5f + RANDOM), 10.0f * (-0.5f + RANDOM), 0, 0}, {RANDOM, RANDOM, RANDOM, 1.0}, R * S * S * glm::transpose(R) });
            }

            /* All of that was used for the smiley face */

            /*float global_scalar = 1.0f;

            for (int i = 0; i < 60; ++i)
            {
                float a = glm::radians(Utils::lerp(0.0f, 361.0f, float(i) / 60.0f));
                glm::mat2 R{cosf(a), -sinf(a), sinf(a), cos(a)};
                glm::mat2 S{0.2f, 0.0f, 0.0f, 1.0f * global_scalar};
                m_sdata.push_back({ {3.0f * sinf(a), 3.0f  * cosf(a), 0, 0}, {GetHSLColor(193, 1.0, 0.75), 1.0}, R * S * S * glm::transpose(R)});
            }

            for (int i = 0; i < 30; ++i)
            {
                float a = glm::radians(Utils::lerp(0.0f, 360.0f, float(i) / 30.0f));
                glm::mat2 R{cosf(a), -sinf(a), sinf(a), cos(a)};
                glm::mat2 S{0.2f, 0.0f, 0.0f, 0.4f * global_scalar};
                m_sdata.push_back({ {-1.5f + (0.5f * sinf(a)), 1.0f + (0.5f * cosf(a)), 0, 0}, {GetHSLColor(193, 1.0, 0.75), 1.0}, R * S * S * glm::transpose(R) });
            }

            for (int i = 0; i < 5; ++i)
            {
                glm::mat2 R{1, 0, 0, 1};
                glm::mat2 S{0.4f, 0.0f, 0.0f, 0.4f };
                m_sdata.push_back({ {-1.5f, 1.0f, 0, 0}, {GetHSLColor(193, 1.0, 0.75), 1.0}, R * S * S * glm::transpose(R) });
            }

            for (int i = 0; i < 30; ++i)
            {
                float a = glm::radians(Utils::lerp(0.0f, 360.0f, float(i) / 30.0f));
                glm::mat2 R{cosf(a), -sinf(a), sinf(a), cos(a)};
                glm::mat2 S{0.2f, 0.0f, 0.0f, 0.4f * global_scalar};
                m_sdata.push_back({ {1.5f + (0.5f * sinf(a)), 1.0f + (0.5f * cosf(a)), 0, 0}, {GetHSLColor(193, 1.0, 0.75), 1.0}, R * S * S * glm::transpose(R) });
            }

            for (int i = 0; i < 5; ++i)
            {
                glm::mat2 R{1, 0, 0, 1};
                glm::mat2 S{0.4f, 0.0f, 0.0f, 0.4f};
                m_sdata.push_back({ {1.5f, 1.0f, 0, 0}, {GetHSLColor(193, 1.0, 0.75), 1.0}, R * S * S * glm::transpose(R) });
            }

            for (int i = 0; i < 15; ++i)
            {
                float a = 0;
                glm::mat2 R{cosf(a), -sinf(a), sinf(a), cos(a)};
                glm::mat2 S{0.2f, 0.0f, 0.0f, 0.8f * global_scalar};
                m_sdata.push_back({ {Utils::lerp(-1.4f, 1.5f, float(i)/15.0f), -0.0f, 0, 0}, {GetHSLColor(193, 1.0, 0.75), 1.0}, R * S * S * glm::transpose(R) });
            }

            for (int i = 0; i < 30; ++i)
            {
                float a = glm::radians(Utils::lerp(95.0f, 270.0f, float(i) / 30.0f));
                glm::mat2 R{cosf(a), -sinf(a), sinf(a), cos(a)};
                glm::mat2 S{0.2f, 0.0f, 0.0f, 0.8f * global_scalar};
                m_sdata.push_back({ {-0.05f + (1.5f * sinf(a)), -0.0f + (1.5f * cosf(a)), 0, 0}, {GetHSLColor(193, 1.0, 0.75), 1.0}, R * S * S * glm::transpose(R) });
            }*/
            /*2D*/

            m_ssbo_splat_data = std::make_unique< ShareStorageBuffer >(m_sdata.data(), m_sdata.size() * sizeof(Splat2DData));
            std::cout << "Done Init: Scenes::RotationMotion\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scenes::RotationMotion\n";
            m_ssbo_splat_data.reset();
            std::cout << "Done unloading Scenes::RotationMotion\n";
        }

        void Render() override
        {

            if (m_showGrid) GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            if (m_showAxis) GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            if (m_showUnitlenght) GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);

            m_Shader.Bind();
            m_Shader.SetUniformMat4f("uView", GetCamera().GetViewMatrix());
            m_Shader.SetUniformMat4f("uProj", GetCamera().GetProjMatrix());

            m_ssbo_splat_data->Bind(1);

            GetRenderer().Draw(quad.QuadVA, quad.QuadIdxBuffer, m_sdata.size());
        }

        void Update(GLFWwindow* hwin) override
        {
            if (glfwGetKey(hwin, GLFW_KEY_M) == GLFW_PRESS) m_SceneMenu = true;
        }

        void GUI() override
        {
            if (!m_SceneMenu) return;
            ImGui::Begin("Combination Scene Menu", &m_SceneMenu);

            ImGui::Checkbox("Grid", &m_showGrid);
            ImGui::Checkbox("Axis", &m_showAxis);
            ImGui::Checkbox("Unit length", &m_showUnitlenght);

            ImGui::NewLine();

            if (ImGui::Button("Reload Scene"))
            {
                unload();
                init();
            }

            ImGui::End();
        }


    };

    /*
        3D Gaussian showcase scene.
    */
    class Gaussians3D : public Scene
    {
    private:


        std::vector<SplatData> m_sdata;

        GLuint m_key_buf = 0;
        GLuint m_values_buf = 0;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        std::vector<GLuint> m_key_buffer_data_pre;
        std::vector<GLfloat> m_val_buffer_data_pre;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf3DSpltas = 0;

        const Geometry::Quad quad;

        bool m_SceneMenu = false;
        bool m_DoSort = false;

    public:
        Gaussians3D(Renderer& r, Camera& c) : Scene(r, c)
        {
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DFragShader.GLSL", GL_FRAGMENT_SHADER);
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DVertexShaderInstanced.GLSL", GL_VERTEX_SHADER);
            m_S4DShaderInstanced.BuildShader();
        }
        ~Gaussians3D()
        {
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
        }

        float p(float x, float mu, float sig)
        {
            float e = ((x - mu) / sig);
            return expf(-0.5 * (e * e));
        }

                void init() override
        {
            GetCamera().SetPosition({ -10, 10, 0 });
            GetCamera().SetOrientation({ 1, -1.0, 0 });
            std::cout << "Init: Scenes::Gaussians3D\n";
            m_numOf3DSpltas = 1;
            m_sdata.clear();
            m_sdata.reserve(m_numOf3DSpltas);

            m_key_buffer_data_pre.clear();
            m_key_buffer_data_pre.reserve(m_numOf3DSpltas);
            m_val_buffer_data_pre.clear();
            m_val_buffer_data_pre.reserve(m_numOf3DSpltas);

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf3DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf3DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);


            Splat4D s4d{
                {0.0, 0.0, 0.0, 0.0},
                glm::normalize(glm::quatLookAt(glm::vec3{1.0,0.0,0.0}, {0.0, 1.0, 0.0})),
                {40,20,10},
                1,
                0.5,
                {0,0,0},
                {GetHSLColor(193, 1.0, 0.6), 1.0}
            };                
            m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });
            m_key_buffer_data_pre.push_back(0);
            m_val_buffer_data_pre.push_back(0.0f);
            

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf3DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

            m_sorter = std::make_unique< radix_sort::sorter >(m_numOf3DSpltas);
            m_ssbo_splat_data = std::make_unique< ShareStorageBuffer >(m_sdata.data(), m_numOf3DSpltas * sizeof(SplatData));
            std::cout << "Done Init: Scenes::RotationMotion\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scenes::RotationMotion\n";
            m_ssbo_splat_data.reset();
            m_sorter.reset();
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
            std::cout << "Done unloading Scenes::RotationMotion\n";
        }

        void Render() override
        {

            GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);

            if (m_DoSort)
            {
                for (int i = 0; i < m_numOf3DSpltas; ++i)
                {
                    m_key_buffer_data_pre[i] = i;
                    glm::vec4 tmp = m_sdata[i].GetMeanInTime(0) - glm::vec4(GetCamera().GetPosition(), 1);
                    m_val_buffer_data_pre[i] = 1.0f / sqrtf(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf3DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf3DSpltas * sizeof(GLfloat), m_val_buffer_data_pre.data());

                m_sorter->sort(m_values_buf, m_key_buf, m_numOf3DSpltas);
            }

            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniformMat4f("uView", GetCamera().GetViewMatrix());
            m_S4DShaderInstanced.SetUniformMat4f("uProj", GetCamera().GetProjMatrix());

            GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_key_buf));
            m_ssbo_splat_data->Bind(2);

            GetRenderer().Draw(quad.QuadVA, quad.QuadIdxBuffer, m_numOf3DSpltas);
        }

        void Update(GLFWwindow* hwin) override
        {
            if (glfwGetKey(hwin, GLFW_KEY_M) == GLFW_PRESS) m_SceneMenu = true;
        }

        void GUI() override
        {
            if (!m_SceneMenu) return;
            ImGui::Begin("Combination Scene Menu", &m_SceneMenu);

            if (ImGui::Button("Reload Scene"))
            {
                unload();
                init();
            }


            ImGui::End();
        }
    };

    /*
        4D Gaussian showcase scene.
        A single gaussian that demostrates motion in time and change in opaicty
    */
    class Gaussians4D : public Scene
    {
    private:

        Shader m_S4DShaderInstanced;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;

        std::vector<SplatData> m_sdata;
        std::vector<Splat4D> m_s4ds;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_max_time = 2.0f;
        float m_min_time = -2.0f;
        float m_time_speed = 0.25f;
        float m_min_opacity = 0.0f;

        bool m_loop = true;
        bool m_doTime = false;
        bool m_SceneMenu = false;
        bool m_show_axis = false;

        bool m_show_splat_menu = false;

    public:
        Gaussians4D(Renderer& r, Camera& c) : Scene(r, c)
        {
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DFragShader.GLSL", GL_FRAGMENT_SHADER);
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DVertexShaderMod.GLSL", GL_VERTEX_SHADER);
            m_S4DShaderInstanced.BuildShader();
        }
        ~Gaussians4D()
        {
            m_ssbo_splat_data.reset();
        }

        float p(float x, float mu, float sig)
        {
            float e = ((x - mu) / sig);
            return expf(-0.5 * (e * e));
        }

        void init() override
        {
            GetCamera().SetPosition({ -10, 16, 5 });
            GetCamera().SetOrientation({ 1, -1.0, 0 });
            std::cout << "Init: Scenes::Gaussians4D\n";

            m_s4ds.push_back({glm::vec4{ 0.0, 0.0, 0.0, 0.0 },
                glm::normalize(glm::quatLookAt(glm::vec3{1.0, 0.0, 1.0}, { 0.0, 1.0, 0.0 })),
                glm::vec3{ 10,20,10 },
                1,
                0.5,
                glm::vec3{ 1,1,1 } * 5.0f,
                glm::vec4{ 1.0, 1.0, 1.0, 1.0 } });

            for (Splat4D &s4d : m_s4ds)
            {
                m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });
            }

            m_ssbo_splat_data = std::make_unique< ShareStorageBuffer >(m_sdata.data(), m_s4ds.size() * sizeof(SplatData));

            std::cout << "Done Init: Scenes::RotationMotion\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scenes::RotationMotion\n";
            m_ssbo_splat_data.reset();
            std::cout << "Done unloading Scenes::RotationMotion\n";
        }

        void Render() override
        {

            GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);

            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
            m_S4DShaderInstanced.SetUniform1f("uMinOpacity", m_min_opacity);
            m_S4DShaderInstanced.SetUniformMat4f("uView", GetCamera().GetViewMatrix());
            m_S4DShaderInstanced.SetUniformMat4f("uProj", GetCamera().GetProjMatrix());

            m_ssbo_splat_data->Bind(1);

            GetRenderer().Draw(quad.QuadVA, quad.QuadIdxBuffer, 1);

            if (m_show_axis) 
            {
                for (Splat4D& s4d : m_s4ds)
                {
                    s4d.SetTime(m_time);
                    s4d.DrawAxis(GetRenderer(), GetCamera());
                }
            }
            

        }

        void Update(GLFWwindow* hwin) override
        {
            if (glfwGetKey(hwin, GLFW_KEY_M) == GLFW_PRESS) m_SceneMenu = true;

            if (m_doTime) m_time += m_time_speed;
            if (m_time > m_max_time && m_loop)
            {
                m_time = 0;
            }
            else if (m_time > m_max_time && !m_loop)
            {
                m_doTime = false;
                m_time = m_max_time;
            }
        }

        void GUI() override
        {
            if (!m_SceneMenu) return;
            ImGui::Begin("Combination Scene Menu", &m_SceneMenu);

            ImGui::SliderFloat("Time Speed", &m_time_speed, -2.0f, 2.0f);
            ImGui::InputFloat("Time Max", &m_max_time);
            ImGui::InputFloat("Time Min", &m_min_time);
            ImGui::SliderFloat("Time", &m_time, m_min_time, m_max_time);

            ImGui::Checkbox("Loop", &m_loop);
            ImGui::Checkbox("Show Axis", &m_show_axis);

            ImGui::NewLine();


            if (ImGui::Button("Run"))
                m_doTime = true;
            if (ImGui::Button("Stop"))
                m_doTime = false;
            if (ImGui::Button("Reset"))
                m_time = 0;

            ImGui::NewLine();

            ImGui::InputFloat("m_min_opacity", &m_min_opacity);

            ImGui::NewLine();

            if (ImGui::Button("Reload Scene"))
            {
                unload();
                init();
            }


            ImGui::End();
        }

    };


    /*
        Trying to break motion.
    */
    class BrokenMotion : public Scene
    {
    private:

        std::vector<glm::mat3> m_vModelData;
        std::vector<SplatData> m_sdata;
        std::vector<glm::vec3> m_positions;

        GLuint m_key_buf = 0;
        GLuint m_values_buf = 0;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        std::vector<GLuint> m_key_buffer_data_pre;
        std::vector<GLfloat> m_val_buffer_data_pre;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf4DSpltas = 0;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_max_time = 90.0f;
        float m_time_speed = 0.25f;
        float m_min_opacity = 0.0f;
        float m_Splat_Speed = 1.0f;
        float m_splat_lifetime = 1.0f;
        float m_splat_fade_offset = 0.5f;
        int m_steps_in_time = 92;

        float m_object_scale = 5.0f;
        float m_splat_scale_x = 4.0f;
        float m_splat_scale_y = 4.0f;
        float m_splat_scale_z = 1.0f;

        bool m_loop = true;
        bool m_doTime = false;
        bool m_SceneMenu = false;

        bool m_DoSort = false;

        bool m_showGrid = true;
        bool m_showAxis = true;
        bool m_showUnitlenght = true;
        bool m_showPath = false;

    public:
        BrokenMotion(Renderer& r, Camera& c) : Scene(r, c)
        {
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DFragShader.GLSL", GL_FRAGMENT_SHADER);
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DVertexShaderInstanced.GLSL", GL_VERTEX_SHADER);
            m_S4DShaderInstanced.BuildShader();
        }
        ~BrokenMotion()
        {
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
        }

        void init() override
        {
            GetCamera().SetPosition({ 0, 60, 60 });
            GetCamera().SetOrientation({ 0.0, -1.0, -1.0 });
            std::cout << "Init: Scenes::BrokenMotion\n";
            m_vModelData = VData::parse("../Objects/teapot.vdata");
            m_numOf4DSpltas = m_vModelData.size() * m_steps_in_time;
            m_sdata.clear();
            m_sdata.reserve(m_numOf4DSpltas);

            m_key_buffer_data_pre.clear();
            m_key_buffer_data_pre.reserve(m_numOf4DSpltas);
            m_val_buffer_data_pre.clear();
            m_val_buffer_data_pre.reserve(m_numOf4DSpltas);

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

            ModelEdges medge = GetModelExtrema(m_vModelData);

            unsigned int s_idx = 0;
            for (int dt = 0; dt < m_steps_in_time; ++dt)
            {
                glm::vec3 posdt{1.0f + dt, fmod((1.0f + dt), 20.0f), 0.0f};
                glm::vec3 posdtn{1.0f + (dt + 1.0f), fmod((1.0f + (dt + 1.0f)), 20.0f), 0.0f};
                m_positions.push_back(posdt);
                for (int i = 0; i < m_vModelData.size(); ++i)
                {
                    glm::vec3 pos = m_vModelData[i][0];
                    glm::vec4 forward{ 1.0, 0.0, 0.0, 0.0 };
                    Splat4D s4d{
                        glm::vec4{(m_object_scale* pos) + (posdt), float(dt)},
                        glm::normalize(glm::quatLookAt(glm::normalize(m_vModelData[i][1]), glm::vec3(0,1,0))),
                        glm::vec3{m_splat_scale_x, m_splat_scale_y, m_splat_scale_z},
                        m_splat_lifetime,
                        m_splat_fade_offset,
                        (posdtn - posdt) * m_Splat_Speed,
                        GetColor(pos, medge, m_vModelData[i][1])
                    };

                    m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });
                    m_key_buffer_data_pre.push_back(s_idx);
                    m_val_buffer_data_pre.push_back(0.0f);
                    ++s_idx;
                }
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

            m_sorter = std::make_unique< radix_sort::sorter >(m_numOf4DSpltas);
            m_ssbo_splat_data = std::make_unique< ShareStorageBuffer >(m_sdata.data(), m_numOf4DSpltas * sizeof(SplatData));
            std::cout << "Done Init: Scenes::NonLinearMotion\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scenes::NonLinearMotion\n";
            m_ssbo_splat_data.reset();
            m_sorter.reset();
            m_positions.clear();
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
            std::cout << "Done unloading Scenes::NonLinearMotion\n";
        }

        void Render() override
        {

            if (m_showGrid) GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            if (m_showAxis) GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            if (m_showUnitlenght) GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);

            if (m_showPath)
            {
                GetRenderer().DrawLine(m_positions, { 1.0,0.0,0.0,1.0 }, GetCamera(), 5.0f);
            }

            if (m_DoSort)
            {
                for (int i = 0; i < m_numOf4DSpltas; ++i)
                {
                    m_key_buffer_data_pre[i] = i;
                    glm::vec4 tmp = m_sdata[i].GetMeanInTime(m_time) - glm::vec4(GetCamera().GetPosition(), 1);
                    m_val_buffer_data_pre[i] = 1.0f / sqrtf(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLfloat), m_val_buffer_data_pre.data());

                m_sorter->sort(m_values_buf, m_key_buf, m_numOf4DSpltas);
            }

            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
            m_S4DShaderInstanced.SetUniform1f("uMinOpacity", m_min_opacity);
            m_S4DShaderInstanced.SetUniformMat4f("uView", GetCamera().GetViewMatrix());
            m_S4DShaderInstanced.SetUniformMat4f("uProj", GetCamera().GetProjMatrix());

            GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_key_buf));
            m_ssbo_splat_data->Bind(2);

            GetRenderer().Draw(quad.QuadVA, quad.QuadIdxBuffer, m_numOf4DSpltas);

        }

        void Update(GLFWwindow* hwin) override
        {
            if (glfwGetKey(hwin, GLFW_KEY_M) == GLFW_PRESS) m_SceneMenu = true;

            if (m_doTime) m_time += m_time_speed;
            if (m_time > m_max_time && m_loop)
            {
                m_time = 0;
            }
            else if (m_time > m_max_time && !m_loop)
            {
                m_doTime = false;
                m_time = m_max_time;
            }
        }

        void GUI() override
        {
            if (!m_SceneMenu) return;
            ImGui::Begin("Non Linear Scene Menu", &m_SceneMenu);

            ImGui::SliderFloat("Time Speed", &m_time_speed, -2.0f, 2.0f);
            ImGui::InputFloat("Time Max", &m_max_time);
            ImGui::SliderFloat("Time", &m_time, 0.0f, m_max_time);

            ImGui::Checkbox("Loop", &m_loop);
            ImGui::Checkbox("Sort", &m_DoSort);

            ImGui::NewLine();

            ImGui::Checkbox("Grid", &m_showGrid);
            ImGui::Checkbox("Axis", &m_showAxis);
            ImGui::Checkbox("Unit length", &m_showUnitlenght);
            ImGui::Checkbox("Path", &m_showPath);

            ImGui::NewLine();

            if (ImGui::Button("Run"))
                m_doTime = true;
            if (ImGui::Button("Stop"))
                m_doTime = false;
            if (ImGui::Button("Reset"))
                m_time = 0;

            ImGui::NewLine();

            ImGui::InputFloat("m_Splat_Speed", &m_Splat_Speed);
            ImGui::InputFloat("m_splat_lifetime", &m_splat_lifetime);
            ImGui::InputFloat("m_min_opacity", &m_min_opacity);
            ImGui::InputFloat("m_splat_fade_offset", &m_splat_fade_offset);
            ImGui::InputInt("m_steps_in_time", &m_steps_in_time);

            ImGui::NewLine();

            ImGui::InputFloat("m_object_scale", &m_object_scale);
            ImGui::InputFloat("m_splat_scale_x", &m_splat_scale_x);
            ImGui::InputFloat("m_splat_scale_y", &m_splat_scale_y);
            ImGui::InputFloat("m_splat_scale_z", &m_splat_scale_z);

            ImGui::NewLine();

            if (ImGui::Button("Reload Scene"))
            {
                unload();
                init();
            }


            ImGui::End();
        }

    };

    /*
        Trying to break motion.
    */
    class SquareMotion : public Scene
    {
    private:

        std::vector<glm::mat3> m_vModelData;
        std::vector<SplatData> m_sdata;
        std::vector<glm::vec3> m_positions;

        GLuint m_key_buf = 0;
        GLuint m_values_buf = 0;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        std::vector<GLuint> m_key_buffer_data_pre;
        std::vector<GLfloat> m_val_buffer_data_pre;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf4DSpltas = 0;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_max_time = 90.0f;
        float m_time_speed = 0.25f;
        float m_min_opacity = 0.0f;
        float m_Splat_Speed = 1.0f;
        float m_splat_lifetime = 1.0f;
        float m_splat_fade_offset = 0.5f;
        float m_square_size = 40.0f;
        int m_steps_in_time = 92;

        float m_object_scale = 5.0f;
        float m_splat_scale_x = 4.0f;
        float m_splat_scale_y = 4.0f;
        float m_splat_scale_z = 1.0f;

        bool m_loop = true;
        bool m_doTime = false;
        bool m_SceneMenu = false;

        bool m_DoSort = false;

        bool m_showGrid = true;
        bool m_showAxis = true;
        bool m_showUnitlenght = true;
        bool m_showPath = false;

    public:
        SquareMotion(Renderer& r, Camera& c) : Scene(r, c)
        {
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DFragShader.GLSL", GL_FRAGMENT_SHADER);
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DVertexShaderInstanced.GLSL", GL_VERTEX_SHADER);
            m_S4DShaderInstanced.BuildShader();
        }
        ~SquareMotion()
        {
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
        }

        void init() override
        {
            GetCamera().SetPosition({ 0, 60, 60 });
            GetCamera().SetOrientation({ 0.0, -1.0, -1.0 });
            std::cout << "Init: Scenes::SquareMotion\n";
            m_vModelData = VData::parse("../Objects/teapot.vdata");
            m_numOf4DSpltas = m_vModelData.size() * m_steps_in_time;
            m_sdata.clear();
            m_sdata.reserve(m_numOf4DSpltas);

            m_key_buffer_data_pre.clear();
            m_key_buffer_data_pre.reserve(m_numOf4DSpltas);
            m_val_buffer_data_pre.clear();
            m_val_buffer_data_pre.reserve(m_numOf4DSpltas);

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

            ModelEdges medge = GetModelExtrema(m_vModelData);

            unsigned int s_idx = 0;
            int side = 0;
            int stepsPerSide = m_steps_in_time / 4;
            float deltaStep = m_square_size / float(stepsPerSide);
            glm::vec3 posdt{m_square_size /2.0f,0.0, m_square_size /2.0f}; //starting pos
            glm::vec3 posdtn = glm::vec3{ m_square_size / 2.0f, 0.0, m_square_size / 2.0f} +
                (deltaStep * glm::vec3{-1.0f, 0.0f, 0.0f}); //starting pos next
            for (int dt = 0; dt < m_steps_in_time; ++dt)
            {
                glm::vec3 dir{0};
                if (dt > 0 && dt % stepsPerSide == 0) side += 1;
                if (side == 0) dir = { -1.0f, 0.0f, 0.0f };
                if (side == 1) dir = { 0.0f, 0.0f, -1.0f };
                if (side == 2) dir = { 1.0f, 0.0f, 0.0f };
                if (side == 3) dir = { 0.0f, 0.0f, 1.0f};
                posdt = posdt + (deltaStep * dir);

                int s2 = side;
                if ((dt+1) > 0 && (dt + 1) % stepsPerSide == 0) s2 += 1;
                if (s2 == 0) dir = { -1.0f, 0.0f, 0.0f };
                if (s2 == 1) dir = { 0.0f, 0.0f, -1.0f };
                if (s2 == 2) dir = { 1.0f, 0.0f, 0.0f };
                if (s2 == 3) dir = { 0.0f, 0.0f, 1.0f };
                posdtn = posdtn + (deltaStep * dir);
                m_positions.push_back(posdt);
                for (int i = 0; i < m_vModelData.size(); ++i)
                {
                    glm::vec3 pos = m_vModelData[i][0];
                    glm::vec4 forward{ 1.0, 0.0, 0.0, 0.0 };
                    Splat4D s4d{
                        glm::vec4{(m_object_scale* pos) + (posdt), float(dt)},
                        glm::normalize(glm::quatLookAt(glm::normalize(m_vModelData[i][1]), glm::vec3(0,1,0))),
                        glm::vec3{m_splat_scale_x, m_splat_scale_y, m_splat_scale_z},
                        m_splat_lifetime,
                        m_splat_fade_offset,
                        (posdtn - posdt) * m_Splat_Speed,
                        GetColor(pos, medge, m_vModelData[i][1])
                    };

                    m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });
                    m_key_buffer_data_pre.push_back(s_idx);
                    m_val_buffer_data_pre.push_back(0.0f);
                    ++s_idx;
                }
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

            m_sorter = std::make_unique< radix_sort::sorter >(m_numOf4DSpltas);
            m_ssbo_splat_data = std::make_unique< ShareStorageBuffer >(m_sdata.data(), m_numOf4DSpltas * sizeof(SplatData));
            std::cout << "Done Init: Scenes::SquareMotion\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scenes::SquareMotion\n";
            m_ssbo_splat_data.reset();
            m_sorter.reset();
            m_positions.clear();
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
            std::cout << "Done unloading Scenes::SquareMotion\n";
        }

        void Render() override
        {

            if (m_showGrid) GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            if (m_showAxis) GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            if (m_showUnitlenght) GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);

            if (m_showPath)
            {
                GetRenderer().DrawLine(m_positions, { 1.0,0.0,0.0,1.0 }, GetCamera(), 5.0f);
            }

            if (m_DoSort)
            {
                for (int i = 0; i < m_numOf4DSpltas; ++i)
                {
                    m_key_buffer_data_pre[i] = i;
                    glm::vec4 tmp = m_sdata[i].GetMeanInTime(m_time) - glm::vec4(GetCamera().GetPosition(), 1);
                    m_val_buffer_data_pre[i] = 1.0f / sqrtf(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLfloat), m_val_buffer_data_pre.data());

                m_sorter->sort(m_values_buf, m_key_buf, m_numOf4DSpltas);
            }

            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
            m_S4DShaderInstanced.SetUniform1f("uMinOpacity", m_min_opacity);
            m_S4DShaderInstanced.SetUniformMat4f("uView", GetCamera().GetViewMatrix());
            m_S4DShaderInstanced.SetUniformMat4f("uProj", GetCamera().GetProjMatrix());

            GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_key_buf));
            m_ssbo_splat_data->Bind(2);

            GetRenderer().Draw(quad.QuadVA, quad.QuadIdxBuffer, m_numOf4DSpltas);

        }

        void Update(GLFWwindow* hwin) override
        {
            if (glfwGetKey(hwin, GLFW_KEY_M) == GLFW_PRESS) m_SceneMenu = true;

            if (m_doTime) m_time += m_time_speed;
            if (m_time > m_max_time && m_loop)
            {
                m_time = 0;
            }
            else if (m_time > m_max_time && !m_loop)
            {
                m_doTime = false;
                m_time = m_max_time;
            }
        }

        void GUI() override
        {
            if (!m_SceneMenu) return;
            ImGui::Begin("Square Scene Menu", &m_SceneMenu);

            ImGui::SliderFloat("Time Speed", &m_time_speed, -2.0f, 2.0f);
            ImGui::InputFloat("Time Max", &m_max_time);
            ImGui::SliderFloat("Time", &m_time, 0.0f, m_max_time);

            ImGui::Checkbox("Loop", &m_loop);
            ImGui::Checkbox("Sort", &m_DoSort);

            ImGui::NewLine();

            ImGui::Checkbox("Grid", &m_showGrid);
            ImGui::Checkbox("Axis", &m_showAxis);
            ImGui::Checkbox("Unit length", &m_showUnitlenght);
            ImGui::Checkbox("Path", &m_showPath);

            ImGui::NewLine();

            if (ImGui::Button("Run"))
                m_doTime = true;
            if (ImGui::Button("Stop"))
                m_doTime = false;
            if (ImGui::Button("Reset"))
                m_time = 0;

            ImGui::NewLine();

            ImGui::InputFloat("m_Splat_Speed", &m_Splat_Speed);
            ImGui::InputFloat("m_splat_lifetime", &m_splat_lifetime);
            ImGui::InputFloat("m_min_opacity", &m_min_opacity);
            ImGui::InputFloat("m_splat_fade_offset", &m_splat_fade_offset);
            ImGui::InputInt("m_steps_in_time", &m_steps_in_time);
            ImGui::InputFloat("m_square_size", &m_square_size);

            ImGui::NewLine();

            ImGui::InputFloat("m_object_scale", &m_object_scale);
            ImGui::InputFloat("m_splat_scale_x", &m_splat_scale_x);
            ImGui::InputFloat("m_splat_scale_y", &m_splat_scale_y);
            ImGui::InputFloat("m_splat_scale_z", &m_splat_scale_z);

            ImGui::NewLine();

            if (ImGui::Button("Reload Scene"))
            {
                unload();
                init();
            }


            ImGui::End();
        }

    };


    /*
        Displaying some objects exported from blender
    */
    class ObjectDisplay : public Scene
    {
    private:

        std::vector<VData::VSplatData> m_vModelData;
        std::vector<SplatData> m_sdata;

        GLuint m_key_buf = 0;
        GLuint m_values_buf = 0;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        std::vector<GLuint> m_key_buffer_data_pre;
        std::vector<GLfloat> m_val_buffer_data_pre;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf4DSpltas = 0;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_max_time = 90.0f;
        float m_time_speed = 0.25f;
        float m_min_opacity = 0.0f;
        float m_Splat_Speed = 1.0f;
        float m_splat_lifetime = 1.0f;
        float m_splat_fade_offset = 0.5f;
        float m_square_size = 40.0f;
        int m_steps_in_time = 1;

        float m_object_scale = 1.0f;

        bool m_loop = true;
        bool m_doTime = false;
        bool m_SceneMenu = false;

        bool m_DoSort = true;

        bool m_showGrid = true;
        bool m_showAxis = true;
        bool m_showUnitlenght = true;

    public:
        ObjectDisplay(Renderer& r, Camera& c) : Scene(r, c)
        {
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DFragShader.GLSL", GL_FRAGMENT_SHADER);
            m_S4DShaderInstanced.AddShaderSource("../Shader/Splats4D/Splat4DVertexShaderInstanced.GLSL", GL_VERTEX_SHADER);
            m_S4DShaderInstanced.BuildShader();
        }
        ~ObjectDisplay()
        {
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
        }

        void init() override
        {
            GetCamera().SetPosition({ 0, 60, 60 });
            GetCamera().SetOrientation({ 0.0, -1.0, -1.0 });
            std::cout << "Init: Scenes::ObjectDisplay\n";
            m_vModelData = VData::parse_splat_data("../Objects/Mage.sd");
            m_numOf4DSpltas = m_vModelData.size() * m_steps_in_time;
            m_sdata.clear();
            m_sdata.reserve(m_numOf4DSpltas);

            m_key_buffer_data_pre.clear();
            m_key_buffer_data_pre.reserve(m_numOf4DSpltas);
            m_val_buffer_data_pre.clear();
            m_val_buffer_data_pre.reserve(m_numOf4DSpltas);

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

            ModelEdges medge = GetModelExtrema(m_vModelData);

            unsigned int s_idx = 0;
            for (int dt = 0; dt < m_steps_in_time; ++dt)
            {
                for (int i = 0; i < m_vModelData.size(); ++i)
                {
                    m_sdata.push_back({ glm::vec4{m_object_scale * m_vModelData[i].pos, 0.0}, m_vModelData[i].color, m_vModelData[i].cov });
                    m_key_buffer_data_pre.push_back(s_idx);
                    m_val_buffer_data_pre.push_back(0.0f);
                    ++s_idx;
                }
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

            m_sorter = std::make_unique< radix_sort::sorter >(m_numOf4DSpltas);
            m_ssbo_splat_data = std::make_unique< ShareStorageBuffer >(m_sdata.data(), m_numOf4DSpltas * sizeof(SplatData));
            std::cout << "Done Init: Scenes::ObjectDisplay\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scenes::ObjectDisplay\n";
            m_ssbo_splat_data.reset();
            m_sorter.reset();
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
            std::cout << "Done unloading Scenes::ObjectDisplay\n";
        }

        void Render() override
        {

            if (m_showGrid) GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            if (m_showAxis) GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            if (m_showUnitlenght) GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);

            if (m_DoSort)
            {
                for (int i = 0; i < m_numOf4DSpltas; ++i)
                {
                    m_key_buffer_data_pre[i] = i;
                    glm::vec4 tmp = m_sdata[i].GetMeanInTime(m_time) - glm::vec4(GetCamera().GetPosition(), 1);
                    m_val_buffer_data_pre[i] = 1.0f / sqrtf(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), m_key_buffer_data_pre.data());

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLfloat), m_val_buffer_data_pre.data());

                m_sorter->sort(m_values_buf, m_key_buf, m_numOf4DSpltas);
            }

            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
            m_S4DShaderInstanced.SetUniform1f("uMinOpacity", m_min_opacity);
            m_S4DShaderInstanced.SetUniformMat4f("uView", GetCamera().GetViewMatrix());
            m_S4DShaderInstanced.SetUniformMat4f("uProj", GetCamera().GetProjMatrix());

            GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_key_buf));
            m_ssbo_splat_data->Bind(2);

            GetRenderer().Draw(quad.QuadVA, quad.QuadIdxBuffer, m_numOf4DSpltas);

        }

        void Update(GLFWwindow* hwin) override
        {
            if (glfwGetKey(hwin, GLFW_KEY_M) == GLFW_PRESS) m_SceneMenu = true;

            if (m_doTime) m_time += m_time_speed;
            if (m_time > m_max_time && m_loop)
            {
                m_time = 0;
            }
            else if (m_time > m_max_time && !m_loop)
            {
                m_doTime = false;
                m_time = m_max_time;
            }
        }

        void GUI() override
        {
            if (!m_SceneMenu) return;
            ImGui::Begin("Square Scene Menu", &m_SceneMenu);

            ImGui::SliderFloat("Time Speed", &m_time_speed, -2.0f, 2.0f);
            ImGui::InputFloat("Time Max", &m_max_time);
            ImGui::SliderFloat("Time", &m_time, 0.0f, m_max_time);

            ImGui::Checkbox("Loop", &m_loop);
            ImGui::Checkbox("Sort", &m_DoSort);

            ImGui::NewLine();

            ImGui::Checkbox("Grid", &m_showGrid);
            ImGui::Checkbox("Axis", &m_showAxis);
            ImGui::Checkbox("Unit length", &m_showUnitlenght);

            ImGui::NewLine();

            if (ImGui::Button("Run"))
                m_doTime = true;
            if (ImGui::Button("Stop"))
                m_doTime = false;
            if (ImGui::Button("Reset"))
                m_time = 0;

            ImGui::NewLine();

            ImGui::InputFloat("m_Splat_Speed", &m_Splat_Speed);
            ImGui::InputFloat("m_splat_lifetime", &m_splat_lifetime);
            ImGui::InputFloat("m_min_opacity", &m_min_opacity);
            ImGui::InputFloat("m_splat_fade_offset", &m_splat_fade_offset);
            ImGui::InputInt("m_steps_in_time", &m_steps_in_time);
            ImGui::InputFloat("m_square_size", &m_square_size);

            ImGui::NewLine();

            ImGui::InputFloat("m_object_scale", &m_object_scale);

            ImGui::NewLine();

            if (ImGui::Button("Reload Scene"))
            {
                unload();
                init();
            }


            ImGui::End();
        }

    };
}