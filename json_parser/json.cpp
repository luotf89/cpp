#include "json.h"
#include <cassert>

Json::Json() {
  value.type = Type::null_type;
  value.data.value = 0;
}

Json::Json(const Json& other) {
  std::cout << "construct copy" << std::endl;
  Data data;
  value.type = other.value.type;
  if (value.type == Type::bool_type ||
      value.type == Type::null_type ||
      value.type == Type::num_type) {
    value.data = other.value.data;
  } else if (value.type == Type::str_type) {
    if (other.value.data.str) {
      data.str = new std::string;
      *(data.str) = *(other.value.data.str);
    } else {
      data.str = nullptr;
    }
    value.data = data;
  } else if (value.type == Type::array_type) {
    if (other.value.data.array) {
      data.array = new std::vector<Json>;
      for (size_t i = 0; i < other.value.data.array->size(); i++) {
        Json json = (*(other.value.data.array))[i];
        data.array->emplace_back(json);
      }
    } else {
      data.array = nullptr;
    }
    value.data = data;
  } else {
    if (other.value.data.map) {
      data.map = new std::map<std::string, Json>;
      for (auto iter = other.value.data.map->begin();
                iter != other.value.data.map->end(); ++iter) {
        std::string key = iter->first;
        Json value = iter->second;
        assert(data.map->insert({key, value}).second);
      }
    } else {
      data.map = nullptr;
    }
    value.data = data;
  }
}

Json::Json(Json&& other) {
  std::cout << "construct move" << std::endl;
  value.type = other.value.type;
  if (value.type == Type::bool_type ||
      value.type == Type::null_type ||
      value.type == Type::num_type) {
    value.data = other.value.data;
  } else if (value.type == Type::str_type) {
    value.data.str = other.value.data.str;
    other.value.data.str = nullptr;
  } else if (value.type == Type::array_type) {
    value.data.array = other.value.data.array;
    other.value.data.array = nullptr;
  } else {
    value.data.map = other.value.data.map;
    other.value.data.map = nullptr;
  }
}

Json& Json::operator=(const Json& other) {
  std::cout << "operator = copy assign" << std::endl;
  if (this != &other) {
    clear();
    Data data;
    value.type = other.value.type;
    if (value.type == Type::bool_type ||
        value.type == Type::null_type ||
        value.type == Type::num_type) {
      value.data = other.value.data;
    } else if (value.type == Type::str_type) {
      if (other.value.data.str) {
        data.str = new std::string;
        *(data.str) = *(other.value.data.str);
      } else {
        data.str = nullptr;
      }
      value.data = data;
    } else if (value.type == Type::array_type) {
      if (other.value.data.array) {
        data.array = new std::vector<Json>;
        for (size_t i = 0; i < other.value.data.array->size(); i++) {
          Json json = (*(other.value.data.array))[i];
          data.array->emplace_back(json);
        }
      } else {
        data.array = nullptr;
      }
      value.data = data;
    } else {
      if (other.value.data.map) {
        data.map = new std::map<std::string, Json>;
        for (auto iter = other.value.data.map->begin();
                  iter != other.value.data.map->end(); ++iter) {
          std::string key = iter->first;
          Json value = iter->second;
          assert(data.map->insert({key, value}).second);
        }
      } else {
        data.map = nullptr;
      }
      value.data = data;
    }
  }
  return *this;
}

Json& Json::operator=(Json&& other) {
  std::cout << "operator = move assign" << std::endl;
  if (this != &other) {
    clear();
    value.type = other.value.type;
    if (value.type == Type::bool_type ||
        value.type == Type::null_type ||
        value.type == Type::num_type) {
      value.data = other.value.data;
    } else if (value.type == Type::str_type) {
      value.data.str = other.value.data.str;
      other.value.data.str = nullptr;
    } else if (value.type == Type::array_type) {
      value.data.array = other.value.data.array;
      other.value.data.array = nullptr;
    } else {
      value.data.map = other.value.data.map;
      other.value.data.map = nullptr;
    }
  }
  return *this;
}

void Json::clear() {
  if (value.type == Type::str_type) {
    if (value.data.str != nullptr) {
      delete value.data.str;
      value.data.str = nullptr;
    }
  } else if (value.type == Type::array_type) {
    if (value.data.array != nullptr) {
      for (auto& elem: *(value.data.array)) {
        elem.~Json();
      }
      delete value.data.array;
      value.data.array = nullptr;
    }
  } else if (value.type == Type::map_type) {
    if (value.data.map != nullptr) {
      for (auto& elem: *(value.data.map)) {
        elem.second.~Json();
      }
      delete value.data.map;
      value.data.map = nullptr;
    }
  }
}
std::ostream& Json::printWithIndent(int64_t& indent) {
  for (int64_t i = 0; i < indent; i++) {
    std::cerr << " ";
  }
  return std::cerr;
}

void Json::printImpl(int64_t& indent) {
  if (value.type == Type::bool_type) {
    std::cerr << value.data.flag;
  } else if (value.type == Type::null_type) {
    std::cerr << "null";
  } else if (value.type == Type::num_type) {
    std::cerr << value.data.value;
  } else if (value.type == Type::str_type) {
    if (value.data.str) {
      std::cerr << *(value.data.str);
    }
  } else if (value.type == Type::array_type) {
    if (value.data.array) {
      std::cerr<< "[" << "\n";
      indent += 2;
      for (auto iter = value.data.array->begin();
                iter != value.data.array->end();
                ++iter) {
        printWithIndent(indent);
        iter->printImpl(indent);
        if (iter != --(value.data.array->end())) {
          std::cerr << ",\n";
        }
      }
      std::cerr << "\n";
      indent -= 2;
      printWithIndent(indent)<< "]";
    }
  } else {
    if (value.data.map) {
      std::cerr<< "{" << "\n";
      indent += 2;
      for (auto iter = value.data.map->begin();
                iter != value.data.map->end();
                ++iter) {
        printWithIndent(indent) << iter->first << " : ";
        iter->second.printImpl(indent);
        if (iter != --(value.data.map->end())) {
          std::cerr << ",\n";
        }
      }
      std::cerr << "\n";
      indent -= 2;
      printWithIndent(indent)<< "}";
    }
  }
}

void Json::print() {
  int64_t indent = 0;
  printImpl(indent);
  std::cerr << std::endl;
}