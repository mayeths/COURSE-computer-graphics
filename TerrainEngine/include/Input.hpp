#pragma once
// https://stackoverflow.com/a/66922475
// unused

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <map>

enum InputEvent { PRESS = 1, RELEASE = 2, REPEAT = 4, NONE = 8 };

static std::map<int, InputEvent> event_map = {{GLFW_PRESS, PRESS}, {GLFW_RELEASE, RELEASE}};

class Input {
   public:
    Input() {}
    Input(GLFWwindow *w) : window(w) {}
    void set_window(GLFWwindow *w) { window = w; };

    void add_key_input(int key, std::function<void(int, float)> f, int events) {
        // if (key == -1) {std::cout << "undefinet key : " << key << '\n'; return;}
        keys[key] = KEY(f, event_map[events]);
    }

    void process(float delta) {
        for (auto &[key, info] : keys) {
            int e = glfwGetKey(window, key);
            // std::cout << key << " " << e << " " << info.action << " " << info.event << " ";

            if (e == GLFW_RELEASE && info.action == NONE)
                info.action = NONE;
            else if (e == GLFW_RELEASE && (info.action == PRESS || info.action == REPEAT))
                info.action = RELEASE;
            else if (e == GLFW_PRESS && (info.action == PRESS || info.action == REPEAT))
                info.action = REPEAT;
            else if (e == GLFW_PRESS && (info.action == NONE || info.action == RELEASE))
                info.action = PRESS;

            // std::cout << info.action << "\n";
            if (info.event & info.action) {
                info.callback(key, delta);
            }
        }
    }

   private:
    struct KEY {
        KEY() : action(RELEASE) {}
        KEY(std::function<void(int, float)> f, InputEvent e) : callback(f), action(RELEASE), event(e) {}

        std::function<void(int, float)> callback;
        InputEvent action;
        InputEvent event;
    };

    GLFWwindow *window;
    std::map<int, KEY> keys;
};
