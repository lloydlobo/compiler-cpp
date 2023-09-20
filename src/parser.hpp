#pragma once

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <variant>
#include <vector>

#include "tokenization.hpp"

struct NodeExprIntLit {
    Token int_lit;
};

struct NodeExprIdent {
    Token ident;
};

struct NodeExpr {
    // Token int_lit;
    std::variant<NodeExprIntLit, NodeExprIdent> var;
};

struct NodeStmtExit {
    NodeExpr expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit, NodeStmtLet> var;
};

struct NodeProg {
    std::vector<NodeStmt> stmts;
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
            // return NodeExpr { .int_lit = consume() };
            return NodeExpr { .var = NodeExprIntLit { .int_lit = consume() } };
        }
        else if (peek().has_value() && peek().value().type == TokenType::ident) {
            return NodeExpr { .var = NodeExprIdent { .ident = consume() } };
        }
        else {
            return {};
        }
    }

    // TODO: Refactor to `parse_stmt`:
    std::optional<NodeStmt> parse_stmt()
    {
        if (peek().value().type == TokenType::exit && peek(1).has_value()
            && peek(1).value().type == TokenType::open_paren) {
            consume();
            consume(); // Also consume the open paranthesis.

            NodeStmtExit stmt_exit;

            if (auto node_expr = parse_expr()) {
                stmt_exit = NodeStmtExit { .expr = node_expr.value() };
            }
            else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::close_paren) {
                consume(); // TEMPNOTE: Avoid infinite loop.
            }
            else {
                std::cerr << "Expected `)` `close_paren`" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume(); // TEMPNOTE: Avoid infinite loop.
            }
            else {
                std::cerr << "Expected `;` `semi`" << std::endl;
                exit(EXIT_FAILURE);
            }

            return NodeStmt { .var = stmt_exit };
        }
        else if (peek().has_value() && peek().value().type == TokenType::let && peek(1).has_value()) {
            consume();
            NodeStmtLet stmt_let = NodeStmtLet { .ident = consume() };
            consume(); // Why consume this? `=`?
            if (auto expr = parse_expr()) {
                stmt_let.expr = expr.value();
            }
            else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume();
            }
            else {
                std::cerr << "Expected `;` `semi`" << std::endl;
                exit(EXIT_FAILURE);
            }

            return NodeStmt { .var = stmt_let };
        }
        else {
            return {};
        }
    }

    std::optional<NodeProg> parse_prog()
    {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            }
            else {
                std::cerr << "Invalid statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        } // m_index = 0;

        return prog;
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
            return m_tokens.at(m_index + offset);
        }
    }

    inline Token consume()
    {
        Token t = m_tokens.at(m_index);
        m_index++;

        return t;
    }
};
