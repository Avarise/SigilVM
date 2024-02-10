#pragma once
/*
    Header for general parser
*/

#include <string>
#include <vector>

namespace sigil {
    namespace parser {
        struct decision_tree_node_t {
            const std::string key;
            bool variable_args;
            int (*handle)(void);
            std::vector<decision_tree_node_t> children;
        };
    }
}