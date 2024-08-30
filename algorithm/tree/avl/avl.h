/*
代码实现参考
https://www.cnblogs.com/gonghr/p/16064797.html
*/

#pragma once

#include <cstddef>
#include <iostream>
#include <stdexcept>

#include "../utils/walk.h"
#include "../utils/visualization.h"

namespace algorithm {

template <typename KeyTy, typename ValueTy>
struct AVLNode {
  KeyTy key_;
  ValueTy value_;
  int height_;
  struct AVLNode* left_ = nullptr;
  struct AVLNode* right_ = nullptr;
};

template <typename KeyTy, typename ValueTy, typename Cmp = CMP<KeyTy>>
class AVLTree {
 public:
  using KeyType = KeyTy;
  using ValueType = ValueTy;
  using NodeTy = AVLNode<KeyTy, ValueTy>;

  void insert(std::pair<KeyTy, ValueTy> elem) {
    root_ = insertImpl(elem, root_);
  }

  void remove(KeyTy key) {
    root_ = removeImpl(key, root_);
  }
  
  template<WalkOrder order>
  void walk(std::function<void(NodeTy*)> func) {
    walkImpl<order>(root_, func);
  }

  void visualization() {
    algorithm::visualization(root_);
  }

  NodeTy* find(KeyTy key) {
    return findImpl(key, root_);
  }

  AVLTree() = default;
  ~AVLTree() {
    walk<WalkOrder::POSTORDER>([](NodeTy* curr) {
      delete curr;
    });
  }

 private:

  NodeTy* findImpl(const KeyTy& key, NodeTy* curr) {
    if (!curr) {
      return nullptr;
    }
    if (cmp_(key, curr->key_) > 0) {
      return findImpl(key, curr->left_);
    } else if (cmp_(key, curr->key_) < 0) {
      return findImpl(key, curr->right_);
    } else {
      return curr;
    }
  }

  NodeTy* insertImpl(std::pair<KeyTy, ValueTy>& elem, NodeTy* curr) {
    if (!curr) {
      curr =
          new NodeTy{.key_ = elem.first, .value_ = elem.second, .height_ = 1};
      return curr;
    }
    if (cmp_(elem.first, curr->value_) > 0) {
      curr->left_ = insertImpl(elem, curr->left_);
      curr->height_ = height(curr);
      if (height(curr->left_) - height(curr->right_) == 2) {
        if (cmp_(elem.first, curr->left_->value_) > 0) {
          // 不平衡是因为插在了左子树的左子树造成的 LL型 右旋
          curr = LLRotary(curr);
        } else {
          // 不平衡是因为插在了左子树的右子树造成的 LR型 右旋
          curr = LRRotary(curr);
        }
      }
    } else if (cmp_(elem.first, curr->value_) < 0) {
      curr->right_ = insertImpl(elem, curr->right_);
      curr->height_ = height(curr);
      if (height(curr->left_) - height(curr->right_) == -2) {
        if (cmp_(elem.first, curr->right_->value_) < 0) {
          // 不平衡是因为插在了右子树的右子树造成的 LL型 右旋
          curr = RRRotary(curr);
        } else {
          curr = RLRotary(curr);
        }
      }
    } else {
      std::cout << "curr: " << curr->value_ << " key: " << elem.first
                << " cmp: " << cmp_(elem.first, curr->key_) << std::endl;
      throw std::runtime_error("curr value alread exist");
    }
    return curr;
  }

  NodeTy* removeImpl(const KeyTy& key, NodeTy* curr) {
    if (!curr) {
      return curr;
    }
    if (cmp_(key, curr->value_) > 0) {
      curr->left_ = removeImpl(key, curr->left_);
    } else if (cmp_(key, curr->value_) < 0) {
      curr->right_ = removeImpl(key, curr->right_);
    } else {
      auto left = curr->left_;
      auto right = curr->right_;
      if (!left) {
        delete curr;
        return right;
      }
      if (!right) {
        delete curr;
        return left;
      }
      if (!right->left_) {
        right->left_ = curr->left_;
        delete curr;
        return right;
      } else {
        NodeTy* prev0 = right;
        NodeTy* prev1 = right->left_;
        NodeTy* prev2 = right->left_->left_;
        while (prev2) {
          prev0 = prev0->left_;
          prev1 = prev1->left_;
          prev2 = prev2->left_;
        }
        prev0->left_ = prev1->right_;
        prev1->left_ = left;
        prev1->right_ = right;
        delete curr;
        return prev1;
      }
    }
    if (!curr) {
      return curr;
    }
    curr->height_ = height(curr);
    if (height(curr->left_) - height(curr->right_) >= 2) {
      if (height(curr->left_->left_) > height(curr->left_->right_)) {
        return LLRotary(curr);
      } else {
        return LRRotary(curr);
      }
    } else if (height(curr->left_) - height(curr->right_) <= -2) {
      if (height(curr->right_->right_) > height(curr->right_->left_)) {
        return RRRotary(curr);
      } else {
        return RLRotary(curr);
      }
    }
    return curr;
  }

  NodeTy* LLRotary(NodeTy* a) {
    NodeTy* b = a->left_;
    a->left_ = b->right_;
    b->right_ = a;
    a->height_ = height(a);
    b->height_ = height(b);
    return b;
  }

  NodeTy* RRRotary(NodeTy* a) {
    NodeTy* b = a->right_;
    a->right_ = b->left_;
    b->left_ = a;
    a->height_ = height(a);
    b->height_ = height(b);
    return b;
  }

  NodeTy* LRRotary(NodeTy* a) {
    a->left_ = RRRotary(a->left_);
    return LLRotary(a);
  }

  NodeTy* RLRotary(NodeTy* a) {
    a->right_ = LLRotary(a->right_);
    return RRRotary(a);
  }

  AVLNode<KeyTy, ValueTy>* root_;
  Cmp cmp_{};
};

}  // namespace algorithm
