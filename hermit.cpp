#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>

#define WIDTH 640
#define HEIGHT 640
#define CURVE_ARR_SIZE 300
#define POINT_ARR_SIZE 24

GLuint vertShader, fragShader, fragShader2;
GLuint shaderProgram, shaderProgram2;

GLfloat curve1[CURVE_ARR_SIZE], curve2[CURVE_ARR_SIZE], curve3[CURVE_ARR_SIZE];
GLfloat* curves[3] = { curve1, curve2, curve3 };
GLfloat t, step = 1.0f / 99.0f;
GLint i, j, dragged = -1;
GLuint VAOs[4], VBOs[4];

GLfloat points[POINT_ARR_SIZE] = {
    -0.8f,  -0.25f, 0.0f,
    -0.65f,  0.6f,  0.0f,
    -0.35f, -0.05f, 0.0f,
    -0.05f, -0.9f,  0.0f,
     0.2f,  -0.2f,  0.0f,
    -0.05f, -0.7f,  0.0f,
     0.7f,  -0.1f,  0.0f,
     0.9f,   0.6f,  0.0f,
};


GLfloat dist2_2d(GLfloat P1x, GLfloat P1y, GLfloat P2x, GLfloat P2y) {

    GLfloat dx = P1x - P2x;
    GLfloat dy = P1y - P2y;
    return dx * dx + dy * dy;
}

GLint getActivePoint(GLfloat* p, GLfloat sensitivity, GLfloat x, GLfloat y) {

    GLfloat	s = sensitivity * sensitivity;
    GLfloat	xNorm = -1 + x / (WIDTH / 2);
    GLfloat	yNorm = -1 + (HEIGHT - y) / (HEIGHT / 2);

    for (GLint i = 0; i < 8; i++)
        if (dist2_2d(p[i * 3], p[i * 3 + 1], xNorm, yNorm) < s)
            return i;

    return -1;
}

void initVBOs() {


    for (int i = 0; i < 3; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, CURVE_ARR_SIZE * sizeof(GLfloat), curves[i], GL_STATIC_DRAW);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
    glBufferData(GL_ARRAY_BUFFER, POINT_ARR_SIZE * sizeof(GLfloat), points, GL_STATIC_DRAW);

}

void createCurves(void) {

    GLfloat R1x, R1y, R2x, R2y;

    for (int j = 0; j < 3; ++j) {

        curves[j][0] = points[j*6];
        curves[j][1] = points[j*6+1];
        curves[j][2] = 0.0f;

        R1x = points[3* j + 3] - points[j * 6];
        R1y = points[3 * j + 4] - points[j * 6 + 1];
        R2x = points[6* j + 9] - points[6*j+6];
        R2y = points[6* j + 10] - points[6*j + 7];

        for (i = 1; i < 99; i++) {
            t = 0 + i * step;
            curves[j][i * 3] = points[j*6] * (2.0f * t * t * t - 3.0f * t * t + 1.0f) + points[6* j + 6] * (-2.0f * t * t * t + 3.0f * t * t) + R1x * (t * t * t - 2.0f * t * t + t) + R2x * (t * t * t - t * t);
            curves[j][i * 3 + 1] = points[j*6 + 1] * (2.0f * t * t * t - 3.0f * t * t + 1.0f) + points[ 6*j +7] * (-2.0f * t * t * t + 3.0f * t * t) + R1y * (t * t * t - 2.0f * t * t + t) + R2y * (t * t * t - t * t);
            curves[j][i * 3 + 2] = 0.0f;

        }

        curves[j][297] = points[(j+1) * 6];
        curves[j][298] = points[(j + 1) * 6+1];
        curves[j][299] = 0.0f;

    }
}

void cursorPosCallback(GLFWwindow* window, double x, double y) {

    if (dragged >= 0) {

        GLfloat	xNorm = -1 + x / (WIDTH / 2);
        GLfloat	yNorm = -1 + (HEIGHT - y) / (HEIGHT / 2);

        points[3 * dragged] = xNorm;  // x coord
        points[3 * dragged + 1] = yNorm;  // y coord

        createCurves();
        initVBOs();
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double	x, y;

        glfwGetCursorPos(window, &x, &y);
        dragged = getActivePoint(points, 0.1f, x, y);
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        dragged = -1;
}


void drawCurve() {
    glUseProgram(shaderProgram);

    for (int i = 0; i < 3; ++i) {
        glLineWidth(20);
        glBindVertexArray(VAOs[i]);
        glDrawArrays(GL_LINE_STRIP, 0, 100);
    }
}

void drawPoints() {
    glPointSize(15);
    glUseProgram(shaderProgram2);
    glBindVertexArray(VAOs[3]);
    glDrawArrays(GL_POINTS, 0, 8);
}

void drawLinesBetweenPoints() {

    glUseProgram(shaderProgram2);
    for (int i = 0; i <= 6; i += 2) {
        glDrawArrays(GL_LINES, i, 2);
    }
}

void initVAOs() {
    glGenVertexArrays(4, VAOs);

    for (int i = 0; i < 4; ++i) {
        glBindVertexArray(VAOs[i]);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    }
}

void initShaders() {
    const char* vShader =
        "#version 410\n"
        "in vec3 vp;"
        "void main () {"
        "  gl_Position = vec4(vp, 1.0);"
        "}";


    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vShader, NULL);
    glCompileShader(vertShader);

    const char* fShader =
        "#version 410\n"
        "out vec4 frag_colour;"
        "void main () {"
        "  frag_colour = vec4(0.0, 0.0, 0.0, 1.0);"
        "}";

    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fShader, NULL);
    glCompileShader(fragShader);

    const char* fShader2 =
        "#version 410\n"
        "out vec4 frag_colour;"
        "void main () {"
        "  frag_colour = vec4(1.0, 0.0, 0.0, 1.0);"
        "}";

    fragShader2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader2, 1, &fShader2, NULL);
    glCompileShader(fragShader2);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, fragShader);
    glAttachShader(shaderProgram, vertShader);
    glLinkProgram(shaderProgram);

    shaderProgram2 = glCreateProgram();
    glAttachShader(shaderProgram2, fragShader2);
    glAttachShader(shaderProgram2, vertShader);
    glLinkProgram(shaderProgram2);

}

int main() {
    GLFWwindow* window = NULL;
    const GLubyte* renderer;
    const GLubyte* version;


    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Hermit Curve", NULL, NULL);
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    renderer = glGetString(GL_RENDERER);
    version = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    createCurves();

    glGenBuffers(4, VBOs);
    initVBOs();
    initVAOs();
    initShaders();

    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    while (!glfwWindowShouldClose(window)) {

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawPoints();
        drawLinesBetweenPoints();
        drawCurve();


        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}