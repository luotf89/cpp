/*
代码实现参考
https://blog.csdn.net/melonyzzZ/article/details/133149060
*/

#pragma once
#include <iostream>
#include <asm-generic/errno.h>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <utility>
#include "../utils/utils.h"

namespace algorithm {

template <typename KeyTy, typename ValueTy>
struct AVLNode {
  KeyTy key_;
  ValueTy value_;
  int balance_ = 0;
  struct AVLNode* left_ = nullptr;
  struct AVLNode* right_ = nullptr;
  struct AVLNode* parent_ = nullptr;
};

template <typename KeyTy, typename ValueTy, typename Cmp = CMP<KeyTy>>
class AVLTree {
 public:
  using KeyType = KeyTy;
  using ValueType = ValueTy;
  using NodeTy = AVLNode<KeyTy, ValueTy>;

  std::pair<bool, NodeTy*> insert(std::pair<KeyTy, ValueTy> elem) {
    if (!root_) {
      root_ = new NodeTy{.key_ = elem.first, .value_ = elem.second};
      return {true, root_};
    }
    NodeTy* prev = root_;
    NodeTy* curr = root_;
    while (curr) {
      if (cmp_(elem.first, curr->key_) > 0) {
        prev = curr;
        curr = curr->left_;
      } else if (cmp_(elem.first, curr->key_) < 0) {
        prev = curr;
        curr = curr->right_;
      } else {
        return {false, nullptr};
      }
    }
    curr = new NodeTy{.key_ = elem.first, .value_ = elem.second, .parent_ = prev};
    if (cmp_(elem.first, prev->key_) > 0) {
      prev->left_ = curr;
    } else {
      prev->right_ = curr;
    }
    auto ret = curr;
    fixAfterInsert(curr);
    return {true, ret};
  }

  bool remove(KeyTy key) {
    auto curr = removeImpl(key);
    if (curr) {
      delete curr;
      return true;
    }
    return false;
  }
  
  template<WalkOrder order>
  void walk(std::function<void(NodeTy*)> func) {
    walkImpl<order>(root_, func);
  }

  void visualization(std::string filename = "avl.dot") {
    algorithm::visualization(root_, filename);
  }

  NodeTy* find(KeyTy key) {
    return findImpl(key, root_, cmp_);
  }

  AVLTree() = default;
  ~AVLTree() {
    walk<WalkOrder::POSTORDER>([](NodeTy* curr) {
      delete curr;
    });
  }

  bool is_balanced() {
    return isBalancedImpl(root_);
  }

 private:
  bool isBalancedImpl(NodeTy* curr) {
    if (!curr) {
      return true;
    }
    int left_height = height(curr->left_);
    int right_height = height(curr->right_);
    return std::abs(left_height - right_height) < 2 && isBalancedImpl(curr->left_) && isBalancedImpl(curr->right_);
  }

  void fixAfterInsert(NodeTy* curr) {
    NodeTy* prev = curr->parent_;
    while(prev) {
      if (curr == prev->left_) {
        prev->balance_++;
      } else {
        prev->balance_--;
      }
      if (prev->balance_ == 0) {
        break;
      } else if (prev->balance_ == 1 || prev->balance_ == -1) {
        curr = prev;
        prev = prev->parent_;
      } else if (prev->balance_ == 2 || prev->balance_ == -2) {
        if (prev->balance_ == 2 && curr->balance_ == 1) {
          LLRotary(prev);
        } else if (prev->balance_ == -2 && curr->balance_ == -1) {
          RRRotary(prev);
        } else if (prev->balance_ == 2 && curr->balance_ == -1) {
          LRRotary(prev);
        } else if (prev->balance_ == -2 && curr->balance_ == 1) {
          RLRotary(prev);
        } else {
          assert(false);
        }
        break;
      } else {
        assert(false);
      }
    }
  }

  void fixAfterRemove(NodeTy* curr) {
    NodeTy* prev = curr->parent_;
    NodeTy* child = curr->left_? curr->left_: curr->right_;
    if (child) {
      child->parent_ = prev;
    }
    if (!prev) {
      root_ = child;
    } else {
      if (prev->left_ == curr) {
        prev->balance_--;
        prev->left_ = child;
      } else {
        prev->balance_++;
        prev->right_ = child;
      }
      bool is_first = true;
      while (prev) {
        if (!is_first) {
          if (prev->left_ == child) {
            prev->balance_--;
          } else {
            prev->balance_++;
          }
        }
        is_first = false;
        if (prev->balance_ == -1 || prev->balance_ == 1) {
          break;
        } else if (prev->balance_ == 2 || prev->balance_ == -2) {
          int sign = 0;
          NodeTy* higher = nullptr;
          if (prev->balance_ < 0) {
            sign = -1;
            higher = prev->right_;
          } else {
            sign = 1;
            higher = prev->left_;
          }
          if (higher->balance_ == 0) {
            if (sign < 0) { 
              RRRotary(prev);
              prev->balance_ = -1;
              higher->balance_ = 1;
            } else {
              LLRotary(prev);
              prev->balance_ = 1;
              higher->balance_ = -1;
            }
            break;
          } else if (higher->balance_ == sign) {
            if (sign < 0) {
              RRRotary(prev);
              prev = higher;
              child = prev->left_;
            } else {
              LLRotary(prev);
              prev = higher;
              child = prev->right_;
            }
          } else {
            if (sign < 0) {
              auto tmp = prev->right_->left_;
              RLRotary(prev);
              prev = tmp;
              child = prev->left_;
            } else {
              auto tmp = prev->left_->right_;
              LRRotary(prev);
              prev = tmp;
              child = prev->right_;
            }
          }
        }
        child = prev;
        prev = prev->parent_;
      }
    }
  }

  NodeTy* removeImpl(KeyTy key) {
    auto curr = findImpl(key, root_, cmp_);
    if (curr) {
      auto prev = curr->parent_;
      if (curr->left_ && curr->right_) {
        prev = curr;
        NodeTy* left_max = curr->left_;
        while (left_max->right_) {
          prev = left_max;
          left_max = left_max->right_;
        }
        std::swap(curr->key_, left_max->key_);
        std::swap(curr->value_, left_max->value_);
        curr = left_max;
      }
      fixAfterRemove(curr);
      return curr;
    }
    return nullptr;
  }

 void LLRotary(NodeTy* a) {
    NodeTy* b = a->left_;
    NodeTy* br = b->right_;
    a->left_ = br;
    if (br) {
      br->parent_ = a;
    }
    b->right_ = a;
    auto tmp = a->parent_;
    a->parent_ = b;
    b->parent_ = tmp;
    if (!tmp) {
      root_ = b;
    } else {
      if(tmp->left_ == a) {
        tmp->left_ = b;
      } else {
        tmp->right_ = b;
      }
    }
    a->balance_ = 0;
    b->balance_ = 0;
  }

  void RRRotary(NodeTy* a) {
    NodeTy* b = a->right_;
    auto bl = b->left_;
    a->right_ = bl;
    if (bl) {
      bl->parent_ = a;
    }
    b->left_ = a;
    auto tmp = a->parent_;
    a->parent_ = b;
    b->parent_ = tmp;
    if (!tmp) {
      root_ = b;
    } else {
      if(tmp->left_ == a) {
        tmp->left_ = b;
      } else {
        tmp->right_ = b;
      }
    }
    a->balance_ = 0;
    b->balance_ = 0;
  }

  void LRRotary(NodeTy* a) {
    auto b = a->left_;
    auto c = b->right_;
    int balance = c->balance_;
    RRRotary(b);
    LLRotary(a);
    if (balance == 0) {
      a->balance_ = 0;
      b->balance_ = 0;
      c->balance_ = 0;
    } else if (balance == 1) {
      a->balance_ = -1;
      b->balance_ = 0;
      c->balance_ = 0;
    } else if (balance == -1) {
      a->balance_ = 0;
      b->balance_ = 1;
      c->balance_ = 0;
    } else {
      assert(false);
    }
  }

  void RLRotary(NodeTy* a) {
    auto b = a->right_;
    auto c = b->left_;
    int balance = c->balance_;
    LLRotary(b);
    RRRotary(a);
    if (balance == 0) {
      a->balance_ = 0;
      b->balance_ = 0;
      c->balance_ = 0;
    } else if (balance == 1) {
      a->balance_ = 0;
      b->balance_ = -1;
      c->balance_ = 0;
    } else if (balance == -1) {
      a->balance_ = 1;
      b->balance_ = 0;
      c->balance_ = 0;
    } else {
      assert(false);
    }
  }

  AVLNode<KeyTy, ValueTy>* root_ = nullptr;
  Cmp cmp_{};
};

}  // namespace algorithm

