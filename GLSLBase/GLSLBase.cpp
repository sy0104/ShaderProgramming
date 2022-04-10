/*
Copyright 2018 Lee Taek Hee (Korea Polytech University)

This program is free software: you can redistribute it and/or modify
it under the terms of the What The Hell License. Do it plz.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.
*/

#include "stdafx.h"
#include <iostream>
#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"

#include "Renderer.h"

Renderer *g_Renderer = NULL;

int g_WindowSizeX = 500;
int g_WindowSizeY = 500;

// 60번에 한번씩 호출이 될 것이다
void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Renderer Test
	//g_Renderer->Test();
	//g_Renderer->Lecture2();
	//g_Renderer->Lecture3();
	g_Renderer->Lecture3_Particle();

	glutSwapBuffers();
}

void Idle(void)
{
	RenderScene();
}

void MouseInput(int button, int state, int x, int y)
{
	RenderScene();
}

void KeyInput(unsigned char key, int x, int y)
{
	RenderScene();
}

void SpecialKeyInput(int key, int x, int y)
{
	RenderScene();
}

int main(int argc, char **argv)
{
	// Initialize GL things

	// GLUT
	// OpenGL은 라이브러리에 불과하고 윈도우 같은 것들을 관리해주지 않는다.
	// 그래서 OpenGL 위에서 동작하는, 즉 윈도우와 두개 사이를 맞춰주는 역할을 하는 툴들이 있어야 한다. 그 중 하나가 GLUT

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);	// window os 상에서 화면 좌표
	glutInitWindowSize(g_WindowSizeX, g_WindowSizeY);	// 500x500
	glutCreateWindow("GLSL KPU");	// 윈도우 생성

	// glew: extension이나 API 관련된 것들을 사용할 수 있도록 만들어 주는 지원 툴
	glewInit();
	if (glewIsSupported("GL_VERSION_4_6"))
	{
		std::cout << " GL Version is 4.6\n ";
	}
	else
	{
		std::cout << "GLEW 4.6 not supported\n ";
	}

	// Initialize Renderer
	g_Renderer = new Renderer(g_WindowSizeX, g_WindowSizeY);

	// callback function pointer 등록
	glutDisplayFunc(RenderScene);
	glutIdleFunc(Idle);
	glutKeyboardFunc(KeyInput);
	glutMouseFunc(MouseInput);
	glutSpecialFunc(SpecialKeyInput);

	// 계속 루프를 지속적으로 돌면서
	// 위 5개와 같은 이벤트가 있을 때마다 해당 함수를 콜백을 통해서 호출해주는 형태로 진행되는 프로젝트
	glutMainLoop();

	delete g_Renderer;

    return 0;
}

