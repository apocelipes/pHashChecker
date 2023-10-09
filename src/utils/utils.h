// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <array>
#include <filesystem>
#include <optional>
#include <string_view>

#include <QDir>
#include <QTemporaryDir>

namespace Utils {
    template<typename Iterator, typename Element>
    [[nodiscard]] constexpr std::optional<std::ptrdiff_t> indexOf(Iterator beginIter, Iterator endIter, const Element &target) noexcept {
        auto iter = std::find(beginIter, endIter, target);
        if (iter == endIter) {
            return std::nullopt;
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

    inline constexpr std::array<std::string_view, 6> imageExtensions{
        ".jpg",
        ".jpeg",
        ".png",
        ".webp",
        ".bmp",
        ".avif",
    };

    [[nodiscard]] inline bool isSupportImageFormat(const std::filesystem::path &img) noexcept {
        auto ext = img.extension().generic_string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        return std::find(imageExtensions.cbegin(), imageExtensions.cend(), ext) != imageExtensions.cend();
    }

    // NOTICE: only init once
    [[nodiscard]] inline QString getTempDirPath() noexcept {
        static QTemporaryDir temp{QDir::tempPath() + QDir::separator() + "pHashChecker-XXXXXX"};
        if (!temp.isValid()) {
            qFatal() << QObject::tr("create temporary dir failed");
        }
        return temp.path();
    }
}

#endif // UTILS_H
