// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#pragma once

#include <QFileInfo>
#include <QImage>
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
        const int spectrum = raw.spectrum();
        const uchar *red = raw.data(0, 0, 0, 0);
        const uchar *green = spectrum >= 3 ? raw.data(0, 0, 0, 1) : red;
        const uchar *blue = spectrum >= 3 ? raw.data(0, 0, 0, 2) : red;
        const uchar *alpha = spectrum >= 4 ? raw.data(0, 0, 0, 3)
                           : spectrum == 2 ? raw.data(0, 0, 0, 1)
                                           : nullptr;
        if (alpha != nullptr) {
            for (int y = 0; y < height; ++y) {
                auto line = reinterpret_cast<QRgb *>(img.scanLine(y));
                const auto rowOffset = static_cast<qsizetype>(y) * width;
                for (int x = 0; x < width; ++x) {
                    const qsizetype idx = rowOffset + x;
                    line[x] = qPremultiply(qRgba(red[idx], green[idx], blue[idx], static_cast<int>(alpha[idx])));
                }
            }
        } else {
            for (int y = 0; y < height; ++y) {
                auto line = reinterpret_cast<QRgb *>(img.scanLine(y));
                const auto rowOffset = static_cast<qsizetype>(y) * width;
                for (int x = 0; x < width; ++x) {
                    const qsizetype idx = rowOffset + x;
                    line[x] = qPremultiply(qRgba(red[idx], green[idx], blue[idx], 255));
                }
            }
        }
        return QPixmap::fromImage(std::move(img));
    }
}
