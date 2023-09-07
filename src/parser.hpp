#pragma once

#include <cassert>
#include <cstdlib>
#include <optional>
#include <ostream>
#include <variant>

#include "tokenization.hpp"

struct NodeExpr {
    Token int_lit;
};

struct NodeExit {
    NodeExpr expr;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens))
    {
    }

    std::optional<NodeExpr> parse_expr()
    {
        if (peek().has_value() && peek().value().type == TokenType::int_lit) {
            return NodeExpr { .int_lit = consume() };
        }
        else {
            return {};
        }
    }

    std::optional<NodeExit> parse()
    {
        std::optional<NodeExit> exit_node;

        while (peek().has_value()) {
            if (peek().value().type == TokenType::exit) {
                consume();
                // TEMPNOTE: Implicitly assign to node_expr as boolean if there is a value. else err.
                if (auto node_expr = parse_expr()) {
                    exit_node = NodeExit { .expr = node_expr.value() };
                }
                else {
                    std::cout << "Invalid expression" << std::endl;
                    exit(EXIT_FAILURE);
                }

                if (peek().has_value() && peek().value().type == TokenType::semi) {
                    consume(); // TEMPNOTE: Avoid infinite loop.
                }
                else {
                    std::cout << "Invalid expression `semi`" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }

        return exit_node;
    }

private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;

    [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const
    {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        else {
            return m_tokens.at(m_index);
        }
    }

    inline Token consume()
    {
        return m_tokens.at(m_index++);
    }
};
