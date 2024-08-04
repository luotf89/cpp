#ifndef __PARSER_H
#define __PARSER_H

#include "lexer.h"
#include "json.h"

class Parser {
public:
  Parser(std::string& filename);

  Json parser_all();
  Json parser_map();
  Json parser_array();
  Json parser_elem();

private:
  Lexer lexer;
  Json json;
};

#endif