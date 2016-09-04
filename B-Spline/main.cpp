#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#define SEGMENT 20
#define SEGMENT_XY 40

using namespace std;

vector<double> cp;
int mouseCount = 0;

// shader code
const GLchar *vertexShaderSource =
"#version 330 core\n"
"\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 in_color;\n"
"out vec3 ex_color;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
"	ex_color = in_color;\n"
"}\n";
const GLchar *fragmentShaderSource =
"#version 330 core\n"
"\n"
"in vec3 ex_color;\n"
"out vec4 color;\n"
"\n"
"void main()\n"
"{\n"
"	color = vec4(ex_color, 1.0);\n"
"}\n";

void coxDeBoor(vector<vector<double>> &bfunc, double t, int n, int d, vector<double> u)
{
	for (int i = 0; i < d; i++) {
		for (int j = 0; j < n + 1; j++) {
			if (i == 0) {
				if (t >= u[j] && t <= u[j + 1])
					bfunc[i][j] = 1;
				else
					bfunc[i][j] = 0;
			}
			else {
				int tmpa = ((u[j + d - 1] - u[j]) == 0) ? 0 : (t - u[j]) / (u[j + d - 1] - u[j]),
					tmpb = ((u[j + d] - u[j + 1]) == 0) ? 0 : (u[j + d] - t) / (u[j + d] - u[j + 1]);

				if (j + 1 > n)
					bfunc[i][j] = tmpa * bfunc[i - 1][j];
				else
					bfunc[i][j] = (tmpa * bfunc[i - 1][j]) + (tmpb * bfunc[i - 1][j + 1]);
			}
		}
	}

	/*for (int i = 0; i < d; i++) {
		for (int j = 0; j < n + 1; j++)
			cout << bfunc[i][j] << ", ";
		cout << endl;
	}*/
}

void computeCurve(GLdouble *curve, GLdouble *cpa, int n, int d, vector<double> u, int umin, int umax)
{
	vector<vector<double>> bfunc(d, vector<double>(n + 1));
	
	/*coxDeBoor(bfunc, 0, n, d, u);
	for (int i = 0; i < d; i++) {
		for (int j = 0; j < n + 1; j++)
			cout << bfunc[i][j] << ", ";
		cout << endl;
	}*/
}

void getControlPoint(GLFWwindow *window)
{
	double xpos, ypos, xposNDC, yposNDC;
	int width, height;
	
	glfwGetCursorPos(window, &xpos, &ypos);
	glfwGetWindowSize(window, &width, &height);
	cout << "getCursoPos: (" << xpos << ", " << ypos << ")" << endl;
	xposNDC = (xpos / width) * 2 - 1;
	yposNDC = -(ypos / height) * 2 + 1;
	cp.push_back(xposNDC);
	cp.push_back(yposNDC);
	cout << "getCursoPos(NDC): (" << xposNDC << ", " << yposNDC << ")" << endl;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mode)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
		mouseCount++;
		getControlPoint(window);
	}
}

int main()
{
	/************** initialize **************/
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	
	GLFWwindow *window = glfwCreateWindow(800, 800, "B-Spline", nullptr, nullptr);
	if (window == nullptr) {
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		cout << "Failed to initialize GLEW" << endl;
		return -1;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	/****************************************/

	/**************** shader ****************/
	// vertexShader
	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	GLint success; // debug
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
	}

	// fragmentShader
	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); // debug
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}

	// shaderProgram
	GLuint shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success); // debug
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	/****************************************/

	// input
	int n, d, cpnum, unum;
	double umin, umax;
	GLdouble *cpa, curve[SEGMENT_XY];
	vector<double> u;

	cout << "Please input the parameter n and d:" << endl;
	cin >> n >> d;
	cpnum = n + 1;
	cpa = new GLdouble[cpnum * 2];
	unum = n + d + 1;
	cout << "Please input the umin and umax:" << endl;
	cin >> umin >> umax;
	cout << "Please input " << unum << " knot values:" << endl;
	for (int i = 0; i < unum; i++) {
		double tmp;
		cin >> tmp;
		u.push_back(tmp);
	}
	cout << "Please input " << cpnum << " control points:(by cursor)" << endl;

	computeCurve(curve, cpa, n, d, u, umin, umax);

	GLuint cpVBO;
	glGenBuffers(1, &cpVBO);
	GLuint cpVAO;
	glGenVertexArrays(1, &cpVAO);

	glBindVertexArray(cpVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cpVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * cpnum * 2, cpa, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_DOUBLE, GL_TRUE, 2 * sizeof(GLdouble), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// loop
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		if (mouseCount >= cpnum){
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glUseProgram(shaderProgram);

			// draw control points
			for (int i = 0; i < cpnum * 2; i++) {
				cpa[i] = cp[i];
			}
			glBindVertexArray(cpVAO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * cpnum * 2, cpa, GL_DYNAMIC_DRAW);
			glVertexAttribPointer(0, 2, GL_DOUBLE, GL_TRUE, 2 * sizeof(GLdouble), (GLvoid*)0);
			glVertexAttrib3f(1, 1.0, 1.0, 1.0);
			glPointSize(20.0f);
			glDrawArrays(GL_POINTS, 0, cpnum);

			cout << "DRAWED." << endl;

			glBindVertexArray(0);
			glfwSwapBuffers(window);
			mouseCount = 0;
			cp.clear();
		}
	}

	glfwTerminate();
	delete[] cpa;
	return 0;
}