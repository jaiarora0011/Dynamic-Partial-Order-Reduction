#ifndef UTIL_HPP
#define UTIL_HPP

#include <unordered_set>
using namespace std;

template <class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename S, typename T>
struct hash<pair<S, T>>
{
    inline size_t operator()(const pair<S, T> & v) const
    {
         size_t seed = 0;
         hash_combine(seed, v.first);
         hash_combine(seed, v.second);
         return seed;
    }
};

template <typename T>
void unordered_set_union(unordered_set<T>& s1, const unordered_set<T>& s2)
{
  s1.insert(s2.begin(), s2.end());
}

template <typename T>
void unordered_set_difference(unordered_set<T>& s1, const unordered_set<T>& s2)
{
  for (auto const& elem : s2) {
    s1.erase(elem);
  }
}

#endif