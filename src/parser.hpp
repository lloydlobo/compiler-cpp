#pragma once

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <variant>
#include <vector>

#include "arena.hpp"
#include "tokenization.hpp"

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr; // Forward declare NodeExpr
                 // use pointer to satisfy incomplete type error tldr; we need to know size of NodeExpr to avoid
                 // circular definitions so we need to dynamically allocate the data and store it somewhere, but they
                 // are small so alloc and free will be expensive as they have small memory footprint and worsens CPU
                 // cache causing cache miss. so we need a contiguos memory with Area Allocator (loan of memory e.g.
                 // 1mb). Then we just advance a pointer by 1 unit or the pointer/elements size. so all elements are
                 // next to each other. |1|  3|    4| 2|1|1|. No prob with lifetimes, and can use raw pointers. Then
                 // free all at once after generation.

struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

// struct NodeBinExprMulti {
//     NodeExpr* lhs;
//     NodeExpr* rhs;
// };
//
// struct NodeBinExprSub {
//     NodeExpr* lhs;
//     NodeExpr* rhs;
// };
//
// struct NodeBinExprDiv {
//     NodeExpr* lhs;
//     NodeExpr* rhs;
// };

struct NodeBinExpr {
    // std::variant<NodeBinExprAdd*, NodeBinExprMulti*> var;
    std::variant<NodeBinExprAdd*> var;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*> var;
};

struct NodeExpr {
    // Token int_lit;
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmtLet {
    Token ident; // no need for it as we moved tokens in Parser?
    NodeExpr* expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit*, NodeStmtLet*> var;
};

struct NodeProg {
    std::vector<NodeStmt*> stmts;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens))
        , m_allocator(1024 * 1024 * 4) // 4 mb
    {
    }

    std::optional<NodeTerm*> parse_term()
    {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        }
        else if (peek().has_value() && peek().value().type == TokenType::ident) {
            auto term_ident = m_allocator.alloc<NodeTermIdent>();
            term_ident->ident = consume();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        }
        else {
            return {};
        }
    }

    std::optional<NodeExpr*> parse_expr()
    {
        if (auto term = parse_term()) {
            if (try_consume(TokenType::plus).has_value()) {
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();
                auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();
                auto lhs_expr = m_allocator.alloc<NodeExpr>();
                lhs_expr->var = term.value(); // (1)
                bin_expr_add->lhs = lhs_expr;
                /* recursion */
                if (auto rhs = parse_expr()) {
                    bin_expr_add->rhs = rhs.value();
                    bin_expr->var = bin_expr_add;
                    auto expr = m_allocator.alloc<NodeExpr>();
                    expr->var = bin_expr;
                    return expr;
                }
                else {
                    std::cerr << "Expected expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            else {
                auto expr = m_allocator.alloc<NodeExpr>();
                expr->var = term.value(); // (2)
                return expr;
            }
        }
        else {
            return {};
        }
    }

    // TODO: Refactor to `parse_stmt`:
    std::optional<NodeStmt*> parse_stmt()
    {
        if (peek().value().type == TokenType::exit && peek(1).has_value()
            && peek(1).value().type == TokenType::open_paren) {
            consume();
            consume(); // also consume the open paranthesis.
            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
            if (auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            }
            else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "Expected `)` `semi`");
            try_consume(TokenType::semi, "Expected `;` `semi`");
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        }
        else if (
            peek().has_value() && peek().value().type == TokenType::let && peek(1).has_value()
            && peek(1).value().type == TokenType::ident && peek(2).has_value()
            && peek(2).value().type == TokenType::eq) {
            consume();
            auto stmt_let = m_allocator.alloc<NodeStmtLet>();
            stmt_let->ident = consume();
            consume(); // also consume `=`?
            if (auto expr = parse_expr()) {
                stmt_let->expr = expr.value();
            }
            else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semi, "Expected `;` `semi`");
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_let;
            return stmt;
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
        }
        return prog;
    }

private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_allocator;

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
        return m_tokens.at(m_index++);
    }

    inline std::optional<Token> try_consume(TokenType type, const std::string& err_msg)
    {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        else {
            std::cerr << err_msg << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    inline std::optional<Token> try_consume(TokenType type)
    {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        else {
            return {};
        }
    }
};
