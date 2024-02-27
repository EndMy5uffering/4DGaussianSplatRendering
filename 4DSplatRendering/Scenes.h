#pragma once
#include "Scene.h"


namespace Scenes 
{

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


    class LinearMotion : public Scene
    {
    private:
        
        std::vector<glm::mat3> m_vModelData;
        std::vector<SplatData> m_sdata;

        GLuint m_key_buf = 0;
        GLuint m_values_buf = 0;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf4DSpltas = 0;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_time_speed = 0.25f;
        float m_max_time = 50.0f;
        float m_Splat_Speed = 1.0f;
        float m_splat_lifetime = 1.0f;
        float m_splat_fade_offset = 0.5f;
        float m_lin_time_multiplyer = 1.0f;
        int m_steps_in_time = 50;

        bool m_loop = true;
        bool m_doTime = false;
        bool m_SceneMenu = false;

    public:
        LinearMotion(Renderer& r, Camera& c) : Scene(r, c)
        {
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

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

            for (int dt = 0; dt < m_steps_in_time; ++dt)
            {
                for (int i = 0; i < m_vModelData.size(); ++i)
                {
                    glm::vec3 pos = m_vModelData[i][0];
                    glm::vec3 dir{1.0, 0.0, 0.0};
                    glm::vec3 timeOffset = dir * float(dt * m_lin_time_multiplyer);
                    Splat4D s4d{
                        glm::vec4{(5.0f * pos) + timeOffset, float(dt)},
                        glm::normalize(glm::quatLookAt(glm::normalize(m_vModelData[i][1]), glm::vec3(0,1,0))),
                        glm::vec3{1.0,1.0,1.0},
                        m_splat_lifetime,
                        m_splat_fade_offset,
                        glm::normalize(dir) * m_Splat_Speed,
                        glm::clamp(glm::vec4{0.1 + pos.x / 15.0, 0.1 + pos.y / 15.0, 0.1 + pos.z / 15.0, 1.0f},
                            {0.0, 0.0, 0.0, 1.0}, {1.0, 1.0, 1.0, 1.0})
                    };

                    m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });

                }
            }
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
            GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);


            std::vector<GLuint> key_buffer_data_pre(m_numOf4DSpltas);
            std::vector<GLfloat> val_buffer_data_pre(m_numOf4DSpltas);
            for (int i = 0; i < m_numOf4DSpltas; ++i)
            {
                key_buffer_data_pre[i] = i;
                glm::vec4 tmp = m_sdata[i].GetMeanInTime(m_time) - glm::vec4(GetCamera().GetPosition(), 1);
                val_buffer_data_pre[i] = q_rsqrt(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), key_buffer_data_pre.data());

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLfloat), val_buffer_data_pre.data());

            m_sorter->sort(m_values_buf, m_key_buf, m_numOf4DSpltas);


            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
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
            ImGui::Begin("Linear Scene Menu", &m_SceneMenu);

            ImGui::SliderFloat("Time Speed", &m_time_speed, -2.0f, 2.0f);
            ImGui::InputFloat("Time Max", &m_max_time);
            ImGui::SliderFloat("Time", &m_time, 0.0f, m_max_time);

            ImGui::Checkbox("Loop", &m_loop);

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
            ImGui::InputFloat("m_splat_fade_offset", &m_splat_fade_offset);
            ImGui::InputFloat("m_lin_time_multiplyer", &m_lin_time_multiplyer);
            ImGui::InputInt("m_steps_in_time", &m_steps_in_time);

            if (ImGui::Button("Reload Scene"))
            {
                unload();
                init();
            }


            ImGui::End();
        }

    };

    class NonLinearMotion : public Scene
    {
    private:


        std::vector<glm::mat3> m_vModelData;
        std::vector<SplatData> m_sdata;

        GLuint m_key_buf = 0;
        GLuint m_values_buf = 0;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf4DSpltas = 0;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_max_time = 90.0f;
        float m_time_speed = 0.25f;
        float m_Splat_Speed = 20.0f;
        float m_splat_lifetime = 1.0f;
        float m_splat_fade_offset = 0.5f;
        float m_rotation_radius = 20.0f;
        float m_angle_multiplyer = 4.0f;
        int m_steps_in_time = 92;

        bool m_loop = true;
        bool m_doTime = false;
        bool m_SceneMenu = false;

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

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

            for (int dt = 0; dt < m_steps_in_time; ++dt)
            {
                for (int i = 0; i < m_vModelData.size(); ++i)
                {
                    glm::vec3 pos = m_vModelData[i][0];
                    glm::vec4 forward{ 1.0, 0.0, 0.0, 0.0 };
                    glm::vec3 timeOffset = glm::vec3{ glm::rotate(forward, glm::radians(float(dt * m_angle_multiplyer)), {0.0, 1.0, 0.0}) };
                    glm::vec3 timeOffset_next = glm::vec3{ glm::rotate(forward, glm::radians(float((dt+1)* m_angle_multiplyer)), {0.0, 1.0, 0.0}) };
                    Splat4D s4d{
                        glm::vec4{(5.0f * pos) + (timeOffset * m_rotation_radius), float(dt)},
                        glm::normalize(glm::quatLookAt(glm::normalize(m_vModelData[i][1]), glm::vec3(0,1,0))),
                        glm::vec3{1.0,1.0,1.0},
                        m_splat_lifetime,
                        m_splat_fade_offset,
                        (timeOffset_next - timeOffset) * m_Splat_Speed,
                        glm::clamp(glm::vec4{0.1 + pos.x / 15.0, 0.1 + pos.y / 15.0, 0.1 + pos.z / 15.0, 1.0f},
                            {0.0, 0.0, 0.0, 1.0}, {1.0, 1.0, 1.0, 1.0})
                    };

                    m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });

                }
            }
            m_sorter = std::make_unique< radix_sort::sorter >(m_numOf4DSpltas);
            m_ssbo_splat_data = std::make_unique< ShareStorageBuffer >(m_sdata.data(), m_numOf4DSpltas * sizeof(SplatData));
            std::cout << "Done Init: Scenes::NonLinearMotion\n";
        }

        void unload() override
        {
            std::cout << "Unloading Scenes::NonLinearMotion\n";
            m_ssbo_splat_data.reset();
            m_sorter.reset();
            if (m_key_buf) GLCall(glDeleteBuffers(1, &m_key_buf));
            if (m_values_buf) GLCall(glDeleteBuffers(1, &m_values_buf));
            std::cout << "Done unloading Scenes::NonLinearMotion\n";
        }

        void Render() override
        {

            GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);


            std::vector<GLuint> key_buffer_data_pre(m_numOf4DSpltas);
            std::vector<GLfloat> val_buffer_data_pre(m_numOf4DSpltas);
            for (int i = 0; i < m_numOf4DSpltas; ++i)
            {
                key_buffer_data_pre[i] = i;
                glm::vec4 tmp = m_sdata[i].GetMeanInTime(m_time) - glm::vec4(GetCamera().GetPosition(), 1);
                val_buffer_data_pre[i] = q_rsqrt(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), key_buffer_data_pre.data());

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLfloat), val_buffer_data_pre.data());

            m_sorter->sort(m_values_buf, m_key_buf, m_numOf4DSpltas);


            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
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
            ImGui::InputFloat("m_splat_fade_offset", &m_splat_fade_offset);
            ImGui::InputFloat("m_rotation_radius", &m_rotation_radius);
            ImGui::InputFloat("m_angle_multiplyer", &m_angle_multiplyer);
            ImGui::InputInt("m_steps_in_time", &m_steps_in_time);

            if (ImGui::Button("Reload Scene")) 
            {
                unload();
                init();
            }


            ImGui::End();
        }

    };


    class RotationMotion : public Scene
    {
    private:


        std::vector<glm::mat3> m_vModelData;
        std::vector<SplatData> m_sdata;

        GLuint m_key_buf = 0;
        GLuint m_values_buf = 0;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf4DSpltas = 0;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_max_time = 90.0f;
        float m_time_speed = 0.25f;
        float m_Splat_Speed = 5.0f;
        float m_splat_lifetime = 0.6f;
        float m_splat_fade_offset = 0.5f;
        float m_rotation_radius = 20.0f;
        float m_angle_multiplyer = 4.0f;
        int m_steps_in_time = 92;

        bool m_loop = true;
        bool m_doTime = false;
        bool m_SceneMenu = false;

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

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

            for (int dt = 0; dt < m_steps_in_time; ++dt)
            {
                for (int i = 0; i < m_vModelData.size(); ++i)
                {
                    glm::vec4 pos{ m_vModelData[i][0], 0.0 };
                    glm::vec3 timeOffset = glm::vec3{ glm::rotate(pos, glm::radians(float(dt * m_angle_multiplyer)), {0.0, 1.0, 0.0}) };
                    glm::vec3 timeOffset_next = glm::vec3{ glm::rotate(pos, glm::radians(float((dt + 1) * m_angle_multiplyer)), {0.0, 1.0, 0.0}) };
                    Splat4D s4d{
                        glm::vec4{(5.0f * timeOffset), float(dt)},
                        glm::normalize(glm::quatLookAt(glm::normalize(m_vModelData[i][1]), glm::vec3(0,1,0))),
                        glm::vec3{1.0,1.0,1.0},
                        m_splat_lifetime,
                        m_splat_fade_offset,
                        (timeOffset_next - timeOffset) * m_Splat_Speed,
                        glm::clamp(glm::vec4{0.1 + pos.x / 15.0, 0.1 + pos.y / 15.0, 0.1 + pos.z / 15.0, 1.0f},
                            {0.0, 0.0, 0.0, 1.0}, {1.0, 1.0, 1.0, 1.0})
                    };

                    m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });

                }
            }
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

            GetRenderer().DrawGrid(2000, 2000, 200, 200, { 1,1,1,0.15 }, GetCamera(), 1);
            GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);


            std::vector<GLuint> key_buffer_data_pre(m_numOf4DSpltas);
            std::vector<GLfloat> val_buffer_data_pre(m_numOf4DSpltas);
            for (int i = 0; i < m_numOf4DSpltas; ++i)
            {
                key_buffer_data_pre[i] = i;
                glm::vec4 tmp = m_sdata[i].GetMeanInTime(m_time) - glm::vec4(GetCamera().GetPosition(), 1);
                val_buffer_data_pre[i] = q_rsqrt(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), key_buffer_data_pre.data());

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLfloat), val_buffer_data_pre.data());

            m_sorter->sort(m_values_buf, m_key_buf, m_numOf4DSpltas);


            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
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
            ImGui::InputFloat("m_splat_fade_offset", &m_splat_fade_offset);
            ImGui::InputFloat("m_rotation_radius", &m_rotation_radius);
            ImGui::InputFloat("m_angle_multiplyer", &m_angle_multiplyer);
            ImGui::InputInt("m_steps_in_time", &m_steps_in_time);

            if (ImGui::Button("Reload Scene"))
            {
                unload();
                init();
            }


            ImGui::End();
        }

    };

    class CombinedMotion : public Scene
    {
    private:


        std::vector<glm::mat3> m_vModelData;
        std::vector<SplatData> m_sdata;

        GLuint m_key_buf = 0;
        GLuint m_values_buf = 0;

        std::unique_ptr< ShareStorageBuffer > m_ssbo_splat_data;
        std::unique_ptr< radix_sort::sorter > m_sorter;

        Shader m_S4DShaderInstanced;
        unsigned int m_numOf4DSpltas = 0;

        const Geometry::Quad quad;

        float m_time = 0.0f;
        float m_time_speed = 0.25f;

        bool doTime = false;

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
            int steps_in_time = 92;
            std::cout << "Init: Scenes::RotationMotion\n";
            m_vModelData = VData::parse("../Objects/teapot.vdata");
            m_numOf4DSpltas = m_vModelData.size() * steps_in_time;
            m_sdata.clear();
            m_sdata.reserve(m_numOf4DSpltas);

            glGenBuffers(1, &m_key_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, &m_values_buf);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, m_numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

            for (int dt = 0; dt < steps_in_time; ++dt)
            {
                for (int i = 0; i < m_vModelData.size(); ++i)
                {
                    float speed = 5.0f;
                    glm::vec4 pos{ m_vModelData[i][0], 0.0 };
                    glm::vec4 dir{1.0, 0.0, 0.0, 0.0};
                    glm::vec3 movedoff = glm::rotate(dir, (sin(glm::radians(float(dt * 4.0f))) + 1.0f) * 1.0f, { 0.0, 1.0, 0.0 });
                    glm::vec3 timeOffset = glm::vec3{ glm::rotate(pos, glm::radians(float(dt * 4.0f)), {0.0, 1.0, 0.0}) };
                    glm::vec3 timeOffset_next = glm::vec3{ glm::rotate(pos, glm::radians(float((dt + 1) * 4.0f)), {0.0, 1.0, 0.0}) };
                    Splat4D s4d{
                        glm::vec4{(5.0f * m_vModelData[i][0]) + (movedoff * float(dt)), float(dt)},
                        glm::normalize(glm::quatLookAt(glm::normalize(m_vModelData[i][1]), glm::vec3(0,1,0))),
                        glm::vec3{1.0,1.0,1.0},
                        0.6f,
                        0.5f,
                        (timeOffset_next - timeOffset) * speed,
                        glm::clamp(glm::vec4{0.1 + pos.x / 15.0, 0.1 + pos.y / 15.0, 0.1 + pos.z / 15.0, 1.0f},
                            {0.0, 0.0, 0.0, 1.0}, {1.0, 1.0, 1.0, 1.0})
                    };

                    m_sdata.push_back({ s4d.GetPosititon(), s4d.GetColor(), s4d.GetGeoInfo() });

                }
            }
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

            GetRenderer().DrawGrid(2000, 2000, 200, 200, {1,1,1,0.15}, GetCamera(), 1);
            GetRenderer().DrawAxis(GetCamera(), 500.0f, 3.0f);
            GetRenderer().DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, GetCamera(), 5.0f);


            std::vector<GLuint> key_buffer_data_pre(m_numOf4DSpltas);
            std::vector<GLfloat> val_buffer_data_pre(m_numOf4DSpltas);
            for (int i = 0; i < m_numOf4DSpltas; ++i)
            {
                key_buffer_data_pre[i] = i;
                glm::vec4 tmp = m_sdata[i].GetMeanInTime(m_time) - glm::vec4(GetCamera().GetPosition(), 1);
                val_buffer_data_pre[i] = q_rsqrt(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_key_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLuint), key_buffer_data_pre.data());

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_values_buf);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numOf4DSpltas * sizeof(GLfloat), val_buffer_data_pre.data());

            m_sorter->sort(m_values_buf, m_key_buf, m_numOf4DSpltas);


            m_S4DShaderInstanced.Bind();
            m_S4DShaderInstanced.SetUniform1f("uTime", m_time);
            m_S4DShaderInstanced.SetUniformMat4f("uView", GetCamera().GetViewMatrix());
            m_S4DShaderInstanced.SetUniformMat4f("uProj", GetCamera().GetProjMatrix());

            GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_key_buf));
            m_ssbo_splat_data->Bind(2);

            GetRenderer().Draw(quad.QuadVA, quad.QuadIdxBuffer, m_numOf4DSpltas);
        }

        void Update(GLFWwindow* hwin) override
        {
            if (glfwGetKey(hwin, GLFW_KEY_0) == GLFW_PRESS) doTime = true;
            if (glfwGetKey(hwin, GLFW_KEY_9) == GLFW_PRESS) doTime = false;
            if (glfwGetKey(hwin, GLFW_KEY_8) == GLFW_PRESS) m_time = 0.0;

            if (doTime) m_time += m_time_speed;
            if (m_time > 90) m_time = 0;
        }

        void GUI() override {}


    };

}