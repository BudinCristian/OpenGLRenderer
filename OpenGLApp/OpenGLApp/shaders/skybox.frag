#version 330

in vec3 TexCoords;

out vec4 color;

uniform samplerCube skybox;

void main()
{
    // Sample the color from the skybox texture
    vec4 skyColor = texture(skybox, TexCoords);

    // Define fog parameters
    float fogStart =-0.5; // Adjust as needed
    float fogEnd = 5;   // Adjust as needed

    // Calculate fog factor based on texture coordinate
    float fogFactor = smoothstep(fogStart, fogEnd, TexCoords.y);

    // Mix the skybox color with a fog color
    vec4 fogColor = mix(vec4(0.4, 0.4, 0.4, 1.0), skyColor, fogFactor);

    // Output the final color
    color = fogColor;
}
