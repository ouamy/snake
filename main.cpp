#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <algorithm>

struct Cube { glm::ivec3 pos; glm::vec3 color; };

constexpr int GRID_SIZE = 20;
constexpr float CUBE_SIZE = 1.0f;
constexpr float MOVE_INTERVAL = 0.15f;

float vertices[] = {
    -0.5f, -0.5f, -0.5f, 1,0,0, 0.5f, -0.5f, -0.5f, 1,0,0,
     0.5f,  0.5f, -0.5f, 1,0,0,-0.5f,  0.5f, -0.5f, 1,0,0,
    -0.5f, -0.5f,  0.5f, 0,1,0, 0.5f, -0.5f,  0.5f, 0,1,0,
     0.5f,  0.5f,  0.5f, 0,1,0,-0.5f,  0.5f,  0.5f, 0,1,0
};

unsigned int indices[] = {
    0,1,2,2,3,0,4,5,6,6,7,4,
    4,5,1,1,0,4,6,7,3,3,2,6,
    4,7,3,3,0,4,1,5,6,6,2,1
};

void framebuffer_size_callback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

enum Direction { UP, DOWN, LEFT, RIGHT };

bool operator==(const glm::ivec3& a, const glm::ivec3& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

glm::ivec3 directionToVec(Direction dir) {
    switch(dir) {
        case UP: return {0,0,-1}; case DOWN: return {0,0,1};
        case LEFT: return {-1,0,0}; case RIGHT: return {1,0,0};
    }
    return {0,0,0};
}

void setCubeColor(float* data, const glm::vec3& c) {
    for(int i=0; i<8; i++) {
        data[i*6+3]=c.r; data[i*6+4]=c.g; data[i*6+5]=c.b;
    }
}

int main() {
    srand((unsigned)time(nullptr));
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Snake", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO); glBindVertexArray(VAO);
    glGenBuffers(1, &VBO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glGenBuffers(1, &EBO); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    const char* vs = "#version 330 core\nlayout(location=0) in vec3 aPos;\nlayout(location=1) in vec3 color;\nuniform mat4 MVP;\nout vec3 vColor;\nvoid main(){gl_Position=MVP*vec4(aPos,1.0);vColor=color;}";
    const char* fs = "#version 330 core\nin vec3 vColor;\nout vec4 FragColor;\nvoid main(){FragColor=vec4(vColor,1.0);}";

    auto compile = [](GLenum t, const char* s) {
        unsigned int id = glCreateShader(t);
        glShaderSource(id, 1, &s, nullptr);
        glCompileShader(id);
        return id;
    };

    unsigned int shader = glCreateProgram();
    glAttachShader(shader, compile(GL_VERTEX_SHADER, vs));
    glAttachShader(shader, compile(GL_FRAGMENT_SHADER, fs));
    glLinkProgram(shader);
    glUseProgram(shader);

    std::deque<glm::ivec3> snake = {{GRID_SIZE/2,0,GRID_SIZE/2}};
    Direction dir = RIGHT;
    int length = 4;
    glm::ivec3 food(rand()%GRID_SIZE,0,rand()%GRID_SIZE);
    float timer = 0.0f;

    while(!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime();
        static float last = now;
        float dt = now - last;
        last = now;
        timer += dt;

        if(timer >= MOVE_INTERVAL) {
            timer = 0.0f;
            glm::ivec3 next = snake.back() + directionToVec(dir);
            next.x = (next.x + GRID_SIZE) % GRID_SIZE;
            next.z = (next.z + GRID_SIZE) % GRID_SIZE;

            if(std::find(snake.begin(), snake.end(), next) != snake.end())
                glfwSetWindowShouldClose(window, 1);

            snake.push_back(next);
            if((int)snake.size() > length) snake.pop_front();

            if(next == food) {
                length++;
                do { food = {rand()%GRID_SIZE,0,rand()%GRID_SIZE}; }
                while(std::find(snake.begin(), snake.end(), food) != snake.end());
            }
        }

        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && dir != DOWN) dir = UP;
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && dir != UP) dir = DOWN;
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && dir != RIGHT) dir = LEFT;
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && dir != LEFT) dir = RIGHT;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = glm::lookAt(glm::vec3(GRID_SIZE/2.f,20.f,GRID_SIZE/2.f+5.f), glm::vec3(GRID_SIZE/2.f,0,GRID_SIZE/2.f), glm::vec3(0,1,0));
        glm::mat4 proj = glm::perspective(glm::radians(45.f), 800.f/600.f, 0.1f, 100.f);

        glBindVertexArray(VAO);
        for(const auto& seg : snake) {
            glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(seg));
            glm::mat4 mvp = proj * view * m;
            glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, &mvp[0][0]);
            setCubeColor(vertices, {0,1,0});
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(food));
        glm::mat4 mvp = proj * view * m;
        glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, &mvp[0][0]);
        setCubeColor(vertices, {1,0,0});
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}