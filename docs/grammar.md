# Grammer

Latex AST definitions for progressions.

---

$$
\begin{align}
    [\text{prog}] &\to [\text{stmt}]^* \\
    [\text{stmt}] &\to
    \begin{cases}
        \text{exit}([\text{expr}]); \\
        \text{let}\space\text{ident} = [\text{expr}];
    \end{cases} \\
    [\text{exit}] &\to
    \begin{cases}
        \text{int\_lit} \\
        \text{ident}
    \end{cases}
\end{align}
$$

---

- `^*` in `[\text{stmt}]^*` - can have more variations: $$[\text{stmt}]^*$$
