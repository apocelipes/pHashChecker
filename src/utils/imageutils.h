// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#pragma once

#include <QFileInfo>
#include <QPixmap>

#include <CImg.h>

namespace Utils {
    // convert image to QPixmap, only support 8-bit colors
    [[nodiscard]] inline QPixmap convertToPixmap(const QString &srcPath, const int width, const int height) noexcept
    {
        if (!QFileInfo::exists(srcPath)) [[unlikely]] {
            qWarning() << QObject::tr("image convert failed:") << srcPath;
            return QPixmap{};
        }

        cimg_library::CImg<uchar> raw;
        raw.load(srcPath.toStdString().c_str());
        raw.resize(width, height);
        QImage img{width, height, QImage::Format_ARGB32_Premultiplied};
        const bool hasAlpha = raw.spectrum() == 4;
        // Currently, it's fast enough, may be not worth to add a cache
        cimg_forXY(raw, x, y) {
            const int alpha = hasAlpha ? static_cast<int>(raw(x, y, 3)) : 255;
            QColor color{raw(x, y, 0), raw(x, y, 1), raw(x, y, 2), alpha};
            img.setPixelColor(x, y, color);
        }
        return QPixmap::fromImage(std::move(img));
    }
}
