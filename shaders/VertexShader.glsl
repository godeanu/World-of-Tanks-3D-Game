// Get vertex attributes from each location
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture;
//layout(location = 3) in vec3 v_color;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Output
// Output values to fragment shader
out vec3 frag_position;
out vec3 frag_color;
out vec2 frag_texture;
out vec3 frag_normal;

uniform vec3 object_color;
uniform float damageLevel;

out vec2 texcoord;

out float frag_damageLevel; 
void main()
{
    vec3 displacedPosition = v_position;

    float deformationIntensity = 1 * damageLevel; 
    displacedPosition += deformationIntensity * normalize(v_normal);

    frag_position = displacedPosition;
    frag_normal = v_normal;
    frag_texture = v_texture;
    frag_color = object_color;
    frag_damageLevel = damageLevel;

    texcoord = v_texture;
    
    gl_Position = Projection * View * Model * vec4(displacedPosition, 1.0f);

}
