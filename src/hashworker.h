#ifndef HASHWORKER_H
#define HASHWORKER_H

#include <QObject>

#include <string>
#include <vector>
#include <unordered_map>

#include <pHash.h>

class QReadWriteLock;

class HashWorker : public QObject
{
    Q_OBJECT
public:
    using ContainerType = const std::vector<std::string>;
    using HashContainerType = std::unordered_map<ulong64, std::string>;

    HashWorker(unsigned long start,
               unsigned long limit,
               ContainerType &c,
               HashContainerType &hashes,
               std::vector<ulong64> &insertHistory,
               QReadWriteLock &lock,
               QObject *parent = nullptr)
        : QObject(parent),
          _start{start},
          _limit{limit},
          _images{c},
          _hashes{hashes},
          _insertHistory{insertHistory},
          _hashesLock{lock}
    {}

    void doWork();

Q_SIGNALS:
    void sameImg(const std::string&, const std::string&);
    void doneOneImg();
    void doneAllWork();

private:
    unsigned long _start{};
    unsigned long _limit{};
    ContainerType &_images;
    HashContainerType &_hashes;
    std::vector<ulong64> &_insertHistory;
    QReadWriteLock &_hashesLock;
};

#endif // HASHWORKER_H
