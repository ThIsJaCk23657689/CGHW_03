#version 330 core
out vec4 FragColor;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light {
	vec3 position;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

in vec3 NaviePos;
in vec3 FragPos;
in vec3 Normal;
in vec2 TextureCoords;

uniform bool isGlowObj;
uniform bool enableTexture;
uniform float alpha;
uniform vec3 color;
uniform vec3 viewPos;
uniform Material material;
uniform Light light;

uniform bool isCubeMap;
uniform samplerCube skybox;

void main() {
	
	vec4 texture_diffuse;
	vec4 texture_specular;

	if (isCubeMap) {
		// 材質類型採用 cubemap
        texture_diffuse = texture(skybox, normalize(NaviePos));
        texture_specular = texture(skybox, normalize(NaviePos));
	} else {
		if (enableTexture) {
			// 使用材質繪圖
			texture_diffuse = texture(material.diffuse, TextureCoords);
			texture_specular = texture(material.specular, TextureCoords);
		} else {
			// 純色填滿
			texture_diffuse = vec4(color, alpha);
			texture_specular = vec4(color, alpha);
		}
	}

	FragColor = texture_diffuse;

	if (isGlowObj) {
		// 繪製發光物體
		vec3 texture_map = texture_diffuse.rgb;
		vec3 emission = texture_diffuse.rgb;
		emission *= 0.5;
		vec3 result = texture_map + emission;
		FragColor = vec4(result, 1.0);
	} else {
		// 光照模型
		vec4 temp = texture_diffuse;
		if (temp.a < 0.1) {
			discard;
		}

		// 環境光
		vec3 ambient = light.ambient * temp.rgb;

		// 漫射
		vec3 norm = normalize(Normal);
		vec3 lightDir = normalize(light.position - FragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = light.diffuse * diff * temp.rgb;

		// 高光
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec3 specular = light.specular * spec * texture_specular.rgb;

		// 光衰弱
		float distance = length(light.position - FragPos);
		float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

		vec3 sunDir = normalize(vec3(light.position.x, light.position.y, light.position.z));
		float t = max(dot(vec3(0.0, 1.0, 0.0), sunDir), 0.1);
		if (!isCubeMap) {
			ambient = vec3(t, t, t) * temp.rgb;
			diffuse *= attenuation;
			specular *= attenuation;
        } else {
			ambient = vec3(t, t, t) * temp.rgb;
			diffuse *= 0.0;
			specular *= 0.0;
        }

		vec3 result = ambient + diffuse + specular;
		FragColor = vec4(result, temp.a);
	}
}