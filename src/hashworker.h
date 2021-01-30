#ifndef HASHWORKER_H
#define HASHWORKER_H

#include <QObject>
#include <QReadWriteLock>

#include <string>
#include <vector>
#include <unordered_map>

#include <pHash.h>

constexpr int SMALLER_DISTANCE = 10;

class HashWorker : public QObject
{
    Q_OBJECT
public:
    using ContainerType = const std::vector<std::string>;
    using HashContainerType = std::unordered_map<ulong64, std::string>;

    HashWorker() = delete;
    HashWorker(unsigned long start, unsigned long limit, ContainerType &c, HashContainerType &hashes, QReadWriteLock &lock, QObject *parent = nullptr)
        : QObject(parent), _start{start}, _limit{limit}, _images{c}, _hashes{hashes}, _lock{lock}
    {}

    void doWork();

signals:
    void sameImg(const std::string&, const std::string&);
    void doneOneImg();
    void doneAllWork();

private:
    unsigned long _start{};
    unsigned long _limit{};
    ContainerType &_images;
    HashContainerType &_hashes;
    QReadWriteLock &_lock;
};

#endif // HASHWORKER_H
