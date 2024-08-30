#pragma once

#include <cstddef>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include "walk.h"

namespace algorithm {

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