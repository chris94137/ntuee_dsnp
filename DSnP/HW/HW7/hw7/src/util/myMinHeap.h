/****************************************************************************
  FileName     [ myMinHeap.h ]
  PackageName  [ util ]
  Synopsis     [ Define MinHeap ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_MIN_HEAP_H
#define MY_MIN_HEAP_H

#include <algorithm>
#include <vector>

template <class Data>
class MinHeap
{
public:
   MinHeap(size_t s = 0) { if (s != 0) _data.reserve(s); }
   ~MinHeap() {}

   void clear() { _data.clear(); }

   // For the following member functions,
   // We don't respond for the case vector "_data" is empty!
   const Data& operator [] (size_t i) const { return _data[i]; }   
   Data& operator [] (size_t i) { return _data[i]; }

   size_t size() const { return _data.size(); }

   // TODO
   const Data& min() const { return _data[0]; }
   void insert(const Data& d)
   {
      _data.push_back(d);
      size_t newPos = _data.size();
      while(newPos > 1)
      {
         size_t parent = newPos / 2;
         if(!(d < _data[parent - 1])) break;
         _data[newPos - 1] = _data[parent - 1];
         newPos = parent;
      }
      _data[newPos - 1] = d;
   }
   void delMin() { delData(0); }
   void delData(size_t i)
   {
      size_t num = i, child = 2 * num + 1;
      Data replace = _data.back(); // no matter how, use last data to replace
      _data.pop_back();
      // take from child tree
      while(child < _data.size())
      {
         if(child + 1 < _data.size()) if(_data[child + 1] < _data[child]) ++child; // to smaller child
         // if(replace < _data[child - 1]) break;
         // cerr << _data[child] << " replace " << _data[num] << endl;
         _data[num] = _data[child];
         num = child;
         child = 2 * num + 1;
      }
      // check if num > parent
      int parent = (num - 1) / 2;
      while(parent > 0)
      {
         if(!(_data[parent] < replace))
         {
            // cerr << _data[num] << " replace " << _data[parent] << endl;
            _data[num] = _data[parent];
            num = parent;
            parent = (num - 1) / 2;
         }
         else break;
      }
      // cerr << replace << " replace " << _data[num] << endl;
      _data[num] = replace;
   }

private:
   // DO NOT add or change data members
   vector<Data>   _data;
};

#endif // MY_MIN_HEAP_H
