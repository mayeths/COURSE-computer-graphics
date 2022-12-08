#pragma once

#include <stdint.h>
#include <atomic>

template<typename T>
class UUID {
    std::atomic<T> counter = T{0};
public:
    void reset(T seed = T{0}) {
        this->counter.store(seed);
    }

    T next() {
        return this->counter++;
    }
};

