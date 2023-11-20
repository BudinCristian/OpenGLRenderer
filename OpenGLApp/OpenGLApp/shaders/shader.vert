#version 330

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec3 norm;

out vec4 vCol;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 DirectionalLightSpacePos;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 directionalLightTransform;

void main()					
{
	gl_Position = projection * view * model * vec4(pos, 1.0);
	DirectionalLightSpacePos = directionalLightTransform * model * vec4(pos, 1.f);

	vCol = vec4(clamp(pos, 0.f, 1.0f), 1.f);

	TexCoord = tex;
	
	Normal = mat3(transpose(inverse(model))) * norm;

	FragPos = (model * vec4(pos, 1.0)).xyz;
}