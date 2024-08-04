#include "parser.h"

Parser::Parser(std::string& filename):lexer(filename) {

}

Json Parser::parser_map() {
  Json json;
  Json::Data data;
  json.setType(Json::Type::map_type);
  std::map<std::string, Json>* real_value = new std::map<std::string, Json>;
  data.map = real_value;
  json.setData(data);
  std::string key;
  Json value;
  do {
    lexer.consumerCurrnetToken();
    if (lexer.getCurrentToken() == tok_bracket_close) {
      break;
    }
    assert(lexer.getCurrentToken() == tok_string);
    key = lexer.getString();
    lexer.consumerCurrnetToken();
    assert(lexer.getCurrentToken() == tok_colon);
    lexer.consumerCurrnetToken();
    if (lexer.getCurrentToken() == tok_bracket_open) {
      value = parser_map();
    } else if (lexer.getCurrentToken() == tok_sbracket_open) {
      value = parser_array();
    } else {
      value = parser_elem();
    }
    (*real_value)[key] = std::move(value);
    lexer.consumerCurrnetToken();
  } while(lexer.getCurrentToken() == tok_comma);
  assert(lexer.getCurrentToken() == tok_bracket_close);
  return json;
}

Json Parser::parser_array() {
  Json json;
  Json::Data data;
  json.setType(Json::Type::array_type);
  std::vector<Json>* real_value = new std::vector<Json>;
  data.array = real_value;
  json.setData(data);
  Json value;
  do {
    lexer.consumerCurrnetToken();
    if (lexer.getCurrentToken() == tok_sbracket_close) {
      break;
    } else if (lexer.getCurrentToken() == tok_bracket_open) {
      value = parser_map();
    } else if (lexer.getCurrentToken() == tok_sbracket_open) {
      value = parser_array();
    } else {
      value = parser_elem();
    }
    real_value->emplace_back(value);
    lexer.consumerCurrnetToken();
  } while(lexer.getCurrentToken() == tok_comma);
  assert(lexer.getCurrentToken() == tok_sbracket_close);
  return json;
}

Json Parser::parser_elem() {
  Json json;
  Json::Data data;
  if (lexer.getCurrentToken() == tok_null) {
    data.value = 0;
    json.setType(Json::Type::null_type);
  } else if (lexer.getCurrentToken() == tok_string) {
    json.setType(Json::Type::str_type);
    data.str = new std::string;
    *(data.str) = std::move(lexer.getString());
  } else if (lexer.getCurrentToken() == tok_number) {
    json.setType(Json::Type::num_type);
    data.value = lexer.getNumber();
  } else if (lexer.getCurrentToken() == tok_true) {
    json.setType(Json::Type::bool_type);
    data.flag = true;
  } else if (lexer.getCurrentToken() == tok_false) {
    json.setType(Json::Type::bool_type);
    data.flag = false;
  } else {
    std::cout << "current token is not support " << lexer.getCurrentToken() << std::endl;
    assert(false);
  }
  json.setData(data);
  return json;
}

Json Parser::parser_all() {
  if (lexer.getCurrentToken() == tok_bracket_open) {
    return parser_map();
  } else if (lexer.getCurrentToken() == tok_sbracket_open) {
    return parser_array();
  }
  return Json();
}