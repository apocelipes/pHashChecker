// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 apocelipes

#pragma once

#include <QObject>

#include <pHash.h>

#include "utils.h"
#include "utils/types.h"

class SearchWorker: public QObject {
    Q_OBJECT
public:
    explicit SearchWorker(
        const Utils::PHashDistance distance,
        const PHashContainer &hashes,
        QObject *parent = nullptr) noexcept
    : QObject{parent},
    _similar_distance{distance},
    _hashes{hashes}
    {}

Q_SIGNALS:
    void sameImg(unsigned int, unsigned int);
    void doneBatch(int);
    void doneAllWork();

public slots:
    void doWork() noexcept;

private:
    [[nodiscard]] bool checkSameImage(const ulong64 a, const ulong64 b) const noexcept
    {
        return ph_hamming_distance(a, b) <= static_cast<int>(_similar_distance);
    }

    const Utils::PHashDistance _similar_distance;
    const PHashContainer &_hashes;
};
