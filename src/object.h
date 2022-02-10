#ifndef SRC_OBJECT_H_
#define SRC_OBJECT_H_

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

enum class ObjectType {
  kInteger,
  kBoolean,
  kNull,
  kReturnValue,
  kError,
  kFunction,
  kString,
  kBuiltIn,
  kArray,
  kHash,
};

inline std::string ObjectTypeToString(ObjectType type) {
  switch (type) {
  case ObjectType::kInteger: return "Integer";
  case ObjectType::kBoolean: return "Boolean";
  case ObjectType::kNull: return "Null";
  case ObjectType::kReturnValue: return "ReturnValue";
  case ObjectType::kError: return "Error";
  case ObjectType::kFunction: return "Function";
  case ObjectType::kBuiltIn: return "BuiltIn";
  case ObjectType::kArray: return "Array";
  case ObjectType::kHash: return "Hash";
  default: return "Unknown";
  }
}

class Object {
 public:
  virtual ObjectType Type() = 0;
  virtual std::string Inspect() = 0;

  virtual size_t Hash() const = 0;
};

class IntegerObject : public Object {
 public:
  explicit IntegerObject(int64_t v) : value(v) {}

  ObjectType Type() override {
    return ObjectType::kInteger;
  }

  std::string Inspect() override {
    return std::to_string(value);
  }

  size_t Hash() const override {
    return std::hash<int64_t>()(value);
  }

  int64_t value;
};

class BooleanObject : public Object {
 public:
  explicit BooleanObject(bool v) : value(v) {}

  ObjectType Type() override {
    return ObjectType::kBoolean;
  }

  std::string Inspect() override {
    return value ? "true" : "false";
  }

  size_t Hash() const override {
    return std::hash<bool>()(value);
  }

  bool value;
};

class NullObject : public Object {
 public:
  ObjectType Type() override {
    return ObjectType::kNull;
  }

  std::string Inspect() override {
    return "null";
  }

  size_t Hash() const override {
    return 0;
  }
};

class ReturnValueObject : public Object {
 public:
  explicit ReturnValueObject(std::shared_ptr<Object> v) : value(v) {}

  ObjectType Type() override {
    return ObjectType::kReturnValue;
  }

  std::string Inspect() override {
    return value->Inspect();
  }

  size_t Hash() const override {
    return 1;
  }

  std::shared_ptr<Object> value;
};

class ErrorObject : public Object {
 public:
  explicit ErrorObject(const std::string& m) : message(m) {}

  ObjectType Type() override { return ObjectType::kError; }

  std::string Inspect() override {
    return "Error: " + message;
  }

  size_t Hash() const override {
    return 2;
  }

  std::string message;
};

class StringObject : public Object {
 public:
  explicit StringObject(const std::string& v) : value(v) {}

  ObjectType Type() override { return ObjectType::kString; }

  std::string Inspect() override {
    return value;
  }

  size_t Hash() const override {
    return std::hash<std::string>()(value);
  }

  std::string value;
};

class Identifier;
class BlockStatement;
class Environment;

class FunctionObject : public Object {
 public:
  FunctionObject(const std::vector<Identifier>& p, std::shared_ptr<BlockStatement> b, std::shared_ptr<Environment> e)
      : parameters(p), body(b), env(e) {}

  ObjectType Type() override { return ObjectType::kFunction; }

  std::string Inspect() override;

  size_t Hash() const override {
    return 3;
  }

  std::vector<Identifier> parameters;
  std::shared_ptr<BlockStatement> body;
  std::shared_ptr<Environment> env;
};

using BuiltInFnType = std::function<std::shared_ptr<Object>(std::vector<std::shared_ptr<Object>>)>;

class BuiltInObject : public Object {
 public:
  BuiltInObject(const BuiltInFnType& f) : fn(f) {}

  ObjectType Type() override { return ObjectType::kBuiltIn; }

  std::string Inspect() override { return "builtin function"; }

  size_t Hash() const override {
    return 4;
  }

  BuiltInFnType fn;
};

class ArrayObject : public Object {
 public:
  ArrayObject() {}

  ObjectType Type() override { return ObjectType::kArray; }

  std::string Inspect() override {
    std::string ret = "[";
    for (auto obj : objects) {
      ret += obj->Inspect();
      ret += ",";
    }
    ret += "]";
    return ret;
  }

  size_t Hash() const override {
    return 5;
  }

  std::vector<std::shared_ptr<Object>> objects;
};

struct ObjectHash {
  size_t operator()(std::shared_ptr<Object> obj) const noexcept {
    return obj->Hash();
  }
};

struct ObjectEqual {
  bool operator()(std::shared_ptr<Object> lhs, std::shared_ptr<Object> rhs) const;
};

class HashObject : public Object {
 public:
  HashObject() {}

  ObjectType Type() override {
    return ObjectType::kHash;
  }

  std::string Inspect() override {
    std::string ret = "[";
    for (auto& pair : table) {
      ret += pair.first->Inspect();
      ret += ": ";
      ret += pair.second->Inspect();
      ret += ",";
    }
    ret += "]";
    return ret;
  }

  size_t Hash() const override {
    return 6;
  }

  std::unordered_map<std::shared_ptr<Object>, std::shared_ptr<Object>,
                     ObjectHash, ObjectEqual> table;
};

// inline bool ObjectEqual(std::shared_ptr<Object> left, std::shared_ptr<Object> right) {
//   if (left->Type() != right->Type()) {
//     return false;
//   }
//   if (left->Type() == ObjectType::kInteger) {
//     int64_t left_value = std::dynamic_pointer_cast<IntegerObject>(left)->value;
//     int64_t right_value = std::dynamic_pointer_cast<IntegerObject>(right)->value;
//     return left_value == right_value;
//   } else if (left->Type() == ObjectType::kBoolean) {
//     int64_t left_value = std::dynamic_pointer_cast<BooleanObject>(left)->value;
//     int64_t right_value = std::dynamic_pointer_cast<BooleanObject>(right)->value;
//     return left_value == right_value;
//   } else {
//     return true;
//   }
// }

inline bool IsTruthy(std::shared_ptr<Object> obj) {
  if (obj->Type() == ObjectType::kInteger) {
    return std::dynamic_pointer_cast<IntegerObject>(obj)->value != 0;
  } else if (obj->Type() == ObjectType::kBoolean) {
    return std::dynamic_pointer_cast<BooleanObject>(obj)->value;
  } else {
    return false;
  }
}

class Environment {
 public:
  Environment(std::shared_ptr<Environment> outer = nullptr) : outer(outer) {
#if 0
    if (outer == nullptr) {
      std::cout << "created base environment: " << this << std::endl;
    } else {
      std::cout << "created environment: " << this << " from environment: " << outer.get() << std::endl;
    }
#endif
  }

  std::shared_ptr<Object> Get(const std::string& name) {
    auto iter = objects.find(name);
    if (iter != objects.end()) {
      return iter->second;
    } else {
      if (outer == nullptr) {
        return nullptr;
      } else {
        return outer->Get(name);
      }
    }
  }

  void Set(const std::string& name, std::shared_ptr<Object> obj) {
    objects.emplace(name, obj);
  }

  std::map<std::string, std::shared_ptr<Object>> objects;
  std::shared_ptr<Environment> outer;
};

extern const std::map<std::string, BuiltInFnType> BuiltInTable;

#endif  // SRC_OBJECT_H_