/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>

template <class T> class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
   friend class DList<T>;
   friend class DList<T>::iterator;

   DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0):
      _data(d), _prev(p), _next(n) {}

   // [NOTE] DO NOT ADD or REMOVE any data member
   T              _data;
   DListNode<T>*  _prev;
   DListNode<T>*  _next;
};


template <class T>
class DList
{
public:
   // TODO: decide the initial value for _isSorted
   DList() {
      _head = new DListNode<T>(T());
      _head->_prev = _head->_next = _head; // _head is a dummy node
      _isSorted = false;
   }
   ~DList() { clear(); delete _head; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class DList;

   public:
      iterator(DListNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return *(this); }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () { _node = _node->_next; return *(this); }
      iterator operator ++ (int) { iterator result(*(this)); _node = _node->_next; return result; }
      iterator& operator -- () { _node = _node->_prev; return *(this); }
      iterator operator -- (int) { iterator result(*(this)); _node = _node->_prev; return result; }

      iterator& operator = (const iterator& i) { _node = i._node; return *(this); }

      bool operator != (const iterator& i) const { return (_node != i._node); }
      bool operator == (const iterator& i) const { return (_node == i._node); }

   private:
      DListNode<T>* _node;
   };

   // TODO: implement these functions
   iterator begin() const { return iterator(_head); }
   iterator end() const { return (--iterator(_head)); }
   bool empty() const { return (begin() == end()); }
   size_t size() const { size_t cnt = 0; for(iterator iter = begin(); iter != end(); ++iter) ++cnt; return cnt; }

   void push_back(const T& x)
	{
		iterator lastiter(end());
      iterator enditer(end());
		--lastiter;
		DListNode<T>* newNode = new DListNode<T>(x, lastiter._node, enditer._node);
		lastiter._node->_next = newNode;
		enditer._node->_prev = newNode;
      if(lastiter == enditer) _head = newNode;
      _isSorted = false;
	}
   void pop_front()
	{
		iterator tmp(begin());
		_head = (++begin())._node;
		begin()._node->_prev = (--end())._node;
		end()._node->_next = begin()._node;
		delete tmp._node;
      _isSorted = false;
	}
   void pop_back()
   {
      iterator tmp(--end());
      iterator last(tmp);
      --last;
      if(last == end()) _head = last._node;
      last._node->_next = tmp._node->_next;
      end()._node->_prev = tmp._node->_prev;
      delete tmp._node;
      _isSorted = false;
   }

   // return false if nothing to erase
   bool erase(iterator pos)
   {
      if(empty()) return false;
      if(pos._node == _head) _head = pos._node->_next;
      pos._node->_prev->_next = pos._node->_next;
      pos._node->_next->_prev = pos._node->_prev;
      delete pos._node;
      _isSorted = false;
      return true;
   }
   bool erase(const T& x) { iterator target(find(x)); if(target == 0) return false; else return erase(target); }

   iterator find(const T& x)
   {
      iterator current(begin());
      for(iterator iter = begin(); iter != end(); ++iter)
         if(iter._node->_data == x) return iter;
      return 0;
   }

   void clear()
	{
      iterator dummy(_head);
      iterator iter(_head);
      --dummy;
		while(iter != dummy)
		{
			iterator tmp = iter;
			++tmp;
			delete iter._node;
			iter = tmp;
		}
      dummy._node->_next = dummy._node;
      dummy._node->_prev = dummy._node;
      _head = dummy._node;
      _isSorted = false;
	}  // delete all nodes except for the dummy node
   void sort() const
   {
      if(_isSorted) return;
      if (!empty())
      {
         insertionSort(iterator(_head), size());
         _isSorted = true;
      }
   }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   DListNode<T>*  _head;     // = dummy node if list is empty
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] helper functions; called by public member functions
   void insertionSort(iterator first, size_t size) const
   {
      iterator current(first);
      iterator choose(first);
      iterator min(first);
      for(size_t i = 0; i < size; i++)
      {
         for(size_t j = 1; j < size - i; j++)
         {
            ++choose;
            if(choose._node->_data < min._node->_data)
               min = choose;
         }
         if(min != current)
            swap(current, min);
         ++current;
         choose = current;
         min = current;
      }
   }
   // void insert(iterator current, iterator min) const
   // {
   //    min._node->_prev->_next = min._node->_next;
   //    min._node->_next->_prev = min._node->_prev;

   //    min._node->_prev = current._node->_prev;
   //    min._node->_next = current._node;

   //    current._node->_prev->_next = min._node;
   //    current._node->_prev = min._node;
   // }
   void swap(iterator a, iterator b) const
   {
      T tmp = a._node->_data;
      a._node->_data = b._node->_data;
      b._node->_data = tmp;
   }
   // void mergeSort(iterator first, size_t size) const
   // {
   //    iterator miditer(first);
   //    size_t mid = size / 2;
   //    for(size_t i = 0; i < mid; ++i)
   //       ++miditer;
   //    if(mid > 1) mergeSort(first, mid);
   //    if(size - mid > 1) mergeSort(miditer, size - mid);
   //    merge(first, miditer, size, size - mid);
   // }
   // void merge(iterator first, iterator miditer, size_t size1, size_t size2) const
   // {
   //    size_t count1 = 0;
   //    size_t count2 = 0;
   //    iterator current(first);
   //    while((size1 + size2) - (count1 + count2))
   //    {
   //       if(count1 == size1 || first._node->_data > miditer._node->_data)
   //       {
   //          swap(current, miditer);
   //          ++current;
   //          ++miditer;
   //          ++count2;
   //       }
   //       else if(count2 == size2 || first._node->_data < miditer._node->_data)
   //       {
   //          swap(current, first);
   //          ++current;
   //          ++first;
   //          ++count1;
   //       }
   //    }
   // }
   // void swap(iterator a, iterator b) const
   // {
   //    iterator tmp(a);
   //    *a = *b;
   //    *b = *tmp; 
   // }
};

#endif // DLIST_H
