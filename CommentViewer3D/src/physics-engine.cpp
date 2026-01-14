#include "physics-engine.hpp"
#include "comment-3d.hpp"
#include <random>
#include <ctime>

PhysicsEngine::PhysicsEngine()
    : m_groundBody(nullptr)
{
}

PhysicsEngine::~PhysicsEngine() {
    Clear();

    if (m_groundBody) {
        m_dynamicsWorld->removeRigidBody(m_groundBody);
        delete m_groundBody->getMotionState();
        delete m_groundBody->getCollisionShape();
        delete m_groundBody;
    }
}

void PhysicsEngine::Initialize() {
    // Bullet Physics setup
    m_collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
    m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfig.get());
    m_broadphase = std::make_unique<btDbvtBroadphase>();
    m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();

    m_dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
        m_dispatcher.get(),
        m_broadphase.get(),
        m_solver.get(),
        m_collisionConfig.get()
    );

    // Set gravity
    m_dynamicsWorld->setGravity(btVector3(0, -15.0, 0));

    // Create ground
    CreateGround();
}

void PhysicsEngine::CreateGround() {
    btCollisionShape* groundShape = new btBoxShape(btVector3(50, 0.5, 50));
    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(0, -0.5, 0));

    btScalar mass = 0.0; // Static object
    btVector3 localInertia(0, 0, 0);

    btDefaultMotionState* motionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, groundShape, localInertia);
    rbInfo.m_restitution = 0.3f; // Some bounce
    rbInfo.m_friction = 0.8f;

    m_groundBody = new btRigidBody(rbInfo);
    m_dynamicsWorld->addRigidBody(m_groundBody);
}

void PhysicsEngine::Update(float deltaTime) {
    if (m_dynamicsWorld) {
        m_dynamicsWorld->stepSimulation(deltaTime, 10);

        // Update comment positions from physics
        for (auto& comment : m_comments) {
            if (comment.rigidBody) {
                btTransform trans;
                comment.rigidBody->getMotionState()->getWorldTransform(trans);
                comment.position = trans.getOrigin();
                comment.rotation = trans.getRotation();
            }
        }
    }
}

void PhysicsEngine::AddComment(const QString& text, double amount) {
    Comment3DData comment;
    comment.text = text;
    comment.amount = amount;

    // Calculate size based on text length and amount
    float textLength = text.length() * 0.15f;
    comment.size = btVector3(
        std::max(2.0f, textLength),  // Width
        1.0f,                         // Height
        0.5f                          // Depth
    );

    CreateCommentRigidBody(comment);
    m_comments.push_back(std::move(comment));
}

void PhysicsEngine::CreateCommentRigidBody(Comment3DData& comment) {
    // Random position in upper sky
    static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_real_distribution<float> xDist(-20.0f, 20.0f);
    std::uniform_real_distribution<float> yDist(30.0f, 60.0f);
    std::uniform_real_distribution<float> zDist(-20.0f, 20.0f);
    std::uniform_real_distribution<float> rotDist(0.0f, 360.0f);

    float x = xDist(rng);
    float y = yDist(rng);
    float z = zDist(rng);

    // Create collision shape (box)
    btCollisionShape* shape = new btBoxShape(btVector3(
        comment.size.x() / 2.0f,
        comment.size.y() / 2.0f,
        comment.size.z() / 2.0f
    ));
    comment.collisionShape = shape;

    // Initial transform with random rotation
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(x, y, z));

    btQuaternion rotation;
    rotation.setEulerZYX(
        rotDist(rng) * 3.14159f / 180.0f,
        rotDist(rng) * 3.14159f / 180.0f,
        rotDist(rng) * 3.14159f / 180.0f
    );
    startTransform.setRotation(rotation);

    // Calculate mass based on size
    btScalar mass = comment.size.x() * comment.size.y() * comment.size.z() * 0.5f;

    btVector3 localInertia(0, 0, 0);
    shape->calculateLocalInertia(mass, localInertia);

    comment.motionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(
        mass,
        comment.motionState,
        shape,
        localInertia
    );

    rbInfo.m_restitution = 0.4f; // Bounce
    rbInfo.m_friction = 0.7f;
    rbInfo.m_linearDamping = 0.1f;
    rbInfo.m_angularDamping = 0.3f;

    comment.rigidBody = new btRigidBody(rbInfo);
    m_dynamicsWorld->addRigidBody(comment.rigidBody);

    // Store initial position
    comment.position = btVector3(x, y, z);
    comment.rotation = rotation;
}

void PhysicsEngine::Clear() {
    for (auto& comment : m_comments) {
        if (comment.rigidBody) {
            m_dynamicsWorld->removeRigidBody(comment.rigidBody);
        }
    }
    m_comments.clear();
}
