// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#pragma once

#include <algorithm>
#include <iterator>
#include <optional>
#include <ranges>

#include <QStringBuilder>

namespace Utils {
    template<std::ranges::range Container, typename Element>
    [[nodiscard]] constexpr std::optional<std::ptrdiff_t> indexOf(const Container &container, const Element &target) noexcept
    {
        auto iter = std::ranges::find(container, target);
        if (iter == std::ranges::cend(container)) {
            return std::nullopt;
        }
	    return std::ranges::distance(std::ranges::cbegin(container), iter);
    }

    enum class PHashDistance: int
    {
        STRICT  = 1,
        PRECISE = 5,
        DEFAULT = 8,
        FUZZY   = 10,
    };

    [[nodiscard]] inline QString getFileExtension(const QString &filePath) noexcept
    {
        return filePath.section(QChar('.'), -1, -1);
    }

    [[nodiscard]] inline bool isFormatNeedConvert(const QString &imgPath) noexcept
    {
        return QString::compare(getFileExtension(imgPath), QStringLiteral(u"avif"), Qt::CaseInsensitive) == 0
                    || QString::compare(getFileExtension(imgPath), QStringLiteral(u"jxl"), Qt::CaseInsensitive) == 0;
    }
}
