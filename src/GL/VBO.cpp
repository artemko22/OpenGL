#include "VBO.h"
#include "glad/glad.h"
#include "glfw3.h"

unsigned int VBO::createVBO(std::vector<float> vertices)
{
    glGenBuffers(1, &this->VBO);

    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices.front(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    return this->VBO;
}
