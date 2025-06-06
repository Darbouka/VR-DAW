#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>

namespace VR_DAW {

class VRPhysics {
public:
    VRPhysics();
    ~VRPhysics();

    // Initialisierung und Shutdown
    bool initialize();
    void shutdown();
    void update(float deltaTime);

    // Physik-Status
    bool isInitialized() const;
    bool isSimulating() const;
    std::string getStatus() const;

    // Physik-Konfiguration
    void setGravity(const glm::vec3& gravity);
    void setTimeScale(float scale);
    void setMaxSteps(int steps);
    void setFixedTimeStep(float timeStep);
    void setSolverIterations(int iterations);
    void setSolverVelocityIterations(int iterations);

    // Rigidbody-Management
    struct Rigidbody {
        std::string name;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 velocity;
        glm::vec3 angularVelocity;
        float mass;
        float drag;
        float angularDrag;
        bool useGravity;
        bool isKinematic;
        bool isTrigger;
    };

    int createRigidbody(const std::string& name, const Rigidbody& rigidbody);
    void destroyRigidbody(int rigidbodyId);
    void updateRigidbody(int rigidbodyId, const Rigidbody& rigidbody);
    Rigidbody getRigidbody(int rigidbodyId) const;

    // Collider-Management
    struct Collider {
        std::string name;
        std::string type;
        glm::vec3 center;
        glm::vec3 size;
        float radius;
        float height;
        bool isTrigger;
    };

    int createCollider(int rigidbodyId, const Collider& collider);
    void destroyCollider(int colliderId);
    void updateCollider(int colliderId, const Collider& collider);
    Collider getCollider(int colliderId) const;

    // Constraint-Management
    struct Constraint {
        std::string name;
        std::string type;
        int rigidbodyA;
        int rigidbodyB;
        glm::vec3 anchorA;
        glm::vec3 anchorB;
        glm::vec3 axisA;
        glm::vec3 axisB;
        float minLimit;
        float maxLimit;
        float spring;
        float damper;
    };

    int createConstraint(const std::string& name, const Constraint& constraint);
    void destroyConstraint(int constraintId);
    void updateConstraint(int constraintId, const Constraint& constraint);
    Constraint getConstraint(int constraintId) const;

    // Kräfte und Impulse
    void addForce(int rigidbodyId, const glm::vec3& force, bool relative = false);
    void addTorque(int rigidbodyId, const glm::vec3& torque, bool relative = false);
    void addImpulse(int rigidbodyId, const glm::vec3& impulse, bool relative = false);
    void addAngularImpulse(int rigidbodyId, const glm::vec3& impulse, bool relative = false);

    // Kollisionserkennung
    struct RaycastHit {
        int rigidbodyId;
        int colliderId;
        glm::vec3 point;
        glm::vec3 normal;
        float distance;
    };

    bool raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit& hit);
    bool sphereCast(const glm::vec3& origin, float radius, const glm::vec3& direction, float maxDistance, RaycastHit& hit);
    bool boxCast(const glm::vec3& center, const glm::vec3& size, const glm::quat& rotation, const glm::vec3& direction, float maxDistance, RaycastHit& hit);

    // Kollisions-Callbacks
    struct Collision {
        int rigidbodyA;
        int rigidbodyB;
        int colliderA;
        int colliderB;
        glm::vec3 point;
        glm::vec3 normal;
        float impulse;
    };

    using CollisionCallback = std::function<void(const Collision&)>;
    void registerCollisionCallback(CollisionCallback callback);
    void unregisterCollisionCallback();

    // Debug
    void enableDebugMode(bool enable);
    void showDebugInfo();
    void renderDebugShapes();

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    bool initialized;
    bool simulating;
    bool debugEnabled;

    // Physik-Parameter
    glm::vec3 gravity;
    float timeScale;
    int maxSteps;
    float fixedTimeStep;
    int solverIterations;
    int solverVelocityIterations;

    // Physik-Objekte
    std::map<int, Rigidbody> rigidbodies;
    std::map<int, Collider> colliders;
    std::map<int, Constraint> constraints;

    // Callbacks
    CollisionCallback collisionCallback;

    void initializePhysics();
    void shutdownPhysics();
    void updatePhysics(float deltaTime);
    void processCollisions();
    void renderDebugInfo();
};

} // namespace VR_DAW 