#version 330 core
/* build_house https://learnopengl-cn.readthedocs.io/zh/latest/04%20Advanced%20OpenGL/09%20Geometry%20Shader/ */
layout(points) in;
layout(points, max_vertices = 10) out;

uniform float deltaUpdateTime;
uniform float now;
uniform float MAX_SIZE;
uniform float MIN_SIZE;
uniform float MAX_LAUNCH;
uniform float MIN_LAUNCH;
uniform float LOWEST_ALIVE_Y;
uniform sampler1D randomTextureID;

// Inputs in the geometry shader must be arrays
in float type_0[];
in vec3 position_0[];
in vec3 velocity_0[];
in float age_0[];
in float size_0[];

out float type_1;
out vec3 position_1;
out vec3 velocity_1;
out float age_1;
out float size_1;

#define PARTICLE_TYPE_BASE    0.0f
#define PARTICLE_TYPE_DERIVED 1.0f

vec3 randvec3(float TexCoord);

vec3 randvec3(float TexCoord) {
    vec3 ret = texture(randomTextureID, TexCoord).xyz;
    return ret;
}

void emitBaseSnow(float age) {
    type_1     = PARTICLE_TYPE_BASE;
    position_1 = position_0[0];
    velocity_1 = velocity_0[0];
    age_1      = age;
    size_1     = 0;
    EmitVertex();
    EndPrimitive();
}

void emitDerivedSnow() {
    type_1     = PARTICLE_TYPE_DERIVED;
    position_1 = position_0[0];
    vec3 direction = randvec3(age_0[0] + now) - vec3(0.5, 0.5, 0.5);
    direction.y = min(direction.y, -0.5f);
    velocity_1 = normalize(direction) * 50;
    age_1      = age_0[0];
    size_1     = (MAX_SIZE - MIN_SIZE) * randvec3(age_0[0] + now).x + MIN_SIZE;
    EmitVertex();
    EndPrimitive();
}

void emitRenewDerivedSnow(float age) {
    vec3 delta_pos = velocity_0[0] * deltaUpdateTime;
    vec3 rand = randvec3(age_0[0] + now);
    if (rand.y > 0) rand.y = -rand.y;
    vec3 delta_vel = deltaUpdateTime * rand;
    type_1         = PARTICLE_TYPE_DERIVED;
    position_1     = position_0[0] + delta_pos;
    velocity_1     = velocity_0[0] + delta_vel;
    age_1          = age;
    size_1         = size_0[0];
    EmitVertex();
    EndPrimitive();
}

void main() {
    float age = age_0[0] - deltaUpdateTime * 1000.0f;
    if (type_0[0] == PARTICLE_TYPE_BASE) {
        if (age <= 0) {
            emitDerivedSnow();
            age = (MAX_LAUNCH - MIN_LAUNCH) * randvec3(age_0[0]).y + MIN_LAUNCH;
        }
        emitBaseSnow(age);
    } else {
        if (position_0[0].y > LOWEST_ALIVE_Y) {
            emitRenewDerivedSnow(age);
        }
    }
}
