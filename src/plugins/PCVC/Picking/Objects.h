#pragma once

#include <memory>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glowl/glowl.h>

namespace OGL4Core2::Plugins::PCVC::CrackVis {
    class CrackVis;

    class Object {
    public:
        Object(CrackVis& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex);
        virtual ~Object() = default;

        inline int getId() const {
            return id;
        }

        void draw(const glm::mat4& projMx, const glm::mat4& viewMx);

        virtual void reloadShaders() = 0;

        glm::mat4 modelMx;

    protected:
        CrackVis& basePlugin;
        int id;
        std::unique_ptr<glowl::GLSLProgram> shaderProgram;
        std::unique_ptr<glowl::Mesh> va;
        std::shared_ptr<glowl::Texture2D> tex;
    };

    class Base : public Object {
    public:
        Base(CrackVis& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex);

        void reloadShaders() override;

    private:
        void initShaders();
    };

    class Cube : public Object {
    public:
        Cube(CrackVis& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex);

        void reloadShaders() override;

    private:
        void initShaders();
    };

    class Sphere : public Object {
    public:
        Sphere(CrackVis& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex);

        void reloadShaders() override;

    private:
        void initShaders();
    };

    class Torus : public Object {
    public:
        Torus(CrackVis& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex);

        void reloadShaders() override;

    private:
        void initShaders();
    };
} // namespace OGL4Core2::Plugins::PCVC::CrackVis
