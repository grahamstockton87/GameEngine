#pragma once

#include <memory>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "Model.h"
#include "Camera.h"

class Enemy {
public:
    Enemy(std::unique_ptr<Model> modelPtr, glm::vec3 startPos, float startHealth = 100.0f)
        : model(std::move(modelPtr)), position(startPos), health(startHealth), isHit(false), isDead(false)
    {
        if (model) {
            model->translate(position.x, position.y, position.z);
        }
    }

    void Update(float deltaTime, Camera& camera, float moveSpeed = 1.0f) {
        if (isDead || !model || !model->IsValid) return;

        glm::vec3 target = camera.getCameraPostion();
        target.y -= camera.radiusY; // Optional grounding logic

        glm::vec3 direction = target - position;
        float distance = glm::length(direction);

        if (distance > 0.1f) {
            direction = glm::normalize(direction);
            position += direction * moveSpeed * deltaTime;
        }

        // Update model transform
        model->ResetModel(); // Important: Reset before applying new transforms
        model->translate(position.x, position.y, position.z);

        glm::vec3 lookDir = glm::normalize(camera.getCameraPostion() - position);
        float angleY = atan2(lookDir.x, lookDir.z); // face the player
        model->rotate(angleY * (180.0f / 3.14159265f), 0.0f, 1.0f, 0.0f);

        model->scale(0.1f, 0.1f, 0.1f);
    }

    void Render(GLuint uniformModel, Material& material, GLuint uniformSpecularIntensity, GLuint uniformShininess) {
        if (isDead || !model || !model->IsValid) return;

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model->model));
        material.UseMaterial(uniformSpecularIntensity, uniformShininess);
        model->RenderModel(uniformModel);
    }

    void TakeDamage(float damageAmount) {
        if (isDead) return;

        isHit = true;
        health -= damageAmount;

        if (health <= 0.0f) {
            isDead = true;
            if (model) model->IsValid = false;
        }
    }

    bool IsAlive() const { return !isDead; }
    bool WasHit() const { return isHit; }
    glm::vec3 GetPosition() const { return position; }
    BoundingBox GetBox() const { return model ? model->GetBox() : BoundingBox(); }

private:
    std::unique_ptr<Model> model;
    glm::vec3 position;
    float health;
    bool isHit;
    bool isDead;
};
