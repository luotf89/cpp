#pragma once


#include <stdexcept>
#include <sstream>
#include <fstream>
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

  void visualization(std::string filename = "graph.dot") {
    std::ostringstream oss;
    oss << "digraph demo {\n";
    std::size_t ident = 4;
    auto print_ident = [&](){
        oss << std::string(ident, ' ');
    };
    std::size_t nullptr_id = 0;
    walkImpl<WalkOrder::PREVORDER>(root_, [&](NodeTy* curr) {
        print_ident();
        oss << "\"key: " << curr->key_ << " value: " << curr->value_
            << " parent: " << (curr->parent_? std::to_string(curr->parent_->key_) : "nullptr")
            << "\"\n";
        if (curr->left_) {
            print_ident();
            oss << "\"key: " << curr->key_ << " value: " << curr->value_
                << " parent: " << (curr->parent_? std::to_string(curr->parent_->key_) : "nullptr")
                << "\"->";
            oss << "\"key: " << curr->left_->key_ << " value: " << curr->left_->value_
                << " parent: " <<curr->key_
                << "\"\n";
        } else {
            print_ident();
            oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
            print_ident();
            oss << "\"key: " << curr->key_ << " value: " << curr->value_
                << " parent: " << (curr->parent_? std::to_string(curr->parent_->key_) : "nullptr")
                << "\"->";
            oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
            nullptr_id++;
        }
        if (curr->right_) {
            print_ident();
            oss << "\"key: " << curr->key_ << " value: " << curr->value_ 
                << " parent: " << (curr->parent_? std::to_string(curr->parent_->key_) : "nullptr")
                << "\"->";
            oss << "\"key: " << curr->right_->key_ << " value: " << curr->right_->value_
                << " parent: " <<curr->key_
                << "\"\n";
        } else {
            print_ident();
            oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
            print_ident();
            oss << "\"key: " << curr->key_ << " value: " << curr->value_
                << " parent: " << (curr->parent_? std::to_string(curr->parent_->key_) : "nullptr")
                << "\"->";
            oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
            nullptr_id++;
        }
    });
    ident -= 4;
    print_ident();
    oss << "}\n";
    std::ofstream fw(filename);
    fw << oss.str();
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
