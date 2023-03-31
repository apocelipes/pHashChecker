// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2022 apocelipes

#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <vector>

namespace Utils {
    template<typename Iterator, typename Element>
    [[nodiscard]] constexpr int indexOf(Iterator beginIter, Iterator endIter, const Element &target) noexcept {
        auto iter = std::find(beginIter, endIter, target);
        if (iter == endIter) {
            return -1;
        }
        return iter - beginIter;
    }

    enum class PHashDistance: int
    {
        STRICT  = 1,
        PRECISE = 5,
        DEFAULT = 8,
        FUZZY   = 10,
    };
}

#endif // UTILS_H
