// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#pragma once

#include <optional>

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringBuilder>

#include <CImg.h>

#include "utils/utils.h"

class ConvertedImage
{
    std::optional<QString> path;
    bool isKeepAlive;

    [[nodiscard]] static QString getConvertedFullName(const QString &srcPath, const int width, const int height) noexcept
    {
        const auto &tmpPath = Utils::getTempDirPath();
        const auto fileName = QFileInfo{srcPath}.baseName();
        // name_WxH.jpg
        const QString &name = fileName % QChar('_')
                           % QString::number(width) % QChar('x') % QString::number(height)
                           % QStringLiteral(u".jpg");
        return QDir::cleanPath(tmpPath % QDir::separator() % name);
    }

    void clear() noexcept
    {
        if (!isKeepAlive && path && !path->isEmpty()) {
            QFile::remove(*path);
        }
    }
public:
    ConvertedImage(const QString &srcPath, const int width, const int height, bool keepAlive = false) noexcept
    : isKeepAlive{keepAlive}
    {
        if (!QFileInfo::exists(srcPath)) [[unlikely]] {
            qWarning() << QObject::tr("image convert failed:") << srcPath;
            return;
        }
        const auto &newPath = getConvertedFullName(srcPath, width, height);
        path.emplace(newPath);
        // maybe some TOCTOU problems, but no impacts on this code
        if (QFileInfo::exists(newPath) && isKeepAlive) {
            return;
        }
        cimg_library::CImg<uchar> raw;
        raw.load(srcPath.toStdString().c_str());
        raw.resize(width, height);
        raw.save_jpeg(newPath.toStdString().c_str(), 70);
    }

    ~ConvertedImage() noexcept {
        clear();
    }

    ConvertedImage(const ConvertedImage&) = delete;
    ConvertedImage& operator=(const ConvertedImage&) = delete;

    ConvertedImage(ConvertedImage &&other) noexcept: path{std::move(other.path)}, isKeepAlive{other.isKeepAlive} {}
    ConvertedImage& operator=(ConvertedImage &&other) noexcept
    {
        clear();
        path = std::move(other.path);
        isKeepAlive = other.isKeepAlive;
        return *this;
    }

    [[nodiscard]] QString getImagePath() const noexcept
    {
        return path.value_or(QString{});
    }
};

