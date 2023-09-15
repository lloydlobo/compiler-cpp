#pragma once

#include "parser.hpp" // NOTE: keep at top.
#include "sstream"

class Generator {
public:
    inline explicit Generator(NodeProg root)
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
    const NodeProg m_root;
};

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
