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

uniform vec3 gloablBackfillLightDirection;
uniform vec4 gloablBackfillLightColor;

uniform vec4 fogColor;
uniform float fogDensity;

uniform float tintScale;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    vec3 lightControbution = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    vec4 tint = colDiffuse * fragColor;

    float alpha = tint.a;

    if (tintScale > 0)
    {
        tint.rgb *= tintScale;
        tint.rgb = min(tint.rgb, vec3(1,1,1));
    }

    float factor = 1;

    // global directional light
    vec3 light = -normalize(gloablLightDirection);
    float NdotL = max(dot(normal, light), 0.0);
    lightControbution += gloablLightColor.rgb * factor * NdotL;

    float specCo = 0.0;
    if (NdotL > 0.0)
        specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0); // 16 refers to shine

    specular += specCo;

    // global backfill light
    light = -normalize(gloablBackfillLightDirection);
    NdotL = max(dot(normal, light), 0.0);
    lightControbution += gloablBackfillLightColor.rgb * factor * NdotL;

    specCo = 0.0;
    if (NdotL > 0.0)
        specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0); // 16 refers to shine

    specular += specCo;

    // process the point and spot lights



    finalColor = (texelColor * ((tint + vec4(specular, 1.0)) * vec4(lightControbution, 1.0)));
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

