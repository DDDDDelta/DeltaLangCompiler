//
// Created by bobzheng on 2023/12/30.
//
#include "keywordtrie.hpp"
#include "charinfo.hpp"

#include <iostream>

KeywordTrie::KeywordTrie(std::initializer_list<TokenType> types) {
    for (TokenType t : types)
        _insert_tok(t);
}

KeywordTrie& KeywordTrie::_insert_tok(TokenType tkt) {
    TrieNode* node = &_root;
    for (char c : token_type_name(tkt)) {
        int index = _get_index(c);
        if (!node->children[index]) {
            node->children[index] = new TrieNode();
        }
        node = node->children[index];
    }
    node->token = tkt;
    return *this;
}

std::optional<TokenType> KeywordTrie::tok_search(const char*& key) const {
    const TrieNode* node = &_root;

    while (*key) {
        // reached the end of the token
        if (!is_letter(*key) && *key != '_') {
            break;
        }

        // definitely not a keyword
        if (!is_lowercase(*key)) {
            return std::nullopt;
        }

        int index = _get_index(*key);
        if (!node->children[index]) {
            node = node->children[index];
            break;
        }
        node = node->children[index];
        key++;
    }

    if (!node || node->token == TokenType::ERROR) {
        return std::nullopt;
    } else {
        return node->token;
    }
}
