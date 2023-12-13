#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include "Shader.h"
#include "Renderer.h"
#include "Camera.h"
#include "Geometry.h"
#include <math.h>

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
    Splat3D(glm::vec4 pos, glm::vec3 v0, glm::vec3 v1, float l0, float l1, float l2, Shader shader, Camera& cam) : 
        mPosition(pos),
        mw0{v0},
        mw1{v1},
        mw2{glm::vec4(0.0f)},
        ml0{l0},
        ml1{l1},
        ml2{l2},
        mShader{shader},
        mCam{cam}
    {
        CalcAndSetSigma();
    }

	~Splat3D() {}

    void CalcAndSetSigma()
    {
        this->mw0 = glm::normalize(this->mw0);
        glm::vec3 tmpW2 = glm::normalize(glm::cross(this->mw0, this->mw1));
        glm::vec3 tmpW1 = glm::normalize(glm::cross(this->mw0, tmpW2));
        this->mw2 = tmpW2;
        this->mw1 = tmpW1;

        glm::mat4 modle = glm::mat4
        {
            mw0.x * ml0, mw0.y, mw0.z, 0.0f,
            mw1.x, mw1.y * ml1, mw1.z, 0.0f,
            mw2.x, mw2.y, mw2.z * ml2, 0.0f,
            mPosition.x, mPosition.y, mPosition.z, mPosition.w
        };

        this->mModle = modle;
    }

    void Draw(Renderer r, Camera c)
    {
        mShader.Bind();

        glm::mat4 modleView = this->mModle * this->mCam.GetViewMatrix();
        glm::vec3 U{modleView[3][0], modleView[3][1], modleView[3][2]};
        float l = glm::l2Norm(U);
        glm::mat3 J{
            1.0f / U.z, 0.0, U.x / l,
            0.0f, 1.0f / U.z, U.y / l,
            -U.x / (U.z * U.z), -U.y / (U.z * U.z), U.z / l
        };
        glm::mat3 W{
            modleView[0][0], modleView[0][1], modleView[0][2],
            modleView[1][0], modleView[1][1], modleView[1][2],
            modleView[2][0], modleView[2][1], modleView[2][2],
        };

        glm::mat3 T3D = J * W;
        glm::mat2 Vhat{
            (T3D[0][0] * T3D[0][0]) + (T3D[0][1] * T3D[0][1]), (T3D[0][0] * T3D[0][1]) + (T3D[0][1] * T3D[1][1]),
            (T3D[0][0] * T3D[1][0]) + (T3D[0][1] * T3D[1][1]), (T3D[1][0] * T3D[1][0]) + (T3D[1][1] * T3D[1][1])
        };


        mBillboard.Render(r);
    }

    void SetLambas(float l0, float l1, float l2)
    {
        this->ml0 = l0;
        this->ml1 = l1;
        this->ml2 = l2;
        CalcAndSetSigma();
    }

    void SetVectors(glm::vec3 v0, glm::vec3 v1)
    {
        this->mw0 = v0;
        this->mw1 = v1;
        CalcAndSetSigma();
    }

    void SetPosition(glm::vec4 pos)
    {
        this->mPosition = pos;
    }

    glm::vec3 GetPosition()
    {
        return mPosition;
    }

    void SetColor(glm::vec3 color)
    {
        this->mColor = color;
    }

private:
    float ml0, ml1, ml2;
    glm::vec3 mColor;
    glm::vec3 mw0;
    glm::vec3 mw1;
    glm::vec3 mw2;
    glm::vec4 mPosition;
    glm::mat4 mModle;
    Shader &mShader;
    Camera& mCam;
    Geometry::Billboard mBillboard;
};


class Splat2D
{
public:
    Splat2D(glm::vec3 pos, glm::vec2 v0, float l0, float l1, Shader &renderShader, glm::vec3 color) :
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

    void SetColor(glm::vec3 color) 
    {
        this->mColor = color;
    }

private:
    float ml0, ml1;
    glm::vec3 mColor;
    glm::vec2 mv0;
    glm::vec2 mv1;
    glm::vec3 mPosition;
    glm::mat2 mSigma;
    Shader &mVertFragShader;
    Geometry::Billboard mBillboard;
};
