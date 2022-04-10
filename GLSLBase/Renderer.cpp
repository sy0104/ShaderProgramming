#include "stdafx.h"
#include "Renderer.h"
#include "LoadPng.h"
#include <Windows.h>
#include <cstdlib>
#include <cassert>

Renderer::Renderer(int windowSizeX, int windowSizeY)
{
	//default settings
	glClearDepth(1.f);

	Initialize(windowSizeX, windowSizeY);
}


Renderer::~Renderer()
{
}

void Renderer::Initialize(int windowSizeX, int windowSizeY)
{
	//Set window size
	m_WindowSizeX = windowSizeX;
	m_WindowSizeY = windowSizeY;

	//Load shaders
	// m_SolidRectShader: 셰이더 프로그램 아이디를 받아와서 저장하는 멤버변수
	m_SolidRectShader = CompileShaders("./Shaders/SolidRect.vs", "./Shaders/SolidRect.fs");
	m_Lecture3Shader = CompileShaders("./Shaders/lecture3.vs", "./Shaders/lecture3.fs");
	m_Lecture3ParticleShader = CompileShaders("./Shaders/lecture3_particle.vs", "./Shaders/lecture3_particle.fs");

	//Create VBOs
	CreateVertexBufferObjects();
	// Create Particles
	CreateParticle(1000);

	//Initialize camera settings
	m_v3Camera_Position = glm::vec3(0.f, 0.f, 1000.f);
	m_v3Camera_Lookat = glm::vec3(0.f, 0.f, 0.f);
	m_v3Camera_Up = glm::vec3(0.f, 1.f, 0.f);
	m_m4View = glm::lookAt(
		m_v3Camera_Position,
		m_v3Camera_Lookat,
		m_v3Camera_Up
	);

	//Initialize projection matrix
	m_m4OrthoProj = glm::ortho(
		-(float)windowSizeX / 2.f, (float)windowSizeX / 2.f,
		-(float)windowSizeY / 2.f, (float)windowSizeY / 2.f,
		0.0001f, 10000.f);
	m_m4PersProj = glm::perspectiveRH(45.f, 1.f, 1.f, 1000.f);

	//Initialize projection-view matrix
	m_m4ProjView = m_m4OrthoProj * m_m4View; //use ortho at this time
	//m_m4ProjView = m_m4PersProj * m_m4View;

	//Initialize model transform matrix :; used for rotating quad normal to parallel to camera direction
	m_m4Model = glm::rotate(glm::mat4(1.0f), glm::radians(0.f), glm::vec3(1.f, 0.f, 0.f));
}

void Renderer::CreateVertexBufferObjects()
{
	float rect[]
		=
	{
		-0.5, -0.5, 0.f, -0.5, 0.5, 0.f, 0.5, 0.5, 0.f, //Triangle1
		-0.5, -0.5, 0.f,  0.5, 0.5, 0.f, 0.5, -0.5, 0.f, //Triangle2
	};

	glGenBuffers(1, &m_VBORect);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);

	// vertex 만드는 작업
	float lecture2[]
		=
	{
		// 가운데		오른쪽		  오른쪽 위
		0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0,
	};	// 9 floats array

	glGenBuffers(1, &m_VBOLecture2);	// openGL쪽에서 buffer를 하나 만들어서 m_VBOLecture2안에 넣어주게됨
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture2);	// 어떤 형태로 넣을지 Bind로 설정
	glBufferData(GL_ARRAY_BUFFER, sizeof(lecture2), lecture2, GL_STATIC_DRAW);	// GL_STATIC_DRAW(사용방식), 보통 한번 넣고 업데이트 안하기 때문에 static_draw 사용

	///////////////////////////////////////////////////////////////////////////////////////////////////

	float lecture3[]
		=
	{	// vectex마다 색상 다르게
		0.0, 0.0, 0.0, 1, 0, 0, 1,
		1.0, 0.0, 0.0, 0, 1, 0, 1,
		1.0, 1.0, 0.0, 0, 0, 1, 1
	};	// 21 floats array

	glGenBuffers(1, &m_VBOLecture3);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lecture3), lecture3, GL_STATIC_DRAW);

	///////////////////////////////////////////////////////////////////////////////////////////////////

	float particleSize = 0.1f;
	float lecture3_singleParticle[]
		=
	{	
		-particleSize, -particleSize, 0.0, 1, 1, 1, 1,
		particleSize,   particleSize, 0.0, 1, 1, 1, 1,
		-particleSize,  particleSize, 0.0, 1, 1, 1, 1,	// triangle 1

		-particleSize, -particleSize, 0.0, 1, 1, 1, 1,
		particleSize,  -particleSize, 0.0, 1, 1, 1, 1,
		particleSize,   particleSize, 0.0, 1, 1, 1, 1,	// triangle 2
	};	// 21 floats array

	glGenBuffers(1, &m_VBOSingleParticleQuad);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOSingleParticleQuad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lecture3_singleParticle), lecture3_singleParticle, GL_STATIC_DRAW);
}

void Renderer::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	//쉐이더 오브젝트 생성
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = (GLint)strlen(pShaderText);
	//쉐이더 코드를 쉐이더 오브젝트에 할당
	glShaderSource(ShaderObj, 1, p, Lengths);

	//할당된 쉐이더 코드를 컴파일
	glCompileShader(ShaderObj);

	GLint success;
	// ShaderObj 가 성공적으로 컴파일 되었는지 확인
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];

		//OpenGL 의 shader log 데이터를 가져옴
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		printf("%s \n", pShaderText);
	}

	// ShaderProgram 에 attach!!
	glAttachShader(ShaderProgram, ShaderObj);
}

bool Renderer::ReadFile(char* filename, std::string *target)
{
	std::ifstream file(filename);
	if (file.fail())
	{
		std::cout << filename << " file loading failed.. \n";
		file.close();
		return false;
	}
	std::string line;
	while (getline(file, line)) {
		target->append(line.c_str());
		target->append("\n");
	}
	return true;
}

GLuint Renderer::CompileShaders(char* filenameVS, char* filenameFS)
{
	GLuint ShaderProgram = glCreateProgram(); //빈 쉐이더 프로그램 생성

	if (ShaderProgram == 0) { //쉐이더 프로그램이 만들어졌는지 확인
		fprintf(stderr, "Error creating shader program\n");
	}

	std::string vs, fs;

	//shader.vs 가 vs 안으로 로딩됨
	if (!ReadFile(filenameVS, &vs)) {
		printf("Error compiling vertex shader\n");
		return -1;
	};

	//shader.fs 가 fs 안으로 로딩됨
	if (!ReadFile(filenameFS, &fs)) {
		printf("Error compiling fragment shader\n");
		return -1;
	};

	// ShaderProgram 에 vs.c_str() 버텍스 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	// ShaderProgram 에 fs.c_str() 프레그먼트 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	//Attach 완료된 shaderProgram 을 링킹함
	glLinkProgram(ShaderProgram);

	//링크가 성공했는지 확인
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (Success == 0) {
		// shader program 로그를 받아옴
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error linking shader program\n" << ErrorLog;
		return -1;
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error validating shader program\n" << ErrorLog;
		return -1;
	}

	glUseProgram(ShaderProgram);
	std::cout << filenameVS << ", " << filenameFS << " Shader compiling is done.\n";

	return ShaderProgram;
}
unsigned char * Renderer::loadBMPRaw(const char * imagepath, unsigned int& outWidth, unsigned int& outHeight)
{
	std::cout << "Loading bmp file " << imagepath << " ... " << std::endl;
	outWidth = -1;
	outHeight = -1;
	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = NULL;
	fopen_s(&file, imagepath, "rb");
	if (!file)
	{
		std::cout << "Image could not be opened, " << imagepath << " is missing. " << std::endl;
		return NULL;
	}

	if (fread(header, 1, 54, file) != 54)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	if (header[0] != 'B' || header[1] != 'M')
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	if (*(int*)&(header[0x1E]) != 0)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	if (*(int*)&(header[0x1C]) != 24)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	outWidth = *(int*)&(header[0x12]);
	outHeight = *(int*)&(header[0x16]);

	if (imageSize == 0)
		imageSize = outWidth * outHeight * 3;

	if (dataPos == 0)
		dataPos = 54;

	data = new unsigned char[imageSize];

	fread(data, 1, imageSize, file);

	fclose(file);

	std::cout << imagepath << " is succesfully loaded. " << std::endl;

	return data;
}

GLuint Renderer::CreatePngTexture(char * filePath)
{
	//Load Pngs: Load file and decode image.
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filePath);
	if (error != 0)
	{
		lodepng_error_text(error);
		assert(error == 0);
		return -1;
	}

	GLuint temp;
	glGenTextures(1, &temp);

	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

	return temp;
}

GLuint Renderer::CreateBmpTexture(char * filePath)
{
	//Load Bmp: Load file and decode image.
	unsigned int width, height;
	unsigned char * bmp
		= loadBMPRaw(filePath, width, height);

	if (bmp == NULL)
	{
		std::cout << "Error while loading bmp file : " << filePath << std::endl;
		assert(bmp != NULL);
		return -1;
	}

	GLuint temp;
	glGenTextures(1, &temp);

	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp);

	return temp;
}

// 그리는 부분
void Renderer::Test()
{
	glUseProgram(m_SolidRectShader);	// 그러려니

	int attribPosition = glGetAttribLocation(m_SolidRectShader, "a_Position");	// 일단 넘어감
	glEnableVertexAttribArray(attribPosition);	// 얘도
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);	// 굳이 꼬이게 할 바에는 매번 Bind 해주는게 낫다
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
}

void Renderer::Lecture2()
{
	glUseProgram(m_SolidRectShader);	// 그러려니

	int attribPosition = glGetAttribLocation(m_SolidRectShader, "a_Position");	// 일단 넘어감
	glEnableVertexAttribArray(attribPosition);	// 얘도
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture2);	// 굳이 꼬이게 할 바에는 매번 Bind 해주는게 낫다
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(attribPosition);
}
 
float gTime = 1.f;

void Renderer::Lecture3()
{
	GLuint shader = m_Lecture3Shader;
	glUseProgram(shader);

	int attribPosition = glGetAttribLocation(shader, "a_Position");
	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture3);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

	int attribColor = glGetAttribLocation(shader, "a_Color");
	glEnableVertexAttribArray(attribColor);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture3);	// 위에서 해줬기 때문에 또 해줄 필요는 없음
	glVertexAttribPointer(attribColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));	// x, y, z 뛰어넘고 4번째부터

	// 선언한 아이에게 값을 줘야함. 값을 주려면 로케이션이 필요
	int uniformLocTime = glGetUniformLocation(shader, "u_Time");
	// ID에 -값이 들어감, u_Time get하는게 실패함 -> u_Time을 안쓰고 있어서 컴파일 타임에서 제외시킴
	glUniform1f(uniformLocTime, gTime);

	int uniformLocColor = glGetUniformLocation(shader, "u_Color");
	glUniform4f(uniformLocColor, 1, 1, 1, 1);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	gTime -= 0.0001f;
	if (gTime < 0.f) gTime = 1.f;

	glDisableVertexAttribArray(attribPosition);
}

void Renderer::CreateParticle(int count)
{
	int floatCount = count * 3 * 3 * 2; //(x, y, z, vx, vy, vz)
	float* particleVertices = new float[floatCount];
	int vertexCount = count * 3 * 2;
	int index = 0;
	float particleSize = 0.01f;
	for (int i = 0; i < count; i++)
	{
		float randomValueX = 0.f;
		float randomValueY = 0.f;
		float randomValueZ = 0.f;
		//float randomValueVX = 0.f;
		//float randomValueVY = 0.f;
		//float randomValueVZ = 0.f;
		randomValueX = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.f; //-1~1
		randomValueY = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.f; //-1~1
		randomValueZ = 0.f;
		//randomValueVX = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.f; //-1~1
		//randomValueVY = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.f; //-1~1
		//randomValueVZ = 0.f;
		//v0
		particleVertices[index] = -particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = -particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++; //Position XYZ
		//particleVertices[index] = randomValueVX;
		//index++;
		//particleVertices[index] = randomValueVY;
		//index++;
		//particleVertices[index] = 0.f;
		//index++; //Velocity XYZ
		//v1
		particleVertices[index] = particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = -particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		//particleVertices[index] = randomValueVX;
		//index++;
		//particleVertices[index] = randomValueVY;
		//index++;
		//particleVertices[index] = 0.f;
		//index++; //Velocity XYZ
		//v2
		particleVertices[index] = particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		//particleVertices[index] = randomValueVX;
		//index++;
		//particleVertices[index] = randomValueVY;
		//index++;
		//particleVertices[index] = 0.f;
		//index++; //Velocity XYZ
		//v3
		particleVertices[index] = -particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = -particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		//particleVertices[index] = randomValueVX;
		//index++;
		//particleVertices[index] = randomValueVY;
		//index++;
		//particleVertices[index] = 0.f;
		//index++; //Velocity XYZ
		//v4
		particleVertices[index] = particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		//particleVertices[index] = randomValueVX;
		//index++;
		//particleVertices[index] = randomValueVY;
		//index++;
		//particleVertices[index] = 0.f;
		//index++; //Velocity XYZ
		//v5
		particleVertices[index] = -particleSize / 2.f + randomValueX;
		index++;
		particleVertices[index] = particleSize / 2.f + randomValueY;
		index++;
		particleVertices[index] = 0.f;
		index++;
		//particleVertices[index] = randomValueVX;
		//index++;
		//particleVertices[index] = randomValueVY;
		//index++;
		//particleVertices[index] = 0.f;
		//index++; //Velocity XYZ
	}
	glGenBuffers(1, &m_VBOManyParticle);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatCount, particleVertices, GL_STATIC_DRAW);
	m_VBOManyParticleVertexCount = vertexCount;
	delete[] particleVertices;
}

void Renderer::Lecture3_Particle()
{
	GLuint shader = m_Lecture3ParticleShader;
	glUseProgram(shader);

	int attribPosition = glGetAttribLocation(shader, "a_Position");
	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// 3과 float의 사이즈가 sizeof(float) * 3 이 사이즈와 동일하면 0으로 해도됨
	//glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);	

	glDrawArrays(GL_TRIANGLES, 0, m_VBOManyParticleVertexCount);

	glDisableVertexAttribArray(attribPosition);
}
