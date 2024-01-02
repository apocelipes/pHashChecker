// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <array>
#include <filesystem>
#include <optional>
#include <ranges>
#include <string_view>

#include <QDir>
#include <QTemporaryDir>

namespace Utils {
    template<std::ranges::range Container, typename Element>
    [[nodiscard]] constexpr std::optional<std::ptrdiff_t> indexOf(const Container &container, const Element &target) noexcept {
        auto iter = std::ranges::find(container, target);
        if (iter == std::ranges::cend(container)) {
            return std::nullopt;
        }
        return iter - std::ranges::cbegin(container);
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

    [[nodiscard]] inline std::string getFileExtension(const std::filesystem::path &filePath) noexcept {
        auto ext = filePath.extension().generic_string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        return ext;
    }

    [[nodiscard]] inline bool isSupportImageFormat(const std::filesystem::directory_entry &img) noexcept {
        const auto &ext = getFileExtension(img.path());
        return std::ranges::find(imageExtensions, ext) != imageExtensions.cend();
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
