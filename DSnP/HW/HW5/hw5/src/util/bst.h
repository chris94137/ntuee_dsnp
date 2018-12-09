/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>

using namespace std;

template <class T> class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
   // TODO: design your own class!!
   friend class BSTree<T>;
   friend class BSTree<T>::iterator;

   BSTreeNode(const T& d, BSTreeNode<T>* l = 0, BSTreeNode<T>* r = 0, BSTreeNode<T>* p = 0):
      _data(d), _leftChild(l), _rightChild(r), _parent(p){}
   T                    _data;
   BSTreeNode<T>*  _leftChild;
   BSTreeNode<T>* _rightChild;
   BSTreeNode<T>*     _parent;
};


template <class T>
class BSTree
{
public:
   BSTree()
   {
      _tail = new BSTreeNode<T>(T());
      _root = _tail;
   }
   // TODO: design your own class!!
   class iterator
   {
      friend class BSTree;

   public:
      iterator(BSTreeNode<T>* n = 0): _node(n) {}
      iterator(const iterator& i)   : _node(i._node){}
      ~iterator() {}
      BSTreeNode<T>* toMin(BSTreeNode<T>* node) const { while(node->_leftChild) node = node->_leftChild; return node; }
      BSTreeNode<T>* toMax(BSTreeNode<T>* node) const { while(node->_rightChild) node = node->_rightChild; return node; }
      const T& operator * () const { return *(this); }
      T& operator * () { return _node->_data; }
      iterator& operator ++ ()
      {
         if(_node->_rightChild)
         {
            _node = _node->_rightChild;
            _node = toMin(_node);
         }
         else
         {
            while(true)
            {
               if(!_node->_parent) break;
               else if(_node == _node->_parent->_leftChild)
               {
                  _node = _node->_parent;
                  break;
               }
               _node = _node->_parent;
            }
         }
         return *(this);
      }
      iterator operator ++ (int)
      {
         iterator tmp = *(this);
         ++(*this);
         return tmp;
      }
      iterator& operator -- ()
      {
         if(_node->_leftChild)
         {
            _node = _node->_leftChild;
            _node = toMax(_node);
         }
         else
         {
            while(true)
            {
               if(!_node->_parent) break;
               else if(_node == _node->_parent->_rightChild)
               {
                  _node = _node->_parent;
                  break;
               }
               _node = _node->_parent;
            }
         }
         return *(this);
      }
      iterator operator -- (int)
      {
         iterator tmp = *(this);
         --(*this);
         return tmp;
      }
      iterator& operator = (const iterator& i)
      {
         _node = i._node;
         return *(this);
      }

      bool operator != (const iterator& i) const { return (_node != i._node); }
      bool operator == (const iterator& i) const { return (_node == i._node); }

   private:
      BSTreeNode<T>* _node;
   };
   iterator begin() const
   {
      BSTreeNode<T>* tmp = _root;
      tmp = toMin(tmp);
      return iterator(tmp);
   }
   iterator end() const
   {
      return iterator(_tail);
   }
   iterator root() const { return iterator(_root); }
   bool empty() const { return (_root == _tail); }
   size_t size() const
   {
      int count = 0;
      for(iterator iter = begin(); iter != end(); ++iter)
         ++count;
      return count;
   }

   void insert(const T& x)
   {
      if(empty())
      {
         //cerr << "make root" << endl;
         BSTreeNode<T>* newNode = new BSTreeNode<T>(x, 0, _tail, 0);
         _root = newNode;
         _tail->_parent = newNode; 
         return;
      }
      else insert(x, _root);
   }
   void pop_front() { if(!empty()) removeNode(begin()); }
   void pop_back() { if(!empty()) removeNode(iterator(_tail->_parent)); }

   bool erase(iterator pos) { if(empty()) return false; else removeNode(pos); return true; }
   bool erase(const T& x) { iterator iter = find(x); if(iter == end()) return false; else return erase(iter); }

   iterator find(const T& x)
   {
      //cerr << "find " << x << endl;
      iterator iter(begin());
      for(;iter != end(); ++iter) if(iter._node->_data == x) return iter;
      return end();
   }

   void clear()
   {
      if(empty()) return;
      size_t bstsize = size();
      iterator iter(begin());
      iterator tmp(iter);
      for(size_t i = 0; i < bstsize; i++)
      {
         if(iter == end()) return;
         tmp = iter++;
         removeNode(tmp);
      }
   }

   void sort() const {}
   void print() const
   {
      if(!empty())
      {
         cout << root()._node->_data << endl;
         showChilds(root(), 2);
      }
   }
   void showChilds(iterator iter, size_t tab) const
   {
      for(size_t i = 0; i < tab; ++i) cout << " ";
      if(iter._node->_leftChild)
      {
         //cerr << "has left : ";
         cout << iter._node->_leftChild->_data << endl;
         showChilds(iterator(iter._node->_leftChild), tab + 2);
      }
      else cout << "[0]" << endl;
      for(size_t i = 0; i < tab; ++i) cout << " ";
      if(iter._node->_rightChild && iter._node->_rightChild != _tail)
      {
         //cerr << "had right : ";
         cout << iter._node->_rightChild->_data << endl;
         showChilds(iterator(iter._node->_rightChild), tab + 2);
      }
      else cout << "[0]" << endl;
   }
private:
   BSTreeNode<T>* _root;
   BSTreeNode<T>* _tail;

   void removeNode(iterator iter)
   {
      //no child or one child
      if(iter._node->_leftChild == 0 || iter._node->_rightChild == 0)
      {
         //cerr << "no child or one child" << endl;
         iterator inheritNode((iter._node->_leftChild)? iter._node->_leftChild : iter._node->_rightChild);
         if(iter._node == _root)
         {
            _root = inheritNode._node;
            inheritNode._node->_parent = 0;
         }
         else
         {
            ((isLeftLeaf(iter))? iter._node->_parent->_leftChild : iter._node->_parent->_rightChild) = inheritNode._node;
            if(inheritNode._node) inheritNode._node->_parent = iter._node->_parent;
         }
         delete iter._node;
      }
      else if(iter._node->_rightChild == _tail)
      {
         //cerr << "one child one tail" << endl;
         iterator newBack(iter);
         --newBack;
         //cerr << "new back " << newBack._node->_data << endl;
         newBack._node->_rightChild = _tail;
         _tail->_parent = newBack._node;

         if(iter._node == _root)
         {
            _root = iter._node->_leftChild;
            iter._node->_leftChild->_parent = 0;
         }
         else
         {
            ((isLeftLeaf(iter))? iter._node->_parent->_leftChild : iter._node->_parent->_rightChild) = iter._node->_leftChild;
            iter._node->_leftChild->_parent = iter._node->_parent;
         }
         delete iter._node;
      }
      //both child
      else
      {
         //cerr << "two child" << endl;
         iterator swapiter(iter);
         ++swapiter;
         if(swapiter == end()) ----swapiter;
         iter._node->_data = swapiter._node->_data;
         removeNode(swapiter);
      }
   }
   bool isLeftLeaf(iterator& iter)
   {
      if(iter._node == iter._node->_parent->_leftChild) return true;
      else return false;
   }
   void insert(const T& x, BSTreeNode<T>* node)
   {
      if(node->_data < x)
      {
         //cerr << "go right" << endl;
         if(!node->_rightChild)
         {
            BSTreeNode<T>* newNode = new BSTreeNode<T>(x, 0, 0, node);
            node->_rightChild = newNode;
         }
         else if(node->_rightChild == _tail)
         {
            BSTreeNode<T>* newNode = new BSTreeNode<T>(x, 0, _tail, node);
            node->_rightChild = newNode;
            _tail->_parent = newNode;
         }
         else insert(x, node->_rightChild);
      }
      else
      {
         //cerr << "go left" << endl;
         if(!node->_leftChild)
         {
            BSTreeNode<T>* newNode = new BSTreeNode<T>(x, 0, 0, node);
            node->_leftChild = newNode;
         }
         else insert(x, node->_leftChild);
      }
   }
   BSTreeNode<T>* toMin(BSTreeNode<T>* node) const { while(node->_leftChild) node = node->_leftChild; return node; }
   BSTreeNode<T>* toMax(BSTreeNode<T>* node) const { while(node->_rightChild) node = node->_rightChild; return node; }
};

#endif // BST_H
