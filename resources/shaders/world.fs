#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Input lighting values
uniform vec4 ambient;
uniform vec3 viewPos;

uniform vec3 gloablLightDirection;
uniform vec4 gloablLightColor;

uniform vec4 fogColor;
uniform float fogDensity;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    vec4 tint = colDiffuse * fragColor;

    vec3 light = -normalize(gloablLightDirection);
    float factor = 1;

    float NdotL = max(dot(normal, light), 0.0);

    lightDot += gloablLightColor.rgb * factor * NdotL;

    float specCo = 0.0;
    if (NdotL > 0.0)
        specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0); // 16 refers to shine

    specular += specCo;

    // final color is the color from the texture 
    //    times the tint color (colDiffuse)
    //    times the fragment color (interpolated vertex color)
    //finalColor = texelColor*colDiffuse*fragColor;

    finalColor = (texelColor * ((tint + vec4(specular, 1.0)) * vec4(lightDot, 1.0)));
    finalColor += texelColor * (ambient) * tint;

    finalColor.a = texelColor.a * tint.a;

     // Fog calculation
    if (fogDensity > 0 && fogColor.a > 0)
    {
     //   fogColor.a = finalColor.a;

        float dist = length(viewPos - fragPosition);

        // Exponential fog
        float fogFactor = 1.0/exp((dist/fogDensity)*(dist/fogDensity));

        fogFactor = clamp(fogFactor, 0.0, 1.0);

        finalColor = mix(fogColor, finalColor, fogFactor);
    }
}

