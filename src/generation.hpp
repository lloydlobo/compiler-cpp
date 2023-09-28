#pragma once

#include "parser.hpp" // NOTE: keep at top.

#include "sstream"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>

class Generator {
public:
    inline explicit Generator(NodeProg prog)
        : m_prog(std::move(prog))
    {
    }

    void gen_term(const NodeTerm* term)
    {
        struct TermVisitor {
            Generator* gen;
            void operator()(const NodeTermIntLit* term_int_lit) const
            {
                gen->m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << "\n";
                gen->push("rax");
            }
            /**
             * Offset(in bytes) stack pointer by 2, copy value, and push copied to top of stack
             * stack_loc,size,etc use 1 (as 64bits) and 8*8 // See 01:20:00 (Compiler Pt.3)
             */
            void operator()(const NodeTermIdent* term_ident) const
            {
                if (!gen->m_vars.contains(term_ident->ident.value.value())) {
                    std::cerr << "Undeclared identifier: " << term_ident->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                const auto& var = gen->m_vars.at(term_ident->ident.value.value());
                std::stringstream offset;
                /*
                 * 64bits - quad word 4 bytes in 32bits and 8 bytes in 64bits
                 * when second var is called instead of the prior. let x = 7; let y = 8; exit(y];
                 */
                auto BIT_MULTIPLIER = 8, OFF_BY_ONE = 1;
                /*
                 * Push what is 8bytes further from the stack top(y), by copying it(x) to the top
                 * Pop it(x_copy) into `rdi` which will then execute it in exit(x_copy)
                 */
                offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_loc - OFF_BY_ONE) * BIT_MULTIPLIER << "]\n";
                gen->push(offset.str());
            }
        };
        TermVisitor visitor({ .gen = this });
        std::visit(visitor, term->var);
    }

    void gen_bin_expr(const NodeBinExpr* bin_expr)
    {
        struct BinExprVisitor {
            Generator* gen;
            void operator()(const NodeBinExprAdd* add) const
            {
                // assert(false); // Not implemented
                gen->gen_expr(add->lhs);
                gen->gen_expr(add->rhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    add rax, rbx\n";
                gen->push("rax");
            }
        };
        BinExprVisitor visitor { .gen = this };
        std::visit(visitor, bin_expr->var);
    }

    void gen_expr(const NodeExpr* expr)
    {
        struct ExprVisitor {
            Generator* gen;
            void operator()(const NodeTerm* term) const
            {
                gen->gen_term(term);
            }
            void operator()(const NodeBinExpr* bin_term) const
            {
                gen->gen_bin_expr(bin_term);
            }
        };
        ExprVisitor visitor { .gen = this };
        std::visit(visitor, expr->var);
    }

    void gen_stmt(const NodeStmt* stmt)
    {
        struct StmtVisitor {
            Generator* gen;
            void operator()(const NodeStmtExit* stmt_exit) const
            {
                gen->gen_expr(stmt_exit->expr);
                gen->m_output << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }
            void operator()(const NodeStmtLet* stmt_let) const
            {
                if (gen->m_vars.contains(stmt_let->ident.value.value())) {
                    std::cerr << "Identifier already used: " << stmt_let->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.insert({ stmt_let->ident.value.value(), Var { .stack_loc = gen->m_stack_size } });
                gen->gen_expr(stmt_let->expr);
            }
        };

        StmtVisitor visitor({ .gen = this });
        std::visit(visitor, stmt->var);
    }

    [[nodiscard]] inline std::string gen_prog()
    {
        m_output << "global _start\n_start:\n";

        for (const NodeStmt* stmt : m_prog.stmts) {
            gen_stmt(stmt);
        }
        /*
         * If no explicit `exit()` exit with 0, else this is unreachable :)
         */
        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall\n";

        return m_output.str();
    }

private:
    struct Var {
        size_t stack_loc;
        // TODO: include types for static typing
    };

    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0; // Our own stack pointer at compile time to move around the entity offset of the
                             // penultimate item. copy that and add it to top of stack? See 01:01:30 (Compiler Pt.3)
                             // limited numbers of registers wants us to utilize the Stack
    std::unordered_map<std::string, Var> m_vars {}; // track variable's positions in stack

    void push(const std::string& reg)
    {
        m_output << "    push " << reg << "\n";
        m_stack_size++;
    }

    void pop(const std::string& reg)
    {
        m_output << "    pop " << reg << "\n";
        m_stack_size--;
    }
};

// 45:46 (Compiler Pt.3) >> Register (rdi) called `stack pointer` keeps track of the top stack item address
//      - automatically incr and decr when popped or pushed on to stack as a convenience
//
// [SO: push / pop instructions used on registers in x86 assembly?](https://stackoverflow.com/a/4584131)
//
// pushing a value (not necessarily stored in a register) means writing it to the stack.
// popping means restoring whatever is on top of the stack into a register.
//
// Those are basic instructions:
// ```asm
// push 0xdeadbeef      ; push a value to the stack
// pop eax              ; eax is now 0xdeadbeef
//
// ; swap contents of registers
// push eax
// mov eax, ebx
// pop ebx
// ```

/*
class Generator {
public:
    inline explicit Generator(NodeExit root)
        : m_root(std::move(root))
    {
    }

    [[nodiscard]] inline std::string generate() const
    {
        std::stringstream output;
        output << "global _start\n_start:\n";
        output << "    mov rax, 60\n";
        output << "    mov rdi, " << m_root.expr.int_lit.value.value() << "\n";
        output << "    syscall";

        return output.str();
    }

private:
    const NodeExit m_root;
};
*/
