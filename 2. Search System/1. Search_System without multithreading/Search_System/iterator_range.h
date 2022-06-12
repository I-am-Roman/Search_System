#pragma once

#include <algorithm>
using namespace std;

template <typename It>
class IteratorRange 
{
private:
  It first, last;
public:
  IteratorRange(It first, It last) : first(first), last(last) 
  {  }

  It begin() const 
  {
    return first;
  }

  It end() const 
  {
    return last;
  }

  size_t size() const 
  {
    return last - first;
  }


};

template <typename Container>
auto Head(Container& c, int top) 
{
  return IteratorRange(begin(c), begin(c) + min<size_t>(top, c.size()));
}
