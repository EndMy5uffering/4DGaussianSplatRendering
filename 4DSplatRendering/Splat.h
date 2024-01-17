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

#define PI 3.141592f
//#define LCalc(mat) (2.0f * powf(PI, 3.f/2.f) * sqrtf(glm::determinant(mat)))

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

        return glm::vec2{m + d, m - d};
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

    static glm::vec3 Swap(const glm::vec3 v, int from, int to)
    {
        glm::vec3 vc(v);

        float temp = vc[from];
        vc[from] = vc[to];
        vc[to] = temp;

        return vc;
    }

    static glm::mat3 SwapRow(const glm::mat3 m, int from, int to)
    {
        glm::mat3 mc(m);

        glm::vec3 temp = m[from];
        mc[from] = mc[to];
        mc[to] = temp;

        return mc;
    }

    static glm::vec3 SolveSystem3x3(const glm::mat3& a, const glm::vec3& b)
    {
        //TODO: implement that :D
        glm::mat3 ac(a);
        glm::vec3 bc(b);

        //Pivo search
        int len = 3, row = 0;
        for (int i = 0; i < len; ++i) {
            row = i;
            double tmp_max = 0;
            for (int j = i; j < len; ++j) {
                if (abs(ac[j][i]) > tmp_max) {
                    tmp_max = abs(ac[j][i]);
                    row = j;
                }
            }

            if (i < row) {
                //swap row of matrix and vector
                bc = Swap(bc, i, row);
                ac = SwapRow(ac, i, row);
            }

            double factor;
            for (int j = i + 1; j < len; ++j) {
                factor = ac[j][i] / ac[i][i];
                for (int k = i; k < len; ++k) {
                    ac[j][k] -= factor * ac[i][k];
                }

                bc[j] -= factor * bc[i];
            }
        }

        //back substitution
        glm::vec3 result{0.0f};
        for (int i = len - 1; i >= 0; --i) {
            result[i] = bc[i] / ac[i][i];
            for (int j = i - 1; j >= 0; --j) {
                bc[j] -= (ac[j][i] * result[i]);
            }
        }

        return result;
    }

}

class Splat4D
{
public:
    Splat4D(glm::vec3 pos, glm::vec4 v0, glm::vec4 v1, glm::vec4 v2, glm::vec4 v3, float l0, float l1, float l2, float l3) : mPosition(pos)
    {

    }

    ~Splat4D() {}

private:
    glm::vec3 mPosition;
    glm::mat4 mSigma;
};

class Splat3D
{
public:
    Splat3D(glm::vec4 pos, glm::quat rot, glm::vec3 scale, Shader& shader, glm::vec4 color, Camera& cam) : 
        mPosition(pos),
        mRot{ rot },
        mScale{ scale },
        mShader{shader},
        mColor{color},
        mCam{cam}
    {

    }

	~Splat3D() {}

    void Draw(Renderer& r)
    {
        mShader.Bind();

        glm::mat4 view = mCam.GetViewMatrix();
        glm::mat3 W
        {
            glm::vec3(view[0]),
            glm::vec3(view[1]),
            glm::vec3(view[2]),
        };
        W = glm::transpose(W);

        glm::vec4 posCamSpace = view * mPosition;
        glm::vec4 posScreenSpace = mCam.GetProjMatrix() * posCamSpace;

        posScreenSpace = posScreenSpace * (1.0f / posScreenSpace.w);

        float z2 = posCamSpace.z * posCamSpace.z;

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

        glm::vec2 v0 = glm::normalize(eigenVecs[0] / mCam.GetViewport());
        glm::vec2 v1 = glm::normalize(eigenVecs[1] / mCam.GetViewport());
        glm::mat2 R{v0, v1};
        glm::mat2 S{l0, 0.0f, 0.0f, l1};
        glm::mat2 Sig = glm::inverse(R * S * glm::transpose(S) * glm::transpose(R));

        float z = posScreenSpace.z / posScreenSpace.w;
        float bound = 1.2 * posScreenSpace.w;
        if (z < 0. || z > 1. || posScreenSpace.x < -bound || posScreenSpace.x > bound || posScreenSpace.y < -bound || posScreenSpace.y > bound) 
        {
            return;
        }

        mShader.SetUniformMat2f("uSigma", Sig);
        mShader.SetUniform2f("uScale", glm::vec2(l0, l1));
        mShader.SetUniform2f("uVec1", v0);
        mShader.SetUniform2f("uVec2", v1);
        mShader.SetUniform2f("uScreenPos", posScreenSpace.x, posScreenSpace.y);
        mShader.SetUniformMat4f("uProj", mCam.GetProjMatrix());
        mShader.SetUniform4f("uColor", mColor);

        mBillboard.Render(r);

        glm::vec3 lv0(mPosition.x, mPosition.y, mPosition.z);
        glm::vec3 lv1(trot[0] * mScale[0]);
        glm::vec3 lv2(trot[1] * mScale[1]);
        glm::vec3 lv3(trot[2] * mScale[2]);
        r.DrawLine(lv0, lv0 + lv1, glm::vec4{1.0, 0.0, 0.0, 1.0}, mCam);
        r.DrawLine(lv0, lv0 + lv2, glm::vec4{0.0, 1.0, 0.0, 1.0}, mCam);
        r.DrawLine(lv0, lv0 + lv3, glm::vec4{0.0, 0.0, 1.0, 1.0}, mCam);
        /*
        glm::vec2 screenPos(0.0, 0.0);
        r.DrawLine(screenPos, screenPos + (v0 * 0.5f), glm::vec4{1.0, 1.0, 1.0, 1.0});
        r.DrawLine(screenPos, screenPos + (v0 * lambdas.x), glm::vec4{1.0, 0.0, 1.0, 1.0});
        r.DrawLine(screenPos, screenPos + (v1 * 0.5f), glm::vec4{1.0, 1.0, 1.0, 1.0});
        r.DrawLine(screenPos, screenPos + (v1 * lambdas.y), glm::vec4{0.0, 1.0, 1.0, 1.0});
        */

    }

    void SetLambas(float l0, float l1, float l2)
    {
        this->mScale = {l0, l1, l2};
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
    Shader& mShader;
    Camera& mCam;
    Geometry::Billboard mBillboard;
};


class Splat2D
{
public:
    Splat2D(glm::vec3 pos, glm::vec2 v0, float l0, float l1, Shader &renderShader, glm::vec4 color) :
        mPosition(pos),
        mBillboard{ pos, glm::vec3(sqrtf(l0), sqrtf(l1), 1.0f) },
        mVertFragShader(renderShader),
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

    void Draw(Renderer& r, Camera c) 
    { 
        mVertFragShader.Bind();
        mVertFragShader.SetUniform4f("uColor", mColor.r, mColor.g, mColor.b, 1.0);
        mVertFragShader.SetUniform2f("uScale", ml0, ml1);
        mVertFragShader.SetUniform2f("uVec1", mv0);
        mVertFragShader.SetUniform2f("uVec2", mv1);
        mVertFragShader.SetUniformMat2f("uSigma", mSigma);
        mVertFragShader.SetUniform4f("uSplatPos", mPosition.x, mPosition.y, mPosition.z, 1.0f);
        mVertFragShader.SetUniformMat4f("uProj", c.GetProjMatrix());
        mVertFragShader.SetUniformMat4f("uView", c.GetViewMatrix());
        mBillboard.Render(r);
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

private:
    float ml0, ml1;
    glm::vec4 mColor;
    glm::vec2 mv0;
    glm::vec2 mv1;
    glm::vec3 mPosition;
    glm::mat2 mSigma;
    Shader &mVertFragShader;
    Geometry::Billboard mBillboard;
};
