#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
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
    Splat3D(glm::vec4 pos, glm::quat quaternion, float l0, float l1, float l2, Shader& shader, glm::vec4 color, Camera& cam) : 
        mPosition(pos),
        mQuat{quaternion},
        ml0{l0},
        ml1{l1},
        ml2{l2},
        mShader{shader},
        mColor{color},
        mCam{cam}
    {

    }

	~Splat3D() {}

    void Draw(Renderer r)
    {
        mShader.Bind();

        glm::mat4 view = mCam.GetViewMatrix();
        glm::mat3 view3
        {
            view[0][0], view[0][1], view[0][2],
            view[1][0], view[1][1], view[1][2],
            view[2][0], view[2][1], view[2][2],
        };

        glm::vec4 posCamSpace = view * mPosition;

        float z2 = posCamSpace.z * posCamSpace.z;

        glm::mat3 J
        {
            1.0f / posCamSpace.z, 0.0, -(1.0f * posCamSpace.x) / z2,
                0.0f, 1.0f / posCamSpace.z, -(1.0f * posCamSpace.y) / z2,
                0.0f, 0.0f, 0.0f
        };

        glm::mat3 V
        {
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0
        };

        glm::mat3 T = J * glm::transpose(view3);

        glm::mat3 cov3 = glm::transpose(T) * V * T;

        float a = cov3[0][0];
        float d = cov3[1][1];
        float b = cov3[0][1];

        float aa = a * a;
        float dd = d * d;
        float bb = b * b;

        float f0 = (d + a);
        
        float k = sqrt((aa - 2 * a * b + bb) + (4.0f * dd));

        float l0 = (f0 + k) * 0.5f;
        float l1 = (f0 - k) * 0.5f;

        glm::vec2 diagVec = glm::normalize(glm::vec2{b, l0 - a});
        glm::vec2 v0 = diagVec;
        glm::vec2 v1 = glm::vec2{diagVec.y, -diagVec.x};

        glm::mat2 R{v0, v1};
        glm::mat2 S = glm::diagonal2x2(glm::vec2{ l0, l1 });

        glm::mat2 Sig = glm::inverse(R * S * glm::transpose(S) * glm::transpose(R));

        //TODO: calculate from top 2x2 mat of cov3
        //see: https://github.com/KeKsBoTer/web-splat/blob/master/src/shaders/preprocess.wgsl

        std::stringstream ss;
        ss << Utils::Mat4ToStr("View", view);
        ss << Utils::V4ToStr("Screen_Space_Pos", posCamSpace);
        ss << Utils::Mat3ToStr("J", J);
        ss << Utils::Mat3ToStr("T", T);
        ss << Utils::Mat3ToStr("cov3", cov3);
        ss << "\n";
        ss << "a: " << a << "\n";
        ss << "d: " << d << "\n";
        ss << "b: " << b << "\n";
        ss << "aa: " << aa << "\n";
        ss << "dd: " << dd << "\n";
        ss << "bb: " << bb << "\n";
        ss << "f0: (" << d << " + " << a << ") = " << f0 << "\n";
        ss << "sqrt_inner: (" << aa << " - 2 * " << a << " * " << b << " + " << bb << ") + (4.0f * " << dd << ") = " << (aa - 2 * a * b + bb) + (4.0f * dd) << "\n";
        ss << "l0: " << l0 << "\n";
        ss << "l1: " << l1 << "\n";
        ss << Utils::V2ToStr("V0", v0);
        ss << Utils::V2ToStr("V1", v1);
        ss << Utils::Mat2ToStr("Sig", Sig);
        this->splatdata = ss.str();

        mShader.SetUniform2f("uScale", l0, l1);
        mShader.SetUniform2f("uVec1", v0);
        mShader.SetUniform2f("uVec2", v1);
        mShader.SetUniformMat4f("uCam", mCam.GetViewMatrix());
        mShader.SetUniformMat4f("uProj", mCam.GetProjMatrix());
        //mShader.SetUniformMat4f("uViewProj", mCam.GetProjMatrix());

        mBillboard.Render(r);
    }

    void SetLambas(float l0, float l1, float l2)
    {
        this->ml0 = l0;
        this->ml1 = l1;
        this->ml2 = l2;
    }

    glm::mat4 GetProjMat() 
    {
        return glm::perspective(glm::radians(mCam.GetFOV()), (mCam.GetScreenWidth() / mCam.GetScreenHeight()), 1.0f, mCam.GetFar());
    }

    void SetQuaternion(glm::quat quat)
    {
        this->mQuat = quat;
    }

    void SetPosition(glm::vec4 pos)
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

    std::string GetSplatData() 
    {
        return this->splatdata;
    }

private:
    float ml0, ml1, ml2;
    glm::vec4 mColor;
    glm::quat mQuat;
    glm::vec4 mPosition;
    Shader &mShader;
    Camera& mCam;
    Geometry::Billboard mBillboard;
    std::string splatdata;
};


class Splat2D
{
public:
    Splat2D(glm::vec3 pos, glm::vec2 v0, float l0, float l1, Shader &renderShader, glm::vec4 color) :
        mPosition(pos),
        mBillboard(Geometry::Billboard(pos, glm::vec3(sqrtf(l0), sqrtf(l1), 1.0f))),
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

    void Draw(Renderer r, Camera c) 
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
