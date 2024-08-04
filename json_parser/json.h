#ifndef __JSON_H
#define __JSON_H

#include <string>
#include <vector>
#include <cstring>
#include <map>
#include <iostream>

class Json {
public:
  enum class Type
  {
    num_type,
    str_type,
    bool_type,
    array_type,
    map_type,
    null_type
  };
  union Data
  {
    bool flag;
    double value;
    std::string* str;
    std::vector<Json>* array;
    std::map<std::string, Json>* map;
  };
  struct Value{
    Type type;
    Data data;
  };
  void setType(Type type) {
    value.type = type;
  }
  void setData(Data data) {
    value.data = data;
  }
  Json();
  Json(const Json& other);

  Json& operator=(const Json& other);

  Json(Json&& other);

  Json& operator=(Json&& other);
  void clear();
  void print();
  void printImpl(int64_t& indent);
  std::ostream& printWithIndent(int64_t& indent);
  ~Json() {
    clear();
  }
// private:
  Value value; 
};


#endif