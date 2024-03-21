/*
    Basic implementation of the different Gaussians from 2D - 4D
    These classes are not in use as there were written for better debugging
    The 4D class is only used for parameterization and to calcualte the covariance matrix
    Same with 3D and 2D.
    Most of the stuff in this file is now handled by the shaders.
*/

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/common.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include "Shader.h"
#include "Renderer.h"
#include "Camera.h"
#include "Geometry.h"
#include <math.h>
#include <sstream>
#include "Utils.h"
#include <GLFW/glfw3.h>

#define PI 3.141592f

constexpr unsigned long SPLAT4D_4_VERTEX_SIZE = 4 * sizeof(Geometry::Splat4DVertex);

const float STD_LOWER = 1.3862943611198906f;

namespace SplatUtils 
{
    static float maxf(float a, float b) 
    {
        return a >= b ? a : b;
    }

    static inline glm::mat2 GetUpperMat2(glm::mat3 mat) 
    {
        return { mat[0][0], mat[0][1], mat[1][0], mat[1][1] };
    }

    static glm::vec2 GetEigenValues2x2(glm::mat2 mat) 
    {
        //https://www.youtube.com/watch?v=e50Bj7jn9IQ

        float m = (mat[0][0] + mat[1][1]) * 0.5f;
        float p = (mat[0][0] * mat[1][1]) - (mat[0][1] * mat[1][0]);
        float d = sqrt((m * m) - p);

        return glm::vec2{maxf(m + d, 0.0000001f), maxf(m - d, 0.0000001f)};
    }

    static glm::mat2 GetEigenVectors2x2(const glm::mat2 &mat, glm::vec2 eigenValues) 
    {
        float offdiag = mat[0][1];
        if (offdiag == 0) 
        {
            return glm::mat2{
                glm::normalize(mat[0] / eigenValues[0]), 
                glm::normalize(mat[1] / eigenValues[1])
            };
        }
        glm::vec2 eigenVec0 = glm::normalize(glm::vec2{ 1, (eigenValues[0] - mat[0][0]) / offdiag });
        glm::vec2 eigenVec1 = glm::normalize(glm::vec2{ eigenVec0.y, -eigenVec0.x});

        float dot = ((eigenVec0.x * eigenVec1.x) + (eigenVec0.y * eigenVec1.y));

        //ASSERT(dot > -0.0000001f && dot < 0.0000001f)

        return { glm::normalize(eigenVec0), glm::normalize(eigenVec1) };
    }

    static glm::mat3 vecToMat(glm::vec3 &v0, glm::vec3 &v1) {
        return glm::mat3
        {
            v0[0] * v1,
            v0[1] * v1,
            v0[2] * v1
        };
    }

}

class Splat4D
{
public:
    bool drawHelperAxis = false;


    Splat4D(glm::vec4 pos, glm::quat rot0, glm::quat rot1, glm::vec4 scalar, glm::vec4 color) :
        mPosition{ pos },
        mColor{ color },
        mTimePos{ 0 },
        mGeoInfo{ 0 },
        mDir{ 0 }
    {
        glm::quat normRot0 = glm::normalize(rot0);
        glm::quat normRot1 = glm::normalize(rot1);
        float a = normRot0.w, b = normRot0.x, c = normRot0.y, d = normRot0.z;
        float p = normRot1.w, q = normRot1.x, r = normRot1.y, s = normRot1.z;
        //Spliting of quaternions in left and right rotational matrices
        glm::mat4 mRl
        {
            a, -b, -c, -d,
                b, a, -d, c,
                c, d, a, -b,
                d, -c, b, a
        };

        glm::mat4 mRr
        {
            p, -q, -r, -s,
            q, p, s, -r,
            r, -s, p, q,
            s, r, -q, p
        };

        glm::mat4 Scale4x4{1};
        Scale4x4[0][0] = scalar[0];
        Scale4x4[1][1] = scalar[1];
        Scale4x4[2][2] = scalar[2];
        Scale4x4[3][3] = scalar[3];

        //Forming rotation matrix
        glm::mat4 rot = mRl * mRr;
        mGeoInfo = rot * Scale4x4 * glm::transpose(Scale4x4) * glm::transpose(rot);

        mDir = glm::vec3{ mGeoInfo[0][3], mGeoInfo[1][3] ,mGeoInfo[2][3] } *(1.0f / mGeoInfo[3][3]);
    }

    Splat4D(glm::vec4 pos, glm::quat rot, glm::vec3 scale, float lifeTime, float fadeof, glm::vec3 dir, glm::vec4 color) :
        mPosition{ pos },
        mColor{ color },
        mTimePos{ 0 },
        mGeoInfo{ 0 },
        mDir{ 0 }
    {
        float stddiviation = (lifeTime * lifeTime) / (fadeof == 0.5f ? STD_LOWER : (-2.0 * log(fadeof)));
        glm::vec3 tdir = dir * stddiviation;
        glm::mat3 R = glm::toMat3(rot);
        glm::mat3 S = glm::diagonal3x3(scale);

        glm::mat3 sig = R * S * S * glm::transpose(R);
        glm::mat3 backProj = (1.0f / stddiviation) * glm::outerProduct(tdir, tdir);

        glm::mat3 upper3x3 = sig + backProj;

        glm::mat4 cov{
            glm::vec4(upper3x3[0], tdir.x),
            glm::vec4(upper3x3[1], tdir.y),
            glm::vec4(upper3x3[2], tdir.z),
            glm::vec4(tdir, stddiviation)
        };

        mGeoInfo = cov;

        mDir = glm::vec3{ cov[0][3], cov[1][3] ,cov[2][3] } * (1.0f / cov[3][3]);
    }

    ~Splat4D() {}

    void Draw(GLFWwindow* wnd, Renderer& renderer, Shader& splatShader, Camera& cam)
    {

        //temp vector for calculation
        //in fomula Sigma_1:3,4 and Sigma_4,1:3
        glm::vec3 sig1_3_4{mGeoInfo[0][3], mGeoInfo[1][3], mGeoInfo[2][3]};
        glm::vec3 sig4_1_3{mGeoInfo[3][0], mGeoInfo[3][1], mGeoInfo[3][2]};

        //calcualtion of 3D mu with respect to time
        glm::vec3 mean_time_dependent = glm::vec3{ mPosition } + (sig1_3_4 * (1.0f / mGeoInfo[3][3]) * (mTime - mPosition.w));
        mTimePos = mean_time_dependent;
        //temp mu in the future for drawing the time axis
        glm::vec3 mean_time_dependent_next = glm::vec3{ mPosition } + (sig1_3_4 * (1.0f / mGeoInfo[3][3]) * ((mTime+1.0f) - mPosition.w));
        glm::vec3 tvec = (1.0f / mGeoInfo[3][3]) * sig4_1_3;
        glm::mat3 sig3x3
        {
            glm::vec3(mGeoInfo[0]),
            glm::vec3(mGeoInfo[1]),
            glm::vec3(mGeoInfo[2])
        };

        sig3x3 = sig3x3 - glm::outerProduct(sig1_3_4, tvec);;

        //From this point on calculation and drawing of 3D splat formula was only copied to save time
        //This part and the part above will be moved into the shader later on

        glm::mat4 view = cam.GetViewMatrix();
        glm::mat3 W
        {
            glm::vec3(view[0]),
            glm::vec3(view[1]),
            glm::vec3(view[2]),
        };
        W = glm::transpose(W);

        glm::vec4 posCamSpace = view * glm::vec4{glm::vec3(mean_time_dependent), 1.0};
        glm::vec4 posScreenSpace = cam.GetProjMatrix() * posCamSpace;
        posScreenSpace = posScreenSpace * (1.0f / posScreenSpace.w);

        float z2 = posCamSpace.z * posCamSpace.z;

        glm::vec2 focal = cam.GetFocal();

        glm::mat3 J
        {
            1.0f / posCamSpace.z, 0.0, -posCamSpace.x / z2,
                0.0f, 1.0f / posCamSpace.z, -posCamSpace.y / z2,
                0.0f, 0.0f, 0.0f
        };

        glm::mat3 T = W * J;

        glm::mat3 cov3 = glm::transpose(T) * sig3x3 * T;
        glm::mat2 upper = SplatUtils::GetUpperMat2(cov3);

        glm::vec2 lambdas = SplatUtils::GetEigenValues2x2(upper);
        float l0 = sqrt(lambdas.x);
        float l1 = sqrt(lambdas.y);

        glm::mat2 eigenVecs = SplatUtils::GetEigenVectors2x2(upper, lambdas);

        glm::vec2 v0 = glm::normalize(eigenVecs[0] / cam.GetViewport());
        glm::vec2 v1 = glm::normalize(eigenVecs[1] / cam.GetViewport());
        glm::mat2 R{v0, v1};
        glm::mat2 S{l0, 0.0f, 0.0f, l1};
        glm::mat2 _Sig = glm::inverse(R * S * glm::transpose(S) * glm::transpose(R));

        float z = posScreenSpace.z / posScreenSpace.w;
        float bound = 1.2 * posScreenSpace.w;

        if (z < 0. || z > 1. || posScreenSpace.x < -bound || posScreenSpace.x > bound || posScreenSpace.y < -bound || posScreenSpace.y > bound)
        {
            return;
        }
        splatShader.Bind();
        splatShader.SetUniformMat2f("uSigma", _Sig);
        splatShader.SetUniform2f("uScale", glm::vec2(l0, l1));
        splatShader.SetUniform2f("uVec1", v0);
        splatShader.SetUniform2f("uVec2", v1);
        splatShader.SetUniform2f("uScreenPos", posScreenSpace.x, posScreenSpace.y);
        splatShader.SetUniformMat4f("uProj", cam.GetProjMatrix());
        splatShader.SetUniform4f("uColor", this->p(mTime, mPosition.w, mGeoInfo[3][3]) * mColor);

        //mBillboard.Render(renderer);
    }


    void DrawAxis(Renderer& renderer, Camera& cam) 
    {
        glm::vec3 mean_t = GetMeanInTime();
        glm::vec3 dir = mDir;
        glm::vec3 dir_n = glm::normalize(dir);
        renderer.DrawLine(mean_t - dir_n * 10000.0f, mean_t + dir_n * 10000.0f, glm::vec4{1.0f, 0.0f, 1.0f, 1.0f}, cam, 2.0f);
        //renderer.DrawLine(mean_t - dir_n * 10000.0f, mean_t + dir_n * 10000.0f, glm::vec4{0.133f, 0.18431f, 0.24314f, 1.0f}, cam, 2.0f);
        renderer.DrawLine(mean_t, mean_t + dir, glm::vec4{0.0f, 0.82352f, 0.82745f, 1.0f}, cam, 5.0f);
    }

    inline glm::vec4 GetMeanInTime() 
    {
        float ctime = (mTime - mPosition.w);
        float x = mPosition.x + mDir.x * ctime;
        float y = mPosition.y + mDir.y * ctime;
        float z = mPosition.z + mDir.z * ctime;

        return glm::vec4{x, y, z, 1};
    }

    inline glm::vec4 GetMeanInTime(float off)
    {
        float ctime = ((mTime + off) - mPosition.w);
        float x = mPosition.x + mDir.x * ctime;
        float y = mPosition.y + mDir.y * ctime;
        float z = mPosition.z + mDir.z * ctime;

        return glm::vec4{x, y, z, 1};
    }

    float GetTime()
    {
        return this->mTime;
    }

    float p(float t, float mu, float sig) 
    {
        return expf(-0.5f * (t - mu) * (1.0f / sig) * (t - mu));
    }

    void SetTime(float t) 
    {
        this->mTime = t;
    }

    glm::vec4 GetColor() 
    {
        return this->mColor;
    }

    void SetColor(glm::vec4 color) 
    {
        this->mColor = color;
    }

    glm::vec4 GetPosititon() 
    {
        return this->mPosition;
    }

    void SetPosition(glm::vec4 p) 
    {
        this->mPosition = p;
    }

    glm::mat4 GetGeoInfo() 
    {
        return this->mGeoInfo;
    }


private:
    glm::vec4 mColor;
    glm::vec4 mPosition;
    glm::vec3 mTimePos;
    glm::mat4 mGeoInfo;
    glm::vec3 mDir;
    std::vector<Geometry::Splat4DVertex> mverts;
    float mTime = 0.0f;
};

class Splat3D
{
public:
    Splat3D(glm::vec4 pos, glm::quat rot, glm::vec3 scale, glm::vec4 color) :
        mPosition(pos),
        mRot{ rot },
        mScale{ scale },
        mColor{ color }
    {
        glm::mat3 tscale = glm::diagonal3x3(mScale);
        glm::mat3 trot = glm::toMat3(mRot);
        mGeoInfo = trot * tscale * tscale * glm::transpose(trot);
        //Utils::Mat3Print(mGeoInfo);
    }

	~Splat3D() {}

    void Draw(Renderer& r, Shader& splatShader, Camera& cam)
    {
        splatShader.Bind();

        //This hole calculation part will be moved into the shader later on
        glm::mat4 view = cam.GetViewMatrix();
        glm::mat3 W
        {
            glm::vec3(view[0]),
            glm::vec3(view[1]),
            glm::vec3(view[2]),
        };
        W = glm::transpose(W);

        glm::vec4 posCamSpace = view * mPosition;
        glm::vec4 posScreenSpace = cam.GetProjMatrix() * posCamSpace;

        posScreenSpace = posScreenSpace * (1.0f / posScreenSpace.w);

        float z2 = posCamSpace.z * posCamSpace.z;

        glm::vec2 focal = cam.GetFocal();

        glm::mat3 J
        {
            1.0f / posCamSpace.z, 0.0,                  -posCamSpace.x / z2,
            0.0f,                 1.0f / posCamSpace.z, -posCamSpace.y / z2,
            0.0f,                 0.0f,                 0.0f
        };

        glm::mat3 tscale = glm::diagonal3x3(mScale);
        glm::mat3 trot = glm::toMat3(mRot);
        glm::mat3 V = trot * tscale * tscale * glm::transpose(trot);

        glm::mat3 T = W * J;

        glm::mat3 cov3 = glm::transpose(T) * V * T;
        glm::mat2 upper = SplatUtils::GetUpperMat2(cov3);
        
        glm::vec2 lambdas = SplatUtils::GetEigenValues2x2(upper);
        float l0 = sqrt(lambdas.x);
        float l1 = sqrt(lambdas.y);

        glm::mat2 eigenVecs = SplatUtils::GetEigenVectors2x2(upper, lambdas);

        glm::vec2 v0 = glm::normalize(eigenVecs[0] ) / cam.GetViewport();
        glm::vec2 v1 = glm::normalize(eigenVecs[1] ) / cam.GetViewport();
        glm::mat2 R{v0, v1};
        glm::mat2 S{l0, 0.0f, 0.0f, l1};
        glm::mat2 Sig = glm::inverse(R * S * glm::transpose(S) * glm::transpose(R));

        float z = posScreenSpace.z / posScreenSpace.w;
        float bound = 1.2 * posScreenSpace.w;
        if (z < 0. || z > 1. || posScreenSpace.x < -bound || posScreenSpace.x > bound || posScreenSpace.y < -bound || posScreenSpace.y > bound) 
        {
            return;
        }

        splatShader.SetUniformMat2f("uSigma", Sig);
        splatShader.SetUniform2f("uScale", glm::vec2(l0 , l1));
        splatShader.SetUniform2f("uVec1", v0);
        splatShader.SetUniform2f("uVec2", v1);
        splatShader.SetUniform2f("uScreenPos", posScreenSpace.x, posScreenSpace.y);
        splatShader.SetUniformMat4f("uProj", cam.GetProjMatrix());
        splatShader.SetUniform4f("uColor", mColor);

        //mBillboard.Render(r);

        glm::vec3 lv0(mPosition.x, mPosition.y, mPosition.z);
        glm::vec3 lv1(trot[0] * mScale[0]);
        glm::vec3 lv2(trot[1] * mScale[1]);
        glm::vec3 lv3(trot[2] * mScale[2]);
        r.DrawLine(lv0, lv0 + lv1, glm::vec4{1.0, 0.0, 0.0, 1.0}, cam);
        r.DrawLine(lv0, lv0 + lv2, glm::vec4{0.0, 1.0, 0.0, 1.0}, cam);
        r.DrawLine(lv0, lv0 + lv3, glm::vec4{0.0, 0.0, 1.0, 1.0}, cam);

    }

    static std::vector<Geometry::Splat3DVertex> GetSplatMesh(glm::vec4 pos, glm::quat rot, glm::vec3 scale, glm::vec4 color)
    {
        glm::mat3 tscale = glm::diagonal3x3(scale);
        glm::mat3 trot = glm::toMat3(rot);
        glm::mat3 geoInfo = trot * tscale * tscale * glm::transpose(trot);

        std::vector<Geometry::Splat3DVertex> vertices;
        vertices.reserve(4);
        vertices.push_back({ {  0.5f,  0.5f }, pos, color, geoInfo });
        vertices.push_back({ {  0.5f, -0.5f }, pos, color, geoInfo });
        vertices.push_back({ { -0.5f, -0.5f }, pos, color, geoInfo });
        vertices.push_back({ { -0.5f,  0.5f }, pos, color, geoInfo });

        return vertices;
    }

    inline std::vector<Geometry::Splat3DVertex> MakeMesh()
    {
        std::vector<Geometry::Splat3DVertex> vertices;
        vertices.reserve(4);
        vertices.push_back({ {  0.5f,  0.5f }, mPosition, mColor, mGeoInfo });
        vertices.push_back({ {  0.5f, -0.5f }, mPosition, mColor, mGeoInfo });
        vertices.push_back({ { -0.5f, -0.5f }, mPosition, mColor, mGeoInfo });
        vertices.push_back({ { -0.5f,  0.5f }, mPosition, mColor, mGeoInfo });

        return vertices;

    }

    inline void MakeMesh(VertexBuffer& vb, unsigned int offset)
    {
        std::vector<Geometry::Splat3DVertex> vertices;
        vertices.reserve(4);
        vertices.push_back({ {  0.5f,  0.5f }, mPosition, mColor, mGeoInfo });
        vertices.push_back({ {  0.5f, -0.5f }, mPosition, mColor, mGeoInfo });
        vertices.push_back({ { -0.5f, -0.5f }, mPosition, mColor, mGeoInfo });
        vertices.push_back({ { -0.5f,  0.5f }, mPosition, mColor, mGeoInfo });

        vb.SubData(offset, vertices.data(), vertices.size() * sizeof(Geometry::Splat3DVertex));

    }

    static std::vector<unsigned int> GetIdxList(unsigned int offset)
    {
        std::vector<unsigned int> idxBuff;
        idxBuff.reserve(6);
        idxBuff.push_back(0 + offset);
        idxBuff.push_back(2 + offset);
        idxBuff.push_back(1 + offset);
        idxBuff.push_back(2 + offset);
        idxBuff.push_back(0 + offset);
        idxBuff.push_back(3 + offset);
        return idxBuff;
    }

    static const VertexBufferLayout GetBufferLayout()
    {
        VertexBufferLayout layout;
        layout.Push<glm::vec2>();
        layout.Push<glm::vec3>();
        layout.Push<glm::vec4>();
        layout.Push<glm::mat3>();

        return layout;
    }

    void SetLambas(glm::vec3 scale)
    {
        this->mScale = scale;
    }

    void SetQuaternion(glm::quat rot)
    {
        this->mRot = rot;
    }

    glm::quat GetQuaternion() 
    {
        return this->mRot;
    }

    glm::vec3 GetScale() 
    {
        return this->mScale;
    }

    void SetPosition(glm::vec4 pos)
    {
        this->mPosition = pos;
    }

    glm::vec4 GetPosition()
    {
        return mPosition;
    }

    void SetColor(glm::vec4 color)
    {
        this->mColor = color;
    }

    glm::vec4 GetColor() 
    {
        return this->mColor;
    }

private:
    glm::vec3 mScale;
    glm::vec4 mColor;
    glm::quat mRot;
    glm::vec4 mPosition;
    //Geometry::Billboard mBillboard;
    glm::mat3 mGeoInfo;
};


class Splat2D
{
public:
    Splat2D(glm::vec3 pos, glm::vec2 v0, float l0, float l1, glm::vec4 color) :
        mPosition(pos),
        mSigma(glm::mat2(0.0f)),
        ml0{sqrtf(l0)},
        ml1{sqrtf(l1)},
        mv0{ glm::normalize(v0) },
        mv1{ glm::normalize(glm::vec2(v0.y, -v0.x)) },
        mColor{color}
    {
        CalcAndSetSigma();
    }

    ~Splat2D() {}

    void CalcAndSetSigma() 
    {
        glm::mat2 S(ml0, 0.0f, 0.0f, ml1);
        glm::mat2 R(mv0, mv1);
        glm::mat2 sig = R * S * glm::transpose(S) * glm::transpose(R);
        mSigma = glm::inverse(sig);
    }

    void Draw(Renderer& r, Shader& shader, Camera c) 
    { 
        shader.Bind();
        shader.SetUniform4f("uColor", mColor.r, mColor.g, mColor.b, 1.0);
        shader.SetUniform2f("uScale", ml0, ml1);
        shader.SetUniform2f("uVec1", mv0);
        shader.SetUniform2f("uVec2", mv1);
        shader.SetUniformMat2f("uSigma", mSigma);
        shader.SetUniform4f("uSplatPos", mPosition.x, mPosition.y, mPosition.z, 1.0f);
        shader.SetUniformMat4f("uProj", c.GetProjMatrix());
        shader.SetUniformMat4f("uView", c.GetViewMatrix());
    }

    void SetLambas(float l0, float l1) 
    {
        this->ml0 = sqrtf(l0);
        this->ml1 = sqrtf(l1);
        CalcAndSetSigma();
    }

    void SetVectors(glm::vec2 v0)
    {
        this->mv0 = glm::normalize(v0);
        this->mv1 = glm::normalize(glm::vec2(v0.y, -v0.x));
        CalcAndSetSigma();
    }

    glm::vec2 GetVector() 
    {
        return mv0;
    }

    void SetPosition(glm::vec3 pos) 
    {
        this->mPosition = pos;
    }

    glm::vec3 GetPosition() 
    {
        return mPosition;
    }

    void SetColor(glm::vec4 color) 
    {
        this->mColor = color;
    }

    float GetLambda0() 
    {
        return ml0;
    }

    float GetLambda1() 
    {
        return ml1;
    }

    glm::vec4 GetColor() 
    {
        return mColor;
    }

private:
    float ml0, ml1;
    glm::vec4 mColor;
    glm::vec2 mv0;
    glm::vec2 mv1;
    glm::vec3 mPosition;
    glm::mat2 mSigma;
};
