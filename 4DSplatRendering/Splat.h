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

        return glm::vec2{maxf(m + d, 0.00001f), maxf(m - d, 0.00001f)};
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

    static glm::mat3 vecToMat(glm::vec3 &v0, glm::vec3 &v1) {
        return glm::mat3
        {
            v0[0] * v1,
            v0[1] * v1,
            v0[2] * v1
        };
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
    Splat4D(glm::vec4 pos, glm::quat rot0, glm::quat rot1, glm::vec4 scalar, glm::vec4 color) :
        mPosition(pos),
        mRot0{ rot0 },
        mRot1{ rot1 },
        mColor{ color },
        mScale{ scalar }
    {

    }

    ~Splat4D() {}

    void Draw(Renderer& renderer, Shader& splatShader, Camera& cam) 
    {

        float a = mRot0.x, b = mRot0.y, c = mRot0.z, d = mRot0.w;
        float p = mRot1.x, q = mRot1.y, r = mRot1.z, s = mRot1.w;
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

        mR = mRl * mRr;

        glm::mat4 sig = mR * glm::diagonal4x4(mScale) * glm::diagonal4x4(mScale) * glm::transpose(mR);

        glm::vec3 sig1_3_4{sig[0][3], sig[1][3], sig[2][3]};
        glm::vec3 sig4_1_3{sig[3][0], sig[3][1], sig[3][2]};

        glm::vec3 scale_time_dependent = glm::vec3{ mScale } + (sig1_3_4 * (1.0f / sig[3][3]) * (mPosition.w - mScale[3]));
        glm::vec3 tvec = (1.0f / sig[3][3]) * sig4_1_3;
        glm::mat3 subPart = SplatUtils::vecToMat(sig1_3_4 , tvec);
        glm::mat3 sig3x3
        {
            glm::normalize(glm::vec3(sig[0][0] - subPart[0][0], sig[0][1] - subPart[0][1], sig[0][2] - subPart[0][2])),
            glm::normalize(glm::vec3(sig[1][0] - subPart[1][0], sig[1][1] - subPart[1][1], sig[1][2] - subPart[1][2])),
            glm::normalize(glm::vec3(sig[2][0] - subPart[2][0], sig[2][1] - subPart[2][1], sig[2][2] - subPart[2][2]))
        };

        glm::mat4 view = cam.GetViewMatrix();
        glm::mat3 W
        {
            glm::vec3(view[0]),
                glm::vec3(view[1]),
                glm::vec3(view[2]),
        };
        W = glm::transpose(W);

        glm::vec4 posCamSpace = view * glm::vec4{glm::vec3(mPosition), 1.0};
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

        glm::mat3 tscale = glm::diagonal3x3(scale_time_dependent);
        glm::mat3 V = sig3x3 * tscale * tscale * glm::transpose(sig3x3);

        glm::mat3 T = W * J;

        glm::mat3 cov3 = glm::transpose(T) * V * T;
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
        splatShader.SetUniform4f("uColor", mColor);

        mBillboard.Render(renderer);

        glm::vec4 c0{0.87843f, 0.33725f, 0.99215f, 1.0f};
        glm::vec4 c1{0.40784f, 0.42745f, 0.87843f, 1.0f};
        glm::vec4 c2{0.58431f, 0.68627f, 0.75294f, 1.0f};
        glm::vec4 c0_0{0.41568f, 0.69019f, 0.29803f, 1.0f};
        glm::vec3 splatPos{mPosition};
        renderer.DrawLine(splatPos, splatPos + sig3x3[0], c0, cam, 5.0f);
        renderer.DrawLine(splatPos, splatPos + sig3x3[1], c1, cam, 5.0f);
        renderer.DrawLine(splatPos, splatPos + sig3x3[2], c2, cam, 5.0f);
        renderer.DrawLine(splatPos, splatPos + glm::vec3{a, b, c}, c0_0, cam, 5.0f);
        renderer.DrawLine(splatPos, splatPos + glm::vec3{p, q, r}, c0_0, cam, 5.0f);
    }

    float GetTime()
    {
        return this->mPosition.w;
    }

    void SetTime(float t) 
    {
        this->mPosition.w = t;
    }

    glm::vec4 GetColor() 
    {
        return this->mColor;
    }

    void SetColor(glm::vec4 color) 
    {
        this->mColor = color;
    }

    glm::quat GetQuat0() 
    {
        return this->mRot0;
    }

    void SetQuat0(glm::quat q) 
    {
        this->mRot0 = q;
    }

    glm::quat GetQuat1() 
    {
        return this->mRot1;
    }

    void SetQuat1(glm::quat q) 
    {
        this->mRot1 = q;
    }

    glm::vec4 GetScale() 
    {
        return this->mScale;
    }

    void SetScale(glm::vec4 s) 
    {
        this->mScale = s;
    }

    glm::vec4 GetPosititon() 
    {
        return this->mPosition;
    }

    void SetPosition(glm::vec4 p) 
    {
        this->mPosition = p;
    }


private:
    glm::vec4 mScale;
    glm::vec4 mColor;
    glm::quat mRot0;
    glm::quat mRot1;
    glm::vec4 mPosition;
    Geometry::Billboard mBillboard;
    glm::mat4 mR;
};

class Splat3D
{
public:
    Splat3D(glm::vec4 pos, glm::quat rot, glm::vec3 scale, glm::vec4 color) : 
        mPosition(pos),
        mRot{ rot },
        mScale{ scale },
        mColor{color}
    {

    }

	~Splat3D() {}

    void Draw(Renderer& r, Shader& splatShader, Camera& cam)
    {
        splatShader.Bind();

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

        glm::vec2 v0 = glm::normalize(eigenVecs[0] / cam.GetViewport());
        glm::vec2 v1 = glm::normalize(eigenVecs[1] / cam.GetViewport());
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

        mBillboard.Render(r);

        glm::vec3 lv0(mPosition.x, mPosition.y, mPosition.z);
        glm::vec3 lv1(trot[0] * mScale[0]);
        glm::vec3 lv2(trot[1] * mScale[1]);
        glm::vec3 lv3(trot[2] * mScale[2]);
        r.DrawLine(lv0, lv0 + lv1, glm::vec4{1.0, 0.0, 0.0, 1.0}, cam);
        r.DrawLine(lv0, lv0 + lv2, glm::vec4{0.0, 1.0, 0.0, 1.0}, cam);
        r.DrawLine(lv0, lv0 + lv3, glm::vec4{0.0, 0.0, 1.0, 1.0}, cam);

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
