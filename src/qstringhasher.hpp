// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#pragma once

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

using SameImagesContainer = ankerl::unordered_dense::map<std::string, std::vector<std::string>>;
