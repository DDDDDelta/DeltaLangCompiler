//
// Created by bobzheng on 2023/12/30.
//
#pragma once

#include "tokentype.hpp"

#include <array>
#include <string>
#include <optional>

namespace deltac {

class KeywordTrie {
private:
    static constexpr int ALPHABET_SIZE = 26;

    struct TrieNode {
        std::array<TrieNode*, ALPHABET_SIZE> children;
        tok::Kind token = tok::ERROR;

        TrieNode() {
            for (auto& child : children) {
                child = nullptr;
            }
        }

        ~TrieNode() {
            for (auto* child : children) {
                delete child;
            }
        }
    };

public:
    KeywordTrie(std::initializer_list<tok::Kind>);

    std::optional<tok::Kind> tok_search(const char*& key) const;

private:
    static constexpr int _get_index(char c) {
        return c - 'a';
    }

private:
    TrieNode _root;
    KeywordTrie& _insert_tok(tok::Kind tkt);
};

}
