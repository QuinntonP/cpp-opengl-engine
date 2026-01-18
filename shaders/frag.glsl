#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

void main()
{
    // Light properties
    vec3 lightPos = vec3(100.0, 100.0, 100.0);  // Position of the sun
    vec3 lightColor = vec3(1.0, 0.95, 0.8);      // Warm sunlight

    // Terrain color (greenish-brown)
    vec3 terrainColor = vec3(0.4, 0.5, 0.3);

    // Ambient lighting (base illumination)
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting (based on surface angle to light)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Combine lighting
    vec3 result = (ambient + diffuse) * terrainColor;

    FragColor = vec4(result, 1.0);
}
