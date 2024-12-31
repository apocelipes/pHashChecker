// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 apocelipes

#pragma once

#include <QDir>

namespace Utils {
    [[nodiscard]] inline QString getAbsPath(const QString &fileName) noexcept
    {
        QDir current;
        return current.cleanPath(current.absoluteFilePath(fileName));
    }
}
