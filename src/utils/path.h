// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 apocelipes

#pragma once

#include <QDir>

#include <concepts>

namespace Utils {
    template <typename T>
    requires std::same_as<QString, T> || std::same_as<std::string, T> || std::same_as<std::string_view, T>
    [[nodiscard]] inline QString getAbsPath(const T &fileName) noexcept
    {
        const QDir current;
        if constexpr (std::same_as<std::string, T>) {
            return QDir::cleanPath(current.absoluteFilePath(QString::fromStdString(fileName)));
        } else if constexpr (std::same_as<std::string_view, T>) {
            return QDir::cleanPath(current.absoluteFilePath(QString::fromUtf8(fileName.data(), fileName.size())));
        } else {
            return QDir::cleanPath(current.absoluteFilePath(fileName));
        }
    }
}
