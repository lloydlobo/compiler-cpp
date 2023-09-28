# Grammer

Latex AST definitions for progressions.

---

$$
\begin{align}
    [\text{Prog}] &\to [\text{Stmt}]^* \\
    [\text{Stmt}] &\to
    \begin{cases}
        \text{exit}([\text{Expr}]); \\
        \text{let}\space\text{ident} = [\text{Expr}];
    \end{cases} \\
    [\text{exit}] &\to
    \begin{cases}
        \text{int\_lit} \\
        \text{ident}
    \end{cases}
    [\text{BinExpr}] \to
    \begin{cases}
        [\text{Expr}] * [\text{Expr}] & \text{prec} = 1 \\
        [\text{Expr}] + [\text{Expr}] & \text{prec} = 0 \\
    \end{cases}
\end{align}
$$

---

- `^*` in `[\text{Stmt}]^*` - can have more variations: $$[\text{Stmt}]^*$$
