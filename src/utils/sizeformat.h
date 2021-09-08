// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

#ifndef SIZEFORMAT_H
#define SIZEFORMAT_H

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

    constexpr BinaryPrefix prefixes[] = {
            BinaryPrefix::Byte,
            BinaryPrefix::KibiByte,
            BinaryPrefix::MibiByte,
            BinaryPrefix::GibiByte,
            BinaryPrefix::TebiByte,
            BinaryPrefix::PebiByte
    };
    constexpr const char *prefixNames[] = {
            "B",
            "KiB",
            "MiB",
            "GiB",
            "TiB",
            "PiB"
    };

    inline QString sizeFormat(const qint64 fileSize) {
        int power = 0;
        auto count = fileSize;
        while (count /= 1024) {
            ++power;
        }
        power = power >= static_cast<int>(std::size(prefixes)) ? static_cast<int>(std::size(prefixes)) - 1 : power;
        return QString::asprintf("%.1lf", static_cast<double>(fileSize) / static_cast<double>(prefixes[power])) + prefixNames[power];
    }
}

#endif // SIZEFORMAT_H
