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


int SCREEN_WIDTH = 1000;
int SCREEN_HEIGHT = 1000;
Camera cam(SCREEN_WIDTH, SCREEN_HEIGHT, {0.0, 0.0, 5.0});

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

    Shader SimpleBillboardShader;
    SimpleBillboardShader.AddShaderSource("../Shader/SimpleBillboardFrag.GLSL", GL_FRAGMENT_SHADER);
    SimpleBillboardShader.AddShaderSource("../Shader/SimpleBillboardVertex.GLSL", GL_VERTEX_SHADER);
    SimpleBillboardShader.BuildShader();

    Shader S3DShader;
    S3DShader.AddShaderSource("../Shader/Splats3D/Splat3DFragShader.GLSL", GL_FRAGMENT_SHADER);
    S3DShader.AddShaderSource("../Shader/Splats3D/Splat3DVertexShader.GLSL", GL_VERTEX_SHADER);
    S3DShader.BuildShader();

    Shader S4DShader;
    S4DShader.AddShaderSource("../Shader/Splats4D/Splat4DFragShaderFull.GLSL", GL_FRAGMENT_SHADER);
    S4DShader.AddShaderSource("../Shader/Splats4D/Splat4DVertexShaderFull.GLSL", GL_VERTEX_SHADER);
    S4DShader.BuildShader();

    std::vector<Splat4D> splats;

    for (int i = 0; i < 500; ++i) 
    {
        splats.push_back(
            {
                glm::vec4{0.0f, 0.0f, 0.0f, 0.0f},
                glm::normalize(glm::quat(frand(-1.0, 1.0), frand(-1.0, 1.0), frand(-1.0, 1.0), frand(-1.0, 1.0))),
                glm::normalize(glm::quat(frand(-1.0, 1.0), frand(-1.0, 1.0), frand(-1.0, 1.0), frand(-1.0, 1.0))),
                glm::vec4{frand(1.0, 5.0), frand(1.0, 5.0), frand(1.0, 5.0), frand(1.0, 20.0)},
                glm::vec4{frand(0.0, 1.0), frand(0.0, 1.0), frand(0.0, 1.0), 1.0}
            });
    }


    Splat4D s4d{
        glm::vec4{0.0f, 0.0f, 0.0f, 0.0f},
        glm::normalize(glm::quat(1.0f, 1.0f, -1.0f, 0.0f)),
        glm::normalize(glm::quat(-1.0f, 1.0f, 1.0f, 0.0f)),
        glm::vec4{2.0, 1.0, 2.0, 1.0},
        glm::vec4{1.0, 0.0, 0.0, 1.0}
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

    glm::vec3 col{1.0f, 0.0f, 0.0f};
    float idata[] = {
        0.0f, 0.0f, 0.1f, 0.2f,
        0.5f, 0.5f, 0.5f, 0.1f
    };

    /*std::vector<Geometry::Vertex> quads;

    for (float i = 0; i < 2500; ++i) 
    {
        for (float j = 0; j < 2500; ++j)
        {
            std::vector<Geometry::Vertex> quad = MakeQuad({ j , i }, { 1.0f, 1.0f }, { (float)((int)(i * j) * 10 % 255) / 255.0f, (float)((int)(i * j) * 2 % 255) / 255.0f, (float)(int(i * j) * 18 % 255) / 255.0f, 1.0f });
            for (Geometry::Vertex &v : quad)
            {
                quads.push_back(v);
            }
        }
    }

    unsigned int *idxBuff = (unsigned int*)malloc(6 * 2500 * 2500 * sizeof(unsigned int));

    for (int i = 0; i < 6 * 2500 * 2500; i += 6)
    {
        idxBuff[i] = 0 + ((i / 6) * 4);
        idxBuff[i + 1] = 1 + ((i / 6) * 4);
        idxBuff[i + 2] = 2 + ((i / 6) * 4);
        idxBuff[i + 3] = 2 + ((i / 6) * 4);
        idxBuff[i + 4] = 0 + ((i / 6) * 4);
        idxBuff[i + 5] = 3 + ((i / 6) * 4);
    }
    

    VertexArray q0_va{};
    IndexBuffer q0_ib{idxBuff, 6 * 2500 * 2500 };
    VertexBuffer vb{ quads.data(), (int)quads.size() * sizeof(Geometry::Vertex)};
    VertexBufferLayout vb_layout;
    vb_layout.Push<glm::vec2>();
    vb_layout.Push<glm::vec4>();
    q0_va.AddBuffer(vb, vb_layout);*/

    std::vector<Geometry::Splat4DVertex> verts = s4d.MakeMesh();
    std::vector<unsigned int> idxBuff = s4d.GetIdxList(0);

    VertexArray splatVertexArray{};
    VertexBuffer splatVertexBuffer{ verts.data(), (unsigned int)verts.size() * sizeof(Geometry::Splat4DVertex) };
    IndexBuffer splatIdxBuffer{ idxBuff.data(), (unsigned int)idxBuff.size() };
    splatVertexArray.AddBuffer(splatVertexBuffer, s4d.GetBufferLayout());

    double time = 0.0f;
    double timeSpeed = 0.1f;
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        renderer.Clear();
        cam.HandleInput(window, ImGui::IsAnyItemActive());
               

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(blendOpt.selected0, blendOpt.selected1);

        //s3d.Draw(renderer, SplatRenderShader3D, cam);
        //s4d.SetTime(time);
        s4d.Draw(renderer, S3DShader, cam);

        /*for (Splat4D& s : splats)
        {
            s.SetTime(time);
            s.Draw(renderer, S3DShader, cam);
        }*/

        //InstanceShader.Bind();
        //ib.RenderInstnaced(renderer, 2);

        // Draw 4D splats with calulating shader
        /*S4DShader.Bind();
        S4DShader.SetUniform1f("uTime", 0.0f);
        S4DShader.SetUniformMat4f("uView", cam.GetViewMatrix());
        S4DShader.SetUniformMat4f("uProj", cam.GetProjMatrix());
        S4DShader.SetUniform2f("uFocal", cam.GetFocal());
        S4DShader.SetUniform2f("uViewPort", cam.GetViewport());

        renderer.Draw(splatVertexArray, splatIdxBuffer);*/
        // End draw 4d

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        renderer.DrawAxis(cam, 500.0f, 3.0f);
        

        //IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DebugMenus::ShaderEditor(&menu2Data, &menueStripData.show_ShaderEditor);
        DebugMenus::BlendOptMenu(&blendOpt, &menueStripData.show_BlendOpt);
        std::string val = getTextInfo(cam);
        DebugMenus::CamInfo(io, val, &menueStripData.show_CamInfo);
        DebugMenus::MainMenuStrip(&menueStripData);

        //DebugMenus::Splat3DMenu(&s3d, &menueStripData.show_3DSplat);
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