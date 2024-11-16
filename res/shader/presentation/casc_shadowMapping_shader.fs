#version 410 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2DArray shadowMap;

uniform samplerCube depthMap;
uniform vec3 lightPos;

uniform vec3 lightDir;
uniform vec3 viewPos;
uniform float farPlane;
uniform float far_plane;
uniform vec3 pointLightColor;

uniform mat4 view;

uniform int showCascades;

layout (std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};
uniform float cascadePlaneDistances[16];
uniform int cascadeCount;   // number of frusta - 1

vec3 colors[4] = vec3[](
    vec3(1.0, 0.5f, 0.5f) *1.2, // red
    vec3(0.5f, 1.0f, 0.5f)*1.2, // green
    vec3(0.5, 0.5, 1.0)*1.2 , // blue
    vec3(1.0f, 1.0f, 0.0f)*1.2  // yellow
);

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

vec2 ShadowCalculation(vec3 fragPosWorldSpace);
float PointShadowCalculation(vec3 fragPos);
vec3 CalcPointLight(vec3 lightColor, vec3 color, vec3 normal, vec3 fragPos);
void main()
{
    // calculate shadow
    vec2 result = ShadowCalculation(fs_in.FragPos);
    float shadow = result.x;
    float layer = result.y;

    vec3 color;
    if(showCascades==1)
    {
        color = texture(texture_diffuse1, fs_in.TexCoords).rgb * (colors[int(layer)]);
    }
    else
    {
        color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    }

    // blinn phong lighting for directional light
    // --------------------------------
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.6);
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;

    // combine results
    vec3 directionalLighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;


    // point light and shadow
    // --------------------------------
    vec3 pointLighting = CalcPointLight(vec3(2.0), color, normal, fs_in.FragPos);

    vec3 finalColor = directionalLighting + pointLighting;

    FragColor = vec4(finalColor, 1.0);
}

vec2 ShadowCalculation(vec3 fragPosWorldSpace)
{
    // select cascade layer
    vec4 fragPosViewSpace = view * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depthValue < cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = cascadeCount;
    }

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
    {
        return vec2(0.0, layer);
    }
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == cascadeCount)
    {
        bias *= 1 / (farPlane * biasModifier);
    }
    else
    {
        bias *= 1 / (cascadePlaneDistances[layer] * biasModifier);
    }

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return vec2(shadow, layer);
}

float PointShadowCalculation(vec3 fragPos)
{
    // vector between fragment position and light position
    vec3 fragToLight = fs_in.FragPos - lightPos;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.5;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);

    return shadow;
}

vec3 CalcPointLight(vec3 lightColor, vec3 color, vec3 normal, vec3 fragPos)
{
    vec3 lightDir2 = normalize(lightPos - fs_in.FragPos);
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    float diff = max(dot(lightDir2, normal), 0.0);
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir2, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    // attenuation
    float constant = 1.0;
    float linear = 0.03;
    float quadratic = 0.013;
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
    // combine results
    ambient  = ambient * lightColor * vec3(texture(texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse  = diff * lightColor * vec3(texture(texture_diffuse1, fs_in.TexCoords));
    vec3 specular = spec * lightColor * vec3(texture(texture_diffuse1, fs_in.TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    float pointShadow = PointShadowCalculation(fs_in.FragPos);

    vec3 lighting = (ambient + (1.0 - pointShadow) * (diffuse + specular)) * color;

    return lighting;
}