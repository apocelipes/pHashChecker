#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <vector>

// 不存在找不到的情况故不做检查
template <typename Iterator, typename Element>
unsigned int indexOf(Iterator beginIter, Iterator endIter, const Element &target)
{
    auto iter = std::find(beginIter, endIter, target);
    return iter - beginIter;
}

#endif // UTILS_H
