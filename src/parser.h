#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_

#include "lexer/lexer.h"
#include "lexer/token.h"
#include "ast.h"

using PrefixParseFn = std::function<std::shared_ptr<Expression>()>;
using InfixParserFn = std::function<std::shared_ptr<Expression>(std::shared_ptr<Expression>)>;

static const int LOWEST = 1;
static const int EQUALS = 2;
static const int LESSGREATER = 3;
static const int SUM = 4;
static const int PRODUCT = 5;
static const int PREFIX = 6;
static const int CALL = 7;
static const int INDEX = 8;

static const std::map<TokenType, int> precedences = {
    {TokenType::kEQ, EQUALS},
    {TokenType::kNEQ, EQUALS},
    {TokenType::kLT, LESSGREATER},
    {TokenType::kGT, LESSGREATER},
    {TokenType::kPlus, SUM},
    {TokenType::kMinus, SUM},
    {TokenType::kSlash, PRODUCT},
    {TokenType::kAsterisk, PRODUCT},
    {TokenType::kLParen, CALL},
    {TokenType::kLBracket, INDEX},
};

class Parser {
 public:
  Parser(Lexer* l) : l_(l) {
    nextToken();
    nextToken();

    registerPrefix(TokenType::kIdent, [this]() {
      std::shared_ptr<Identifier> ret = std::make_shared<Identifier>();
      ret->token = curToken_;
      ret->value = curToken_.literal;
      return ret;
    });
    registerPrefix(TokenType::kInt, [this]() {
      std::shared_ptr<IntegerLiteral> ret = std::make_shared<IntegerLiteral>();
      ret->token = curToken_;
      ret->value = std::stoll(curToken_.literal);
      return ret;
    });
    auto parseBoolean = [this]() {
      std::shared_ptr<Boolean> ret = std::make_shared<Boolean>();
      ret->token = curToken_;
      ret->value = (curToken_.type == TokenType::kTrue);
      return ret;
    };
    registerPrefix(TokenType::kTrue, parseBoolean);
    registerPrefix(TokenType::kFalse, parseBoolean);
    auto parsePrefixExpression = [this]() {
      std::shared_ptr<PrefixExpression> ret = std::make_shared<PrefixExpression>();
      ret->token = curToken_;
      ret->op = curToken_.literal;
      nextToken();
      ret->right = parseExpression(PREFIX);
      return ret;
    };
    registerPrefix(TokenType::kBang, parsePrefixExpression);
    registerPrefix(TokenType::kMinus, parsePrefixExpression);
    registerPrefix(TokenType::kLParen, [this]() -> std::shared_ptr<Expression> {
      nextToken();
      std::shared_ptr<Expression> ret = parseExpression(LOWEST);
      if (!expectPeek(TokenType::kRParen)) {
        return nullptr;
      }
      return ret;
    });
    registerPrefix(TokenType::kIf, [this]() -> std::shared_ptr<Expression> {
      std::shared_ptr<IfExpression> ret = std::make_shared<IfExpression>();
      ret->token = curToken_;
      if (!expectPeek(TokenType::kLParen)) {
        return nullptr;
      }
      nextToken();
      ret->condition = parseExpression(LOWEST);
      if (!expectPeek(TokenType::kRParen)) {
        return nullptr;
      }
      if (!expectPeek(TokenType::kLBrace)) {
        return nullptr;
      }
      ret->consequence = parseBlockStatement();
      if (peekToken_.type == TokenType::kElse) {
        nextToken();
        if (!expectPeek(TokenType::kLBrace)) {
          return nullptr;
        }
        ret->alternative = parseBlockStatement();
      }
      return ret;
    });
    registerPrefix(TokenType::kFunction, [this]() -> std::shared_ptr<Expression> {
      std::shared_ptr<FunctionLiteral> ret = std::make_shared<FunctionLiteral>();
      ret->token = curToken_;
      if (!expectPeek(TokenType::kLParen)) {
        return nullptr;
      }
      ret->parameters = parseFunctionParameters();
      if (!expectPeek(TokenType::kLBrace)) {
        return nullptr;
      }
      ret->body = parseBlockStatement();
      return ret;
    });
    registerPrefix(TokenType::kString, [this]() {
      std::shared_ptr<StringLiteral> ret = std::make_shared<StringLiteral>();
      ret->token = curToken_;
      ret->value = curToken_.literal;
      return ret;
    });
    registerPrefix(TokenType::kLBracket, [this]() -> std::shared_ptr<Expression> {
      std::shared_ptr<ArrayLiteral> arr = std::make_shared<ArrayLiteral>();
      arr->token = curToken_;
      if (peekToken_.type == TokenType::kRBracket) {
        nextToken();
        return arr;
      }
      nextToken();
      arr->elements.push_back(parseExpression(LOWEST));
      while (peekToken_.type == TokenType::kComma) {
        nextToken();
        nextToken();
        arr->elements.push_back(parseExpression(LOWEST));
      }
      if (!expectPeek(TokenType::kRBracket)) {
        return nullptr;
      }
      return arr;
    });
    registerPrefix(TokenType::kLBrace, [this]() -> std::shared_ptr<Expression> {
      std::shared_ptr<HashLiteral> table = std::make_shared<HashLiteral>();
      table->token = curToken_;
      while (peekToken_.type != TokenType::kRBrace) {
        nextToken();
        std::shared_ptr<Expression> key = parseExpression(LOWEST);
        if (!expectPeek(TokenType::kColon)) {
          return nullptr;
        }
        nextToken();
        std::shared_ptr<Expression> value = parseExpression(LOWEST);
        table->pairs[key] = value;
        if (peekToken_.type != TokenType::kRBrace && !expectPeek(TokenType::kComma)) {
          return nullptr;
        }
      }
      if (!expectPeek(TokenType::kRBrace)) {
        return nullptr;
      }
      return table;
    });
    auto parseInfixExpression = [this](std::shared_ptr<Expression> left) {
      std::shared_ptr<InfixExpression> ret = std::make_shared<InfixExpression>();
      ret->token = curToken_;
      ret->op = curToken_.literal;
      ret->left = left;
      int p = curPrecedence();
      nextToken();
      ret->right = parseExpression(p);
      return ret;
    };
    registerInfix(TokenType::kPlus, parseInfixExpression);
    registerInfix(TokenType::kMinus, parseInfixExpression);
    registerInfix(TokenType::kSlash, parseInfixExpression);
    registerInfix(TokenType::kAsterisk, parseInfixExpression);
    registerInfix(TokenType::kEQ, parseInfixExpression);
    registerInfix(TokenType::kNEQ, parseInfixExpression);
    registerInfix(TokenType::kLT, parseInfixExpression);
    registerInfix(TokenType::kGT, parseInfixExpression);
    registerInfix(TokenType::kLParen, [this](std::shared_ptr<Expression> function) {
      std::shared_ptr<CallExpression> ret = std::make_shared<CallExpression>();
      ret->token = curToken_;
      ret->function = function;
      ret->arguments = parseCallArguments();
      return ret;
    });
    registerInfix(TokenType::kLBracket, [this](std::shared_ptr<Expression> arr) -> std::shared_ptr<Expression> {
      std::shared_ptr<IndexExpression> ret = std::make_shared<IndexExpression>();
      ret->token = curToken_;
      ret->left = arr;
      nextToken();
      ret->right = parseExpression(LOWEST);
      if (!expectPeek(TokenType::kRBracket)) {
        return nullptr;
      }
      return ret;
    });
  }

  std::shared_ptr<Program> ParseProgram() {
    std::shared_ptr<Program> program = std::make_shared<Program>();
    while (curToken_.type != TokenType::kEOF) {
      std::shared_ptr<Statement> stmt = parseStatement();
      if (stmt != nullptr) {
        program->statements.push_back(stmt);
      }
      nextToken();
    }
    return program;
  }

  const std::vector<std::string> Errors() const {
    return errors_;
  }

 private:
  void nextToken() {
    curToken_ = peekToken_;
    peekToken_ = l_->NextToken();
    // std::cout << "cur token is " << TokenTypeToName(curToken_.type) << "("
    //           << curToken_.literal << "), peek token is " << TokenTypeToName(peekToken_.type) << "("
    //           << peekToken_.literal << ")" << std::endl;
  }

  std::shared_ptr<Statement> parseStatement() {
    switch (curToken_.type) {
    case TokenType::kLet:
      return parseLetStatement();
    case TokenType::kReturn:
      return parseReturnStatement();
    default:
      return parseExpressionStatement();
    }
  }

  std::shared_ptr<LetStatement> parseLetStatement() {
    std::shared_ptr<LetStatement> stmt = std::make_shared<LetStatement>();
    stmt->token = curToken_;
    if (!expectPeek(TokenType::kIdent)) {
      return nullptr;
    }
    stmt->name = std::make_shared<Identifier>();
    stmt->name->token = curToken_;
    stmt->name->value = curToken_.literal;
    if (!expectPeek(TokenType::kAssign)) {
      return nullptr;
    }
    nextToken();
    stmt->value = parseExpression(LOWEST);
    if (peekToken_.type == TokenType::kSemicolon) {
      nextToken();
    }
    return stmt;
  }

  std::shared_ptr<ReturnStatement> parseReturnStatement() {
    std::shared_ptr<ReturnStatement> stmt = std::make_shared<ReturnStatement>();
    stmt->token = curToken_;
    nextToken();
    stmt->ret_value = parseExpression(LOWEST);
    if (peekToken_.type == TokenType::kSemicolon) {
      nextToken();
    }
    return stmt;
  }

  std::shared_ptr<ExpressionStatement> parseExpressionStatement() {
    std::shared_ptr<ExpressionStatement> stmt = std::make_shared<ExpressionStatement>();
    stmt->token = curToken_;

    stmt->expression = parseExpression(LOWEST);

    if (peekToken_.type == TokenType::kSemicolon) {
      nextToken();
    }

    return stmt;
  }

  std::shared_ptr<BlockStatement> parseBlockStatement() {
    std::shared_ptr<BlockStatement> stmt = std::make_shared<BlockStatement>();
    stmt->token = curToken_;
    nextToken();
    while (curToken_.type != TokenType::kRBrace && curToken_.type != TokenType::kEOF) {
      std::shared_ptr<Statement> curStmt = parseStatement();
      if (curStmt != nullptr) {
        stmt->statements_.push_back(curStmt);
      }
      nextToken();
    }
    return stmt;
  }

  std::shared_ptr<Expression> parseExpression(int p) {
    std::shared_ptr<Expression> leftExp;
    {
      auto iter = prefixParseFns.find(curToken_.type);
      if (iter == prefixParseFns.end()) {
        noPrefixParseFnError(curToken_.type);
        return nullptr;
      }
      leftExp = iter->second();
    }
    while (peekToken_.type != TokenType::kSemicolon && p < peekPrecedence()) {
      auto iter = infixParseFns.find(peekToken_.type);
      if (iter == infixParseFns.end()) {
        return leftExp;
      }
      nextToken();
      leftExp = iter->second(leftExp);
    }

    return leftExp;
  }

  void noPrefixParseFnError(TokenType type) {
    std::string msg = "no prefix parse function for " + TokenTypeToName(type) + " found";
    errors_.push_back(msg);
  }

  bool expectPeek(TokenType type) {
    if (peekToken_.type == type) {
      nextToken();
      return true;
    } else {
      peekError(type);
      return false;
    }
  }

  void peekError(TokenType type) {
    std::string msg = "expected next token to be " + std::to_string((int) type)
                      + ", got " + std::to_string((int) curToken_.type) + " instead";
    errors_.push_back(msg);
  }

  void registerPrefix(TokenType tokenType, PrefixParseFn fn) {
    prefixParseFns[tokenType] = fn;
  }

  void registerInfix(TokenType tokenType, InfixParserFn fn) {
    infixParseFns[tokenType] = fn;
  }

  int peekPrecedence() {
    auto iter = precedences.find(peekToken_.type);
    if (iter == precedences.end()) {
      return LOWEST;
    } else {
      return iter->second;
    }
  }

  int curPrecedence() {
    auto iter = precedences.find(curToken_.type);
    if (iter == precedences.end()) {
      return LOWEST;
    } else {
      return iter->second;
    }
  }

  std::vector<Identifier> parseFunctionParameters() {
    std::vector<Identifier> ret;
    if (peekToken_.type == TokenType::kRParen) {
      nextToken();
      return ret;
    }
    nextToken();
    Identifier ident;
    ident.token = curToken_;
    ident.value = curToken_.literal;
    ret.push_back(ident);

    while (peekToken_.type == TokenType::kComma) {
      nextToken();
      nextToken();
      Identifier cur_ident;
      cur_ident.token = curToken_;
      cur_ident.value = curToken_.literal;
      ret.push_back(cur_ident);
    }

    if (!expectPeek(TokenType::kRParen)) {
      ret.clear();
      return ret;
    }

    return ret;
  }

  std::vector<std::shared_ptr<Expression>> parseCallArguments() {
    std::vector<std::shared_ptr<Expression>> ret;
    if (peekToken_.type == TokenType::kRParen) {
      nextToken();
      return ret;
    }
    nextToken();
    ret.push_back(parseExpression(LOWEST));
    while (peekToken_.type == TokenType::kComma) {
      nextToken();
      nextToken();
      ret.push_back(parseExpression(LOWEST));
    }
    if (!expectPeek(TokenType::kRParen)) {
      ret.clear();
      return ret;
    }
    return ret;
  }

  Lexer* l_;
  Token curToken_;
  Token peekToken_;
  std::vector<std::string> errors_;

  std::map<TokenType, PrefixParseFn> prefixParseFns;
  std::map<TokenType, InfixParserFn> infixParseFns;
};

#endif  // SRC_PARSER_H_