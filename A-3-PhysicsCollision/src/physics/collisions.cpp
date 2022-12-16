#include "physics/collisions.hpp"

namespace _462 {

bool collides(SphereBody& body1, SphereBody& body2, real_t collision_damping)
{
    // TODO detect collision. If there is one, update velocity
    Vector3 p1 = body1.position, p2 = body2.position;
    real_t dist = distance(p1, p2);
    if (body1.id == body2.id)
        return false;

    // Distance test
    if (dist >= body1.radius + body2.radius)
        return false;

    // Collided, update with Conservation of Momentum (m1 * v1_old + m2 * v2_old = m1 * v1_new + m2 * v2_new)
    real_t m1 = body1.mass;
    real_t m2 = body2.mass;
    Vector3 direction = (p1 - p2) / dist;
    Vector3 v1_old = body1.velocity;
    Vector3 v2_old = body2.velocity;
    Vector3 v2_new = v2_old + (2 * (m1 / (m1 + m2)) * dot(v1_old - v2_old, direction) * direction);
    Vector3 v1_new = (m1 * v1_old + m2 * v2_old - m2 * v2_new) / m1;
    // Energy loss is simulated by velocity damping
    // But damping between sphere is not mentioned in project requirements
    // So the following lines get collision_damping == 0
    body1.velocity = (1 - collision_damping) * v1_new;
    body2.velocity = (1 - collision_damping) * v2_new;
    return true;
}

bool collides(SphereBody& body1, TriangleBody& body2, real_t collision_damping)
{
    // TODO detect collision. If there is one, update velocity
    Vector3 p = body1.position;
    Vector3 p1 = body2.vertices[0];
    Vector3 p2 = body2.vertices[1];
    Vector3 p3 = body2.vertices[2];
    Vector3 normal = normalize(cross(p2 - p1, p3 - p1));

    // Distance test
    real_t d = dot(p - p1, normal);
    if (abs(d) > body1.radius)
        return false;

    // Inside test
    Vector3 w = cross(p1 - p, p2 - p);
    Vector3 u = cross(p2 - p, p3 - p);
    Vector3 v = cross(p3 - p, p1 - p);
    if (dot(w, u) < 0 || dot(w, v) < 0 || dot(u, v) < 0)
        return false;

    // Collided, update with Reflection Law
    body1.velocity = (body1.velocity - 2 * dot(body1.velocity, normal) * normal);
    // Energy loss is simulated by velocity damping
    // And only on normal direction to support rolling on the plane
    // Use "body1.velocity = (1 - collision_damping) * body1.velocity" directly if you want to simulate friction too
    Vector3 velocity_loss_on_normal = collision_damping * normal * dot(body1.velocity, normal);
    body1.velocity -= velocity_loss_on_normal;
    return true;
}

bool collides(SphereBody& body1, PlaneBody& body2, real_t collision_damping)
{
    // TODO detect collision. If there is one, update velocity
    Vector3 p1 = body1.position;
    Vector3 p2 = body2.position;
    Vector3 normal = body2.normal;

    // Distance test
    real_t d = dot(p1 - p2, normal);
    if (abs(d) > body1.radius)
        return false;

    // Infinity plane, skip inside test

    // Collided, update with Reflection Law
    body1.velocity = (body1.velocity - 2.0*dot(body1.velocity, body2.normal) * body2.normal);
    // Energy loss is simulated by velocity damping
    // And only on normal direction to support rolling on the plane
    // Use "body1.velocity = (1 - collision_damping) * body1.velocity" directly if you want to simulate friction too
    Vector3 velocity_loss_on_normal = collision_damping * normal * dot(body1.velocity, normal);
    body1.velocity -= velocity_loss_on_normal;
    return true;
}

}
