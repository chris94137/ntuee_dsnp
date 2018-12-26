/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data>
class HashSet
{
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;

   public:
      iterator()
         : _pos(0), _bucketIndex(0), _numBuckets(0), _currentBuckets(0), _node(0) { }
      iterator(size_t p, size_t i, size_t n, vector<Data>* b)
         : _pos(p), _bucketIndex(i), _numBuckets(n), _currentBuckets(b), _node(&(b[_bucketIndex][_pos])) { }
      iterator(const iterator& i) { (*this) = i; }
      const Data& operator * () const { return *_node; }
      Data& operator * () { return *_node; }
      iterator& operator ++ ()
      {
         if(_bucketIndex == _numBuckets) { _pos = 0; return (*this); } // last element in hash
         if(_pos != _currentBuckets[_bucketIndex].size() - 1) { ++_pos; _node = &(_currentBuckets[_bucketIndex][_pos]); return (*this); }
         _pos = 0;
         while(++_bucketIndex < _numBuckets)
            if(_currentBuckets[_bucketIndex].size()) break;
         if(_bucketIndex != _numBuckets) _node = &_currentBuckets[_bucketIndex][_pos];
         return (*this);
      }
      iterator operator ++(int) { auto tmp = (*this); ++(*this); return tmp; }
      iterator& operator -- ()
      {
         if(_pos == 0)
         {
            size_t prevIndex = _bucketIndex;
            while(prevIndex-- != 0)
               if(_currentBuckets[prevIndex].size())
               {
                  _pos = _currentBuckets[prevIndex].size() - 1;
                  _bucketIndex = prevIndex;
                  break;
               }
         }
         _node = &_currentBuckets[_bucketIndex][_pos];
         return (*this);
      }
      iterator operator --(int) { auto tmp = (*this); --(*this); return tmp; }
      iterator& operator = (const iterator& i)
      {
         _node = i._node;
         _pos = i._pos;
         _bucketIndex = i._bucketIndex;
         _numBuckets = i._numBuckets;
         _currentBuckets = i._currentBuckets;
         return (*this);
      }
      bool operator == (const iterator& i) const { if(_pos == i._pos && _bucketIndex == i._bucketIndex) return true; return false; }
      bool operator != (const iterator& i) const { if((*this) == i) return false; return true; }
   private:
      size_t _pos, _bucketIndex, _numBuckets;
      vector<Data>* _currentBuckets;
      Data* _node;
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const
   {
      size_t i = 0;
      while(true)
      {
         if(i == _numBuckets) break;
         if(_buckets[i].size()) break;
         ++i;
      };
      return iterator(0, i, _numBuckets, _buckets);
   }
   // Pass the end
   iterator end() const { return iterator(0, _numBuckets, _numBuckets, _buckets); }
   // return true if no valid data
   bool empty() const { if(begin() == end()) return true; return false; }
   // number of valid data
   size_t size() const { size_t s = 0; for(size_t i = 0; i < _numBuckets; ++i) s += _buckets[i].size(); return s; }

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const
   {
      size_t hash = bucketNum(d);
      for(size_t i = 0; i < _buckets[hash].size(); ++i)
         if(_buckets[hash][i] == d) return true;
      return false;
   }

   // query if d is in the hash..._numBuckets
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const
   {
      size_t hash = bucketNum(d), pos = 0;
      while(true)
      {
         if(pos == _buckets[hash].size()) return false;
         if(_buckets[hash][pos] == d) { d = _buckets[hash][pos]; return true; }
         ++pos;
      }
   }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d)
   {
      size_t hash = bucketNum(d), pos = 0;
      while(true)
      {
         if(pos == _buckets[hash].size()) { _buckets[hash].push_back(d); return false; }
         if(_buckets[hash][pos] == d) { _buckets[hash][pos] = d; return true; }
         ++pos;
      }
   }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
   bool insert(const Data& d)
   {
      size_t hash = bucketNum(d), pos = 0;
      while(true)
      {
         if(pos == _buckets[hash].size()) { _buckets[hash].push_back(d); return true; }
         if(_buckets[hash][pos] == d) return false;
         ++pos;
      }
   }

   // return true if removed successfully (i.e. d is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const Data& d)
   {
      if(empty()) return false;
      size_t hash = bucketNum(d), pos = 0;
      while(true)
      {
         if(pos == _buckets[hash].size()) return false;
         if(_buckets[hash][pos] == d) { _buckets[hash][pos] = _buckets[hash].back(); _buckets[hash].pop_back(); return true; }
         ++pos;
      }
   }

private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;

   size_t bucketNum(const Data& d) const {
      return (d() % _numBuckets); }
};

#endif // MY_HASH_SET_H
