/*
实现参考 https://blog.csdn.net/code_peak/article/details/120643910
*/

#pragma once

#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>
#include <vector>
#include "../utils/utils.h"

namespace algorithm {

enum class Color:int {
    RED = 0,
    BLACK
};

template<typename KeyTy, typename ValueTy>
struct RBNode {
    KeyTy key_{};
    ValueTy value_{};
    Color color_ = Color::RED;
    RBNode* left_ = nil;
    RBNode* right_ = nil;
    RBNode* parent_ = nil;

    inline static RBNode* nil = [](){
        static RBNode nil_obj{
            .key_ = KeyTy{},
            .value_ = ValueTy{},
            .color_ = Color::BLACK,
            .left_ = nullptr,
            .right_ = nullptr,
            .parent_ = nullptr
        };
        return &nil_obj;
    }();
};


template<typename KeyTy, typename ValueTy, typename Cmp = CMP<KeyTy>>
class RBTree {
public:
    using KeyType = KeyTy;
    using ValueType = ValueTy;
    using NodeTy = RBNode<KeyTy, ValueTy>;

    RBTree() = default;

    std::pair<bool, NodeTy*> insert(std::pair<KeyTy, ValueTy> elem) {
        NodeTy* curr = root_;
        NodeTy* prev = root_;
        while(curr != NodeTy::nil) {
            if (cmp_(elem.first, curr->key_) > 0) {
                prev = curr;
                curr = curr->left_;
            } else if (cmp_(elem.first, curr->key_) < 0) {
                prev = curr;
                curr = curr->right_;
            } else [[unlikely]]{
                return {false, curr};
            }
        }
        curr = new NodeTy{.key_= elem.first, .value_= elem.second,
                          .color_ = Color::RED, .parent_ = prev};
        if (prev == NodeTy::nil) {
            root_ = curr;
            root_->color_ = Color::BLACK;
        } else {
            if (cmp_(elem.first, prev->key_) > 0) {
                prev->left_ = curr;
            } else {
                prev->right_ = curr;
            }
        }
        auto ret = curr;
        fixAfterInsert(curr);
        return {true, ret};
    }

    bool remove(KeyTy key) {
        auto curr = findImpl(key, root_, cmp_);
        if (curr != NodeTy::nil) {
            removeImpl(curr);
            delete curr;
            return true;
        }
        return false;
    }

    bool checkValid() {
        if (root_->color_ != Color::BLACK) {
            std::cout << "root is not black" << std::endl;
            return false;
        }
        if (checkColorValid(root_) == false) {
            std::cout << "color is not valid" << std::endl;
            return false;
        }
        if (checkBlackHeightValid(root_) == false) {
            std::cout << "black height is not valid" << std::endl;
            return false;
        }
        if (checkDataValid() == false) {
            std::cout << "data is not valid" << std::endl;
            return false;
        }
        return true;
        // return root_->color_ == Color::BLACK &&
        //        checkColorValid(root_)&&
        //        checkBlackHeightValid(root_) &&
        //        checkDataValid();
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
                << " color: " << (curr->color_ == Color::RED ? "red" : "black")
                << " parent: " << (curr->parent_? std::to_string(curr->parent_->key_) : "nullptr")
                << "\"\n";
            if (curr->left_) {
                print_ident();
                oss << "\"key: " << curr->key_ << " value: " << curr->value_
                    << " color: " << (curr->color_ == Color::RED ? "red" : "black")
                    << " parent: " << (curr->parent_? std::to_string(curr->parent_->key_) : "nullptr")
                    << "\"->";
                oss << "\"key: " << curr->left_->key_ << " value: " << curr->left_->value_
                    << " color: " << (curr->left_->color_ == Color::RED ? "red" : "black")
                    << " parent: " <<curr->key_
                    << "\"\n";
            } else {
                print_ident();
                oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
                print_ident();
                oss << "\"key: " << curr->key_ << " value: " << curr->value_
                    << " color: " << (curr->color_ == Color::RED ? "red" : "black")
                    << " parent: " << (curr->parent_? std::to_string(curr->parent_->key_) : "nullptr")
                    << "\"->";
                oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
                nullptr_id++;
            }
            if (curr->right_) {
                print_ident();
                oss << "\"key: " << curr->key_ << " value: " << curr->value_
                    << " color: " << (curr->color_ == Color::RED ? "red" : "black")
                    << " parent: " << (curr->parent_? std::to_string(curr->parent_->key_) : "nullptr")
                    << "\"->";
                oss << "\"key: " << curr->right_->key_ << " value: " << curr->right_->value_
                    << " color: " << (curr->right_->color_ == Color::RED ? "red" : "black")
                    << " parent: " <<curr->key_
                    << "\"\n";
            } else {
                print_ident();
                oss << "\"nullptr:" << nullptr_id << "\"[style=invis]\n";
                print_ident();
                oss << "\"key: " << curr->key_ << " value: " << curr->value_
                    << " color: " << (curr->color_ == Color::RED ? "red" : "black")
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

private:
    bool checkDataValid() {
        std::vector<KeyTy> keys;
        walkImpl<WalkOrder::INORDER>(root_, [&](NodeTy* curr) {
            keys.push_back(curr->key_);
        });
        if (keys.size() < 2) {
            return true;
        }
        for (int i = 1; i < keys.size(); i++) {
            if (cmp_(keys[i-1], keys[i]) <= 0) {
                return false;
            }
        }
        return true;
    }

    bool checkColorValid(NodeTy* curr) {
        if (curr == NodeTy::nil) {
            return true;
        }
        if (curr->color_ == Color::RED) {
            if (curr->left_->color_ == Color::RED) {
                return false;
            }
            if (curr->right_->color_ == Color::RED) {
                return false;
            }
        } 
        return checkColorValid(curr->left_) &&
               checkColorValid(curr->right_);
    }

    int blackHeight(NodeTy* curr) {
        if (curr == NodeTy::nil) {
            return 0;
        }
        int left_height = blackHeight(curr->left_);
        int right_height = blackHeight(curr->right_);
        int max_height = std::max(left_height, right_height);
        return curr->color_ == Color::BLACK? max_height + 1 : max_height;
    }

    bool checkBlackHeightValid(NodeTy* curr) {
        if (curr == NodeTy::nil) {
            return true;
        }
        int left_height = blackHeight(curr->left_);
        int right_height = blackHeight(curr->right_);
        if (left_height != right_height) {
            return false;
        }
        return checkBlackHeightValid(curr->left_) &&
               checkBlackHeightValid(curr->right_);
    }



    void transplant(NodeTy* u, NodeTy* v) {
        if (u->parent_ == NodeTy::nil) //u的父节点为空
        {
            root_ = v; //直接令根root为v
        }
        else if (u == u->parent_->left_) //u父节点不为空，且u在左子树
        {
            u->parent_->left_ = v;
        }
        else //u在右子树
        {
            u->parent_->right_ = v;
        }
        v->parent_ = u->parent_;
    }

    NodeTy* minimum(NodeTy* curr) {
        NodeTy* prev = curr;
        while (curr != NodeTy::nil) {
            prev = curr;
            curr = curr->left_;
        }
        return prev;
    }

    void removeImpl(NodeTy* curr) {
        NodeTy *x = NodeTy::nil;
        NodeTy *y = curr;    //y记住传进来的z结点
        Color ycolor = y->color_; //
        if (curr->left_ == NodeTy::nil) {
            x = curr->right_;
            transplant(curr, curr->right_);
        } else if (curr->right_ == NodeTy::nil) {
            x = curr->left_;
            transplant(curr, curr->left_);
        } else {
            y = minimum(curr->right_); //y是z右子树的的最左子树
            ycolor = y->color_;
            x = y->right_;
            if (y->parent_ == curr) //z的右子结点没有左节点或为Nil
            {
                x->parent_ = y;
            } else { //z的右子结点有左节点或为Nil
                transplant(y, y->right_);
                y->right_ = curr->right_;
                y->right_->parent_ = y;
            }
            transplant(curr, y);
            //改变指向
            y->left_ = curr->left_;
            y->left_->parent_ = y;
            y->color_ = curr->color_;
        }
        if (ycolor == Color::BLACK) {
            fixAfterRemove(x);
        }
    }

    /* 右转，对z结点进行右转
    *       |                 |
    *       y                 x
    *      / \    ===>       / \
    *     x   r             a    y  
    *    / \                    / \
    *   a   b                  b   r
    */
    void rightRotary(NodeTy* y) {
        auto  x = y->left_;
        y->left_ = x->right_;
        if (x->right_ != NodeTy::nil) {
            x->right_->parent_ = y;
        }
        x->parent_ = y->parent_;
        if (y->parent_ == NodeTy::nil) {
            root_ = x;
        } else if (y->parent_->left_ == y) {
            y->parent_->left_ = x;
        } else {
            y->parent_->right_ = x;
        }
        x->right_ = y;
        y->parent_ = x;
    }

    /* 左转，对z结点左转
     *     |                   |
     *     x                   y
     *    / \      ===>       / \
     *   a   y               x   r
     *      / \             / \
     *     b   r           a   b  
     */
    void leftRotary(NodeTy* x) {
        auto y = x->right_;
        x->right_ = y->left_;
        if (y->left_ != NodeTy::nil) {
            y->left_->parent_ = x;
        }
        y->parent_ = x->parent_;
        if (x->parent_ == NodeTy::nil) {
            root_ = y;
        } else if (x->parent_->left_ == x) {
            x->parent_->left_ = y;
        } else {
            x->parent_->right_ = y;
        }
        y->left_ = x;
        x->parent_ = y;
    }

    void fixAfterInsert(NodeTy* curr) {
        while(curr->parent_->color_ == Color::RED) {
            // 因为根节点为黑色，当前父节点为红色，所以一定有爷爷节点
            if (curr->parent_ == curr->parent_->parent_->left_) {
                NodeTy* uncle = curr->parent_->parent_->right_;
                if (uncle->color_ == Color::RED) {
                    curr->parent_->color_ = Color::BLACK;
                    uncle->color_ = Color::BLACK;
                    curr->parent_->parent_->color_ = Color::RED;
                    curr = curr->parent_->parent_;
                } else {
                    if (curr == curr->parent_->right_) {
                        curr = curr->parent_;
                        leftRotary(curr);
                    }
                    curr->parent_->color_ = Color::BLACK;
                    curr->parent_->parent_->color_ = Color::RED;
                    rightRotary(curr->parent_->parent_);
                }
            } else {
                NodeTy* uncle = curr->parent_->parent_->left_;
                if (uncle->color_ == Color::RED) {
                    curr->parent_->color_ = Color::BLACK;
                    uncle->color_ = Color::BLACK;
                    curr->parent_->parent_->color_ = Color::RED;
                    curr = curr->parent_->parent_;
                } else {
                    if (curr == curr->parent_->left_) {
                        curr = curr->parent_;
                        rightRotary(curr);
                    }
                    curr->parent_->color_ = Color::BLACK;
                    curr->parent_->parent_->color_ = Color::RED;
                    leftRotary(curr->parent_->parent_);
                }
            }
        }
        root_->color_ = Color::BLACK;
    }

    void fixAfterRemove(NodeTy* x) {
        while(x != root_ && x->color_ == Color::BLACK) {
            if (x == x->parent_->left_) {
                NodeTy* w = x->parent_->right_;
                if (w->color_ == Color::RED) {
                    w->color_ = Color::BLACK;
                    x->parent_->color_ = Color::RED;
                    leftRotary(x->parent_);
                    w = x->parent_->right_;
                }
                if (w->left_->color_ == Color::BLACK &&
                    w->right_->color_ == Color::BLACK) {
                    w->color_ = Color::RED;
                    x = x->parent_;
                } else {
                    if (w->right_->color_ == Color::BLACK) {
                        w->left_->color_ = Color::BLACK;
                        w->color_ = Color::RED;
                        rightRotary(w);
                        w = x->parent_->right_;
                    }
                    w->color_ = x->parent_->color_;
                    x->parent_->color_ = Color::BLACK;
                    w->right_->color_ = Color::BLACK;
                    leftRotary(x->parent_);
                    x = root_; //结束循环
                }
            } else {
                NodeTy* w = x->parent_->left_;
                if (w->color_ == Color::RED) {
                    w->color_ = Color::BLACK;
                    x->parent_->color_ = Color::RED;
                    rightRotary(x->parent_);
                    w = x->parent_->left_;
                }
                if (w->left_->color_ == Color::BLACK &&
                    w->right_->color_ == Color::BLACK) {
                    w->color_ = Color::RED;
                    x = x->parent_;
                } else {
                    if (w->left_->color_ == Color::BLACK) {
                        w->right_->color_ = Color::BLACK;
                        w->color_ = Color::RED;
                        leftRotary(w);
                        w = x->parent_->left_;
                    }
                    //情况4
                    w->color_ = x->parent_->color_;
                    x->parent_->color_ = Color::BLACK;
                    w->left_->color_ = Color::BLACK;
                    rightRotary(x->parent_);
                    x = root_; //结束循环
                }
            }
        }
        x->color_ = Color::BLACK;
    }

    NodeTy* root_ = NodeTy::nil;
    Cmp cmp_;
};

} // namespace algorithm
