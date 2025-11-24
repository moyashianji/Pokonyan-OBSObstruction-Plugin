#pragma once

#include <btBulletDynamicsCommon.h>
#include <vector>
#include <memory>
#include <QString>

struct Comment3DData;

class PhysicsEngine {
public:
    PhysicsEngine();
    ~PhysicsEngine();

    void Initialize();
    void Update(float deltaTime);
    void AddComment(const QString& text, double amount);

    const std::vector<Comment3DData>& GetComments() const { return m_comments; }
    void Clear();

private:
    void CreateGround();
    void CreateCommentRigidBody(Comment3DData& comment);

    std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
    std::unique_ptr<btCollisionDispatcher> m_dispatcher;
    std::unique_ptr<btBroadphaseInterface> m_broadphase;
    std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
    std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;

    std::vector<Comment3DData> m_comments;
    btRigidBody* m_groundBody;
};
