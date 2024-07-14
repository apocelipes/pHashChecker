// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

#ifndef SIZEFORMAT_H
#define SIZEFORMAT_H

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

    constexpr std::array<BinaryPrefix, 6> prefixes = {
            BinaryPrefix::Byte,
            BinaryPrefix::KibiByte,
            BinaryPrefix::MibiByte,
            BinaryPrefix::GibiByte,
            BinaryPrefix::TebiByte,
            BinaryPrefix::PebiByte
    };
    constexpr std::array<const char *, 6> prefixNames = {
            "B",
            "KiB",
            "MiB",
            "GiB",
            "TiB",
            "PiB"
    };

    [[nodiscard]] inline QString sizeFormat(const quint64 fileSize) noexcept {
        std::size_t power = static_cast<std::size_t>(std::floor(std::log(fileSize) / std::log(1024)));
        const std::size_t prefixsLength = std::size(prefixes);
        power = power >= prefixsLength ? prefixsLength - 1 : power;
        return QStringLiteral(u"%1").arg(static_cast<double>(fileSize) / static_cast<double>(prefixes[power]), 0, 'f', 1)
                               + prefixNames[power];
    }
}

#endif // SIZEFORMAT_H
