#pragma once

#include <cstdint>


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
requires Constraint<NodeTy>
NodeTy* findImpl(const KeyTy& key, NodeTy* curr, CmpFunc&& cmp_func) {
  if constexpr (requires { NodeTy::nil;}) {
    if (!curr || curr == NodeTy::nil) {
      return curr;
    }
  } else {
    if (!curr) {
      return curr;
    }
  }
  if (cmp_func(key, curr->key_) > 0) {
    return findImpl(key, curr->left_, cmp_func);
  } else if (cmp_func(key, curr->key_) < 0) {
    return findImpl(key, curr->right_, cmp_func);
  } else {
    return curr;
  }
}

template<typename NodeTy>
requires Constraint<NodeTy> && requires(NodeTy node) {
  node.parent_;
}
NodeTy* predecessorImpl(NodeTy* curr) {
  if (!curr) {
    return nullptr;
  }
  if (curr->left_) {
    curr = curr->left_;
    while(curr->right_) {
      curr = curr->right_;
    }
    return curr;
  } else {
    auto parent = curr->parent_;
    while(parent && curr == parent->left_) {
      curr = parent;
      parent = parent->parent_;
    }
    return parent;
  }
}

template<typename NodeTy>
requires Constraint<NodeTy> && requires(NodeTy node) {
  node.parent_;
}
NodeTy* successorImpl(NodeTy* curr) {
  if (!curr) {
    return nullptr;
  }
  if (curr->right_) {
    curr = curr->right_;
    while(curr->left_) {
      curr = curr->left_;
    }
    return curr;
  } else {
    auto parent = curr->parent_;
    while(parent && curr == parent->right_) {
      curr = parent;
      parent = parent->parent_;
    }
    return parent;
  }
}

template <WalkOrder order, typename NodeTy>
requires Constraint<NodeTy>
void walkImpl(NodeTy* curr, auto func) {
  if constexpr (requires { NodeTy::nil;}) {
    if (!curr || curr == NodeTy::nil) {
      return;
    } 
  } else {  
    if (!curr) {
      return;
    }
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

} // namespace algorithm
