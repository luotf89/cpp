#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

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

} // namespace algorithm
