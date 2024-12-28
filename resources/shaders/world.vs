#version 330

#define MAX_BONE_NUM 128

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
in vec4 vertexBoneIds;
in vec4 vertexBoneWeights;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

uniform mat4 boneMatrices[MAX_BONE_NUM];

uniform int animate;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// NOTE: Add here your custom variables

void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    vec4 vertPos = vec4(vertexPosition,1);
    vec4 vertNormal = vec4(vertexNormal,0);

    if (animate != 0)
    {
        int boneIndex0 = int(vertexBoneIds.x);
        int boneIndex1 = int(vertexBoneIds.y);
        int boneIndex2 = int(vertexBoneIds.z);
        int boneIndex3 = int(vertexBoneIds.w);
    
        // postion
        vertPos =
            vertexBoneWeights.x*(boneMatrices[boneIndex0] * vec4(vertexPosition, 1.0)) +
            vertexBoneWeights.y*(boneMatrices[boneIndex1] * vec4(vertexPosition, 1.0)) + 
            vertexBoneWeights.z*(boneMatrices[boneIndex2] * vec4(vertexPosition, 1.0)) + 
            vertexBoneWeights.w*(boneMatrices[boneIndex3] * vec4(vertexPosition, 1.0));

        // normals
        vec4 normal4 = vec4(vertexNormal, 0.0f);
        vertNormal =
            boneMatrices[boneIndex0] * normal4 * vertexBoneWeights.x +
            boneMatrices[boneIndex1] * normal4 * vertexBoneWeights.y +
            boneMatrices[boneIndex2] * normal4 * vertexBoneWeights.z +
            boneMatrices[boneIndex3] * normal4 * vertexBoneWeights.w;
        vertNormal.w = 0.0f;
    }
        
    // Calculate final vertex data position
    fragPosition = vec3(matModel * vertPos);
    fragNormal = normalize(vec3(matNormal * vertNormal));
    gl_Position = mvp * vertPos;
}