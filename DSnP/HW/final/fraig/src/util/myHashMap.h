/****************************************************************************
  FileName     [ myHashMap.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashMap and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>

using namespace std;

// TODO: (Optionally) Implement your own HashMap and Cache classes.

//-----------------------
// Define HashMap classes
//-----------------------
// To use HashMap ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
class HashKey
{
public:
   HashKey(size_t gate1, size_t gate2, bool inv1, bool inv2)
   {
      if(inv1) _fanin1 = (gate1 | 1);
		else _fanin1 = gate1;
		if(inv2) _fanin2 = (gate2 | 1);
		else _fanin2 = gate2;
      if(_fanin1 > _fanin2)
      {
         size_t a = _fanin1;
         _fanin1 = _fanin2;
         _fanin2 = a;
      }
   }

   size_t operator() () const { return ((_fanin1 << 32) + ((_fanin2 << 32) >> 32)); }

   bool operator == (const HashKey& k) const { if(k._fanin1 == _fanin1 && k._fanin2 == _fanin2) return true; return false; }

private:
   size_t _fanin1;
   size_t _fanin2;
};

template <class HashKey, class HashData>
class HashMap
{
typedef pair<HashKey, HashData> HashNode;

public:
   HashMap(size_t b=0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashMap() { reset(); }

   // [Optional] TODO: implement the HashMap<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the HashMap
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashMap<HashKey, HashData>;

   public:
      iterator()
         : _pos(0), _bucketIndex(0), _numBuckets(0), _currentBuckets(0), _node(0) { }
      iterator(size_t p, size_t i, size_t n, HashNode b)
         : _pos(p), _bucketIndex(i), _numBuckets(n), _currentBuckets(b), _node(&(b[_bucketIndex][_pos])) { }
      iterator(const iterator& i) { (*this) = i; }
      const HashNode& operator * () const { return *_node; }
      HashNode& operator * () { return *_node; }
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
      vector<HashNode>* _currentBuckets;
      HashNode* _node;
   };

   void init(size_t b) {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

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

   // check if k is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const HashKey& k) const
   {
      size_t hash = bucketNum(k);
      for(size_t i = 0; i < _buckets[hash].size(); ++i)
         if(_buckets[hash][i].first == k) return true;
      return false;
   }

   // query if k is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(const HashKey& k, HashData& d) const
   {
      size_t hash = bucketNum(k), pos = 0;
      while(true)
      {
         if(pos == _buckets[hash].size()) return false;
         if(_buckets[hash][pos].first == k) { d = _buckets[hash][pos].second; return true; }
         ++pos;
      }
   }

   // update the entry in hash that is equal to k (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const HashKey& k, HashData& d)
   {
      size_t hash = bucketNum(k), pos = 0;
      while(true)
      {
         if(pos == _buckets[hash].size()) { _buckets[hash].push_back(make_pair(k, d)); return false; }
         if(_buckets[hash][pos].first == k) { _buckets[hash][pos].second = d; return true; }
         ++pos;
      }
   }

   // return true if inserted d successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d)
   {
      size_t hash = bucketNum(k), pos = 0;
      while(true)
      {
         if(pos == _buckets[hash].size()) { _buckets[hash].push_back(make_pair(k, d)); return true; }
         if(_buckets[hash][pos].first == k) return false;
         ++pos;
      }
   }

   // return true if removed successfully (i.e. k is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const HashKey& k)
   {
      if(empty()) return false;
      size_t hash = bucketNum(k), pos = 0;
      while(true)
      {
         if(pos == _buckets[hash].size()) return false;
         if(_buckets[hash][pos].first == k) { _buckets[hash][pos] = _buckets[hash].back(); _buckets[hash].pop_back(); return true; }
         ++pos;
      }
   }

private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//    
//    size_t operator() () const { return 0; }
//   
//    bool operator == (const CacheKey&) const { return true; }
//       
// private:
// }; 
// 
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { reset(); _size = s; _cache = new CacheNode[s]; }
   void reset() {  _size = 0; if (_cache) { delete [] _cache; _cache = 0; } }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const {
      size_t i = k() % _size;
      if (k == _cache[i].first) {
         d = _cache[i].second;
         return true;
      }
      return false;
   }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) {
      size_t i = k() % _size;
      _cache[i].first = k;
      _cache[i].second = d;
   }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
