#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float radius;
uniform vec4 zenithColor;
uniform vec4 horizonColor;
uniform vec4 undergroundColor;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    // Texel color fetching from texture sampler

    float factor = fragPosition.y/radius;
    if (factor >= 0)
        finalColor = mix(horizonColor,zenithColor, factor);
    else
        finalColor = mix(horizonColor,undergroundColor, -factor);
}

