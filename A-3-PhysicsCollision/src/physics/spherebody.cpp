#include "physics/spherebody.hpp"
#include "math/vector.hpp"
#include "math/matrix.hpp"
#include "scene/sphere.hpp"
#include <iostream>
#include <exception>
#include <algorithm>

namespace _462 {

SphereBody::SphereBody( Sphere* geom )
{
    sphere = geom;
    position = sphere->position;
    radius = sphere->radius;
    orientation = sphere->orientation;
    mass = 0.0;
    velocity = Vector3::Zero;
    angular_velocity = Vector3::Zero;
    force = Vector3::Zero;
    torque = Vector3::Zero;
    type = 100;
}

Vector3 SphereBody::step_position( real_t dt, real_t motion_damping )
{
    this->position = this->position + motion_damping * this->velocity * dt;
    this->sphere->position = this->position;
    return Vector3::Zero; // Return nothing. (I don't understand why they need return value here)
}

Vector3 SphereBody::step_orientation( real_t dt, real_t motion_damping )
{
    // https://stackoverflow.com/a/46924782/11702338
    // Angular velocity was integrate as quaternion rotation in Quaternion(this->angular_velocity, dt)
    this->orientation = motion_damping * this->orientation * Quaternion(this->angular_velocity, dt);
    this->sphere->orientation = this->orientation;
    return Vector3::Zero; // Return nothing. (I don't understand why they need return value here)
}

Vector3 SphereBody::get_force()
{
    return this->force;
}

void SphereBody::apply_force( const Vector3& f, const Vector3& offset )
{
    // TODO apply force/torque to sphere
    this->force += f;
    if (offset != Vector3::Zero)
        this->torque += cross(offset, f);
}

void SphereBody::reset_force()
{
    this->force = Vector3::Zero;
    this->torque = Vector3::Zero;
}

}
