#version 330 core
layout (location = 0) in float init_type;
layout (location = 1) in vec3 init_position;
layout (location = 2) in vec3 init_velocity;
layout (location = 3) in float init_age;
layout (location = 4) in float init_size;

out float type_0;
out vec3 position_0;
out vec3 velocity_0;
out float age_0;
out float size_0;

void main() {
    type_0 = init_type;
    position_0 = init_position;
    velocity_0 = init_velocity;
    age_0 = init_age;
    size_0 = init_size;
}
