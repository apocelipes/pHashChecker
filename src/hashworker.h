// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#ifndef HASHWORKER_H
#define HASHWORKER_H

#include <QObject>

#include <string>
#include <vector>

#include <pHash.h>
#include <ankerl/unordered_dense.h>

#include "utils.h"

class QReadWriteLock;

class HashWorker : public QObject
{
    Q_OBJECT
public:
    using ContainerType = const std::vector<std::string>;

    HashWorker(const std::size_t start,
               const std::size_t limit,
               const Utils::PHashDistance distance,
               ContainerType &c,
               std::vector<std::pair<ulong64, std::size_t>> &matchHistory,
               QObject *parent = nullptr) noexcept
        : QObject(parent),
          _start{start},
          _limit{limit},
          _similar_distance{distance},
          _images{c},
          _matchHistory{matchHistory}
    {}

public Q_SLOTS:
    void doWork();

Q_SIGNALS:
    void sameImg(std::size_t, std::size_t);
    void doneOneImg();
    void doneAllWork();

private:
    std::size_t _start{};
    std::size_t _limit{};
    Utils::PHashDistance _similar_distance = Utils::PHashDistance::DEFAULT;
    ContainerType &_images;
    std::vector<std::pair<ulong64, std::size_t>> &_matchHistory;

    [[nodiscard]] bool checkSameImage(ulong64 a, ulong64 b) noexcept
    {
        return ph_hamming_distance(a, b) <= static_cast<int>(_similar_distance);
    }
};

#endif // HASHWORKER_H
