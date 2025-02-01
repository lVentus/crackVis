#include "Objects.h"

#include <cmath>
#include <iostream>
#include <utility>
#include <vector>

#include "CrackVis.h"

using namespace OGL4Core2::Plugins::PCVC::CrackVis;

static const double pi = std::acos(-1.0);

Object::Object(CrackVis& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex)
    : modelMx(glm::mat4(1.0f)),
      basePlugin(basePlugin),
      id(id),
      shaderProgram(nullptr),
      va(nullptr),
      tex(std::move(tex)) {}

/**
 * The object is rendered.
 * @param projMx   projection matrix
 * @param viewMx   view matrix
 */
void Object::draw(const glm::mat4& projMx, const glm::mat4& viewMx) {
    if (shaderProgram == nullptr || va == nullptr) {
        return;
    }

    // --------------------------------------------------------------------------------
    //  TODO Implement object rendering!
    // --------------------------------------------------------------------------------
    shaderProgram->use();
    shaderProgram->setUniform("projMx", projMx);
    shaderProgram->setUniform("viewMx", viewMx);
    shaderProgram->setUniform("modelMx", modelMx);
    shaderProgram->setUniform("pickId", id);

    if (tex) {
        glActiveTexture(GL_TEXTURE0);
        tex->bindTexture();
        shaderProgram->setUniform("tex", 0);
    }

    va->draw();
}

Base::Base(CrackVis& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex)
    : Object(basePlugin, id, std::move(tex)) {
    initShaders();

    const std::vector<float> baseVertices{
        // clang-format off
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        // clang-format on
    };

    const std::vector<float> baseNormals{
        // clang-format off
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        // clang-format on
    };

    const std::vector<float> baseTexCoords{
        // clang-format off
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        // clang-format on
    };

    const std::vector<GLuint> baseIndices{0, 1, 2, 3};

    glowl::Mesh::VertexDataList<float> vertexDataBase{
        {baseVertices, {12, {{3, GL_FLOAT, GL_FALSE, 0}}}},
        {baseNormals, {12, {{3, GL_FLOAT, GL_FALSE, 0}}}},
        {baseTexCoords, {8, {{2, GL_FLOAT, GL_FALSE, 0}}}},
    };
    va = std::make_unique<glowl::Mesh>(vertexDataBase, baseIndices, GL_UNSIGNED_INT, GL_TRIANGLE_STRIP);
}

/**
 * This function is caled from the plugin after pressing 'R' to reload/recompile shaders.
 */
void Base::reloadShaders() {
    initShaders();
}

/**
 * Creates the shader program for this object.
 */
void Base::initShaders() {
    try {
        shaderProgram = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, basePlugin.getStringResource("shaders/base.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, basePlugin.getStringResource("shaders/base.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }
}

/**
 * Constructs a new cube instance.
 * @param basePlugin A reference to your CrackVis plugin
 * @param id         Unique ID to use for the new cube instance
 * @param tex        Texture for the new cube instance
 */
Cube::Cube(CrackVis& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex)
    : Object(basePlugin, id, std::move(tex)) {
    initShaders();

    // --------------------------------------------------------------------------------
    //  TODO: Init cube vertex data!
    // --------------------------------------------------------------------------------
    // Cube geometry is constructed in geometry shader, therefore only a single point vertex is specified here.
    const std::vector<float> cubeVertices{0.0f, 0.0f, 0.0f};
    const std::vector<GLuint> cubeIndices{0};

    glowl::Mesh::VertexDataList<float> vertexDataCube{{cubeVertices, {3, {{3, GL_FLOAT, GL_FALSE, 0}}}}};
    va = std::make_unique<glowl::Mesh>(vertexDataCube, cubeIndices, GL_UNSIGNED_INT, GL_POINTS);
}

void Cube::reloadShaders() {
    initShaders();
}

void Cube::initShaders() {
    // --------------------------------------------------------------------------------
    //  TODO: Init cube shader program!
    // --------------------------------------------------------------------------------
    try {
        shaderProgram = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, basePlugin.getStringResource("shaders/cube.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, basePlugin.getStringResource("shaders/cube.frag")},
            {glowl::GLSLProgram::ShaderType::Geometry, basePlugin.getStringResource("shaders/cube.geom")}});
    } catch (const std::runtime_error& e) {
        std::cerr << "Error compiling shader: " << e.what() << std::endl;
    }
}

/**
 * Constructs a new sphere instance.
 * @param basePlugin A reference to your CrackVis plugin
 * @param id         Unique ID to use for the new sphere instance
 * @param tex        Texture for the new sphere instance
 */
Sphere::Sphere(CrackVis& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex)
    : Object(basePlugin, id, std::move(tex)) {
    initShaders();

    // --------------------------------------------------------------------------------
    //  TODO: Init sphere vertex data.
    // --------------------------------------------------------------------------------
    const std::size_t resTheta = 128;
    const std::size_t resPhi = 64;
    const double radiusSphere = 0.5;

    std::vector<float> vertices;
    std::vector<float> texCoords;
    std::vector<int> indices;

    for (int i = 0; i <= resTheta; ++i) {
        float theta = i * glm::pi<float>() / resTheta;
        for (int j = 0; j <= resPhi; ++j) {
            float phi = j * 2 * glm::pi<float>() / resPhi;

            float x = radiusSphere * sin(theta) * cos(phi);
            float y = radiusSphere * cos(theta);
            float z = radiusSphere * sin(theta) * sin(phi);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            texCoords.push_back(static_cast<float>(j) / resPhi);
            texCoords.push_back(static_cast<float>(i) / resTheta);
        }
    }

    for (int i = 0; i < resTheta; ++i) {
        for (int j = 0; j < resPhi; ++j) {
            int first = (i * (resPhi + 1)) + j;
            int second = first + resPhi + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    glowl::Mesh::VertexDataList<float> vertexData{
        {vertices, {3 * sizeof(float), {{3, GL_FLOAT, GL_FALSE, 0}}}},
        {texCoords, {2 * sizeof(float), {{2, GL_FLOAT, GL_FALSE, 0}}}}
    };
    va = std::make_unique<glowl::Mesh>(vertexData, indices);
}

void Sphere::reloadShaders() {
    initShaders();
}

void Sphere::initShaders() {
    // --------------------------------------------------------------------------------
    //  TODO: Init sphere shader program!
    // --------------------------------------------------------------------------------
   try {
        shaderProgram = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, basePlugin.getStringResource("shaders/sphere.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, basePlugin.getStringResource("shaders/sphere.frag")}});
   } catch (const std::runtime_error& e) {
        std::cerr << "Error compiling shader: " << e.what() << std::endl;
   }
}

/**
 * Constructs a new torus instance.
 * @param basePlugin A reference to your CrackVis plugin
 * @param id         Unique ID to use for the new torus instance
 * @param tex        Texture for the new torus instance
 */
Torus::Torus(CrackVis& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex)
    : Object(basePlugin, id, std::move(tex)) {
    initShaders();

    // --------------------------------------------------------------------------------
    //  TODO: Init torus vertex data.
    // --------------------------------------------------------------------------------
    const int resT = 64; // Toroidal
    const int resP = 64; // Poloidal
    const float radiusOut = 0.34f;
    const float radiusIn = 0.16f;

   std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<GLuint> indices;

    for (int i = 0; i <= resT; ++i) {
        float theta = i * 2 * glm::pi<float>() / resT;
        for (int j = 0; j <= resP; ++j) {
            float phi = j * 2 * glm::pi<float>() / resP;

            float x = (radiusOut + radiusIn * cos(phi)) * cos(theta);
            float y = (radiusOut + radiusIn * cos(phi)) * sin(theta);
            float z = radiusIn * sin(phi);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            normals.push_back(cos(phi) * cos(theta));
            normals.push_back(cos(phi) * sin(theta));
            normals.push_back(sin(phi));

            texCoords.push_back(static_cast<float>(i) / resT);
            texCoords.push_back(static_cast<float>(j) / resP);
        }
    }

    for (int i = 0; i < resT; ++i) {
        for (int j = 0; j < resP; ++j) {
            int first = (i * (resP + 1)) + j;
            int second = first + resP + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    glowl::Mesh::VertexDataList<float> vertexData{
        {vertices, {3 * sizeof(float), {{3, GL_FLOAT, GL_FALSE, 0}}}},
        {texCoords, {2 * sizeof(float), {{2, GL_FLOAT, GL_FALSE, 0}}}},
        {normals, {3 * sizeof(float), {{3, GL_FLOAT, GL_FALSE, 0}}}}
    };
    va = std::make_unique<glowl::Mesh>(vertexData, indices);
}

void Torus::reloadShaders() {
    initShaders();
}

void Torus::initShaders() {
    // --------------------------------------------------------------------------------
    //  TODO: Init torus shader program!
    // --------------------------------------------------------------------------------
    /*try {
        shaderProgram = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, basePlugin.getStringResource("shaders/torus.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, basePlugin.getStringResource("shaders/torus.frag")}});
    } catch (const std::runtime_error& e) {
        std::cerr << "Error compiling shader: " << e.what() << std::endl;
    }*/
}
