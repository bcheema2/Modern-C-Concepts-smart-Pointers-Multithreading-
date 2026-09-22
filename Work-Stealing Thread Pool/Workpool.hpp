//
// Created by Bibenpreet Cheema on 9/22/26.
//

#pragma once
#include <concepts>

namespace Workpool {
    template <typename F,typename... Args>
    concept TaskCallable =
            std::invocable<F,Args...> &&
            std::move_constructible<F&>;
}
