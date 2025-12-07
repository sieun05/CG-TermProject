#version 330 core
// out_Color: Color value from vertex shader
// out_TexCoord: Texture coordinates from vertex shader
// out_Normal: Normal vector from vertex shader
// out_FragPos: Fragment position from vertex shader
// FragColor: Final pixel color output

in vec3 out_Color; // Color from vertex shader
in vec2 out_TexCoord; // Texture coordinates from vertex shader
in vec3 out_Normal; // Normal from vertex shader
in vec3 out_FragPos; // Fragment position from vertex shader
in vec4 out_FragPosLightSpace; // Fragment position in light space

uniform sampler2D textureSampler; // Texture sampler
uniform sampler2D shadowMap; // Shadow map sampler
uniform bool useTexture; // Texture usage flag
uniform bool useLighting; // Lighting usage flag
uniform bool useShadows; // Shadow usage flag

// Lighting parameters
uniform vec3 lightDir; // Directional light direction
uniform vec3 lightColor; // Light color
uniform vec3 viewPos; // Camera position
uniform float ambientStrength; // Ambient light strength
uniform float specularStrength; // Specular light strength
uniform float shininess; // Shininess (specular power)

out vec4 FragColor; // Final color output

// Calculate shadow factor (0.0 = in shadow, 1.0 = fully lit)
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDirection)
{
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // Check whether current fragment is in shadow
    // Add bias to prevent shadow acne
    float bias = max(0.05 * (1.0 - dot(normal, lightDirection)), 0.005);

    // PCF (Percentage Closer Filtering) for soft shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // Keep the shadow at 0.0 when outside the far plane region of the light's frustum
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return 1.0 - shadow;
}

void main()
{
    vec4 objectColor;

    if (useTexture) {
        // Use texture
        vec4 texColor = texture(textureSampler, out_TexCoord);

        // Transparency handling
        if(texColor.a < 0.1)
            discard;

        objectColor = texColor;
    } else {
        // Use vertex color
        objectColor = vec4(out_Color, 1.0);
    }

    if (useLighting) {
        // Normalize normal vector
        vec3 norm = normalize(out_Normal);
        vec3 lightDirection = normalize(-lightDir); // Light direction

        // Ambient lighting
        vec3 ambient = ambientStrength * lightColor;

        // Diffuse lighting
        float diff = max(dot(norm, lightDirection), 0.0);
        vec3 diffuse = diff * lightColor;

        // Specular lighting (Blinn-Phong)
        vec3 viewDir = normalize(viewPos - out_FragPos);
        vec3 halfwayDir = normalize(lightDirection + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        vec3 specular = specularStrength * spec * lightColor;

        // Calculate shadow
        float shadow = 1.0;
        if (useShadows) {
            shadow = ShadowCalculation(out_FragPosLightSpace, norm, lightDirection);
        }

        // Combine lighting (ambient is not affected by shadows)
        vec3 result = (ambient + shadow * (diffuse + specular)) * objectColor.rgb;
        FragColor = vec4(result, objectColor.a);
    } else {
        // No lighting
        FragColor = objectColor;
    }
}