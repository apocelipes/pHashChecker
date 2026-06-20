// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#pragma once

#include <QObject>

#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>

#include "utils.h"
#include "utils/types.h"

class QReadWriteLock;

class HashWorker : public QObject
{
    Q_OBJECT
public:
    using ContainerType = const std::vector<std::string>;

    HashWorker(ContainerType &c,
               std::size_t startIndex,
               std::size_t endIndex,
               PHashContainer &matchHistory,
               QObject *parent = nullptr) noexcept
        : QObject(parent),
          _images{c},
          _matchHistory{matchHistory},
          _startIndex{startIndex},
          _endIndex{endIndex}
    {}

public Q_SLOTS:
    void doWork() noexcept;

Q_SIGNALS:
    void doneBatch(int);
    void doneAllWork();

private:
    ContainerType &_images;
    PHashContainer &_matchHistory;
    const std::size_t _startIndex;
    const std::size_t _endIndex;
};
