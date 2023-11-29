#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
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
    Splat3D(glm::vec3 pos, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, float l0, float l1, float l2) : mPosition(pos)
    {

    }

	~Splat3D() {}

private:
	glm::vec3 mPosition;
    glm::mat3 mSigma;
};


class Splat2D
{
public:
    Splat2D(glm::vec3 pos, glm::vec2 v0, glm::vec2 v1, float l0, float l1, Shader &renderShader, glm::vec3 color) :
        mPosition(pos),
        mBillboard(Geometry::Billboard(pos, glm::vec3(sqrtf(l0), sqrtf(l1), 1.0f))),
        mVertFragShader(renderShader),
        mSigma(glm::mat2(0.0f)),
        ml0{sqrtf(l0)},
        ml1{sqrtf(l1)},
        mv0{ glm::normalize(v0) },
        mv1{ glm::normalize(v1) },
        mColor{color}
    {
        CalcAndSetSigma();
    }

    ~Splat2D() {}

    void CalcAndSetSigma() 
    {
        glm::mat2 S(ml0, 0.0f, 0.0f, ml1);
        glm::mat2 R(mv0, mv1);
        mSigma = glm::inverse(R * S * glm::transpose(S) * glm::transpose(R));
    }

    void Draw(Renderer r, Camera c, float rotAngle) 
    { 
        mVertFragShader.Bind();
        mVertFragShader.SetUniform4f("uColor", mColor.r, mColor.g, mColor.b, 1.0);
        mVertFragShader.SetUniform2f("uScale", ml0, ml1);
        mVertFragShader.SetUniformMat2f("uSigma", mSigma);
        mVertFragShader.SetUniform4f("uSplatPos", mPosition.x, mPosition.y, mPosition.z, 1.0f);
        mVertFragShader.SetUniform1f("uRot", rotAngle);
        mVertFragShader.SetUniformMat4f("uModle", mBillboard.GetTransform());
        mVertFragShader.SetUniformMat4f("uProj", c.GetProjMatrix());
        mVertFragShader.SetUniformMat4f("uView", c.GetViewMatrix());
        mBillboard.Render(r);
    }

    void SetLambas(float l0, float l1) 
    {
        this->ml0 = l0;
        this->ml1 = l1;
        CalcAndSetSigma();
    }

    void SetVectors(glm::vec2 v0, glm::vec2 v1)
    {
        this->mv0 = glm::normalize(v0);
        this->mv1 = glm::normalize(v1);
        CalcAndSetSigma();
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
    Shader mVertFragShader;
    Geometry::Billboard mBillboard;
};

class Splat2DCompute
{
public:
    Splat2DCompute(const Splat2D&) = delete;
    Splat2DCompute(glm::vec3 pos, glm::vec2 v0, glm::vec2 v1, float l0, float l1, Shader& computeShader, Shader& renderShader) :
        mPosition(pos),
        mComputeTexture(Texture((int)(sqrtf(l0) * 10.0f), (int)(sqrtf(l1) * 10.0f), GL_RGBA32F, GL_RGBA, GL_FLOAT)),
        mBillboard(Geometry::Billboard(pos, glm::vec3(sqrtf(l0), sqrtf(l1), 1.0f))),
        mComputeShader(computeShader),
        mVertFragShader(renderShader),
        mSigma(glm::mat2(0.0f)),
        ml0{ sqrtf(l0) },
        ml1{ sqrtf(l1) }
    {
        glm::mat2 S(ml0, 0.0f, 0.0f, ml1);
        glm::mat2 R(glm::normalize(v0), glm::normalize(v1));
        mSigma = glm::inverse(R * S * glm::transpose(S) * glm::transpose(R));

        
        mComputeTexture.BindAsComputeTexture(0);
        mComputeShader.Bind();
        mComputeShader.SetUniformMat2f("uSigma", mSigma);
        mComputeShader.DispatchCompute(mComputeTexture);
        
    }

    ~Splat2DCompute() {}

    void Draw(Renderer r, Camera c, float rotAngle)
    {

        mComputeTexture.Bind(0);
        mVertFragShader.Bind();
        mVertFragShader.SetUniform1i("u_tex0", 0);
        mVertFragShader.SetUniform2f("uScale", ml0, ml1);
        mVertFragShader.SetUniformMat2f("uSigma", mSigma);
        mVertFragShader.SetUniform1f("uRot", rotAngle);
        mVertFragShader.SetUniformMat4f("uModle", mBillboard.GetTransform());
        mVertFragShader.SetUniformMat4f("uProj", c.GetProjMatrix());
        mVertFragShader.SetUniformMat4f("uView", c.GetViewMatrix());
        mBillboard.Render(r);
    }

private:
    float ml0, ml1;
    glm::vec3 mPosition;
    glm::mat2 mSigma;
    Shader mComputeShader;
    Shader mVertFragShader;
    Geometry::Billboard mBillboard;
    Texture mComputeTexture;
};
