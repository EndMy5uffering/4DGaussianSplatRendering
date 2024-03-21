#version 440 core

#define iData data[gl_InstanceID]
//#define WORLD_SPACE_POS //Makes the splats stick to world space position
#define SCREEN_SPACE_POS //Makes the splats stick to screen space position

struct SplatData
{
  vec4 iSPos;
  vec4 iColor;
  vec4 iSig;
};

layout(location = 0) in vec2 iVPos;

layout (binding = 1, std140) buffer InstanceData {
    SplatData data[];
};

uniform mat4 uView;
uniform mat4 uProj;

out mat2 oSig;
out vec4 oColor;
out vec4 oFragPos;
out vec2 oSSPos;
out mat2 eigenVecs;
out vec2 eigenVals;

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
    // https://math.stackexchange.com/questions/395698/fast-way-to-calculate-eigen-of-2x2-matrix-using-a-formula
    if(mat[0][1] == 0)
    {
        return mat2(vec2(1, 0), vec2(0, 1));      
    }
    vec2 eigenVec0 = normalize(vec2( mat[0][1], eigenValues[0] - mat[0][0] ));
    vec2 eigenVec1 = vec2( eigenVec0.y, -eigenVec0.x );
    return mat2(normalize(eigenVec0), normalize(eigenVec1));
}


void main()
{
    mat2 geoinf = mat2(iData.iSig.xy, iData.iSig.zw);

    vec4 posCamSpace = uView * vec4(iData.iSPos.xy, 0, 1);
    vec4 posScreenSpace = uProj * vec4(iData.iSPos.xy, -1, 1); //posCamSpace;
    posScreenSpace = (1.0 / posScreenSpace.w) * posScreenSpace;

    vec2 lambdas = GetEigenValues2x2(geoinf);
    float l0 = sqrt(lambdas.x * 2.0);
    float l1 = sqrt(lambdas.y * 2.0);

    mat2 eigenVecs = GetEigenVectors2x2(geoinf, lambdas);
    vec2 v0 = normalize(eigenVecs[0]);
    vec2 v1 = normalize(eigenVecs[1]);
    
    mat2 R = mat2(v0, v1);
    mat2 S = mat2(l1, 0.0, 0.0, l0);
    eigenVecs = R;
    eigenVals = vec2(l0, l1);
    mat2 _Sig = inverse(R * S * S * transpose(R));

#ifdef WORLD_SPACE_POS
    gl_Position = (uView * vec4(R * S * iVPos.xy, 0.0, 1.0)) + posCamSpace;
    gl_Position = uProj * gl_Position; 
#endif // WORLD_SPACE_POS
#ifdef SCREEN_SPACE_POS
    gl_Position = vec4(R * S * iVPos.xy, -5.0, 1.0);
    gl_Position = uProj * (gl_Position + posScreenSpace); //Makes them stick to screen space position
#endif // SCREEN_SPACE_POS
    oFragPos = vec4(R * 8 * S * iVPos.xy, 0.0, 1.0);
    //oFragPos = gl_Position;
    oSSPos = posScreenSpace.xy;
    oSig = _Sig;
    oColor = iData.iColor;
}