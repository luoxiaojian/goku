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