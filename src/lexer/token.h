#ifndef SRC_LEXER_TOKEN_H_
#define SRC_LEXER_TOKEN_H_

#include <string>
#include <map>
#include <set>

enum class TokenType {
  kIllegal,
  kEOF,
  kIdent,
  kInt,
  kAssign,
  kPlus,
  kMinus,
  kBang,
  kSlash,
  kAsterisk,
  kLT,
  kGT,
  kComma,
  kSemicolon,
  kLParen,
  kRParen,
  kLBrace,
  kRBrace,
  kFunction,
  kLet,
  kTrue,
  kFalse,
  kIf,
  kElse,
  kReturn,
  kEQ,
  kNEQ,
  kString,
};

inline std::string TokenTypeToName(TokenType type) {
  switch (type) {
  case TokenType::kEOF:
    return "EOF";
  case TokenType::kIdent:
    return "Identifier";
  case TokenType::kInt:
    return "Integer";
  case TokenType::kAssign:
    return "Assign";
  case TokenType::kPlus:
    return "Plus";
  case TokenType::kMinus:
    return "Minus";
  case TokenType::kBang:
    return "Bang";
  case TokenType::kSlash:
    return "Slash";
  case TokenType::kAsterisk:
    return "Asterisk";
  case TokenType::kLT:
    return "LT";
  case TokenType::kGT:
    return "GT";
  case TokenType::kComma:
    return "Comma";
  case TokenType::kSemicolon:
    return "Semicolon";
  case TokenType::kLParen:
    return "LParen";
  case TokenType::kRParen:
    return "RParen";
  case TokenType::kLBrace:
    return "LBrace";
  case TokenType::kRBrace:
    return "RBrace";
  case TokenType::kFunction:
    return "Function";
  case TokenType::kLet:
    return "Let";
  case TokenType::kTrue:
    return "True";
  case TokenType::kFalse:
    return "False";
  case TokenType::kIf:
    return "If";
  case TokenType::kElse:
    return "Else";
  case TokenType::kReturn:
    return "Return";
  case TokenType::kEQ:
    return "EQ";
  case TokenType::kNEQ:
    return "NEQ";
  case TokenType::kString:
    return "String";
  default:
    return "Illegal";
  }
}

static const std::map<char, TokenType> CharTokenTypeMap = {
    {'=', TokenType::kAssign},
    {'+', TokenType::kPlus},
    {'-', TokenType::kMinus},
    {'!', TokenType::kBang},
    {'/', TokenType::kSlash},
    {'*', TokenType::kAsterisk},
    {'<', TokenType::kLT},
    {'>', TokenType::kGT},
    {',', TokenType::kComma},
    {';', TokenType::kSemicolon},
    {'(', TokenType::kLParen},
    {')', TokenType::kRParen},
    {'{', TokenType::kLBrace},
    {'}', TokenType::kRBrace},
};

static const std::map<std::string, TokenType> TokenTypeMap = {
    {"fn", TokenType::kFunction},
    {"let", TokenType::kLet},
    {"true", TokenType::kTrue},
    {"false", TokenType::kFalse},
    {"if", TokenType::kIf},
    {"else", TokenType::kElse},
    {"return", TokenType::kReturn},
};

struct Token {
  TokenType type = TokenType::kIllegal;
  std::string literal;
};

#endif //SRC_LEXER_TOKEN_H_