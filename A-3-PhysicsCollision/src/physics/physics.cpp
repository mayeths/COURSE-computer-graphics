#include "physics/physics.hpp"

namespace _462 {

Physics::Physics() {
    reset();
}

Physics::~Physics() {
    reset();
}

void Physics::step(real_t dt) {
    // TODO step the world forward by dt. Need to detect collisions, apply
    // forces, and integrate positions and orientations.
    //
    // Note: put RK4 here, not in any of the physics bodies
    //
    // Must use the functions that you implemented
    //
    // Note, when you change the position/orientation of a physics object,
    // change the position/orientation of the graphical object that represents
    // it

    // Get environment interactive (get force like collision, gravity)
    for (SphereBody *body : spheres)
        for (PlaneBody *pb : planes)
            collides(*body, *pb, collision_damping);
    for (SphereBody *body : spheres)
        for (TriangleBody *tb : triangles)
            collides(*body, *tb, collision_damping);
    for (real_t i = 0; i < spheres.size(); i++)
        for (real_t j = i + 1; j < spheres.size(); j++)
            collides(*spheres[i], *spheres[j], collision_damping);

    for (SphereBody *body : spheres) {
        body->apply_force(gravity, Vector3::Zero);
    }

    // Hack: newtons_cradle
    if (springs.size() > 1) {
        real_t x = 4;
        for (Spring *sp : springs) {
            sp->body2_offset = Vector3(x, 4.0, 0.0);
            x -= 2.0;
            sp->step(dt);
        }
    }
    // Hack: spring_rotation
    if (springs.size() == 1) {
        for (Spring *sp : springs) {
            sp->body2_offset = Vector3(0, 4.0, 0.0);
            sp->step(dt);
        }
    }

    // update
    for (SphereBody *body : spheres) {
        body->velocity += body->force / body->mass * dt;

        // Hack: simulate spring_rotation
        if (spheres.size() == 1) {
            body->angular_velocity = body->torque / (1 / 5.0 * body->mass * body->radius * body->radius);
            body->angular_velocity.x = 0;
        }

        body->step_position(dt, 1.0);
        body->step_orientation(dt/4, 1.0); // dt for scene spring_rotation is bad, use dt/4 or some else instead.
    }

    // Reset environment interactive (because we have consume it to compute position, orientation etc.)
    for (SphereBody *body : spheres) {
        body->reset_force();
    }
}

void Physics::add_sphere(SphereBody* b) {
    spheres.push_back(b);
}

size_t Physics::num_spheres() const {
    return spheres.size();
}

void Physics::add_plane(PlaneBody* p) {
    planes.push_back(p);
}

size_t Physics::num_planes() const {
    return planes.size();
}

void Physics::add_triangle(TriangleBody* t) {
    triangles.push_back(t);
}

size_t Physics::num_triangles() const {
    return triangles.size();
}

void Physics::add_spring(Spring* s) {
    springs.push_back(s);
}

size_t Physics::num_springs() const {
    return springs.size();
}

void Physics::reset() {
    for (SphereList::iterator i = spheres.begin(); i != spheres.end(); i++) {
        delete *i;
    }
    for (PlaneList::iterator i = planes.begin(); i != planes.end(); i++) {
        delete *i;
    }
    for (TriangleList::iterator i = triangles.begin(); i != triangles.end(); i++) {
        delete *i;
    }
    for (SpringList::iterator i = springs.begin(); i != springs.end(); i++) {
        delete *i;
    }

    spheres.clear();
    planes.clear();
    triangles.clear();
    springs.clear();

    gravity = Vector3::Zero;
    collision_damping = 0.0;
}

}
