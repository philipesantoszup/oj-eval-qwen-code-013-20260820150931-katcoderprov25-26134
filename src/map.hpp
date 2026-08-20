/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   /**
  * the internal type of data.
  * it should have a default constructor, a copy constructor.
    */
   typedef pair<const Key, T> value_type;

  private:
   struct Node {
      value_type data;
      Node *parent;
      Node *left;
      Node *right;
      int height;

      Node() : parent(nullptr), left(nullptr), right(nullptr), height(1) {}
      Node(const value_type &d, Node *p) : data(d), parent(p), left(nullptr), right(nullptr), height(1) {}
   };

   Node *root_;
   size_t size_;
   Compare comp_;

   int get_height(Node *n) const {
      return n ? n->height : 0;
   }

   int get_balance(Node *n) const {
      return n ? get_height(n->left) - get_height(n->right) : 0;
   }

   void update_height(Node *n) {
      if (n) {
         n->height = 1 + (get_height(n->left) > get_height(n->right) ? get_height(n->left) : get_height(n->right));
      }
   }

   void rotate_right(Node *y) {
      Node *x = y->left;
      Node *T2 = x->right;

      x->right = y;
      y->left = T2;

      x->parent = y->parent;
      if (y->parent) {
         if (y->parent->left == y) y->parent->left = x;
         else y->parent->right = x;
      } else {
         root_ = x;
      }

      y->parent = x;
      if (T2) T2->parent = y;

      update_height(y);
      update_height(x);
   }

   void rotate_left(Node *x) {
      Node *y = x->right;
      Node *T2 = y->left;

      y->left = x;
      x->right = T2;

      y->parent = x->parent;
      if (x->parent) {
         if (x->parent->left == x) x->parent->left = y;
         else x->parent->right = y;
      } else {
         root_ = y;
      }

      x->parent = y;
      if (T2) T2->parent = x;

      update_height(x);
      update_height(y);
   }

   void rebalance_after_insert(Node *z) {
      while (z) {
         update_height(z);
         int balance = get_balance(z);

         if (balance > 1) {
            if (get_balance(z->left) < 0) {
               rotate_left(z->left);
            }
            rotate_right(z);
            z = z->parent;
         } else if (balance < -1) {
            if (get_balance(z->right) > 0) {
               rotate_right(z->right);
            }
            rotate_left(z);
            z = z->parent;
         }

         if (z) z = z->parent;
      }
   }

   Node *find_min(Node *n) const {
      if (!n) return nullptr;
      while (n->left) n = n->left;
      return n;
   }

   Node *find_max(Node *n) const {
      if (!n) return nullptr;
      while (n->right) n = n->right;
      return n;
   }

   Node *successor(Node *n) const {
      if (!n) return nullptr;
      if (n->right) return find_min(n->right);
      Node *p = n->parent;
      while (p && n == p->right) {
         n = p;
         p = p->parent;
      }
      return p;
   }

   Node *predecessor(Node *n) const {
      if (!n) return nullptr;
      if (n->left) return find_max(n->left);
      Node *p = n->parent;
      while (p && n == p->left) {
         n = p;
         p = p->parent;
      }
      return p;
   }

   Node *find_node(const Key &key) const {
      Node *cur = root_;
      while (cur) {
         if (comp_(key, cur->data.first)) {
            cur = cur->left;
         } else if (comp_(cur->data.first, key)) {
            cur = cur->right;
         } else {
            return cur;
         }
      }
      return nullptr;
   }

   void erase_subtree(Node *n) {
      if (!n) return;
      erase_subtree(n->left);
      erase_subtree(n->right);
      delete n;
   }

   Node *copy_subtree(Node *n, Node *parent) {
      if (!n) return nullptr;
      Node *nn = new Node(n->data, parent);
      nn->height = n->height;
      nn->left = copy_subtree(n->left, nn);
      nn->right = copy_subtree(n->right, nn);
      return nn;
   }

   void rebalance_after_erase(Node *start) {
      Node *cur = start;
      while (cur) {
         update_height(cur);
         int balance = get_balance(cur);

         if (balance > 1) {
            if (get_balance(cur->left) < 0) {
               rotate_left(cur->left);
            }
            rotate_right(cur);
            cur = cur->parent;
         } else if (balance < -1) {
            if (get_balance(cur->right) > 0) {
               rotate_right(cur->right);
            }
            rotate_left(cur);
            cur = cur->parent;
         } else {
            cur = cur->parent;
         }
      }
   }

  public:
   class const_iterator;
   class iterator {
      private:
       Node *node_;
       map *tree_;
       friend class map<Key, T, Compare>;
       friend class const_iterator;
      public:
       iterator() : node_(nullptr), tree_(nullptr) {}
       iterator(Node *n, map *t) : node_(n), tree_(t) {}

       iterator(const iterator &other) : node_(other.node_), tree_(other.tree_) {}

       iterator &operator=(const iterator &other) {
          node_ = other.node_;
          tree_ = other.tree_;
          return *this;
       }

       iterator operator++(int) {
          iterator tmp(*this);
          ++(*this);
          return tmp;
       }

       iterator &operator++() {
          if (!node_) throw invalid_iterator();
          if (node_->right) {
             node_ = node_->right;
             while (node_->left) node_ = node_->left;
          } else {
             Node *p = node_->parent;
             while (p && node_ == p->right) {
                node_ = p;
                p = p->parent;
             }
             node_ = p;
          }
          return *this;
       }

       iterator operator--(int) {
          iterator tmp(*this);
          --(*this);
          return tmp;
       }

       iterator &operator--() {
          if (!node_) {
             if (!tree_ || !tree_->root_) throw invalid_iterator();
             node_ = tree_->find_max(tree_->root_);
             return *this;
          }
          if (node_->left) {
             node_ = node_->left;
             while (node_->right) node_ = node_->right;
          } else {
             Node *p = node_->parent;
             while (p && node_ == p->left) {
                node_ = p;
                p = p->parent;
             }
             node_ = p;
          }
          if (!node_) throw invalid_iterator();
          return *this;
       }

       value_type &operator*() const {
          if (!node_) throw invalid_iterator();
          return node_->data;
       }

       bool operator==(const iterator &rhs) const {
          return node_ == rhs.node_;
       }

       bool operator==(const const_iterator &rhs) const {
          return node_ == rhs.node_;
       }

       bool operator!=(const iterator &rhs) const {
          return node_ != rhs.node_;
       }

       bool operator!=(const const_iterator &rhs) const {
          return node_ != rhs.node_;
       }

       value_type *operator->() const
           noexcept {
          return &node_->data;
       }
   };

   class const_iterator {
      private:
       Node *node_;
       const map *tree_;
       friend class map<Key, T, Compare>;
      public:
       const_iterator() : node_(nullptr), tree_(nullptr) {}
       const_iterator(Node *n, const map *t) : node_(n), tree_(t) {}
       const_iterator(const const_iterator &other) : node_(other.node_), tree_(other.tree_) {}
       const_iterator(const iterator &other) : node_(other.node_), tree_(other.tree_) {}

       const_iterator &operator=(const const_iterator &other) {
          node_ = other.node_;
          tree_ = other.tree_;
          return *this;
       }

       const_iterator operator++(int) {
          const_iterator tmp(*this);
          ++(*this);
          return tmp;
       }

       const_iterator &operator++() {
          if (!node_) throw invalid_iterator();
          if (node_->right) {
             node_ = node_->right;
             while (node_->left) node_ = node_->left;
          } else {
             Node *p = node_->parent;
             while (p && node_ == p->right) {
                node_ = p;
                p = p->parent;
             }
             node_ = p;
          }
          return *this;
       }

       const_iterator operator--(int) {
          const_iterator tmp(*this);
          --(*this);
          return tmp;
       }

       const_iterator &operator--() {
          if (!node_) {
             if (!tree_ || !tree_->root_) throw invalid_iterator();
             node_ = tree_->find_max(tree_->root_);
             return *this;
          }
          if (node_->left) {
             node_ = node_->left;
             while (node_->right) node_ = node_->right;
          } else {
             Node *p = node_->parent;
             while (p && node_ == p->left) {
                node_ = p;
                p = p->parent;
             }
             node_ = p;
          }
          if (!node_) throw invalid_iterator();
          return *this;
       }

       const value_type &operator*() const {
          if (!node_) throw invalid_iterator();
          return node_->data;
       }

       bool operator==(const const_iterator &rhs) const {
          return node_ == rhs.node_;
       }

       bool operator==(const iterator &rhs) const {
          return node_ == rhs.node_;
       }

       bool operator!=(const const_iterator &rhs) const {
          return node_ != rhs.node_;
       }

       bool operator!=(const iterator &rhs) const {
          return node_ != rhs.node_;
       }

       const value_type *operator->() const
           noexcept {
          return &node_->data;
       }
   };

   /**
  * two constructors
    */
   map() : root_(nullptr), size_(0) {}

   map(const map &other) : root_(nullptr), size_(0), comp_(other.comp_) {
      root_ = copy_subtree(other.root_, nullptr);
      size_ = other.size_;
   }

   /**
  * assignment operator
    */
   map &operator=(const map &other) {
      if (this == &other) return *this;
      clear();
      comp_ = other.comp_;
      root_ = copy_subtree(other.root_, nullptr);
      size_ = other.size_;
      return *this;
   }

   /**
  * Destructors
    */
   ~map() {
      clear();
   }

   /**
  * access specified element with bounds checking
    */
   T &at(const Key &key) {
      Node *n = find_node(key);
      if (!n) throw index_out_of_bound();
      return n->data.second;
   }

   const T &at(const Key &key) const {
      Node *n = find_node(key);
      if (!n) throw index_out_of_bound();
      return n->data.second;
   }

   /**
  * access specified element
    */
   T &operator[](const Key &key) {
      Node *n = find_node(key);
      if (n) return n->data.second;
      value_type v(key, T());
      return insert(v).first->second;
   }

   /**
  * behave like at() throw index_out_of_bound if such key does not exist.
    */
   const T &operator[](const Key &key) const {
      return at(key);
   }

   /**
  * return a iterator to the beginning
    */
   iterator begin() {
      return iterator(find_min(root_), this);
   }

   const_iterator cbegin() const {
      return const_iterator(find_min(root_), this);
   }

   /**
  * return a iterator to the end
    */
   iterator end() {
      return iterator(nullptr, this);
   }

   const_iterator cend() const {
      return const_iterator(nullptr, this);
   }

   /**
  * checks whether the container is empty
    */
   bool empty() const {
      return size_ == 0;
   }

   /**
  * returns the number of elements.
    */
   size_t size() const {
      return size_;
   }

   /**
  * clears the contents
    */
   void clear() {
      erase_subtree(root_);
      root_ = nullptr;
      size_ = 0;
   }

   /**
  * insert an element.
    */
   pair<iterator, bool> insert(const value_type &value) {
      if (!root_) {
         root_ = new Node(value, nullptr);
         size_++;
         return pair<iterator, bool>(iterator(root_, this), true);
      }

      Node *cur = root_;
      Node *parent = nullptr;
      while (cur) {
         parent = cur;
         if (comp_(value.first, cur->data.first)) {
            cur = cur->left;
         } else if (comp_(cur->data.first, value.first)) {
            cur = cur->right;
         } else {
            return pair<iterator, bool>(iterator(cur, this), false);
         }
      }

      Node *new_node = new Node(value, parent);
      if (comp_(value.first, parent->data.first)) {
         parent->left = new_node;
      } else {
         parent->right = new_node;
      }
      size_++;

      rebalance_after_insert(parent);

      return pair<iterator, bool>(iterator(new_node, this), true);
   }

   /**
  * erase the element at pos.
    */
   void erase(iterator pos) {
      if (!pos.node_) throw invalid_iterator();
      Node *n = pos.node_;

      Node *parent = n->parent;

      if (n->left && n->right) {
         Node *succ = successor(n);
         n->data.~value_type();
         new (&n->data) value_type(succ->data);
         n = succ;
         parent = n->parent;
      }

      Node *child = n->left ? n->left : n->right;

      if (child) {
         child->parent = n->parent;
      }

      if (!n->parent) {
         root_ = child;
      } else if (n == n->parent->left) {
         n->parent->left = child;
      } else {
         n->parent->right = child;
      }

      delete n;
      size_--;

      if (parent) {
         rebalance_after_erase(parent);
      }
   }

   /**
  * Returns the number of elements with key
    */
   size_t count(const Key &key) const {
      return find_node(key) ? 1 : 0;
   }

   /**
  * Finds an element with key equivalent to key.
    */
   iterator find(const Key &key) {
      return iterator(find_node(key), this);
   }

   const_iterator find(const Key &key) const {
      return const_iterator(find_node(key), this);
   }
};

}

#endif
