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
    using HashContainerType = ankerl::unordered_dense::map<ulong64, size_t>;

    HashWorker(const size_t start,
               const size_t limit,
               const Utils::PHashDistance distance,
               ContainerType &c,
               HashContainerType &hashes,
               std::vector<ulong64> &insertHistory,
               QReadWriteLock &lock,
               QObject *parent = nullptr) noexcept
        : QObject(parent),
          _start{start},
          _limit{limit},
          _similar_distance{distance},
          _images{c},
          _hashes{hashes},
          _insertHistory{insertHistory},
          _hashesLock{lock}
    {}

public Q_SLOTS:
    void doWork();

Q_SIGNALS:
    void sameImg(size_t, size_t);
    void doneOneImg();
    void doneAllWork();

private:
    size_t _start{};
    size_t _limit{};
    Utils::PHashDistance _similar_distance = Utils::PHashDistance::DEFAULT;
    ContainerType &_images;
    HashContainerType &_hashes;
    std::vector<ulong64> &_insertHistory;
    QReadWriteLock &_hashesLock;

    [[nodiscard]] bool checkSameImage(ulong64 a, ulong64 b) noexcept
    {
        return ph_hamming_distance(a, b) <= static_cast<int>(_similar_distance);
    }
};

#endif // HASHWORKER_H
