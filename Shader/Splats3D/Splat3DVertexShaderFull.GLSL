#version 440 core

layout(location = 0) in vec2 iVPos;
layout(location = 1) in vec3 iSPos;
layout(location = 2) in vec4 iColor;
layout(location = 3) in mat3 iSig;

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

vec2 GetEigenValues2x2(mat2 mat) 
{
    //https://www.youtube.com/watch?v=e50Bj7jn9IQ

    float m = (mat[0][0] + mat[1][1]) * 0.5;
    float p = (mat[0][0] * mat[1][1]) - (mat[0][1] * mat[1][0]);
    float d = sqrt((m * m) - p);

    return vec2(maxf(m - d, 0.000001), maxf(m + d, 0.000001));
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

    mat3 W = mat3(
        uView[0].xyz,
        uView[1].xyz,
        uView[2].xyz
    );

    vec4 posCamSpace = uView * vec4(iSPos.xyz, 1);
    vec4 posScreenSpace = uProj * posCamSpace;
    posScreenSpace = (1.0 / posScreenSpace.w) * posScreenSpace;

    float z2 = posCamSpace.z * posCamSpace.z;

    mat3 J = mat3(
        uFocal.x / posCamSpace.z, 0.0, -(posCamSpace.x * uFocal.x) / z2,
        0.0, uFocal.y / posCamSpace.z, -(posCamSpace.y * uFocal.y) / z2,
        0.0, 0.0, 0.0
    );

    mat3 T = transpose(W) * J;
    mat3 cov3 = transpose(T) * iSig * T;
    mat2 upper = mat2(
        cov3[0].xy,
        cov3[1].xy
    );

    vec2 lambdas = GetEigenValues2x2(upper);
    float l0 = sqrt(lambdas.x);
    float l1 = sqrt(lambdas.y);

    mat2 eigenVecs = GetEigenVectors2x2(upper, lambdas);
    vec2 v0 = normalize(eigenVecs[0] / uViewPort);
    vec2 v1 = normalize(eigenVecs[1] / uViewPort);
    
    mat2 R = mat2(v0, v1);
    mat2 S = mat2(l0, 0.0, 0.0, l1);

    mat2 _Sig = inverse(R * S * S * transpose(R));
    //float aspect = uViewPort.x / uViewPort.y;
    //This part is not done yet there are bugs somewhere
    gl_Position = uProj * vec4(R * S * 4 * iVPos.xy, 0.0, 1.0);
    FragPos = gl_Position;
    gl_Position = gl_Position + (uProj * uView * vec4(iSPos.xyz, 1.0));

    oSig = _Sig;
    oColor = iColor;
}
