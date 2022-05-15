#version 450

in vec3 a_Position;
in vec3 a_Velocity;
in float a_EmitTime;
in float a_LifeTime;

uniform float u_Time;
uniform vec3 u_Accel;

void main()
{
	vec3 newPos;
	float t = u_Time - a_EmitTime;	// 각 파티클만의 독립적인 상대시간
	float tt = t * t;

	if (t > 0)
	{
		newPos = a_Position + a_Velocity * t + 0.5 * u_Accel * tt;
		gl_Position = vec4(newPos, 1);
	}
	else	// 아직 태어나지 않았을 때
	{
		newPos = vec3(-1000000, -100000, -100000);
	}

}
