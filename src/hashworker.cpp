// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#include <QDebug>
#include <QThread>

#include "hashworker.h"
#include "path.h"

constexpr unsigned int batchSize = 32;

void HashWorker::doWork() noexcept
{
    auto batchCount = 0u;
    QThread* myThread = QThread::currentThread();
    for (std::size_t idx = _startIndex; idx < _endIndex; ++idx) {
        if (myThread->isInterruptionRequested()) {
            qInfo() << tr("HashWorker thread exit");
            break;
        }

        ulong64 hash = 0;
        if (ph_dct_imagehash(_images[idx].c_str(), hash) < 0) [[unlikely]] {
            qWarning() << tr("calculating pHash failed, skip: ") % Utils::getAbsPath(_images[idx]);
            goto end;
        }

        _matchHistory[idx] = hash;
        end:
        batchCount = (batchCount + 1) & (batchSize - 1);
        if (batchCount == 0) {
            Q_EMIT doneBatch(batchSize);
        } else if (idx == _endIndex - 1) {
            Q_EMIT doneBatch(static_cast<int>(batchCount));
        }
    }

    Q_EMIT doneAllWork();
}
