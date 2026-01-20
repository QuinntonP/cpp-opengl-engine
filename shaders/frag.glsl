#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform bool useTexture;
uniform vec3 viewPos;

// Fog settings
const vec3 fogColor = vec3(0.6, 0.7, 0.8);  // Light blue-grey fog
const float fogStart = 5.0;   // Distance where fog starts
const float fogEnd = 50.0;     // Distance where fog is fully opaque

void main()
{
    // Base color - use texture if available, otherwise use terrain color
    vec3 baseColor;
    if (useTexture) {
        baseColor = texture(texture1, TexCoord).rgb;
    } else {
        baseColor = vec3(0.4, 0.5, 0.3);  // Terrain color (greenish-brown)
    }

    // Light properties
    vec3 lightPos = vec3(100.0, 100.0, 100.0);  // Position of the sun
    vec3 lightColor = vec3(1.0, 0.95, 0.8);      // Warm sunlight

    // Ambient lighting (base illumination)
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting (based on surface angle to light)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Combine lighting with base color
    vec3 result = (ambient + diffuse) * baseColor;

    // Calculate fog
    float distance = length(viewPos - FragPos);
    float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);

    // Mix result with fog color based on distance
    result = mix(fogColor, result, fogFactor);

    FragColor = vec4(result, 1.0);
}
