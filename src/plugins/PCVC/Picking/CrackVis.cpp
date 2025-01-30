#include "CrackVis.h"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <utility>
#include <fileSystem>
#include <imgui.h>

#include "Objects.h"
#include "Mesh.h"
#include "core/Core.h"

using namespace OGL4Core2;
using namespace OGL4Core2::Plugins::PCVC::CrackVis;

/**
 * @brief CrackVis constructor.
 * Initlizes all variables with meaningful values and initializes
 * geometry, objects and shaders used in the scene.
 */
CrackVis::CrackVis(const Core::Core& c)
    : Core::RenderPlugin(c),
      wWidth(10),
      wHeight(10),
      aspect(1.6f),
      lastMouseX(0.0),
      lastMouseY(0.0),
      moveMode(ObjectMoveMode::None),
      projMx(glm::mat4(1.0)),
      fbo(0),
      fboTexColor(0),
      fboTexId(0),
      fboTexNormals(0),
      fboTexDepth(0),
      pickedObjNum(-1),
      lightZnear(0.1f),
      lightZfar(50.0f),
      lightProjMx(glm::mat4(1.0)),
      lightViewMx(glm::mat4(1.0)),
      lightFboWidth(2048),
      lightFboHeight(2048),
      lightFbo(0),
      lightFboTexColor(0),
      lightFboTexDepth(0),
      backgroundColor(glm::vec3(0.f, 0.f, 0.f)),
      useWireframe(false),
      showFBOAtt(0),
      fovY(45.0),
      zNear(0.01f),
      zFar(20.0f),
      lightLong(0.0f),
      lightLat(90.0f),
      lightDist(10.0f),
      lightFoV(45.0f) {
    // Init Camera
    camera = std::make_shared<Core::OrbitCamera>(5.0f);
    core_.registerCamera(camera);

    // Initialize shaders and vertex arrays for quad and box.
    initShaders();
    initVAs();

    // --------------------------------------------------------------------------------
    //  TODO: Load textures from the "resources/textures" folder.
    //        Use the "getTextureResource" helper function.
    // --------------------------------------------------------------------------------
    texDice = getTextureResource("textures/dice.png");
    texBoard = getTextureResource("textures/board.png");
    texEarth = getTextureResource("textures/earth.png");

    // --------------------------------------------------------------------------------
    //  TODO: Setup the 3D scene. Add a dice, a sphere, and a torus.
    // --------------------------------------------------------------------------------
    std::string path = getResourceFilePath("models/backpack/backpack.obj").string();
    Model backpack(*this, path);


    std::shared_ptr<Object> o1 = std::make_shared<Base>(*this, 1, texBoard);
    o1->modelMx = glm::translate(o1->modelMx, glm::vec3(0.0f, 0.0f, -0.6f));
    o1->modelMx = glm::scale(o1->modelMx, glm::vec3(5.0f, 5.0f, 0.01f));
    objectList.emplace_back(o1);

    std::shared_ptr<Object> cube = std::make_shared<Cube>(*this, 2, texDice);       
    cube->modelMx = glm::translate(cube->modelMx, glm::vec3(-0.5f, 1.3f, 0.0f));
    objectList.push_back(cube);

    std::shared_ptr<Object> sphere = std::make_shared<Sphere>(*this, 3, texEarth);
    sphere->modelMx = glm::translate(sphere->modelMx, glm::vec3(1.0f, 0.0f, 0.0f));
    sphere->modelMx = glm::scale(sphere->modelMx, glm::vec3(1.1f));
    objectList.push_back(sphere);

    std::shared_ptr<Object> torus = std::make_shared<Torus>(*this, 4, nullptr);
    torus->modelMx = glm::translate(torus->modelMx, glm::vec3(-0.5f, -0.5f, -0.2f));
    torus->modelMx = glm::scale(torus->modelMx, glm::vec3(1.5f));
    objectList.push_back(torus);

    // Request some parameters
    GLint maxColAtt;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColAtt);
    std::cerr << "Maximum number of color attachments: " << maxColAtt << std::endl;

    GLint maxGeomOuputVerts;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxGeomOuputVerts);
    std::cerr << "Maximum number of geometry output vertices: " << maxGeomOuputVerts << std::endl;

    // Initialize clear color and enable depth testing
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

/**
 * @brief CrackVis destructor.
 */
CrackVis::~CrackVis() {
    // --------------------------------------------------------------------------------
    //  Note: The glowl library uses the RAII principle. OpenGL objects are deleted in
    //        the destructor of the glowl wrapper objects. Therefore we must not delete
    //        them on our own. But keep this in mind and remember that this not always
    //        happens automatically.
    //
    //  TODO: Do not forget to clear all other allocated sources!
    // --------------------------------------------------------------------------------

    // Reset OpenGL state.
    glDisable(GL_DEPTH_TEST);

    deleteFBOs();

    shaderQuad.reset();
    shaderBox.reset();
    vaQuad.reset();
    vaBox.reset();

    objectList.clear();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief Render GUI.
 */
void CrackVis::renderGUI() {
    if (ImGui::CollapsingHeader("CrackVis", ImGuiTreeNodeFlags_DefaultOpen)) {
        camera->drawGUI();
        ImGui::ColorEdit3("Background Color", reinterpret_cast<float*>(&backgroundColor), ImGuiColorEditFlags_Float);
        ImGui::Checkbox("Wireframe", &useWireframe);
        // Dropdown menu to choose which framebuffer attachment to show
        ImGui::Combo("FBO attach.", &showFBOAtt, "Color\0IDs\0Normals\0Depth\0LightView\0LightDepth\0Deferred\0");

        // --------------------------------------------------------------------------------
        //  TODO: Setup ImGui elements for 'fovY', 'zNear', 'ZFar', 'lightLong', 'lightLat',
        //        'lightDist'. For the bonus task, you also need 'lightFoV'.
        // --------------------------------------------------------------------------------
        ImGui::SliderFloat("FOV Y", &fovY, 50.0f, 120.0f);
        ImGui::SliderFloat("Near Clipping Plane", &zNear, 0.01f, 10.0f);
        ImGui::SliderFloat("Far Clipping Plane", &zFar, 10.0f, 1000.0f);

        ImGui::SliderFloat("Light Longitude", &lightLong, -180.0f, 180.0f);
        ImGui::SliderFloat("Light Latitude", &lightLat, -90.0f, 90.0f);
        ImGui::SliderFloat("Light Distance", &lightDist, 0.1f, 10.0f);
        ImGui::SliderFloat("Light FoV", &lightFoV, 20.0f, 45.0f);
    }
}

/**
 * @brief CrackVis render callback.
 */
void CrackVis::render() {
    renderGUI();

    // Update the matrices for current frame.
    updateMatrices();

    // --------------------------------------------------------------------------------
    //  TODO: First render pass to fill the FBOs. Call the the drawTo... method(s).
    // --------------------------------------------------------------------------------
    drawToFBO();

    // --------------------------------------------------------------------------------
    //  TODO: In the second render pass, a window filling quad is drawn and the FBO
    //    textures are used for deferred shading.
    // --------------------------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, wWidth, wHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 orthoMx = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

    shaderQuad->use();
    switch (showFBOAtt) {
        case 0:
            shaderQuad->setUniform("showMode", 0);
            break;
        case 1:
            shaderQuad->setUniform("showMode", 1);
            break;
        case 2:
            shaderQuad->setUniform("showMode", 2);
            break;
        case 3:
            shaderQuad->setUniform("showMode", 3);
            break;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboTexColor);
    shaderQuad->setUniform("tex", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fboTexId);
    shaderQuad->setUniform("idTex", 1);

    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, fboTexNormals);
    shaderQuad->setUniform("normalTex", 2);

    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, fboTexDepth);
    shaderQuad->setUniform("depthTex", 3);

    shaderQuad->setUniform("orthoProjMx", orthoMx);

    shaderQuad->setUniform("lightLong", glm::radians(lightLong));
    shaderQuad->setUniform("lightLat", glm::radians(lightLat));
    shaderQuad->setUniform("lightDist", lightDist);
    shaderQuad->setUniform("lightFoV", lightFoV);

    shaderQuad->setUniform("projMx", projMx);
    shaderQuad->setUniform("viewMx", camera->viewMx());

    vaQuad->draw();


    glUseProgram(0); // release shaderQuad
}

/**
 * @brief CrackVis resize callback.
 * @param width  The current width of the window
 * @param height The current height of the window
 */
void CrackVis::resize(int width, int height) {

    if (width > 0 && height > 0) {
        wWidth = width;
        wHeight = height;
        aspect = static_cast<float>(wWidth) / static_cast<float>(wHeight);
        updateMatrices();
        // Every time the window size changes, the size, of the fbo has to be adapted.
        deleteFBOs();
        initFBOs();
    }
}

/**
 * @brief CrackVis keyboard callback.
 * @param key      Which key caused the event
 * @param action   Which key action was performed
 * @param mods     Which modifiers were active (e. g. shift)
 */
void CrackVis::keyboard(Core::Key key, Core::KeyAction action, [[maybe_unused]] Core::Mods mods) {
    if (action != Core::KeyAction::Press) {
        return;
    }

    if (key == Core::Key::R) {
        std::cout << "Reload shaders!" << std::endl;
        initShaders();
        for (const auto& object : objectList) {
            object->reloadShaders();
        }
    } else if (key >= Core::Key::Key1 && key <= Core::Key::Key7) {
        showFBOAtt = static_cast<int>(key) - static_cast<int>(Core::Key::Key1);
    }
}

/**
 * @brief CrackVis mouse callback.
 * @param button   Which mouse button caused the event
 * @param action   Which mouse action was performed
 * @param mods     Which modifiers were active (e. g. shift)
 */
void CrackVis::mouseButton(Core::MouseButton button, Core::MouseButtonAction action, Core::Mods mods) {
    // --------------------------------------------------------------------------------
    //  TODO: Add mouse button functionality.
    // --------------------------------------------------------------------------------
    if (button == Core::MouseButton::Left && mods.control() && action == Core::MouseButtonAction::Press) {
        int mouseX = static_cast<int>(lastMouseX);
        int mouseY = static_cast<int>(lastMouseY);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        glReadPixels(mouseX, wHeight - mouseY, 1, 1, GL_RED_INTEGER, GL_INT, &pickedObjNum);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        std::cout << pickedObjNum << std::endl;
        if (pickedObjNum <= 0) {
            pickedObjNum = -1;
        };
    };
    if (button == Core::MouseButton::Right && mods.control() && action == Core::MouseButtonAction::Press)
        moveMode = ObjectMoveMode::XY;
    else if (button == Core::MouseButton::Middle && mods.control() && action == Core::MouseButtonAction::Press)
        moveMode = ObjectMoveMode::Z;
    else
        moveMode = ObjectMoveMode::None;
}

/**
 * @brief CrackVis mouse move callback.
 * Called after the mouse was moved, coordinates are measured in screen coordinates but
 * relative to the top-left corner of the window.
 * @param xpos     The x position of the mouse cursor
 * @param ypos     The y position of the mouse cursor
 */
void CrackVis::mouseMove(double xpos, double ypos) {
    // --------------------------------------------------------------------------------
    //  TODO: Add mouse move functionality.
    // --------------------------------------------------------------------------------

    float dx = static_cast<float>(xpos - lastMouseX);
    float dy = static_cast<float>(ypos - lastMouseY);
    lastMouseX = xpos;
    lastMouseY = ypos;

    if (pickedObjNum <= 0 || moveMode == ObjectMoveMode::None)
        return;
    glm::mat4 viewMx = camera->viewMx();

    glm::vec3 right = glm::vec3(viewMx[0][0], viewMx[1][0], viewMx[2][0]);
    glm::vec3 up = glm::vec3(viewMx[0][1], viewMx[1][1], viewMx[2][1]);
    glm::vec3 forward = glm::vec3(viewMx[0][2], viewMx[1][2], viewMx[2][2]);

    if (moveMode == ObjectMoveMode::XY) {
        objectList[pickedObjNum-1]->modelMx = glm::translate(objectList[pickedObjNum-1]->modelMx, right * dx * 0.01f - up * dy * 0.01f);
    } else if (moveMode == ObjectMoveMode::Z) {
        objectList[pickedObjNum-1]->modelMx = glm::translate(objectList[pickedObjNum-1]->modelMx, forward * dy * 0.01f);
    }
}

/**
 * @brief Init shaders for the window filling quad and the box that is drawn around picked objects.
 */
void CrackVis::initShaders() {
    try {
        shaderQuad = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/quad.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/quad.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    // --------------------------------------------------------------------------------
    //  TODO: Init box shader.
    // --------------------------------------------------------------------------------
    try {
        shaderBox = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/box.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/box.frag")}});
    } catch (const std::runtime_error& e) {
        std::cerr << "Error compiling shader: " << e.what() << std::endl;
    }
    }

/**
 * @brief Init vertex arrays.
 */
void CrackVis::initVAs() {
    //  Create a vertex array for the window filling quad.
    std::vector<float> quadVertices{
        // clang-format off
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
        // clang-format on
    };
    std::vector<uint32_t> quadIndices{
        // clang-format off
        0, 1, 2,
        2, 1, 3
        // clang-format on
    };
    glowl::Mesh::VertexDataList<float> vertexDataQuad{{quadVertices, {8, {{2, GL_FLOAT, GL_FALSE, 0}}}}};
    vaQuad = std::make_unique<glowl::Mesh>(vertexDataQuad, quadIndices);

    // The box is used to indicate the selected object. It is made up of the eight corners of a unit cube that are
    // connected by lines.
    std::vector<float> boxVertices{
        // clang-format off
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        // clang-format on
    };

    std::vector<GLuint> boxEdges{
        // clang-format off
        0, 1,  0, 2,  1, 3,  2, 3,
        0, 4,  1, 5,  2, 6,  3, 7,
        4, 5,  4, 6,  5, 7,  6, 7
        // clang-format on
    };

    // --------------------------------------------------------------------------------
    //  TODO: Create the vertex arrays for box.
    // --------------------------------------------------------------------------------
    glowl::Mesh::VertexDataList<float> vertexDataBox{{boxVertices, {3 * sizeof(float), {{3, GL_FLOAT, GL_FALSE, 0}}}}};
    vaBox = std::make_unique<glowl::Mesh>(vertexDataBox, boxEdges, GL_UNSIGNED_INT, GL_LINES);
}

/**
 * @brief Initialize all frame buffer objects (FBOs).
 */
void CrackVis::initFBOs() {
    if (wWidth <= 0 || wHeight <= 0) {
        return;
    }

    // --------------------------------------------------------------------------------
    //  TODO: Create a frame buffer object (FBO) for multiple render targets.
    //        Use the createFBOTexture method to initialize an empty texture.
    // --------------------------------------------------------------------------------
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    fboTexColor = createFBOTexture(wWidth, wHeight, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR);
    fboTexId = createFBOTexture(wWidth, wHeight, GL_R32I, GL_RED_INTEGER, GL_INT, GL_NEAREST);
    fboTexNormals = createFBOTexture(wWidth, wHeight, GL_RGB32F, GL_RGB, GL_FLOAT, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexColor, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fboTexId, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fboTexNormals, 0);

    fboTexDepth = createFBOTexture(wWidth, wHeight, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fboTexDepth, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // --------------------------------------------------------------------------------
    //  TODO (BONUS TASK): Create a frame buffer object for the view from the spot light.
    // --------------------------------------------------------------------------------
}

/**
 * @brief Delete all framebuffer objects.
 */
void CrackVis::deleteFBOs() {
    // --------------------------------------------------------------------------------
    //  TODO: Clean up all FBOs.
    // --------------------------------------------------------------------------------
    if (glIsFramebuffer(fbo)) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }

    if (glIsFramebuffer(lightFbo)) {
        glDeleteFramebuffers(1, &lightFbo);
        lightFbo = 0;
    }

    if (glIsTexture(fboTexColor)) {
        glDeleteTextures(1, &fboTexColor);
        fboTexColor = 0;
    }
    if (glIsTexture(fboTexId)) {
        glDeleteTextures(1, &fboTexId);
        fboTexId = 0;
    }
    if (glIsTexture(fboTexNormals)) {
        glDeleteTextures(1, &fboTexNormals);
        fboTexNormals = 0;
    }
    if (glIsTexture(fboTexDepth)) {
        glDeleteTextures(1, &fboTexDepth);
        fboTexDepth = 0;
    }

    if (glIsTexture(lightFboTexColor)) {
        glDeleteTextures(1, &lightFboTexColor);
        lightFboTexColor = 0;
    }
    if (glIsTexture(lightFboTexDepth)) {
        glDeleteTextures(1, &lightFboTexDepth);
        lightFboTexDepth = 0;
    }
}

/**
 * @brief Check status of bound framebuffer object (FBO).
 */
void CrackVis::checkFBOStatus() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE: {
            std::cout << "FBO status: complete." << std::endl;
            break;
        }
        case GL_FRAMEBUFFER_UNDEFINED: {
            std::cerr << "FBO status: undefined." << std::endl;
            break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: {
            std::cerr << "FBO status: incomplete attachment." << std::endl;
            break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: {
            std::cerr << "FBO status: no buffers are attached to the FBO." << std::endl;
            break;
        }
        case GL_FRAMEBUFFER_UNSUPPORTED: {
            std::cerr << "FBO status: combination of internal buffer formats is not supported." << std::endl;
            break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: {
            std::cerr << "FBO status: number of samples or the value for ... does not match." << std::endl;
            break;
        }
        default: {
            std::cerr << "FBO status: unknown framebuffer status error." << std::endl;
            break;
        }
    }
}

/**
 * @brief Create a texture for use in the framebuffer object.
 * @param width            Texture width
 * @param height           Texture height
 * @param internalFormat   Internal format of the texture
 * @param format           Format of the data: GL_RGB,...
 * @param type             Data type: GL_UNSIGNED_BYTE, GL_FLOAT,...
 * @param filter           Texture filter: GL_LINEAR or GL_NEAREST

 * @return texture handle
 */
GLuint CrackVis::createFBOTexture(int width, int height, const GLenum internalFormat, const GLenum format,
    const GLenum type, GLint filter) {
    // --------------------------------------------------------------------------------
    //  TODO: Generate an empty 2D texture. Set min/mag filters. Set wrap mode in (s,t).
    // --------------------------------------------------------------------------------
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return textureID;
}

/**
 * @brief Update the matrices.
 */
void CrackVis::updateMatrices() {
    // --------------------------------------------------------------------------------
    //  TODO: Update the projection matrix (projMx).
    // --------------------------------------------------------------------------------
    projMx = glm::perspective(glm::radians(fovY), aspect, zNear,zFar);
    // --------------------------------------------------------------------------------
    //  TODO: Update the light matrices (for bonus task only).
    // --------------------------------------------------------------------------------
}

/**
 * @brief Draw to framebuffer object.
 */
void CrackVis::drawToFBO() {
    if (!glIsFramebuffer(fbo)) {
        return;
    }

    // --------------------------------------------------------------------------------
    //  TODO: Render the scene to the FBO.
    // --------------------------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, drawBuffers);


    glClearTexImage(fboTexColor, 0, GL_RGBA, GL_UNSIGNED_BYTE, &backgroundColor);
    GLint zeroID = 0;
    glClearTexImage(fboTexId, 0, GL_RED_INTEGER, GL_INT, &zeroID);
    GLfloat zeroNormal[] = {0.0f, 0.0f, 0.0f};
    glClearTexImage(fboTexNormals, 0, GL_RGB, GL_FLOAT, zeroNormal);

    glClear(GL_DEPTH_BUFFER_BIT);

    for (const auto& object : objectList) {
        object->draw(projMx, camera->viewMx());
    }

    if (pickedObjNum > 0) {
        shaderBox->use();
        shaderBox->setUniform("scale", 1.2f);
        shaderBox->setUniform("projMx", projMx);
        shaderBox->setUniform("viewMx", camera->viewMx());
        shaderBox->setUniform("modelMx", objectList[pickedObjNum - 1]->modelMx);

        vaBox->draw();
    }

    // 解绑 FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief Draw view of spot light into corresponding FBO.
 */
void CrackVis::drawToLightFBO() {
    if (!glIsFramebuffer(lightFbo)) {
        return;
    }

    // --------------------------------------------------------------------------------
    //  TODO: Render the scene from the view of the light (bonus task only).
    // --------------------------------------------------------------------------------
}

