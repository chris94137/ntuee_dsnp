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
      if(empty()) return;
		iterator tmp(begin());
		_head = (++begin())._node;
		begin()._node->_prev = (--end())._node;
		end()._node->_next = begin()._node;
		delete tmp._node;
	}
   void pop_back()
   {
      if(empty()) return;
      iterator tmp(--end());
      iterator e(end());
      if(tmp._node->_prev == end()._node) _head = end()._node;
      iterator last(tmp);
      --last;
      last._node->_next = tmp._node->_next;
      e._node->_prev = tmp._node->_prev;
      delete tmp._node;
   }

   // return false if nothing to erase
   bool erase(iterator pos)
   {
      if(empty()) return false;
      if(pos._node == _head) _head = pos._node->_next;
      pos._node->_prev->_next = pos._node->_next;
      pos._node->_next->_prev = pos._node->_prev;
      delete pos._node;
      return true;
   }
   bool erase(const T& x) { iterator target(find(x)); if(target == end()) return false; else return erase(target); }

   iterator find(const T& x)
   {
      iterator current(begin());
      for(iterator iter = begin(); iter != end(); ++iter)
         if(iter._node->_data == x) return iter;
      return end();
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
	}  // delete all nodes except for the dummy node
   void sort() const
   {
      if(_isSorted) return;
      if (!empty())
      {
         //insertionSort(iterator(_head), size());
         quickSort(begin(), --end(), size());
         //mergeSort(begin(), size());
         //mergeSort_iteration(begin(), end(), size());
         _isSorted = true;
      }
   }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   DListNode<T>*  _head;     // = dummy node if list is empty
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] helper functions; called by public member functions
   void swap(iterator a, iterator b) const
   {
      T tmp = a._node->_data;
      a._node->_data = b._node->_data;
      b._node->_data = tmp;
   }
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
   void quickSort(iterator first, iterator last, size_t size) const
   {
      if(size == 0) return;
      int top = 0;
      pair<iterator, iterator> stack[size];
      stack[top++] = make_pair(first, last);
      while(top)
      {
         --top;
         iterator head(stack[top].first);
         iterator tail(stack[top].second);
         iterator start(head);
         iterator end(tail--);
         if(start == end) continue;
         iterator mid(end);
         while(head != tail && head != tail._node->_next)
         {
            while(head._node->_data < mid._node->_data && head != tail) ++head;
            while(tail._node->_data >= mid._node->_data && head != tail) --tail;
            swap(head, tail);
         }
         if(head._node->_data >= mid._node->_data) swap(head, mid);
         else ++head;
         iterator newEnd(head);
         if(newEnd != start) stack[top++] = make_pair(start, --newEnd);
         if(head != end) stack[top++] = make_pair(++head, end);
      }
   }
   void mergeSort(iterator first, size_t size) const
   {
      iterator miditer(first);
      size_t mid = size / 2;
      for(size_t i = 0; i < mid; ++i)
         ++miditer;
      if(mid > 1) mergeSort(first, mid);
      if(size - mid > 1) mergeSort(miditer, size - mid);
      merge_inplace(first, miditer, mid, size - mid);
   }
   void mergeSort_iteration(iterator first, iterator last, size_t size) const
   {
      iterator start;
      size_t usedSize;
      for(size_t currentSize = 1; currentSize <= size; currentSize = currentSize * 2)
      {
         for(start = first, usedSize = 0; start != last; usedSize += 2 * currentSize)
         {
            if(size - usedSize < currentSize) break;
            iterator mid(start);
            for(size_t i = 0; i < currentSize; ++i) ++mid;
            size_t remainSize = (usedSize + 2 * currentSize > size)? size - usedSize - currentSize : currentSize;
            merge_inplace(start, mid, currentSize, remainSize);
            for(size_t i = 0; i < currentSize * 2; ++i)
            {
               if(start == last) break;
               ++start;
            }
         }
      }
   }
   /*//use container
   void merge(iterator first, iterator second, size_t size1, size_t size2) const
   {
      size_t total = size1 + size2;
      vector<T> tmp;
      iterator start1(first);
      iterator start2(second);
      while(size1 > 0 || size2 > 0)
      {
         if((start1._node->_data <= start2._node->_data || size2 == 0) && size1 != 0)
         {
            tmp.push_back(start1._node->_data);
            ++start1;
            --size1;
         }
         else if((start1._node->_data > start2._node->_data || size1 == 0) && size2 != 0)
         {
            tmp.push_back(start2._node->_data);
            ++start2;
            --size2;
         }
      }
      int push = 0;
      for(;total > 0; --total, ++first, ++push) first._node->_data = tmp[push];
   }
   */
   void merge_inplace(iterator first, iterator second, size_t size1, size_t size2) const
   {
      while(size1 > 0 && size2 > 0)
      {
         if(first._node->_data <= second._node->_data)
         {
            ++first;
            --size1;
         }
         else
         {
            shift(first, second);
            ++first;
            ++second;
            --size2;
         }
      }
   }
   void shift(iterator a, iterator b) const
   {
      T tmp = b._node->_data;
      iterator c(b);
      for(; c != a; --c) c._node->_data = c._node->_prev->_data;
      a._node->_data = tmp; 
   }
};

#endif // DLIST_H
