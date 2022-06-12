#pragma once

#include <mutex>
using namespace std;

template <typename T>
class Synchronized 
{
private:
  T value;
  mutex m;

public:
  explicit Synchronized(T init)
    : value(move(init))
  {

  }

  struct Access 
  {
    T& ref_to_value;
    // lock_guard<mutex> only skips one thread
    lock_guard<mutex> guard;
  };

  Access GetAccess() 
  {
    return {value, lock_guard(m)};
  }

};



