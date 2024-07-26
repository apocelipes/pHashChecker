// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#pragma once

#include <QLayout>

namespace Utils {
    inline void freezeLayout(QLayout *layout, bool flag) noexcept
    {
        for (int i = 0; i < layout->count(); ++i) {
            auto it = layout->itemAt(i);
            if (it->layout()) {
                freezeLayout(it->layout(), flag);
            } else if (auto widget = it->widget()) {
                widget->setEnabled(!flag);
            }
        }
    }
}
