#ifndef __LEXER_H
#define __LEXER_H

#include <fstream>
#include <string>
#include <assert.h>
#include <iostream>

enum Token : int {
  tok_comma = ',',
  tok_colon = ':',
  tok_double_quotation = '\"',
  tok_bracket_open = '{',
  tok_bracket_close = '}',
  tok_sbracket_open = '[',
  tok_sbracket_close = ']',
  tok_string = 1,
  tok_number = 2,
  tok_null = 3,
  tok_true = 4,
  tok_false = 5,

  tok_eof = -1,
};

class Lexer {
public:
  Lexer(std::string& filepath);
  Lexer() = delete;
  Lexer(const Lexer&) = delete;
  Lexer(Lexer&&) = delete;
  Lexer& operator=(const Lexer&) = delete;
  Lexer& operator=(Lexer&&) = delete;
  ~Lexer();
  std::string getString() {
    return str;
  }

  double getNumber() {
    return number;
  }

  int64_t getRow() {
    return row;
  }

  int64_t getCol() {
    return col;
  }
  Token getCurrentToken() const {
    return curr_tok;
  }

  void consumerCurrnetToken() {
    curr_tok = getNextToken();
  }

private:
  Token getNextToken();
  void getNextLine();
  std::fstream fr;
  std::string current_line;
  int64_t row;
  int64_t col;
  char* buffer;
  char* start;
  char* begin;
  char* end;
  std::string str;
  double number;
  Token curr_tok;
};

#endif