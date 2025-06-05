#version 450 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D text;
uniform vec4 textColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool useLighting;
uniform float ambientStrength;
uniform float specularStrength;

void main() {
    // Textur-Sample
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoord).r);
    vec4 color = textColor * sampled;
    
    if (useLighting) {
        // Ambient
        vec3 ambient = ambientStrength * vec3(1.0);
        
        // Diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0);
        
        // Specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * vec3(1.0);
        
        // Kombinieren
        vec3 result = (ambient + diffuse + specular) * color.rgb;
        FragColor = vec4(result, color.a);
    } else {
        FragColor = color;
    }
} 