// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

#pragma once

#include <array>
#include <cmath>

#include <QtGlobal>
#include <QString>

namespace Utils {
    enum class BinaryPrefix: qint64 {
        Byte = 1,
        KibiByte = Byte << 10,
        MibiByte = Byte << 20,
        GibiByte = Byte << 30,
        TebiByte = Byte << 40,
        PebiByte = Byte << 50
    };

    constexpr std::array prefixes = {
            BinaryPrefix::Byte,
            BinaryPrefix::KibiByte,
            BinaryPrefix::MibiByte,
            BinaryPrefix::GibiByte,
            BinaryPrefix::TebiByte,
            BinaryPrefix::PebiByte
    };
    constexpr std::array prefixNames = {
            "B",
            "KiB",
            "MiB",
            "GiB",
            "TiB",
            "PiB"
    };

    [[nodiscard]] inline QString sizeFormat(const quint64 fileSize) noexcept {
        if (fileSize == 0) [[unlikely]] {
            return "0.00B";
        }

        auto power = static_cast<std::size_t>(std::floor(std::log(fileSize) / std::log(1024)));
        constexpr std::size_t prefixesLength = std::size(prefixes);
        power = power >= prefixesLength ? prefixesLength - 1 : power;
        return QStringLiteral(u"%1").arg(static_cast<double>(fileSize) / static_cast<double>(prefixes[power]), 0, 'f', 2)
                               + prefixNames[power];
    }
}
