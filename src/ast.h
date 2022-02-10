#ifndef SRC_AST_H_
#define SRC_AST_H_

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "lexer/token.h"
#include "object.h"

class Node {
 public:
  virtual std::string TokenLiteral() = 0;
  virtual std::string String() = 0;

  virtual std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) {
    return std::make_shared<NullObject>();
  }
};

class Statement : public Node {
 public:
  virtual void statementNode() = 0;
};

class Expression : public Node {
 public:
  virtual void expressionNode() = 0;
};

class Program : public Node {
 public:
  std::string TokenLiteral() override {
    if (!statements.empty()) {
      return statements[0]->TokenLiteral();
    } else {
      return "";
    }
  }

  std::string String() override {
    std::string ret;
    for (auto stmt : statements) {
      ret += stmt->String();
    }
    return ret;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    std::shared_ptr<Object> ret;
    for (auto stmt : statements) {
      ret = stmt->Eval(env);
      if (ret == nullptr) {
        continue;
      }
      if (ret->Type() == ObjectType::kReturnValue) {
        return std::dynamic_pointer_cast<ReturnValueObject>(ret)->value;
      } else if (ret->Type() == ObjectType::kError) {
        return ret;
      }
    }
    return ret;
  }

  std::vector<std::shared_ptr<Statement>> statements;
};

class Identifier : public Expression {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void expressionNode() override {}

  std::string String() override {
    return value;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    std::shared_ptr<Object> ret = env->Get(value);
    if (ret == nullptr) {
      auto iter = BuiltInTable.find(value);
      if (iter == BuiltInTable.end()) {
        return std::make_shared<ErrorObject>("identifier not found: " + value);
      } else {
        return std::make_shared<BuiltInObject>(iter->second);
      }
    }
    return ret;
  }

  Token token;
  std::string value;
};

class IntegerLiteral : public Expression {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void expressionNode() override {}

  std::string String() override {
    return token.literal;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    return std::make_shared<IntegerObject>(value);
  }

  Token token;
  int64_t value;
};

class StringLiteral : public Expression {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void expressionNode() override {}

  std::string String() override {
    return token.literal;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    return std::make_shared<StringObject>(value);
  }

  Token token;
  std::string value;
};

class Boolean : public Expression {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void expressionNode() override {}

  std::string String() override {
    return token.literal;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    return std::make_shared<BooleanObject>(value);
  }

  Token token;
  bool value;
};

class PrefixExpression : public Expression {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void expressionNode() override {}

  std::string String() override {
    std::string ret;
    ret += "(";
    ret += op;
    ret += right->String();
    ret += ")";
    return ret;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    std::shared_ptr<Object> evaluated_right = right->Eval(env);
    if (evaluated_right != nullptr && evaluated_right->Type() == ObjectType::kError) {
      return evaluated_right;
    }
    if (op == "-") {
      if (evaluated_right->Type() == ObjectType::kInteger) {
        auto casted_right = std::dynamic_pointer_cast<IntegerObject>(evaluated_right);
        casted_right->value = -casted_right->value;
        return casted_right;
      }
    } else if (op == "!") {
      if (evaluated_right->Type() == ObjectType::kBoolean) {
        auto casted_right = std::dynamic_pointer_cast<BooleanObject>(evaluated_right);
        casted_right->value = !casted_right->value;
        return casted_right;
      } else {
        return std::make_shared<BooleanObject>(false);
      }
    }
    return std::make_shared<ErrorObject>("unknown operator: " + op + " " + ObjectTypeToString(evaluated_right->Type()));
  }

  Token token;
  std::string op;
  std::shared_ptr<Expression> right;
};

class InfixExpression : public Expression {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void expressionNode() override {}

  std::string String() override {
    std::string ret;
    ret += "(";
    ret += left->String();
    ret += " ";
    ret += op;
    ret += " ";
    ret += right->String();
    ret += ")";
    return ret;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    std::shared_ptr<Object> evaluated_left = left->Eval(env);
    if (evaluated_left != nullptr && evaluated_left->Type() == ObjectType::kError) {
      return evaluated_left;
    }
    std::shared_ptr<Object> evaluated_right = right->Eval(env);
    if (evaluated_right != nullptr && evaluated_right->Type() == ObjectType::kError) {
      return evaluated_right;
    }
    if (evaluated_left->Type() == ObjectType::kInteger && evaluated_right->Type() == ObjectType::kInteger) {
      int64_t left_value =
          std::dynamic_pointer_cast<IntegerObject>(evaluated_left)->value;
      int64_t right_value =
          std::dynamic_pointer_cast<IntegerObject>(evaluated_right)->value;
      if (op == "+") {
        return std::make_shared<IntegerObject>(left_value + right_value);
      } else if (op == "-") {
        return std::make_shared<IntegerObject>(left_value - right_value);
      } else if (op == "*") {
        return std::make_shared<IntegerObject>(left_value * right_value);
      } else if (op == "/") {
        return std::make_shared<IntegerObject>(left_value / right_value);
      } else if (op == "<") {
        return std::make_shared<BooleanObject>(left_value < right_value);
      } else if (op == ">") {
        return std::make_shared<BooleanObject>(left_value > right_value);
      } else if (op == "==") {
        return std::make_shared<BooleanObject>(left_value == right_value);
      } else if (op == "!=") {
        return std::make_shared<BooleanObject>(left_value != right_value);
      } else {
        return std::make_shared<ErrorObject>("unknown operator " + op +
                                             " between integers");
      }
    } else if (evaluated_left->Type() == ObjectType::kString && evaluated_right->Type() == ObjectType::kString && op == "+") {
      return std::make_shared<StringObject>(std::dynamic_pointer_cast<StringObject>(evaluated_left)->value
                                            + std::dynamic_pointer_cast<StringObject>(evaluated_right)->value);
    } else if (op == "==") {
      return std::make_shared<BooleanObject>(ObjectEqual()(evaluated_left, evaluated_right));
    } else if (op == "!=") {
      return std::make_shared<BooleanObject>(!ObjectEqual()(evaluated_left, evaluated_right));
    } else {
      return std::make_shared<ErrorObject>("unkown operator "
                                           + ObjectTypeToString(evaluated_left->Type())
                                           + " " + op + " "
                                           + ObjectTypeToString(evaluated_right->Type()));
    }
  }

  Token token;
  std::shared_ptr<Expression> left;
  std::string op;
  std::shared_ptr<Expression> right;
};

class LetStatement : public Statement {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void statementNode() override {}

  std::string String() override {
    std::string ret;
    ret += token.literal;
    ret += " ";
    ret += name->String();
    ret += " = ";
    if (value != nullptr) {
      ret += value->String();
    }
    ret += ";";
    return ret;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    std::shared_ptr<Object> evaluated_value = value->Eval(env);
    if (evaluated_value != nullptr && evaluated_value->Type() == ObjectType::kError) {
      return evaluated_value;
    }
    env->Set(name->value, evaluated_value);
    return nullptr;
  }

  Token token;
  std::shared_ptr<Identifier> name;
  std::shared_ptr<Expression> value;
};

class ReturnStatement : public Statement {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void statementNode() override {}

  std::string String() override {
    std::string ret;
    ret += token.literal;
    ret += " ";
    if (ret_value != nullptr) {
      ret += ret_value->String();
    }
    ret += ";";
    return ret;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    std::shared_ptr<Object> ret = ret_value->Eval(env);
    if (ret != nullptr && ret->Type() == ObjectType::kError) {
      return ret;
    }
    return std::make_shared<ReturnValueObject>(ret);
  }

  Token token;
  std::shared_ptr<Expression> ret_value;
};

class ExpressionStatement : public Statement {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void statementNode() override {}

  std::string String() override {
    if (expression != nullptr) {
      return expression->String();
    }
    return "";
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    return expression->Eval(env);
  }

  Token token;
  std::shared_ptr<Expression> expression;
};

class BlockStatement : public Statement {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void statementNode() override {}

  std::string String() override {
    std::string ret = "{\n";
    for (auto stmt : statements_) {
      ret += "\t";
      ret += stmt->String();
      ret += "\n";
    }
    ret += "}\n";
    return ret;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    std::shared_ptr<Object> ret;
    for (auto stmt : statements_) {
      ret = stmt->Eval(env);
      if (ret != nullptr) {
        if (ret->Type() == ObjectType::kReturnValue || ret->Type() == ObjectType::kError) {
          return ret;
        }
      }
    }
    return ret;
  }

  Token token;
  std::vector<std::shared_ptr<Statement>> statements_;
};

class IfExpression : public Expression {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void expressionNode() override {}

  std::string String() override {
    std::string ret = "if ";
    ret += condition->String();
    ret += consequence->String();
    if (alternative != nullptr) {
      ret += " else ";
      ret += alternative->String();
    }
    return ret;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    std::shared_ptr<Object> evaluated_condition = condition->Eval(env);
    if (evaluated_condition != nullptr && evaluated_condition->Type() == ObjectType::kError) {
      return evaluated_condition;
    }
    if (IsTruthy(evaluated_condition)) {
      return consequence->Eval(env);
    } else if (alternative != nullptr) {
      return alternative->Eval(env);
    } else {
      return std::make_shared<NullObject>();
    }
  }

  Token token;
  std::shared_ptr<Expression> condition;
  std::shared_ptr<BlockStatement> consequence;
  std::shared_ptr<BlockStatement> alternative;
};

class FunctionLiteral : public Expression {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void expressionNode() override {}

  std::string String() override {
    std::string ret = token.literal;
    ret += "(";
    for (auto &p : parameters) {
      ret += p.String();
      ret += ", ";
    }
    ret += ")";
    ret += body->String();
    return ret;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    return std::make_shared<FunctionObject>(parameters, body, env);
  }

  Token token;
  std::vector<Identifier> parameters;
  std::shared_ptr<BlockStatement> body;
};

class CallExpression : public Expression {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void expressionNode() override {}

  std::string String() override {
    std::string ret = function->String();
    ret += "(";
    for (auto p : arguments) {
      ret += p->String();
      ret += ", ";
    }
    ret += ")";
    return ret;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    std::shared_ptr<Object> evaluated_function = function->Eval(env);
    if (evaluated_function != nullptr && evaluated_function->Type() == ObjectType::kError) {
      return evaluated_function;
    }
    if (evaluated_function == nullptr) {
      return std::make_shared<ErrorObject>("function is null");
    }
    if (evaluated_function->Type() == ObjectType::kFunction) {
      std::shared_ptr<FunctionObject> casted_function =
          std::dynamic_pointer_cast<FunctionObject>(evaluated_function);
      std::vector<std::shared_ptr<Object>> args;
      for (auto exp : arguments) {
        std::shared_ptr<Object> evaluated = exp->Eval(env);
        if (evaluated != nullptr && evaluated->Type() == ObjectType::kError) {
          return evaluated;
        }
        args.push_back(evaluated);
      }

      std::shared_ptr<Environment> nested_env =
          std::make_shared<Environment>(casted_function->env);
      int paramNum = casted_function->parameters.size();
      for (int i = 0; i < paramNum; ++i) {
        nested_env->Set(casted_function->parameters[i].value, args[i]);
      }
      std::shared_ptr<Object> ret = casted_function->body->Eval(nested_env);
      if (ret != nullptr && ret->Type() == ObjectType::kReturnValue) {
        return std::dynamic_pointer_cast<ReturnValueObject>(ret)->value;
      } else {
        return ret;
      }
    } else if (evaluated_function->Type() == ObjectType::kBuiltIn) {
      std::shared_ptr<BuiltInObject> casted_function =
          std::dynamic_pointer_cast<BuiltInObject>(evaluated_function);
      std::vector<std::shared_ptr<Object>> args;
      for (auto exp : arguments) {
        std::shared_ptr<Object> evaluated = exp->Eval(env);
        if (evaluated != nullptr && evaluated->Type() == ObjectType::kError) {
          return evaluated;
        }
        args.push_back(evaluated);
      }
      return casted_function->fn(args);
    } else {
      return std::make_shared<ErrorObject>("wrong type in call statement: " + ObjectTypeToString(evaluated_function->Type()));
    }
  }

  Token token;
  std::shared_ptr<Expression> function;
  std::vector<std::shared_ptr<Expression>> arguments;
};

class ArrayLiteral : public Expression {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void expressionNode() override {}

  std::string String() override {
    std::string ret = "[";
    for (auto elem : elements) {
      ret += elem->String();
      ret += ", ";
    }
    ret += "]";
    return ret;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    std::shared_ptr<ArrayObject> arr = std::make_shared<ArrayObject>();
    for (auto elem : elements) {
      arr->objects.push_back(elem->Eval(env));
    }
    return arr;
  }

  Token token;
  std::vector<std::shared_ptr<Expression>> elements;
};

class IndexExpression : public Expression {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void expressionNode() override {}

  std::string String() override {
    std::string ret = left->String();
    ret += "[";
    ret += right->String();
    ret += "]";
    return ret;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    std::shared_ptr<Object> evaluated_left = left->Eval(env);
    if (evaluated_left == nullptr || evaluated_left->Type() == ObjectType::kError) {
      return evaluated_left;
    }
    if (evaluated_left->Type() == ObjectType::kArray) {
      std::shared_ptr<ArrayObject> casted_left =
          std::dynamic_pointer_cast<ArrayObject>(evaluated_left);
      std::shared_ptr<Object> evaluated_right = right->Eval(env);
      if (evaluated_right == nullptr ||
          evaluated_right->Type() == ObjectType::kError) {
        return evaluated_right;
      }
      if (evaluated_right->Type() != ObjectType::kInteger) {
        return std::make_shared<ErrorObject>(
            "index should be integer, got " +
            ObjectTypeToString(evaluated_right->Type()));
      }
      int64_t index =
          std::dynamic_pointer_cast<IntegerObject>(evaluated_right)->value;
      if (index >= casted_left->objects.size()) {
        return std::make_shared<ErrorObject>(
            "index(" + std::to_string(index) + ") exceeds array size(" +
            std::to_string(casted_left->objects.size()) + ")");
      }
      return casted_left->objects[index];
    } else if (evaluated_left->Type() == ObjectType::kHash) {
      std::shared_ptr<HashObject> casted_left = std::dynamic_pointer_cast<HashObject>(evaluated_left);
      std::shared_ptr<Object> evaluated_right = right->Eval(env);
      if (evaluated_right != nullptr && evaluated_left->Type() == ObjectType::kError) {
        return evaluated_right;
      }
      auto iter = casted_left->table.find(evaluated_right);
      if (iter == casted_left->table.end()) {
        return std::make_shared<NullObject>();
      } else {
        return iter->second;
      }
    } else {
      return std::make_shared<ErrorObject>
          ("index operator not supported: " + ObjectTypeToString(evaluated_left->Type()));
    }
  }

  Token token;
  std::shared_ptr<Expression> left;
  std::shared_ptr<Expression> right;
};

class HashLiteral : public Expression {
 public:
  std::string TokenLiteral() override {
    return token.literal;
  }

  void expressionNode() override {}

  std::string String() override {
    std::string ret = "{";
    for (auto& pair : pairs) {
      ret += pair.first->String();
      ret += ": ";
      ret += pair.second->String();
      ret += ", ";
    }
    ret += "}";
    return ret;
  }

  std::shared_ptr<Object> Eval(std::shared_ptr<Environment> env) override {
    std::shared_ptr<HashObject> ret = std::make_shared<HashObject>();
    for (auto& pair : pairs) {
      std::shared_ptr<Object> key = pair.first->Eval(env);
      if (key != nullptr && key->Type() == ObjectType::kError) {
        return key;
      }
      std::shared_ptr<Object> value = pair.second->Eval(env);
      if (value != nullptr && value->Type() == ObjectType::kError) {
        return value;
      }
      ret->table[key] = value;
    }
    return ret;
  }

  Token token;
  std::unordered_map<std::shared_ptr<Expression>, std::shared_ptr<Expression>> pairs;
};

#endif  // SRC_AST_H_