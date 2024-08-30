#pragma once


#include <stdexcept>
#include <iostream>
#include "../utils/walk.h"
#include "../utils/visualization.h"

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
  using NodeTy = BSNode<KeyTy, ValueTy>;

  void insert(std::pair<KeyTy, ValueTy> elem) {
    root = insertImpl(elem, root);
  }

  void remove(KeyTy key) {
    root = removeImpl(key, root);
  }
  
  template<WalkOrder order>
  void walk(std::function<void(NodeTy*)> func) {
    walkImpl<order>(root, func);
  }

  void visualization() {
    algorithm::visualization(root);
  }

  NodeTy* find(KeyTy key) {
    return findImpl(key, root);
  }

  BSTree() = default;
  ~BSTree() {
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
      curr = new NodeTy{.key_ = elem.first, .value_ = elem.second};
      return curr;
    }
    if (cmp_(elem.first, curr->key_) > 0) {
      curr->left_ = insertImpl(elem, curr->left_);
    } else if (cmp_(elem.first, curr->key_) < 0) {
      curr->right_ = insertImpl(elem, curr->right_);
    } else {
      std::cout << "curr: " << curr->value_
                << " key: " << elem.first
                << " cmp: " << cmp_(elem.first, curr->key_)
                << std::endl;
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
      } else if (!right) {
        delete curr;
        return left;
      } else if (!right->left_) {
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
        prev1->left_ = curr->left_;
        prev1->right_ = right;
        delete curr;
        return prev1;
      }
    }
    return curr;
  }

  NodeTy* root = nullptr;
  Cmp cmp_{};
};

} // namespace algorithm
