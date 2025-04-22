#pragma once

#include <cassert>
#include <set>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

/*
有效信息挂在叶子节点 的set里面
*/

namespace algorithm {

template<typename T>
struct RadixTreeNode {
  explicit RadixTreeNode(
    const std::string& prefix = "",
    bool is_end = false): prefix_(prefix), is_end_(is_end), idx_(g_idx++) {}
  std::map<char, std::shared_ptr<RadixTreeNode>> children_;
  std::string prefix_;
  std::set<T> metas_;
  int idx_;
  bool is_end_;
  static int g_idx;
};

template<typename T>
int RadixTreeNode<T>::g_idx = 0;

/*
为了避免删除操作删除root节点，导致频繁调整root节点，将root 节点作为一个dummy节点
*/
template<typename T>
class RadixTree{
public:
  RadixTree():root_(std::make_shared<RadixTreeNode<T>>()) {}
  virtual ~RadixTree() = default;

  bool insert(const std::string& str, T meta) {
    if (str.empty()) {
      return false;
    }
    return insertImpl(str,  meta, root_);
  }

  std::shared_ptr<RadixTreeNode<T>> search(const std::string& str) {
    if (str.empty()) {
      return nullptr;
    }
    return searchImpl(str, root_);
  }

  bool remove(const std::string& str) {
    if (str.empty()) {
      return false;
    }
    return removeImpl(str, root_);
  }

  void visualization(std::string filename = "graph.dot") {
    std::ostringstream oss;
    int ident = 4;
    oss << "digraph demo {\n";
    visualizationImpl(oss, ident, root_);
    oss << "}\n";
    std::ofstream fw(filename);
    fw << oss.str();
  }

  bool isValid() {
    return isValidImpl(root_);
  }


private:
  bool insertImpl(
    const std::string& str,
      T meta,
      std::shared_ptr<RadixTreeNode<T>> node);
    
    std::shared_ptr<RadixTreeNode<T>> searchImpl(
      const std::string& str,
      std::shared_ptr<RadixTreeNode<T>> node);

  bool removeImpl(
    const std::string& str,
    std::shared_ptr<RadixTreeNode<T>> node);
  
  bool isValidImpl(std::shared_ptr<RadixTreeNode<T>> node);
  
  void visualizationImpl(std::ostringstream& oss, int ident,
    std::shared_ptr<RadixTreeNode<T>> node);

  std::shared_ptr<RadixTreeNode<T>> root_;

};

template<typename T>
bool RadixTree<T>::insertImpl(
    const std::string& str,
    T meta,
    std::shared_ptr<RadixTreeNode<T>> node) {
  // 因为采用 shared_ptr 可以不采用引用
  if (node->children_.count(str[0]) != 0) {
    auto child = node->children_.at(str[0]);
    int start  = 0;
    for (start = 0;
         start < child->prefix_.size() && start < str.size();
         start++) {
      if (str[start] != child->prefix_[start]) {
        break;
      }
    }
    if (start < child->prefix_.size()) {
      // split child
      auto new_node = std::make_shared<RadixTreeNode<T>>(
        child->prefix_.substr(start), child->is_end_);
      new_node->children_ = child->children_;
      new_node->metas_ = child->metas_;
      child->children_.clear();
      child->metas_.clear();
      child->is_end_ = false;
      child->prefix_ = child->prefix_.substr(0, start);
      auto ret = child->children_.insert({new_node->prefix_[0], new_node}).second;
      assert(ret);
      if (start == str.size()) {
        child->is_end_ = true;
        child->metas_.insert(meta);
      } else {
        auto rest = std::make_shared<RadixTreeNode<T>>(str.substr(start), true);
        rest->metas_.insert(meta);
        child->children_.insert({rest->prefix_[0], rest});
      }
      return true;
    } else {
      if (start == str.size()) {
        child->is_end_ = true;
        return child->metas_.insert(meta).second;
      } else {
        return insertImpl(str.substr(start), meta, child);
      }
    }
  } else {
    auto new_node = std::make_shared<RadixTreeNode<T>>(str, true);
    new_node->metas_.insert(meta);
    return node->children_.insert({new_node->prefix_[0], new_node}).second;
  }
}

template<typename T>
std::shared_ptr<RadixTreeNode<T>> RadixTree<T>::searchImpl(
    const std::string& str,
    std::shared_ptr<RadixTreeNode<T>> node) {
  if (node->children_.count(str[0]) > 0) {
    auto child = node->children_.at(str[0]);
    int start  = 0;
    for (start = 0; start < child->prefix_.size() && start < str.size(); start++) {
      if (str[start] != child->prefix_[start]) {
        break;
      }
    }
    if (start == child->prefix_.size()) {
      if (start == str.size()) {
        return child->is_end_? child: nullptr;
      } else {
        return searchImpl(str.substr(start), child);
      }
    } else {
      return nullptr;
    }
  }
  return nullptr;
}

template <typename T>
bool RadixTree<T>::removeImpl(
    const std::string& str,
    std::shared_ptr<RadixTreeNode<T>> node) {
  if (node->children_.count(str[0]) > 0) {
    auto child = node->children_.at(str[0]);
    int start  = 0;
    for (start = 0; start < child->prefix_.size() && start < str.size(); start++) {
      if (str[start] != child->prefix_[start]) {
        break;
      }
    }
    if (start == child->prefix_.size()) {
      if (start == str.size()) {
        if (child->is_end_) {
          child->is_end_ = false;
          child->metas_.clear();
          // 1、如果child 节点的子节点为空这直接删除child节点
          if (child->children_.empty()) {
            node->children_.erase(str[0]);
          }
          // 2、如果child节点的子节点为只有一个 者压缩child 子节点到child节点
          // 注意这部分判断修改了了child->children 不能和 1 互换 
          if (child->children_.size() == 1) {
            auto grandson = child->children_.begin()->second;
            child->prefix_.append(grandson->prefix_);
            child->children_ = grandson->children_;
            child->metas_ = grandson->metas_;
            child->is_end_ = grandson->is_end_;
          }
          // 3、如果该child被删除后 导致node节点只有一个子节点，者压缩child节点到子节点
          // 注意当前采用sub_node 节点而不采用child节点，因为可能child节点被删除了。
          if (node->children_.size() == 1 && !node->is_end_ && node != root_) {
            auto sub_node = node->children_.begin()->second;
            node->prefix_.append(sub_node->prefix_);
            node->is_end_ = sub_node->is_end_;
            node->metas_ = sub_node->metas_;
            node->children_ = sub_node->children_;
          }
          return true;
        }
        // 如果child 节点不是叶子节点 者返回false
        return false;
      } else {
        return removeImpl(str.substr(start), child);
      }
    }
  }
  return false;
}


template<typename T>
void RadixTree<T>::visualizationImpl(std::ostringstream& oss, int ident, std::shared_ptr<RadixTreeNode<T>> node) {
  for (auto child: node->children_) {
    std::string node_metas;
    std::string child_metas;
    for (auto meta : node->metas_) {
      node_metas += std::to_string(meta);
    }
    for (auto meta : child.second->metas_) {
      child_metas += std::to_string(meta);
    }
    oss << std::string(ident, ' ');
    oss << "\"" << node->prefix_ << "_" << (node->is_end_ ? "true": "false") << "_" << node->idx_ << "_" <<"_metas_" << node_metas << "\""
        << " -> "
        << "\""<< child.second->prefix_ << "_" << (child.second->is_end_ ? "true": "false") << "_" << child.second->idx_ << "_" << "_metas_" << child_metas << "\""
        << "\n";
    if (!child.second->children_.empty()) {
      visualizationImpl(oss, ident+4, child.second);
    }
  }
}

template<typename T>
bool RadixTree<T>::isValidImpl(std::shared_ptr<RadixTreeNode<T>> node) {
  if (node != root_ && node->prefix_.empty()) {
    std::cout << "node prefix is empty\n";
    return false;
  }
  if (node->children_.empty()) {
    return true;
  } else if (node->children_.size() == 1) {
    if (node != root_ && ! node->is_end_) {
      std::cout << "node is not end and children num is one\n";
      return false;
    }
  }
  if (!node->is_end_ && !node->metas_.empty()) {
    std::cout << "node is not end but not empty\n";
    return false;
  }
  if (node->is_end_ && node->metas_.empty()) {
    std::cout << "node is end but empty\n";
    return false;
  }
  std::set<char> first_prefix_set;
  for (auto child : node->children_) {
    if (first_prefix_set.count(child.second->prefix_[0]) > 0) {
      return false;
    }
    first_prefix_set.insert(child.second->prefix_[0]);
    if (!isValidImpl(child.second)) {
      return false;
    }
  }
  return true;
}




} // namespace algorithm