#include "object.h"
#include "ast.h"

std::string FunctionObject::Inspect() {
  std::string ret = "fn(";
  for (auto& ident : parameters) {
    ret += ident.String();
    ret += ", ";
  }
  ret += ") {\n";
  ret += body->String();
  ret += "\n}";
  return ret;
}

const std::map<std::string, BuiltInFnType> BuiltInTable = {
    {"len", [](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
      if (args.size() != 1) {
        return std::make_shared<ErrorObject>("wrong number of arguments");
      }
      std::shared_ptr<Object> obj = args[0];
      if (obj->Type() == ObjectType::kString) {
        return std::make_shared<IntegerObject>(
            std::dynamic_pointer_cast<StringObject>(obj)->value.size());
      } else if (obj->Type() == ObjectType::kArray) {
        return std::make_shared<IntegerObject>(std::dynamic_pointer_cast<ArrayObject>(obj)->objects.size());
      } else {
        return std::make_shared<ErrorObject>("argument to len not supported, got " + ObjectTypeToString(obj->Type()));
      }
    }},
    {"first", [](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
      if (args.size() != 1) {
        return std::make_shared<ErrorObject>("wrong number of arguments");
      }
      std::shared_ptr<Object> obj = args[0];
      if (obj->Type() != ObjectType::kArray) {
        return std::make_shared<ErrorObject>("argument to first must be Array, got " + ObjectTypeToString(obj->Type()));
      }
      std::shared_ptr<ArrayObject> arr = std::dynamic_pointer_cast<ArrayObject>(obj);
      if (arr->objects.size() > 0) {
        return arr->objects[0];
      } else {
        return std::make_shared<ErrorObject>("index(0) exceeds array size(0)");
      }
    }},
    {"last", [](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
      if (args.size() != 1) {
        return std::make_shared<ErrorObject>("wrong number of arguments");
      }
      std::shared_ptr<Object> obj = args[0];
      if (obj->Type() != ObjectType::kArray) {
        return std::make_shared<ErrorObject>("argument to first must be Array, got " + ObjectTypeToString(obj->Type()));
      }
      std::shared_ptr<ArrayObject> arr = std::dynamic_pointer_cast<ArrayObject>(obj);
      if (arr->objects.size() > 0) {
        return arr->objects[arr->objects.size() - 1];
      } else {
        return std::make_shared<ErrorObject>("index(0) exceeds array size(0)");
      }
    }},
    {"rest", [](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
      if (args.size() != 1) {
        return std::make_shared<ErrorObject>("wrong number of arguments");
      }
      std::shared_ptr<Object> obj = args[0];
      if (obj->Type() != ObjectType::kArray) {
        return std::make_shared<ErrorObject>("argument to first must be Array, got " + ObjectTypeToString(obj->Type()));
      }
      std::shared_ptr<ArrayObject> arr = std::dynamic_pointer_cast<ArrayObject>(obj);
      if (arr->objects.size() > 0) {
        std::shared_ptr<ArrayObject> ret = std::make_shared<ArrayObject>();
        for (int i = 1; i < arr->objects.size(); ++i) {
          ret->objects.push_back(arr->objects[i]);
        }
        return ret;
      } else {
        return std::make_shared<ErrorObject>("index(0) exceeds array size(0)");
      }
    }},
    {"push", [](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
      if (args.size() != 2) {
        return std::make_shared<ErrorObject>("wrong number of arguments");
      }
      if (args[0]->Type() != ObjectType::kArray) {
        return std::make_shared<ErrorObject>("argument to push must be Array, got " + ObjectTypeToString(args[0]->Type()));
      }
      std::shared_ptr<ArrayObject> ret = std::make_shared<ArrayObject>();
      std::shared_ptr<ArrayObject> input = std::dynamic_pointer_cast<ArrayObject>(args[0]);
      ret->objects = input->objects;
      ret->objects.push_back(args[1]);
      return ret;
    }},
    {"map", [](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
      if (args.size() != 2) {
        return std::make_shared<ErrorObject>("wrong number of arguments");
      }
      if (args[0]->Type() != ObjectType::kArray) {
        return std::make_shared<ErrorObject>("argument to map must be Array, got " + ObjectTypeToString(args[0]->Type()));
      }
      if (args[1]->Type() != ObjectType::kFunction) {
        return std::make_shared<ErrorObject>("operation of map must be function, got " + ObjectTypeToString(args[1]->Type()));
      }
      std::shared_ptr<ArrayObject> ret = std::make_shared<ArrayObject>();
      std::shared_ptr<ArrayObject> input = std::dynamic_pointer_cast<ArrayObject>(args[0]);
      std::shared_ptr<FunctionObject> fn = std::dynamic_pointer_cast<FunctionObject>(args[1]);
      if (fn->parameters.size() != 1) {
        return std::make_shared<ErrorObject>("operator of map parameter number should be 1");
      }
      for (auto elem : input->objects) {
        std::shared_ptr<Environment> nested_env = std::make_shared<Environment>(fn->env);
        nested_env->Set(fn->parameters[0].value, elem);
        std::shared_ptr<Object> output = fn->body->Eval(nested_env);
        if (output != nullptr && output->Type() == ObjectType::kError) {
          return output;
        } else {
          ret->objects.push_back(output);
        }
      }
       return ret;
     }},
};


bool ObjectEqual::operator()(std::shared_ptr<Object> lhs, std::shared_ptr<Object> rhs) const {
  if (lhs->Type() != rhs->Type()) {
    return false;
  }
  switch (lhs->Type()) {
  case ObjectType::kInteger:
    return std::dynamic_pointer_cast<IntegerObject>(lhs)->value == std::dynamic_pointer_cast<IntegerObject>(rhs)->value;
  case ObjectType::kBoolean:
    return std::dynamic_pointer_cast<BooleanObject>(lhs)->value == std::dynamic_pointer_cast<BooleanObject>(rhs)->value;
  case ObjectType::kNull:
    return true;
  case ObjectType::kReturnValue:
    return ObjectEqual()(std::dynamic_pointer_cast<ReturnValueObject>(lhs)->value, std::dynamic_pointer_cast<ReturnValueObject>(rhs)->value);
  case ObjectType::kError:
    return std::dynamic_pointer_cast<ErrorObject>(lhs)->message == std::dynamic_pointer_cast<ErrorObject>(rhs)->message;
  case ObjectType::kFunction:
    return false;
  case ObjectType::kString:
    return std::dynamic_pointer_cast<StringObject>(lhs)->value == std::dynamic_pointer_cast<StringObject>(rhs)->value;
  case ObjectType::kBuiltIn:
    return false;
  case ObjectType::kArray:
  {
    auto& lhs_array = std::dynamic_pointer_cast<ArrayObject>(lhs)->objects;
    auto& rhs_array = std::dynamic_pointer_cast<ArrayObject>(rhs)->objects;
    if (lhs_array.size() != rhs_array.size()) {
      return false;
    }
    for (size_t i = 0; i < lhs_array.size(); ++i) {
      if (!ObjectEqual()(lhs_array[i], rhs_array[i])) {
        return false;
      }
    }
    return true;
  }
  case ObjectType::kHash:
  {
    auto& lhs_map = std::dynamic_pointer_cast<HashObject>(lhs)->table;
    auto& rhs_map = std::dynamic_pointer_cast<HashObject>(rhs)->table;
    if (lhs_map.size() != rhs_map.size()) {
      return false;
    }
    for (auto& pair : lhs_map) {
      auto iter = rhs_map.find(pair.first);
      if (iter == rhs_map.end()) {
        return false;
      } else if (!ObjectEqual()(pair.second, iter->second)) {
        return false;
      }
    }
    return true;
  }
  default:
    return false;
  }
}
