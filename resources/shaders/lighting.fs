#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec4 fogColor;
uniform float fogDensity;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1
#define     LIGHT_SPOT              2

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 direction;
    vec4 color;
    float attenuation;
    float falloff;
    float cone;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);
	
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);
            float factor = 1;
            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].direction);
            }
            else
            {
                float dist = distance(lights[i].position, fragPosition);

                if (lights[i].attenuation > 0 && dist > lights[i].attenuation)
                {
                    if (dist > lights[i].attenuation + lights[i].falloff)
                        factor = 0;
                    else
                        factor = 1.0f - ((dist - lights[i].attenuation)/lights[i].falloff);
                }

                light = normalize(lights[i].position - fragPosition);

                if (lights[i].type == LIGHT_SPOT)
                {
                    float dotp = dot(-light, normalize(lights[i].direction));
                    if (dotp < lights[i].cone)
                    {   
                        factor = 0;
                    }
                    else
                    {
                        if (dotp > lights[i].cone * 0.5f)
                            factor *= ((dotp - lights[i].cone) * (1.0f/(1.0f- lights[i].cone)));
                    }
                }
            }

            float NdotL = max(dot(normal, light), 0.0);

            lightDot += lights[i].color.rgb * factor * NdotL;

            float specCo = 0.0;
            if (NdotL > 0.0)
                specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0); // 16 refers to shine

            specular += specCo;
        }
    }
	
	vec4 tint = colDiffuse * fragColor;

    finalColor = (texelColor * ((tint + vec4(specular, 1.0)) * vec4(lightDot, 1.0)));
    finalColor += texelColor * (ambient) * tint;

    // Gamma correction
    //finalColor = pow(finalColor, vec4(1.0/2.2));
	
    
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
