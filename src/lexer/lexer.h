#ifndef SRC_LEXER_LEXER_H_
#define SRC_LEXER_LEXER_H_

#include <string>

#include "token.h"

class Lexer {
 public:
  Lexer(const std::string& input) : input_(input), readPosition_(0) {
    readChar();
  }

  Token NextToken() {
    Token tok;
    skipWhiteSpaces();
    auto cm_iter = CharTokenTypeMap.find(ch_);
    if (cm_iter != CharTokenTypeMap.end()) {
      tok.type = cm_iter->second;
      tok.literal.resize(1);
      tok.literal[0] = ch_;
      if (tok.type == TokenType::kBang) {
        if (peekChar() == '=') {
          readChar();
          tok.type = TokenType::kNEQ;
          tok.literal.resize(2);
          tok.literal[1] = '=';
        }
      } else if (tok.type == TokenType::kAssign) {
        if (peekChar() == '=') {
          readChar();
          tok.type = TokenType::kEQ;
          tok.literal.resize(2);
          tok.literal[1] = '=';
        }
      }
      readChar();
    } else if (isLetter(ch_)) {
      tok.literal = readIdentifier();
      auto iter = TokenTypeMap.find(tok.literal);
      if (iter != TokenTypeMap.end()) {
        tok.type = iter->second;
      } else {
        tok.type = TokenType::kIdent;
      }
    } else if (isDigit(ch_)) {
      tok.type = TokenType::kInt;
      tok.literal = readNumber();
    } else if (ch_ == '"') {
      tok.type = TokenType::kString;
      tok.literal = readString();
      if (ch_ == '"') {
        readChar();
      }
    } else if (ch_ == 0) {
      tok.type = TokenType::kEOF;
    } else {
      tok.type = TokenType::kIllegal;
      tok.literal = std::string(ch_, 1);
    }
    return tok;
  }

 private:
  void readChar() {
    if (readPosition_ >= input_.length()) {
      ch_ = 0;
    } else {
      ch_ = input_[readPosition_];
    }
    position_ = readPosition_;
    readPosition_ += 1;
  }

  char peekChar() {
    if (readPosition_ >= input_.length()) {
      return 0;
    } else {
      return input_[readPosition_];
    }
  }

  bool isLetter(char ch) {
    return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_';
  }

  bool isDigit(char ch) {
    return '0' <= ch && ch <= '9';
  }

  std::string readIdentifier() {
    int begin = position_;
    while (isLetter(ch_)) {
      readChar();
    }
    return input_.substr(begin, position_ - begin);
  }

  std::string readNumber() {
    int begin = position_;
    while (isDigit(ch_)) {
      readChar();
    }
    return input_.substr(begin, position_ - begin);
  }

  std::string readString() {
    int begin = position_ + 1;
    while (true) {
      readChar();
      if (ch_ == '"' || ch_ == 0) {
        break;
      }
    }
    return input_.substr(begin, position_ - begin);
  }

  void skipWhiteSpaces() {
    while (ch_ == ' ' || ch_ == '\t' || ch_ == '\n' || ch_ == '\r') {
      readChar();
    }
  }

  std::string input_;
  int position_;
  int readPosition_;
  char ch_;
};

#endif  // SRC_LEXER_LEXER_H_