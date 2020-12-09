#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextureCoords;

out vec3 NaviePos;
out vec3 FragPos;
out vec3 Normal;
out vec2 TextureCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool isCubeMap;

void main() {
	NaviePos = aPosition;
	FragPos =  vec3(model * vec4(aPosition, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
	TextureCoords = aTextureCoords;

	if (isCubeMap) {
		// Ã¸»s¤ÑªÅ²°
		mat4 view_new = mat4(mat3(view));
		vec4 pos = projection * view_new * vec4(FragPos, 1.0);
		gl_Position = pos.xyww;
	} else {
		gl_Position = projection * view * vec4(FragPos, 1.0);
	}
}