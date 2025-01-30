#include "HelloCube.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "core/Core.h"

using namespace OGL4Core2;
using namespace OGL4Core2::Plugins::PCVC::HelloCube;

const static float INIT_CAMERA_DOLLY = 5.0f;
const static float INIT_CAMERA_FOVY = 45.0f;

const static float MIN_CAMERA_PITCH = -89.99f;
const static float MAX_CAMERA_PITCH = 89.99f;

/**
 * @brief HelloCube constructor
 */
HelloCube::HelloCube(const Core::Core& c)
    : Core::RenderPlugin(c),
      aspectRatio(1.6f),
      va(0),
      vbo(0),
      ibo(0),
      shaderProgram(0),
      numCubeFaceTriangles(0),
      texture(0),
      projectionMatrix(glm::mat4(1.0f)),
      viewMatrix(glm::mat4(1.0f)),
      modelMatrix(glm::mat4(1.0f)),
      // --------------------------------------------------------------------------------
      //  TODO: Initialize other variables!
      // --------------------------------------------------------------------------------
      backgroundColor(glm::vec3(0.2f, 0.2f, 0.2f)),
      mouseControlMode(0),
      transformCubeMode(0),
      cameraDolly(INIT_CAMERA_DOLLY),
      cameraFoVy(INIT_CAMERA_FOVY),
      cameraPitch(0.0f),
      cameraYaw(0.0f),
      objTranslateX(0.0f),
      objTranslateY(0.0f),
      objTranslateZ(0.0f),
      objRotateX(0.0f),
      objRotateY(0.0f),
      objRotateZ(0.0f),
      objScaleX(1.0f),
      objScaleY(1.0f),
      objScaleZ(1.0f),
      showWireframe(false),
      showTexture(false),
      patternFreq(0) {
    // Init buffer, shader program and texture.
    initBuffers();
    initShaderProgram();
    initTexture();

    // --------------------------------------------------------------------------------
    //  TODO: Set other stuff if necessary!
    // --------------------------------------------------------------------------------

    setViewMatrix();
    setProjectionMatrix();
    setModelMatrix();

    // Enable depth testing.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

/**
 * @brief HelloCube destructor.
 */
HelloCube::~HelloCube() {
    // --------------------------------------------------------------------------------
    //  TODO: Do not forget to cleanup the plugin!
    // --------------------------------------------------------------------------------
    glDeleteVertexArrays(1, &va);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);
    // Reset OpenGL state.
    glDisable(GL_DEPTH_TEST);
}

/**
 * @brief Render GUI.
 */
void HelloCube::renderGUI() {
    bool cameraChanged = false;
    bool objectChanged = false;
    if (ImGui::CollapsingHeader("Hello Cube", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::ColorEdit3("Background Color", reinterpret_cast<float*>(&backgroundColor), ImGuiColorEditFlags_Float);
        ImGui::Combo("Mouse Control", &mouseControlMode, "Camera\0Object\0");
        if (mouseControlMode == 0) {
            cameraChanged |= ImGui::SliderFloat("Camera Dolly", &cameraDolly, 0.1f, 100.0f);
            cameraChanged |= ImGui::SliderFloat("Camera FoVy", &cameraFoVy, 1.0f, 90.0f);
            cameraChanged |= ImGui::SliderFloat("Camera Pitch", &cameraPitch, MIN_CAMERA_PITCH, MAX_CAMERA_PITCH);
            cameraChanged |= ImGui::SliderFloat("Camera Yaw", &cameraYaw, -360.0f, 360.0f);
        } else {
            ImGui::Combo("TransformCube", &transformCubeMode, "Translate\0Rotate\0Scale\0");
            objectChanged |= ImGui::SliderFloat("Transl. X", &objTranslateX, -10.0f, 10.0f);
            objectChanged |= ImGui::SliderFloat("Transl. Y", &objTranslateY, -10.0f, 10.0f);
            objectChanged |= ImGui::SliderFloat("Transl. Z", &objTranslateZ, -10.0f, 10.0f);
            objectChanged |= ImGui::SliderFloat("Rotate X", &objRotateX, -360.0f, 360.0f);
            objectChanged |= ImGui::SliderFloat("Rotate Y", &objRotateY, -360.0f, 360.0f);
            objectChanged |= ImGui::SliderFloat("Rotate Z", &objRotateZ, -360.0f, 360.0f);
            objectChanged |= ImGui::SliderFloat("Scale X", &objScaleX, 0.1f, 10.0f);
            objectChanged |= ImGui::SliderFloat("Scale Y", &objScaleY, 0.1f, 10.0f);
            objectChanged |= ImGui::SliderFloat("Scale Z", &objScaleZ, 0.1f, 10.0f);
        }
        if (ImGui::Button("Reset cube")) {
            resetCube();
        }
        if (ImGui::Button("Reset camera")) {
            resetCamera();
        }
        if (ImGui::Button("Reset all")) {
            resetCube();
            resetCamera();
        }
        ImGui::Checkbox("Wireframe", &showWireframe);
        ImGui::Checkbox("Texture", &showTexture);
        ImGui::InputInt("Pattern Freq.", &patternFreq);
        patternFreq = std::max(0, std::min(15, patternFreq));
        if (cameraChanged) {
            // --------------------------------------------------------------------------------
            //  TODO: Handle camera parameter changes!
            // --------------------------------------------------------------------------------
            setViewMatrix();
            setProjectionMatrix();
        }
        if (objectChanged) {
            // --------------------------------------------------------------------------------
            //  TODO: Handle object parameter changes!
            // --------------------------------------------------------------------------------
            setModelMatrix();
        }
    }
}

/**
 * @brief HelloCube render callback.
 */
void HelloCube::render() {
    renderGUI();

    // --------------------------------------------------------------------------------
    //  TODO: Implement rendering!
    // --------------------------------------------------------------------------------
    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (showWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    float showTextureControl = showTexture ? 1.0f : 0.0f;
    glUniform1f(glGetUniformLocation(shaderProgram, "showTexture"), showTextureControl);
    glUniform1i(glGetUniformLocation(shaderProgram, "patternFreq"), patternFreq);

    glBindVertexArray(va);
    glDrawElements(GL_TRIANGLES, numCubeFaceTriangles * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/**
 * @brief HelloCube resize callback.
 * @param width
 * @param height
 */
void HelloCube::resize(int width, int height) {
    // --------------------------------------------------------------------------------
    //  TODO: Handle resize event!
    // --------------------------------------------------------------------------------
    glViewport(0, 0, width, height);
    aspectRatio = float(width) / float(height);
    std::cout << "Aspect:" << aspectRatio << "\n "<< std::endl;

    setViewMatrix();
    setProjectionMatrix();
}

/**
 * @brief HellCube mouse move callback.
 * @param xpos
 * @param ypos
 */
void HelloCube::mouseMove(double xpos, double ypos) {
    // --------------------------------------------------------------------------------
    //  TODO: Handle camera and object control!
    // --------------------------------------------------------------------------------
    double dx = xpos - pxpos;
    double dy = ypos - pypos;
    pxpos = xpos;
    pypos = ypos;
    double sensitivity = 50.0;
    if (mouseControlMode == 0){
        if (core_.isMouseButtonPressed(Core::MouseButton::Left)) {
            cameraYaw -= dx / sensitivity;
            cameraPitch -= dy / sensitivity;
        } else if (core_.isMouseButtonPressed(Core::MouseButton::Middle)) {
            cameraFoVy += dy / sensitivity;
        } else if (core_.isMouseButtonPressed(Core::MouseButton::Right)) {
            cameraDolly += dy / sensitivity;
        }

        setViewMatrix();
        setProjectionMatrix();
    }
    else {
        if (transformCubeMode == 0) {
            if (core_.isMouseButtonPressed(Core::MouseButton::Left)) {
                objTranslateX += dx / sensitivity;
                objTranslateY -= dy / sensitivity;
            } else if (core_.isMouseButtonPressed(Core::MouseButton::Middle)) {
                objTranslateX += dx / sensitivity;
                objTranslateZ -= dy / sensitivity;
            } else if (core_.isMouseButtonPressed(Core::MouseButton::Right)) {
                objTranslateY += dx / sensitivity;
                objTranslateZ -= dy / sensitivity;
            }
        } else if (transformCubeMode == 1) {
            if (core_.isMouseButtonPressed(Core::MouseButton::Left)) {
                objRotateY += dx / sensitivity;
                objRotateX += dy / sensitivity;
            } else if (core_.isMouseButtonPressed(Core::MouseButton::Middle)) {
                objRotateX += dx / sensitivity;
                objRotateZ += dy / sensitivity;
            } else if (core_.isMouseButtonPressed(Core::MouseButton::Right)) {
                objRotateY += dx / sensitivity;
                objRotateZ += dy / sensitivity;
            }
        } else {
            if (core_.isMouseButtonPressed(Core::MouseButton::Left)) {
                objScaleX += dx / sensitivity;
                objScaleY -= dy / sensitivity;
            } else if (core_.isMouseButtonPressed(Core::MouseButton::Middle)) {
                objScaleX += dx / sensitivity;
                objScaleZ -= dy / sensitivity;
            } else if (core_.isMouseButtonPressed(Core::MouseButton::Right)) {
                objScaleY += dx / sensitivity;
                objScaleZ -= dy / sensitivity;
            }
        }

        setModelMatrix();
    }

}

/**
 * @brief Initialize buffers.
 */
void HelloCube::initBuffers() {
    // --------------------------------------------------------------------------------
    //  TODO: Init vertex buffer and index buffer!
    // --------------------------------------------------------------------------------
    const int numCubeVertices = 8;

    const std::vector<float> cubeVertices{
        -0.5f, -0.5f, -0.5f,    0.25f, 0.25f, // left  bottom back
        0.5f, -0.5f, -0.5f,     0.5f, 0.25f, // right bottom back
        -0.5f,  0.5f, -0.5f,    0.25f, 1.0f, // left  top    back
        0.5f,  0.5f, -0.5f,     0.5f, 1.0f, // right top    back
        -0.5f, -0.5f,  0.5f,    0.25f, 0.5f, // left  bottom front
        0.5f, -0.5f,  0.5f,     0.5f, 0.5f, // right bottom front
        -0.5f,  0.5f,  0.5f,    0.25f, 0.75f, // left  top    front
        0.5f,  0.5f,  0.5f,     0.5f, 0.75f, // right top    front

        -0.5f, -0.5f, -0.5f,    0.0f, 0.5f, // left  bottom back    left face
        -0.5f, 0.5f, -0.5f,     0.0f, 0.75f,   // left  top    back    left face

        0.5f, -0.5f, -0.5f,     0.75f, 0.5f, // right bottom back     right and back face
        0.5f, 0.5f, -0.5f,      0.75f, 0.75f,   // right top    back     right and back face

        -0.5f, -0.5f, -0.5f,    1.0f, 0.5f, // left  bottom back    back face
        -0.5f, 0.5f, -0.5f,     1.0f, 0.75f,   // left  top    back    back face
    };


    numCubeFaceTriangles = 12;

    const std::vector<GLuint> cubeFaces{
        12, 13, 10,  10, 13, 11, // back
        5, 7, 4,  4, 7, 6, // front
        1, 5, 0,  0, 5, 4, // bottom
        7, 3, 6,  6, 3, 2, // top
        4, 6, 8,  8, 6, 9, // left
        10, 11, 5,  5, 11, 7  // right
    };

    glGenVertexArrays(1, &va);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    glBindVertexArray(va);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeFaces.size() * sizeof(GLuint), cubeFaces.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

}


/**
 * @brief HelloCube init shaders
 */
void HelloCube::initShaderProgram() {
    // --------------------------------------------------------------------------------
    //  TODO: Init shader program!
    // --------------------------------------------------------------------------------
    int success;
    char infoLog[512];
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open("../src/plugins/PCVC/HelloCube/resources/cube.vert");
        fShaderFile.open("../src/plugins/PCVC/HelloCube/resources/cube.frag");
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }else
        std::cout << "SHADER::VERTEX::COMPILATION_SUCCESSED\n" << std::endl;

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    } else
        std::cout << "SHADER::FRAGMENT::COMPILATION_SUCCESSED\n" << std::endl;

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::fuck LINKING_FAILED\n" << infoLog << std::endl;
    } else
        std::cout << "SHADER::PROGRAM::LINKING_SUCCESSED\n" << std::endl;

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);
}

/**
 * @brief Initialize textures.
 */
void HelloCube::initTexture() {
    // --------------------------------------------------------------------------------
    //  TODO: Init texture!
    // --------------------------------------------------------------------------------
    int width, height;
    auto image = getPngResource("texture.png", width, height);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLint textureLoc = glGetUniformLocation(shaderProgram, "Tex");
    glUniform1i(textureLoc, 0);
}

/**
 * @brief Reset cube.
 */
void HelloCube::resetCube() {
    // --------------------------------------------------------------------------------
    //  TODO: Reset cube!
    // --------------------------------------------------------------------------------
    objTranslateX = 0.0f;
    objTranslateY = 0.0f;
    objTranslateZ = 0.0f;
    objRotateX = 0.0f;
    objRotateY = 0.0f;
    objRotateZ = 0.0f;
    objScaleX = 1.0f;
    objScaleY = 1.0f;
    objScaleZ = 1.0f;

    setModelMatrix();
}

/**
 * @brief Reset camera.
 */
void HelloCube::resetCamera() {
    // --------------------------------------------------------------------------------
    //  TODO: Reset camera!
    // --------------------------------------------------------------------------------
    cameraDolly = INIT_CAMERA_DOLLY;
    cameraFoVy = INIT_CAMERA_FOVY;
    cameraPitch = 0.0f;
    cameraYaw = 0.0f;

    setViewMatrix();
    setProjectionMatrix();
}

/**
 * @brief Set projection matrix.
 */
void HelloCube::setProjectionMatrix() {
    // --------------------------------------------------------------------------------
    //  TODO: Set projection matrix!
    // --------------------------------------------------------------------------------

    float tanHalfFovy = std::tan(glm::radians(cameraFoVy) / 2.0f);
    float near = 0.1;
    float far = 100.0; // cameraDolly is limited to 0.1 to 100.0, so far plane doesn't need to be very far. near can be very near.

    projectionMatrix[0][0] = 1.0f / (aspectRatio * tanHalfFovy);
    projectionMatrix[1][1] = 1.0f / (tanHalfFovy);
    projectionMatrix[2][2] = -(far + near) / (far - near);
    projectionMatrix[2][3] = -1.0f;
    projectionMatrix[3][2] = -(2.0f * far * near) / (far - near);
    projectionMatrix[3][3] = 0.0f;

    GLint projLoc = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
}

/**
 * @brief Set view matrix.
 */
void HelloCube::setViewMatrix() {
    // --------------------------------------------------------------------------------
    //  TODO: Set view matrix!
    // --------------------------------------------------------------------------------

    glm::mat4 translationMatrix = glm::mat4(1.0f);
    translationMatrix[3][0] = 0.0f;
    translationMatrix[3][1] = 0.0f;
    translationMatrix[3][2] = -cameraDolly;

    float cosPitch = std::cos(glm::radians(cameraPitch));
    float sinPitch = std::sin(glm::radians(cameraPitch));

    glm::mat4 pitchMatrix = glm::mat4(1.0f);
    pitchMatrix[1][1] = cosPitch;
    pitchMatrix[1][2] = -sinPitch;
    pitchMatrix[2][1] = sinPitch;
    pitchMatrix[2][2] = cosPitch;

    float cosYaw = std::cos(glm::radians(cameraYaw));
    float sinYaw = std::sin(glm::radians(cameraYaw));

    glm::mat4 yawMatrix = glm::mat4(1.0f);
    yawMatrix[0][0] = cosYaw;
    yawMatrix[0][2] = sinYaw;
    yawMatrix[2][0] = -sinYaw;
    yawMatrix[2][2] = cosYaw;

    viewMatrix = translationMatrix * yawMatrix * pitchMatrix;

    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
}

/**
 * @brief Set model matrix.
 */
void HelloCube::setModelMatrix() {
    // --------------------------------------------------------------------------------
    //  TODO: Set model matrix!
    // --------------------------------------------------------------------------------

    // scale matrix
    glm::mat4 scaleMatrix = glm::mat4(1.0f);
    scaleMatrix[0][0] = objScaleX;
    scaleMatrix[1][1] = objScaleY;
    scaleMatrix[2][2] = objScaleZ;

    // translation matrix
    glm::mat4 translationMatrix = glm::mat4(1.0f);
    translationMatrix[3][0] = objTranslateX;
    translationMatrix[3][1] = objTranslateY;
    translationMatrix[3][2] = objTranslateZ;

    // rotation matrix
    glm::mat4 rotationXMatrix = glm::mat4(1.0f);
    float cosX = std::cos(glm::radians(objRotateX));
    float sinX = std::sin(glm::radians(objRotateX));
    rotationXMatrix[1][1] = cosX;
    rotationXMatrix[1][2] = -sinX;
    rotationXMatrix[2][1] = sinX;
    rotationXMatrix[2][2] = cosX;

    glm::mat4 rotationYMatrix = glm::mat4(1.0f);
    float cosY = std::cos(glm::radians(objRotateY));
    float sinY = std::sin(glm::radians(objRotateY));
    rotationYMatrix[0][0] = cosY;
    rotationYMatrix[0][2] = sinY;
    rotationYMatrix[2][0] = -sinY;
    rotationYMatrix[2][2] = cosY;

    glm::mat4 rotationZMatrix = glm::mat4(1.0f);
    float cosZ = std::cos(glm::radians(objRotateZ));
    float sinZ = std::sin(glm::radians(objRotateZ));
    rotationZMatrix[0][0] = cosZ;
    rotationZMatrix[0][1] = -sinZ;
    rotationZMatrix[1][0] = sinZ;
    rotationZMatrix[1][1] = cosZ;

    modelMatrix = translationMatrix * rotationZMatrix * rotationYMatrix * rotationXMatrix * scaleMatrix;

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    modelLoc = glGetUniformLocation(shaderProgram, "scale");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaleMatrix));
}
