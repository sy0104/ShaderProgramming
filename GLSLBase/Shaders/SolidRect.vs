#version 450	// 셰이더 버전

// in: attribute
in vec3 a_Position;

void main()
{
	gl_Position = vec4(a_Position, 1);
}
