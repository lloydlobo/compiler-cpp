#pragma once

#include "parser.hpp" // NOTE: keep at top.
#include "sstream"
#include <cassert>
#include <cstddef>
#include <sstream>
#include <unordered_map>
#include <variant>

class Generator {
public:
    inline explicit Generator(NodeProg prog)
        : m_prog(std::move(prog))
    {
    }

    void gen_expr(const NodeExpr& expr)
    {
        struct ExprVisitor {
            Generator* gen;
            void operator()(const NodeExprIntLit& term) const
            {
                gen->m_output << "    mov rax, " << term.int_lit.value.value() << "\n";
                gen->m_output << "    push rax\n";
            }
            void operator()(const NodeExprIdent& ident)
            {
                // TODO
                // gen->m_output << "mov rax, " << ident.ident.value.value() << "\n";
                // gen->m_output << "push rax \n";
            }
        };
        ExprVisitor visitor { .gen = this };
        std::visit(visitor, expr.var);
    }

    /* NOTETOME: vim `ysiw"` to surround word with `"` */

    // "const" NodeStmt prevents copying around
    void gen_stmt(const NodeStmt& stmt)
    {
        struct StmtVisitor {
            Generator* gen;
            void operator()(const NodeStmtExit& stmt_exit)
            {
                gen->gen_expr(stmt_exit.expr);
                gen->m_output << "    mov rax, 60\n";
                gen->m_output << "    pop rdi\n"; // expr can be two things as well...
                gen->m_output << "    syscall\n";
            }
            void operator()(const NodeStmtLet& stmt_let)
            {
            }
        };
        StmtVisitor visitor({ .gen = this });
        std::visit(visitor, stmt.var);
    }

    [[nodiscard]] inline std::string gen_prog()
    {
        m_output << "global _start\n_start:\n";

        for (const NodeStmt stmt : m_prog.stmts) {
            gen_stmt(stmt);
        }
        // if no explicit `exit()` exit with 0, else this is unreachable :)
        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall\n";
        return m_output.str();
    }

private:
    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0; // limited numbers of registers wants us to utilize the Stack
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
