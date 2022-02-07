#ifndef SRC_REPL_H_
#define SRC_REPL_H_

#include <iostream>
#include <string>

#include "lexer/lexer.h"
#include "parser.h"

const std::string PROMPT = ">> ";

void Start(std::istream& in, std::ostream& out) {
  std::shared_ptr<Environment> env = std::make_shared<Environment>();
  while (true) {
    out << PROMPT;

    std::string line;
    getline(in, line);
    if (line.empty()) {
      return ;
    }

    Lexer lexer(line);
    // for (Token tok = lexer.NextToken(); tok.type != TokenType::kEOF; tok = lexer.NextToken()) {
    //   out << static_cast<int>(tok.type) << " " << tok.literal << std::endl;
    // }
    Parser parser(&lexer);
    std::shared_ptr<Program> program = parser.ParseProgram();
    if (!parser.Errors().empty()) {
      for (auto& str : parser.Errors()) {
        std::cout << str << std::endl;
      }
    }
    // std::cout << program->String() << std::endl;
    std::shared_ptr<Object> evalueted = program->Eval(env);
    if (evalueted != nullptr) {
      std::cout << evalueted->Inspect() << std::endl;
    }
  }

}

#endif  // SRC_REPL_H_