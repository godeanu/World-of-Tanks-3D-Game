uniform sampler2D texture_1;
//Get values from vertex shader
in vec3 frag_position;
in vec3 frag_color;
in vec2 frag_texture;
in vec3 frag_normal;
in vec2 texcoord;
uniform bool mixtext;
in float frag_damageLevel; 
layout(location = 0) out vec4 out_color;
uniform float damageLevel;

void main()
{

	vec4 color1 = texture2D(texture_1, texcoord);
    vec3 darkerShade = vec3(0.0, 0.0, 0.0);

    float interpolationFactor = frag_damageLevel * 0.25;

    vec3 modifiedColor = mix(frag_color, darkerShade, interpolationFactor);

    if(mixtext==true)
	{
		out_color = color1;
	}
	else
	{
		out_color = vec4(modifiedColor, 1.0);
	}

}
