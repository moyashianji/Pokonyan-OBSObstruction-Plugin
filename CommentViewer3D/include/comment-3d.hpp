#pragma once

#include <QString>
#include <btBulletDynamicsCommon.h>
#include <memory>

struct Comment3DData {
    QString text;
    double amount;
    btRigidBody* rigidBody;
    btCollisionShape* collisionShape;
    btDefaultMotionState* motionState;

    // Visual properties
    btVector3 position;
    btQuaternion rotation;
    btVector3 size; // Width, height, depth of the text box

    Comment3DData()
        : amount(0.0)
        , rigidBody(nullptr)
        , collisionShape(nullptr)
        , motionState(nullptr)
        , position(0, 0, 0)
        , rotation(0, 0, 0, 1)
        , size(2.0, 1.0, 0.5)
    {}

    ~Comment3DData() {
        if (rigidBody && rigidBody->getMotionState()) {
            delete rigidBody->getMotionState();
        }
        delete rigidBody;
        delete collisionShape;
    }
};

class Comment3DRenderer {
public:
    static void RenderComment(const Comment3DData& comment);
    static void RenderText3D(const QString& text, float x, float y, float z);

private:
    static void DrawCube(float width, float height, float depth);
};
