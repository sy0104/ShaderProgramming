#version 450

in vec3 a_Position;
in vec3 a_Velocity;

uniform float u_Time;

void main()
{
	vec3 newPos;
	newPos = a_Position + u_Time * a_Velocity;

	gl_Position = vec4(newPos, 1);
}
