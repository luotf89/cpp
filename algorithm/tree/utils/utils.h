#pragma once

#include <cstddef>
#include <cstdint>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>

namespace algorithm {


template <typename T>
struct CMP {
  int operator()(T a, T b) {
    if (a < b) {
      return 1;
    } else if (a > b) {
      return -1;
    } else {
      return 0;
    }
  }
};

template <typename NodeTy>
concept Constraint = requires(NodeTy node) {
  node.left_;
  node.right_;
};

enum class WalkOrder : int32_t {
  PREVORDER = 0,
  INORDER = 1,
  POSTORDER = 2
};

template<typename KeyTy, typename NodeTy, typename CmpFunc>
NodeTy* findImpl(const KeyTy& key, NodeTy* curr, CmpFunc&& cmp_func) {
  if (!curr) {
    return curr;
  }
  if (cmp_func(key, curr->key_) > 0) {
    return findImpl(key, curr->left_, cmp_func);
  } else if (cmp_func(key, curr->key_) < 0) {
    return findImpl(key, curr->right_, cmp_func);
  } else {
    return curr;
  }
}

template <WalkOrder order, typename NodeTy>
requires Constraint<NodeTy>
void walkImpl(NodeTy* curr, auto func) {
  if (!curr) {
    return;
  }
  if (order == WalkOrder::PREVORDER) {
    func(curr);
  }
  if (curr->left_) {
    walkImpl<order>(curr->left_, func);
  }
  if (order == WalkOrder::INORDER) {
    func(curr);
  }
  if (curr->right_) {
    walkImpl<order>(curr->right_, func);
  }
  if (order == WalkOrder::POSTORDER) {
    func(curr);
  }
}

template <typename NodeTy>
requires Constraint<NodeTy>
int height(NodeTy* curr) {
  if (!curr) {
    return 0;
  }
  return std::max(height(curr->left_), height(curr->right_)) + 1;
}

template<typename NodeTy>
requires Constraint<NodeTy> || requires (NodeTy t) {
    t.key_;
    t.value_;
}
void visualization(NodeTy* curr) {
    std::ostringstream oss;
    oss << "digraph demo {\n";
    std::size_t ident = 4;
    auto print_ident = [&](){
        oss << std::string(ident, ' ');
    };
    std::size_t nullptr_id = 0;
    // std::function<void(NodeTy*)> func = [&](NodeTy* curr) {
    //     print_ident();
    //     oss << "\"key: " << curr->key_ << " value: " << curr->value_ << "\"\n";
    //     if (curr->left_) {
    //         print_ident();
    //         oss << "\"key: " << curr->key_ << " value: " << curr->value_ << "\"->";
    //         oss << "\"key: " << curr->left_->key_ << " value: " << curr->left_->value_ << "\"\n";
    //     } else {
    //         print_ident();
    //         oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
    //         print_ident();
    //         oss << "\"key: " << curr->key_ << " value: " << curr->value_ << "\"->";
    //         oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
    //         nullptr_id++;
    //     }
    //     if (curr->right_) {
    //         print_ident();
    //         oss << "\"key: " << curr->key_ << " value: " << curr->value_ << "\"->";
    //         oss << "\"key: " << curr->right_->key_ << " value: " << curr->right_->value_ << "\"\n";
    //     } else {
    //         print_ident();
    //         oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
    //         print_ident();
    //         oss << "\"key: " << curr->key_ << " value: " << curr->value_ << "\"->";
    //         oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
    //         nullptr_id++;
    //     }
    // };
    // walkImpl<WalkOrder::PREVORDER>(curr, func);
    walkImpl<WalkOrder::PREVORDER>(curr, [&](NodeTy* curr) {
        print_ident();
        oss << "\"key: " << curr->key_ << " value: " << curr->value_ << "\"\n";
        if (curr->left_) {
            print_ident();
            oss << "\"key: " << curr->key_ << " value: " << curr->value_ << "\"->";
            oss << "\"key: " << curr->left_->key_ << " value: " << curr->left_->value_ << "\"\n";
        } else {
            print_ident();
            oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
            print_ident();
            oss << "\"key: " << curr->key_ << " value: " << curr->value_ << "\"->";
            oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
            nullptr_id++;
        }
        if (curr->right_) {
            print_ident();
            oss << "\"key: " << curr->key_ << " value: " << curr->value_ << "\"->";
            oss << "\"key: " << curr->right_->key_ << " value: " << curr->right_->value_ << "\"\n";
        } else {
            print_ident();
            oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
            print_ident();
            oss << "\"key: " << curr->key_ << " value: " << curr->value_ << "\"->";
            oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
            nullptr_id++;
        }
    });
    ident -= 4;
    print_ident();
    oss << "}\n";
    std::ofstream fw("./graph.dot");
    fw << oss.str();
}

} // namespace algorithm
