#define DEBUG_OUTPUT

#include<GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stb_image.h>
#include <algorithm>
#include <chrono>

#include "Camera.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "Texture.h"

#include "Geometry.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/quaternion.hpp>

#include "Splat.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "DebugMenus.h"

#include "Utils.h"


int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 800;
Camera cam(SCREEN_WIDTH, SCREEN_HEIGHT, { 0, 0.0, 100.0 }, glm::normalize(glm::vec3{0.0, 0.0, -1.0}));


std::vector<Geometry::Vertex> MakeQuad(const glm::vec2 pos, const glm::vec2 scale, const glm::vec4 color) 
{
    std::vector<Geometry::Vertex> out;
    out.push_back({ glm::vec2{pos.x + (0.5f * scale.x), pos.y + (0.5f * scale.y)}, color });
    out.push_back({ glm::vec2{pos.x + (0.5f * scale.x), pos.y - (0.5f * scale.y)}, color });
    out.push_back({ glm::vec2{pos.x - (0.5f * scale.x), pos.y - (0.5f * scale.y)}, color });
    out.push_back({ glm::vec2{pos.x - (0.5f * scale.x), pos.y + (0.5f * scale.y)}, color });
    return out;
}

float frand(float from, float to)
{
    //std::srand(std::time(nullptr));
    return from + ((to - from) * ((float)std::rand() / (float)RAND_MAX));
}

glm::vec2 getVecFromAngle(float ang) 
{
    float _ang = ang * PI / 180.0f;
    return glm::vec2{ cos(_ang), sin(_ang) };
}

std::string getTextInfo(Camera cam) 
{
    glm::mat4 vmat = cam.GetViewMatrix();
    glm::mat4 projmat = cam.GetProjMatrix();
    std::string vmats = Utils::Mat4ToStr("View", vmat);
    std::string projMats = Utils::Mat4ToStr("Proj", projmat);
    return vmats + "\n" + projMats;
}


void updateScreenSize(GLFWwindow* window, int width, int height) 
{
    SCREEN_WIDTH = width;
    SCREEN_HEIGHT = height;
    cam.Resize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_MAXIMIZED, 1);
    //glfwWindowHint(GLFW_SAMPLES, 16);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "4D Gaussian Splats", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window); 
    //glfwSwapInterval(0); //Disable vsync

    if (glewInit() != GLEW_OK) 
    {
        std::cout << "[Error]: Glew init" << std::endl;
    }
    std::cout << glGetString(GL_VERSION) << std::endl;

    glfwSetWindowSizeCallback(window, updateScreenSize);
    //IMGUI

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    //IMGUI
    
    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    Renderer renderer;
    glEnable(GL_DEPTH_TEST);
    
    /* Loop until the user closes the window */
    glClearColor(0.34901960784313724f, 0.3843137254901961f, 0.4588235294117647f,1.0f);
    cam.SetFar(5000.0f);

    Geometry::Box background(glm::vec3(0.0f), glm::vec3(5000.0f));

    Shader S3DShader;
    S3DShader.AddShaderSource("../Shader/Splats3D/Splat3DFragShader.GLSL", GL_FRAGMENT_SHADER);
    S3DShader.AddShaderSource("../Shader/Splats3D/Splat3DVertexShader.GLSL", GL_VERTEX_SHADER);
    S3DShader.BuildShader();

    Shader S3DShaderFull;
    S3DShaderFull.AddShaderSource("../Shader/Splats3D/Splat3DFragShaderFull.GLSL", GL_FRAGMENT_SHADER);
    S3DShaderFull.AddShaderSource("../Shader/Splats3D/Splat3DVertexShaderFull.GLSL", GL_VERTEX_SHADER);
    S3DShaderFull.BuildShader();

    Shader S4DShader;
    S4DShader.AddShaderSource("../Shader/Splats4D/Splat4DFragShader.GLSL", GL_FRAGMENT_SHADER);
    S4DShader.AddShaderSource("../Shader/Splats4D/Splat4DVertexShader.GLSL", GL_VERTEX_SHADER);
    S4DShader.BuildShader();

    Splat4D s4d{
        glm::vec4{0.0f, 0.0f, 0.0f, 0.0f},
        glm::normalize(glm::quat(1.0f, 1.0f, -1.0f, 0.0f)),
        glm::normalize(glm::quat(-1.0f, 1.0f, 1.0f, 0.0f)),
        glm::vec4{2.0, 1.0, 2.0, 1.0},
        glm::vec4{1.0, 0.0, 0.0, 1.0}
    };

    Splat3D s3d
    {
        glm::vec4{0.0f, 0.0f, 0.0f, 1.0f},
        glm::quatLookAt(glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0}),
        glm::vec3{10.0f, 3.0f, 1.0f},
        glm::vec4{0.0f, 1.0f, 0.0f, 1.0f}
    };

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    DebugMenus::Menu02Data menu2Data;
    menu2Data.buffer = "This could be your shader :D";

    DebugMenus::BlendOpt blendOpt;
    blendOpt.selected0 = GL_ONE;
    blendOpt.selected1 = GL_ONE_MINUS_SRC_ALPHA;

    DebugMenus::MenueStripData menueStripData;
    menueStripData.cam = &cam;

    int splatNum = 500;
    std::vector<Splat3D> splats;
    splats.reserve(splatNum);
    std::vector<unsigned int> idxBuff3d;
    idxBuff3d.reserve(6 * splatNum);
    float splat_pos_range = 50;

    for (int i = 0; i < splatNum; ++i) 
    {
        splats.push_back(Splat3D{
            50.0f * glm::normalize(glm::vec4{frand(-splat_pos_range, splat_pos_range), frand(-splat_pos_range, splat_pos_range), frand(-splat_pos_range, splat_pos_range), 0.0}),
            glm::quatLookAt(glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0f, 1.0f, 0.0}),
            {20.0, 20.0, 1.0},
            {frand(0.0, 1.0), frand(0.0, 1.0), frand(0.0, 1.0), 1.0}
            });

        std::vector<unsigned int> idx = Splat3D::GetIdxList(i * 4);
        idxBuff3d.insert(idxBuff3d.end(), std::make_move_iterator(idx.begin()), std::make_move_iterator(idx.end()));

    }

    VertexArray splatVertexArray3D{};
    VertexBuffer splatVertexBuffer3D{ nullptr, (unsigned int)splats.size() * 4 * sizeof(Geometry::Splat3DVertex) };
    IndexBuffer splatIdxBuffer3D{ idxBuff3d.data(), (unsigned int)idxBuff3d.size() };
    splatVertexArray3D.AddBuffer(splatVertexBuffer3D, s3d.GetBufferLayout());


    double time = 0.0f;
    double timeSpeed = 0.1f;
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        renderer.Clear();
        cam.HandleInput(window, ImGui::IsAnyItemActive());
        cam.SetIsViewFixedOnPoint(true, glm::vec4(0.0, 0.0, 0.0, 0.0));


        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(blendOpt.selected0, blendOpt.selected1);

        //s3d.Draw(renderer, S3DShader, cam);
        //s4d.SetTime(time);
        //s4d.Draw(window, renderer, S3DShader, cam);

        // Draw 4D splats with calulating shader
        /*S4DShader.Bind();
        S4DShader.SetUniform1f("uTime", 0.0f);
        S4DShader.SetUniformMat4f("uView", cam.GetViewMatrix());
        S4DShader.SetUniformMat4f("uProj", cam.GetProjMatrix());
        S4DShader.SetUniform2f("uFocal", cam.GetFocal());
        S4DShader.SetUniform2f("uViewPort", cam.GetViewport());

        renderer.Draw(splatVertexArray, splatIdxBuffer);*/
        // End draw 4d

        // Draw 3D splats with calulating shader
        S3DShaderFull.Bind();
        S3DShaderFull.SetUniformMat4f("uView", cam.GetViewMatrix());
        S3DShaderFull.SetUniformMat4f("uProj", cam.GetProjMatrix());

        std::sort(splats.begin(), splats.end(), [](Splat3D& s0, Splat3D& s1) {
            glm::vec4 proj0 = cam.GetViewProjMatrix() * s0.GetPosition();
            glm::vec4 proj1 = cam.GetViewProjMatrix() * s1.GetPosition();
            return proj0.w > proj1.w;
        });
        
        for (int i = 0; i < splats.size(); ++i) 
        {
            splats[i].MakeMesh(splatVertexBuffer3D, i * 4 * sizeof(Geometry::Splat3DVertex));
        }

        renderer.Draw(splatVertexArray3D, splatIdxBuffer3D);


        // End draw 3d
        
        renderer.DrawAxis(cam, 500.0f, 3.0f);
        renderer.DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, {1.0, 1.0, 1.0, 1.0}, cam, 5.0f);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        

        //IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DebugMenus::ShaderEditor(&menu2Data, &menueStripData.show_ShaderEditor);
        DebugMenus::BlendOptMenu(&blendOpt, &menueStripData.show_BlendOpt);
        std::string val = getTextInfo(cam);
        DebugMenus::CamInfo(io, val, &menueStripData.show_CamInfo);
        DebugMenus::MainMenuStrip(&menueStripData);

        DebugMenus::Splat3DMenu(&s3d, &menueStripData.show_3DSplat);
        DebugMenus::Splat4DMenu(&s4d, &cam, &menueStripData.show_4DSplat);
        //IMGUI

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        GLCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCall(glfwPollEvents());

        time += timeSpeed;
        if (time > 10 || time < -10)
            timeSpeed *= -1;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    //free(idxBuff);

    glfwTerminate();
    return 0;
}