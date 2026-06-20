// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 apocelipes
#include <QDebug>
#include <QThread>

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

#include "searchworker.h"

#include <iostream>

constexpr unsigned int batchSize = 1024;

void SearchWorker::doWork() noexcept
{
    auto flags = boost::dynamic_bitset(_hashes.size());
    QThread* myThread = QThread::currentThread();

    for (auto i = 0u; i < _hashes.size(); ++i) {
        if (myThread->isInterruptionRequested()) [[unlikely]] {
            qInfo() << tr("SearchWorker thread exit");
            break;
        }

        if (!flags[i]) {
            const auto originHash = _hashes[i];
            // we never go back, so not really need set every bit
            // flags.set(i);
            for (auto j = i+1; j < _hashes.size(); j++) {
                if (flags[j]) {
                    continue;
                }
                if (checkSameImage(originHash, _hashes[j])) {
                    flags.set(j);
                    Q_EMIT sameImg(i, j);
                }
            }
        }

        if (const auto batchCount = static_cast<int>((i+1) % batchSize); batchCount == 0) {
            Q_EMIT doneBatch(batchSize);
        } else if (i == _hashes.size()-1) {
            Q_EMIT doneBatch(batchCount);
        }
    }

    Q_EMIT doneAllWork();
}
