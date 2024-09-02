#pragma once


#include <stdexcept>
#include <iostream>
#include <functional>
#include "../utils/utils.h"


namespace algorithm {


template <typename KeyTy, typename ValueTy>
struct BSNode {
  KeyTy key_;
  ValueTy value_;
  struct BSNode* left_ = nullptr;
  struct BSNode* right_ = nullptr;
};

template <typename KeyTy, typename ValueTy, typename Cmp = CMP<KeyTy>>
class BSTree {
 public:
  using KeyType = KeyTy;
  using ValueType = ValueTy;
  using NodeTy = BSNode<KeyTy, ValueTy>;

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
    return findImpl(key, root_, cmp_);
  }

  BSTree() = default;
  ~BSTree() {
    walk<WalkOrder::POSTORDER>([](NodeTy* curr) {
      delete curr;
    });
  }

 private:

  NodeTy* insertImpl(std::pair<KeyTy, ValueTy>& elem, NodeTy* curr) {
    if (!curr) {
      curr = new NodeTy{.key_ = elem.first, .value_ = elem.second};
      return curr;
    }
    if (cmp_(elem.first, curr->key_) > 0) {
      curr->left_ = insertImpl(elem, curr->left_);
    } else if (cmp_(elem.first, curr->key_) < 0) {
      curr->right_ = insertImpl(elem, curr->right_);
    } else {
      throw std::runtime_error("curr value alread exist");
    }
    return curr;
  }

  NodeTy* removeImpl(const KeyTy& key, NodeTy* curr) {
    if (!curr) {
      return curr;
    }
    if (cmp_(key, curr->key_) > 0) {
      curr->left_ = removeImpl(key, curr->left_);
    } else if (cmp_(key, curr->key_) < 0) {
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
        auto prev0 = right;
        auto prev1 = right->left_;
        auto prev2 = right->left_->left_;
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
    return curr;
  }

  NodeTy* root_ = nullptr;
  Cmp cmp_{};
};

} // namespace algorithm
