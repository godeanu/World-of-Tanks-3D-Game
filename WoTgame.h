#pragma once

#include "components/simple_scene.h"
#include "lab_m1/WoTgame/lab_camera.h"
#include <unordered_map>


namespace m1
{
    class WoTgame : public gfxc::SimpleScene
    {
    public:
        struct ViewportArea
        {
            ViewportArea() : x(0), y(0), width(1), height(1) {}
            ViewportArea(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {}
            int x;
            int y;
            int width;
            int height;
        };

        WoTgame();
        ~WoTgame();

        void Init() override;


    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;
 

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;
        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color = glm::vec3(1), float damageLevel = 0, glm::mat4 proj = glm::mat4(1), glm::mat4 viewMatrix = glm::mat4(1), Texture2D *texture1=NULL);
        void RenderScene(implemented::Camera* cameraTo, glm::mat4 viewMatrix, glm::mat4 proj);
    protected:
        glm::mat4 modelMatrix;
        float translateX, translateY, translateZ;
        float scaleX, scaleY, scaleZ;
        float angularStepOX, angularStepOY, angularStepOZ;
        GLenum polygonMode;
        ViewportArea miniViewportArea;

    protected:
        implemented::Camera* camera;
        //second minimap camera
        implemented::Camera* minimapCamera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;


        std::unordered_map<std::string, Texture2D*> mapTextures;
        GLfloat fov;
        bool projection;
        GLfloat right, left, bottom, top;
        GLfloat zNear = 0.1f, zFar = 50.f;
        GLboolean mixTextures;
    };
}  

