// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#pragma once

#include <cstdint>

#include <QString>

#include <ankerl/unordered_dense.h>

template <>
struct ankerl::unordered_dense::hash<QString> {
    using is_avalanching = void;

    [[nodiscard]] uint64_t operator()(const QString &x) const noexcept {
        const auto bytes = x.toLocal8Bit();
        return detail::wyhash::hash(bytes.constData(), bytes.size());
    }
};

struct std_string_hash {
    using is_transparent = void; // enable heterogeneous lookup
    using is_avalanching = void; // mark class as high quality avalanching hash

    [[nodiscard]] uint64_t operator()(const char* str) const noexcept {
        return ankerl::unordered_dense::hash<std::string_view>{}(str);
    }

    [[nodiscard]] uint64_t operator()(std::string_view str) const noexcept {
        return ankerl::unordered_dense::hash<std::string_view>{}(str);
    }

    [[nodiscard]] uint64_t operator()(std::string const& str) const noexcept {
        return ankerl::unordered_dense::hash<std::string_view>{}(str);
    }
};

struct std_string_eq {
    using is_transparent = void;

    template <class T, class U>
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    [[nodiscard]] decltype(auto) operator()(T&& lhs, U&& rhs) const noexcept {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
        return std::forward<T>(lhs) == std::forward<U>(rhs);
    }
};

using SameImagesContainer = ankerl::unordered_dense::map<std::string, std::vector<std::string>, std_string_hash, std_string_eq>;
