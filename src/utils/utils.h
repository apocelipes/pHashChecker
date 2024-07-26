// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#pragma once

#include <algorithm>
#include <array>
#include <filesystem>
#include <iterator>
#include <optional>
#include <ranges>
#include <string_view>

#include <QDir>
#include <QStringBuilder>
#include <QTemporaryDir>

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

    // NOTICE: only init once
    [[nodiscard]] inline QString getTempDirPath() noexcept
    {
        static QTemporaryDir temp{QDir::tempPath() % QDir::separator() % QStringLiteral(u"pHashChecker-XXXXXX")};
        if (!temp.isValid()) [[unlikely]] {
            qFatal() << QObject::tr("create temporary dir failed");
        }
        return temp.path();
    }

    [[nodiscard]] inline bool isFormatNeedConvert(const QString &imgPath) noexcept
    {
        return QString::compare(getFileExtension(imgPath), QStringLiteral(u"avif"), Qt::CaseInsensitive) == 0;
    }
}
