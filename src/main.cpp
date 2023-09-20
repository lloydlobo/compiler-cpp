#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

#include "generation.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Incorrect usage: Correct usage is..." << std::endl;
        std::cerr << "io <input.io>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize(); // tokenize(contents);

    // { std::cout << "DEBUG::START" << "\n";
    //     for (int i = 0; i < tokens.capacity() - 1; i++) { std::cout << ">>" << tokens.at(i).value.value_or("") <<
    //     "\n"; } std::cout << "DEBUG::END" << "\n"; } // dbg!

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value()) {
        std::cerr << "Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(prog.value());
    {
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog(); // tokens_to_asm(tokens);
    }

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}


/*
    std::string tokens_to_asm(const std::vector<Token>& tokens)
    {
        std::stringstream output;
        output << "global _start\n_start:\n";
        size_t len = tokens.size();

        for (int i = 0; i < len; i++) {
            const Token& token = tokens.at(i);
            if (token.type == TokenType::exit) {
                if (i + 1 < tokens.size() && tokens.at(i + 1).type == TokenType::int_lit) {
                    if (i + 2 < tokens.size() && tokens.at(i + 2).type == TokenType::semi) {
                        output << "    mov rax, 60\n";
                        output << "    mov rdi, " << tokens.at(i + 1).value.value() << "\n";
                        output << "    syscall";
                    }
                }
            }
        }

        return output.str();
    }
*/
