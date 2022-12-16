#include "math/math.hpp"
#include "math/vector.hpp"
#include "math/quaternion.hpp"
#include "math/matrix.hpp"
#include "physics/spring.hpp"
#include "physics/body.hpp"
#include "physics/spherebody.hpp"
#include <iostream>

namespace _462 {

Spring::Spring() {
    body1_offset = Vector3::Zero;
    body2_offset = Vector3::Zero;
    damping = 0.0;
}

void Spring::step(real_t dt) {
    // TODO apply forces to attached bodies

    Vector3 gravity(0.0f, -9.8f, 0.0f);

    Vector3 p = body1->position;
    if (body1_offset.y == 0.0) {
        //newton_gradle
        Vector3 F = -constant * normalize(p - body2_offset) * (distance(p, body2_offset) - equilibrium);
        body1->apply_force(F, Vector3::Zero);
    } else {
        //spring_rotation
        Vector3 F = -constant * normalize(p + body1_offset - body2_offset) * (distance(p + body1_offset, body2_offset) - equilibrium);
        body1->apply_force(-body1->velocity, body1_offset);
        body1->apply_force(F+body1->velocity, Vector3::Zero);
    }
}

}


