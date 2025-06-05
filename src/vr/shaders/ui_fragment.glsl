#version 410 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 Color;

out vec4 FragColor;

uniform sampler2D texture1;
uniform bool useTexture = true;
uniform bool useLighting = true;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor = vec3(1.0);
uniform float alpha = 1.0;

void main() {
    vec4 texColor = useTexture ? texture(texture1, TexCoord) : vec4(1.0);
    vec4 finalColor = texColor * Color;
    
    if (useLighting) {
        // Ambient
        vec3 ambient = 0.1 * lightColor;
        
        // Diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        
        // Specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = spec * lightColor;
        
        vec3 result = (ambient + diffuse + specular) * finalColor.rgb;
        FragColor = vec4(result, finalColor.a * alpha);
    } else {
        FragColor = vec4(finalColor.rgb, finalColor.a * alpha);
    }
} 