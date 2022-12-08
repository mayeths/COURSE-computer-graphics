#pragma once

// https://codereview.stackexchange.com/a/247491
// https://codereview.stackexchange.com/a/248060 std::array的OpenGL Buffer

// 只关注一个场景：有且只有一个handler的资源，比如PETSC的Mat，Vec，HPSS的Handler，OpenGL的各类Buffer（经典例子）等。
// 只做一件事：自动Destructor，不拦截其他操作，用get()直接暴露，你可以拿去直接和库交互。所以我们只管Constructor和Destructor
// 也就是说，我依赖一个库，这个库有资源acquire/destroy操作。我只想自动destroy，其他事情不要想太多（比如都想wrap起来），这是过度设计。
// 全小写名字，这么短的单词驼峰太奇怪了

#include <string>

struct guard_t
{
protected:
    bool valid = false;
public:
    void set() { this->valid = true; }
    void unset() { this->valid = false; }
    bool get() { return this->valid; }
    void ensure() const {
        if (!this->valid)
            throw std::runtime_error("Invalid operation on guard_t object");
    }
    void operator()() { this->ensure(); }
};

template<typename DERIVED, typename T>
class raii
{
protected:
    T res = DERIVED::invalid_res;
    bool valid = false;
    void set(T new_res) {
        this->res = new_res;
        this->valid = true;
    }
    void unset() {
        this->res = DERIVED::invalid_res;
        this->valid = false;
    }
public:
    T& get() {
        this->ensure();
        return this->res;
    }
    void ensure() const {
        if (!this->valid)
            throw std::runtime_error("Invalid operation on RAII object");
    }

};


class VAO_raii : public raii<VAO_raii, unsigned int>
{
public:
    static const unsigned int invalid_res = 0;
    int num;
    VAO_raii() {}
    ~VAO_raii() { this->destroy(); }

    void create(int num) {
        unsigned int id;
        glGenVertexArrays(num, &id);
        this->set(id);
        this->num = num;
    }

    void destroy() {
        if (this->valid) {
            glDeleteVertexArrays(this->num, &this->res);
        }
        this->unset();
    }

};

class VBO_raii : public raii<VBO_raii, unsigned int>
{
public:
    static const unsigned int invalid_res = 0;
    int num;
    VBO_raii() {}
    ~VBO_raii() { this->destroy(); }

    void create(int num) {
        unsigned int id;
        glGenBuffers(1, &id);
        this->set(id);
        this->num = num;
    }

    void destroy() {
        if (this->valid) {
            glDeleteBuffers(this->num, &this->res);
        }
        this->unset();
    }

};
