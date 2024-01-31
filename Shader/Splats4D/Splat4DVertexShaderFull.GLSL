#version 440 core

layout(location = 0) in vec2 iVPos;
layout(location = 1) in vec4 iSPos;
layout(location = 2) in vec4 iScale;
layout(location = 3) in mat4 iSig;
layout(location = 4) in vec4 iColor;

uniform float uTime;
uniform mat4 uView;
uniform mat4 uProj;
uniform vec2 uFocal;
uniform vec2 uViewPort;

out mat2 oSig;
out vec4 oColor;
out vec4 FragPos;

float maxf(float a, float b) 
{
    return a >= b ? a : b;
}

mat3 vecToMat(vec3 v0, vec3 v1)
{
    return transpose(mat3(
        v0.x * v1,
        v0.y * v1,
        v0.z * v1
    ));
}

vec2 GetEigenValues2x2(mat2 mat) 
{
    //https://www.youtube.com/watch?v=e50Bj7jn9IQ

    float m = (mat[0][0] + mat[1][1]) * 0.5;
    float p = (mat[0][0] * mat[1][1]) - (mat[0][1] * mat[1][0]);
    float d = sqrt((m * m) - p);

    return vec2(maxf(m + d, 0.00001), maxf(m - d, 0.00001));
}

mat2 GetEigenVectors2x2(mat2 mat, vec2 eigenValues) 
{
    float offdiag = mat[0][1];
    if (offdiag == 0) 
    {
        return mat2(
            normalize(mat[0] / eigenValues[0]), 
            normalize(mat[1] / eigenValues[1])
        );
    }
    vec2 eigenVec0 = normalize(vec2( 1, (eigenValues[0] - mat[0][0]) / offdiag ));
    vec2 eigenVec1 = normalize(vec2( eigenVec0.y, -eigenVec0.x ));

    float dot = ((eigenVec0.x * eigenVec1.x) + (eigenVec0.y * eigenVec1.y));

    return mat2(normalize(eigenVec0), normalize(eigenVec1));
}


void main()
{
    vec3 sig1_3_4 = vec3(iSig[3].x, iSig[3].y, iSig[3].z);
    vec3 sig4_1_3 = vec3(iSig[0].z, iSig[1].z, iSig[2].z);

    vec3 mean_time_dependent = iSPos.xyz + ((1.0 / iSig[3].z) * (uTime - iSPos.w) * sig4_1_3);
    vec3 tvec = (1.0 / iSig[3].z) * sig4_1_3;

    mat3 subpart = vecToMat(sig1_3_4, tvec);
    mat3 sig3x3 = mat3(
        iSig[0].xyz,
        iSig[1].xyz,
        iSig[2].xyz
    );

    sig3x3 = sig3x3 - subpart;

    mat3 W = mat3(
        uView[0].xyz,
        uView[1].xyz,
        uView[2].xyz
    );

    W = transpose(W);

    vec4 posCamSpace = uView * vec4(mean_time_dependent, 1);
    vec4 posScreenSpace = uProj * posCamSpace;
    posScreenSpace = (1.0 / posScreenSpace.w) * posScreenSpace;

    float z2 = posCamSpace.z * posCamSpace.z;

    mat3 J =
    mat3(
        1.0 / posCamSpace.z, 0.0, -posCamSpace.x / z2,
        0.0, 1.0 / posCamSpace.z, -posCamSpace.y / z2,
        0.0, 0.0, 0.0
    );

    mat3 T = W * transpose(J);
    mat3 cov3 = transpose(T) * sig3x3 * T;
    mat2 upper = mat2(
        cov3[0].xy,
        cov3[1].xy
    );

    vec2 lambdas = GetEigenValues2x2(upper);
    float l0 = sqrt(lambdas.x*3.0);
    float l1 = sqrt(lambdas.y*3.0);

    mat2 eigenVecs = GetEigenVectors2x2(upper, lambdas);
    vec2 v0 = eigenVecs[0] / uViewPort;
    vec2 v1 = eigenVecs[1] / uViewPort;
    
    mat2 R = transpose(mat2(v0, v1));
    R = transpose(R);
    mat2 S = mat2(l0, 0.0, 0.0, l1);

    mat2 _Sig = R * S * S * transpose(R);

    //This part is not done yet there are bugs somewhere
    gl_Position = uProj * vec4(S * iVPos.xy, 0.0, 1.0);
    FragPos = gl_Position;
    gl_Position = gl_Position + (uProj * uView * vec4(iSPos.xyz, 1.0));

    oSig = _Sig;
    oColor = iColor;
}