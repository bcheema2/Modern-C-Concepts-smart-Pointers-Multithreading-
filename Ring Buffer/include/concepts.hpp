//
// Created by Bibenpreet Cheema on 9/16/26.
//

#pragma once

#include <concepts>
#include <mutex>

namespace RingBuffer {
    template <typename T>
    concept Bufferable =
        std::movable<T> &&
        std::destructible<T>;

    template <typename T>
    concept LockFreeBufferable = Bufferable<T> && std::is_trivially_copyable_v<T>;
}


