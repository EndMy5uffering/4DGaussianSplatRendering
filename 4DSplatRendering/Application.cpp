#define DEBUG_OUTPUT

#define SPLAT4D_DRAW
//#define SPLAT3D_DRAW

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
#include <functional>

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
#include <glm/common.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "Splat.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "DebugMenus.h"

#include "Utils.h"

#include "radix_sort.hpp"

#include "BSPTree.h"


int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 800;
Camera cam(SCREEN_WIDTH, SCREEN_HEIGHT, { 100, 100, 100 }, glm::normalize(glm::vec3{-1.0, -1.0, -1.0}));

constexpr unsigned long SIZE_OF_SPLAT4D = sizeof(Geometry::Splat4DVertex);
constexpr unsigned long SIZE_OF_4_SPLAT4D = 4 * sizeof(Geometry::Splat4DVertex);

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
    glfwSwapInterval(0); //Disable vsync

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
    
    /* Loop until the user closes the window */
    glClearColor(0.34901960784313724f, 0.3843137254901961f, 0.4588235294117647f,1.0f);
    cam.SetFar(5000.0f);

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

    Splat2D s2d{
        glm::vec3{0,0,0},
        glm::vec2{1,0},
        1,1,
        glm::vec4{1,1,1,1}
    };

    //Splat4D s4d{
    //    glm::vec4{0.0f, 0.0f, 0.0f, 0.0f},
    //    glm::normalize(glm::quat(2.0f, 4.0f, -2.0f, 1.0f)),
    //    glm::normalize(glm::quat(3.0f, -1.0f, 1.0f, -2.0f)),
    //    glm::vec4{10.0, 20.0, 20.0, 10.0},
    //    glm::vec4{1.0, 0.0, 0.0, 1.0}
    //};

    Splat4D s4d{
        glm::vec4{0.0f, 0.0f, 0.0f, 0.0f},
        glm::quatLookAt(glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{0.0f,1.0f,0.0f}),
        glm::vec3{25.0f, 10.0f, 5.0f},
        70.0f,
        glm::vec3{50.0f, 50.0f, 0.0f},
        glm::vec4{0.0f, 1.0f, 0.961f, 1.0f}
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
    blendOpt.selected0 = GL_SRC_ALPHA;
    blendOpt.selected1 = GL_ONE_MINUS_SRC_ALPHA;

    DebugMenus::MenueStripData menueStripData;
    menueStripData.cam = &cam;

#ifdef SPLAT3D_DRAW
    int splatNum = 0;
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
    splatVertexArray3D.AddBuffer(splatVertexBuffer3D, Splat3D::GetBufferLayout());

#endif // SPLAT3D_DRAW

#ifdef SPLAT4D_DRAW
    const int numOf4DSpltas = 5000;
    std::vector<Splat4D> splats4D;
    splats4D.reserve(numOf4DSpltas);
    std::vector<unsigned int> idxBuff4d;
    idxBuff4d.reserve(6 * numOf4DSpltas);

    GLuint key_buf;
    glGenBuffers(1, &key_buf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, key_buf);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, numOf4DSpltas * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);

    GLuint values_buf;
    glGenBuffers(1, &values_buf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, values_buf);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, numOf4DSpltas * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);


    //splats4D.push_back(s4d);
    //std::vector<unsigned int> idx4D = Splat4D::GetIdxList(0);
    //idxBuff4d.insert(idxBuff4d.end(), std::make_move_iterator(idx4D.begin()), std::make_move_iterator(idx4D.end()));


    float splat_pos_range = 50;
    for (int i = 0; i < numOf4DSpltas; ++i)
    {
        //glm::quat q0 = glm::quatLookAt(glm::vec3{frand(-1.0, 1.0), frand(-1.0, 1.0), frand(-1.0, 1.0)}, glm::vec3{0.0f, 1.0f, 0.0});
        //glm::quat q1 = glm::quatLookAt(glm::vec3{-q0.x, -q0.y, -q0.z}, glm::vec3{0.0f, 1.0f, 0.0});
        //glm::vec4 pos, glm::quat rot0, glm::quat rot1, glm::vec4 scalar, glm::vec4 color
        glm::vec3 pos{frand(-splat_pos_range, splat_pos_range), frand(-splat_pos_range, splat_pos_range), frand(-splat_pos_range, splat_pos_range)};
        splats4D.push_back(Splat4D{
            50.0f * glm::normalize(glm::vec4{pos, 0.0f}),
            glm::normalize(glm::quatLookAt(glm::normalize(pos), glm::vec3(0,1,0))),
            glm::vec3{20.0,20.0,20.0},
            500.0f,
            glm::normalize(pos.x > 0 ? glm::vec3{1.0, 0.0, 0.0} : glm::vec3{-1.0, 0.0, 0.0}) * 500.0f,
            glm::vec4{frand(0.0f, 1.0f), frand(0.0f, 1.0f), frand(0.0f, 1.0f), 1.0f}
            });

        std::vector<unsigned int> idx4D = Splat4D::GetIdxList(i * 4);
        idxBuff4d.insert(idxBuff4d.end(), std::make_move_iterator(idx4D.begin()), std::make_move_iterator(idx4D.end()));
    }
    radix_sort::sorter sorter(numOf4DSpltas);

    VertexArray splatVertexArray4D{};
    VertexBuffer splatVertexBuffer4D{ nullptr, (unsigned int)splats4D.size() * SIZE_OF_4_SPLAT4D };
    IndexBuffer splatIdxBuffer4D{ idxBuff4d.data(), (unsigned int)idxBuff4d.size() };
    splatVertexArray4D.AddBuffer(splatVertexBuffer4D, Splat4D::GetBufferLayout());

    for (int i = 0; i < splats4D.size(); ++i)
    {
        splats4D[i].MakeMesh(splatVertexBuffer4D, i * SIZE_OF_4_SPLAT4D);
    }


    VertexArray singleSplat4DVA{};
    VertexBuffer singleSplat4DVB{nullptr, SIZE_OF_4_SPLAT4D };
    std::vector<unsigned int> singleSplatIndexList = Splat4D::GetIdxList(0);
    IndexBuffer singleSplat4DIB{ singleSplatIndexList.data(), 6};
    s4d.MakeMesh(singleSplat4DVB, 0);

    singleSplat4DVA.AddBuffer(singleSplat4DVB, Splat4D::GetBufferLayout());

#endif // SPLAT4D_DRAW

    double time = 0.0f;
    double timeSpeed = 0.25f;
    bool doTime = false;
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        renderer.Clear();
        cam.HandleInput(window, ImGui::IsAnyItemActive());
        cam.SetIsViewFixedOnPoint(true, glm::vec4(0.0, 0.0, 0.0, 0.0));


        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(blendOpt.selected0, blendOpt.selected1);


        renderer.DrawAxis(cam, 500.0f, 3.0f);
        renderer.DrawLine({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, cam, 5.0f);


        //s3d.Draw(renderer, S3DShader, cam);

        // Draw 4D splats with calulating shader
#ifdef SPLAT4D_DRAW


        /*std::sort(std::begin(idxListForSorting), std::end(idxListForSorting), [&splats4D](unsigned int s0, unsigned int s1) {
            glm::vec4 proj0 = cam.GetViewProjMatrix() * splats4D[s0].GetMeanInTime();
            glm::vec4 proj1 = cam.GetViewProjMatrix() * splats4D[s1].GetMeanInTime();
            return proj0.w > proj1.w;
        });*/
        /*std::vector<GLuint> key_buffer_data_pre(numOf4DSpltas);
        std::vector<GLfloat> val_buffer_data_pre(numOf4DSpltas);
        for (int i = 0; i < numOf4DSpltas; ++i)
        {
            splats4D[i].SetTime(time);
            key_buffer_data_pre[i] = i;
            val_buffer_data_pre[i] = (cam.GetViewProjMatrix() * splats4D[i].GetMeanInTime()).z;
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, key_buf);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numOf4DSpltas * sizeof(GLuint), key_buffer_data_pre.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, values_buf);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numOf4DSpltas * sizeof(GLfloat), val_buffer_data_pre.data());

        sorter.sort(values_buf, key_buf, numOf4DSpltas);

        std::vector<GLuint> key_buf_data(numOf4DSpltas);
        std::vector<GLfloat> val_buf_data(numOf4DSpltas);
        GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, key_buf));
        GLCall(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numOf4DSpltas * sizeof(GLuint), key_buf_data.data()));
        GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, values_buf));
        GLCall(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numOf4DSpltas * sizeof(GLfloat), val_buf_data.data()));

        int localidx = 0;
        for(size_t i = key_buf_data.size()-1; i != -1; --i)
        {
            splats4D[key_buf_data[i]].MakeMesh(splatVertexBuffer4D, localidx * 4 * sizeof(Geometry::Splat4DVertex));
            localidx += 1;
        }*/
        
        int localidx = 0;
        glm::vec4 vp = cam.GetViewProjMatrix()[2];
        std::function<bool(Splat4D*, Splat4D*)> treeSorter = [&vp](Splat4D* nSplat, Splat4D* current)
        {
            //glm::vec3{ mPosition } + (sig1_3_4 * (1.0f / mGeoInfo[3][3]) * (mTime - mPosition.w));
            float nPosTime = glm::dot(nSplat->GetMeanInTime(), vp);
            float currentPos = glm::dot(current->GetMeanInTime(), vp);
            return nPosTime >= currentPos;
            //return nSplat->GetMeanInTime().z >= current->GetMeanInTime().z;
        };

        BSPTree<Splat4D> tree{treeSorter, splats4D};
        std::function<void(Splat4D*)> op = [&localidx, &splatVertexBuffer4D](Splat4D* c)
        {
            c->MakeMesh(splatVertexBuffer4D, localidx * SIZE_OF_4_SPLAT4D);
            localidx += 1;
        };
        tree.BackToFront(op);


        S4DShader.Bind();
        S4DShader.SetUniform1f("uTime", time);
        S4DShader.SetUniformMat4f("uView", cam.GetViewMatrix());
        S4DShader.SetUniformMat4f("uProj", cam.GetProjMatrix());

        renderer.Draw(splatVertexArray4D, splatIdxBuffer4D);


        //s4d.SetTime(time);
        //s4d.Draw(window, renderer, S3DShader, cam);
        //renderer.Draw(singleSplat4DVA, singleSplat4DIB);
        //s4d.DrawAxis(renderer, cam);

#endif
        // End draw 4d

        // Draw 3D splats with calulating shader
#ifdef SPLAT3D_DRAW
        S3DShaderFull.Bind();
        S3DShaderFull.SetUniformMat4f("uView", cam.GetViewMatrix());
        S3DShaderFull.SetUniformMat4f("uProj", cam.GetProjMatrix());

        /*std::sort(splats.begin(), splats.end(), [](Splat3D& s0, Splat3D& s1) {
            glm::vec4 proj0 = cam.GetViewProjMatrix() * s0.GetPosition();
            glm::vec4 proj1 = cam.GetViewProjMatrix() * s1.GetPosition();
            return proj0.w > proj1.w;
        });*/
        
        /*for (int i = 0; i < splats.size(); ++i)
        {
            splats[i].MakeMesh(splatVertexBuffer3D, i * 4 * sizeof(Geometry::Splat3DVertex));
        }*/

        renderer.Draw(splatVertexArray3D, splatIdxBuffer3D);
#endif

        // End draw 3d
        
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        

        //IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DebugMenus::ShaderEditor(&menu2Data, &menueStripData.show_ShaderEditor);
        DebugMenus::BlendOptMenu(&blendOpt, &menueStripData.show_BlendOpt);
        std::string val = getTextInfo(cam);
        DebugMenus::CamInfo(io, val, cam, &menueStripData.show_CamInfo);
        DebugMenus::MainMenuStrip(&menueStripData);

        DebugMenus::Splat2DMenu(&s2d, &menueStripData.show_2DSplat);
        DebugMenus::Splat3DMenu(&s3d, &menueStripData.show_3DSplat);
        DebugMenus::Splat4DMenu(&s4d, &cam, &menueStripData.show_4DSplat);
        //IMGUI

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        GLCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCall(glfwPollEvents());

        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) doTime = true;
        if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) doTime = false;
        if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) time = 0.0;

        if(doTime) time += timeSpeed;
        //if (time > 100 || time < -100)
        //    timeSpeed *= -1;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    //free(idxBuff);

    glfwTerminate();
    return 0;
}