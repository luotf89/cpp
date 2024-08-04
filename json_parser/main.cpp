#include "lexer.h"
#include "parser.h"
#include <iostream>

int main() {
  std::string filename{"./test.json"};
  // Lexer lexer(filename);
  // Token token;
  // std::cout << "================= parser token ==================" << std::endl;
  // while(token = lexer.getCurrentToken(), token != tok_eof) {
  //   if (token == tok_comma) {
  //     std::cout << " row: " << lexer.getRow() << " col: " << lexer.getCol() << " " << " , " << std::endl;
  //   } else if(token == tok_colon) {
  //     std::cout << " row: " << lexer.getRow() << " col: " << lexer.getCol() << " " << " : " << std::endl;
  //   } else if(token == tok_double_quotation) {
  //     std::cout << " row: " << lexer.getRow() << " col: " << lexer.getCol() << " " << " \" " << std::endl;
  //   } else if(token == tok_bracket_open) {
  //     std::cout << " row: " << lexer.getRow() << " col: " << lexer.getCol() << " " << " { " << std::endl;
  //   } else if(token == tok_bracket_close) {
  //     std::cout << " row: " << lexer.getRow() << " col: " << lexer.getCol() << " " << " } " << std::endl;
  //   } else if(token == tok_sbracket_open) {
  //     std::cout << " row: " << lexer.getRow() << " col: " << lexer.getCol() << " " << " [ " << std::endl;
  //   } else if(token == tok_sbracket_close) {
  //     std::cout << " row: " << lexer.getRow() << " col: " << lexer.getCol() << " " << " ] " << std::endl;
  //   } else if(token == tok_string) {
  //     std::cout << " row: " << lexer.getRow() << " col: " << lexer.getCol() << " " << lexer.getString() << std::endl;
  //   } else if(token == tok_number) {
  //     std::cout << " row: " << lexer.getRow() << " col: " << lexer.getCol() << " " << lexer.getNumber() << std::endl;
  //   } else if(token == tok_null) {
  //     std::cout << " row: " << lexer.getRow() << " col: " << lexer.getCol() << " " << " null " << std::endl;
  //   } else if(token == tok_true) {
  //     std::cout << " row: " << lexer.getRow() << " col: " << lexer.getCol() << " " << " ture " << std::endl;
  //   } else if(token == tok_false) {
  //     std::cout << " row: " << lexer.getRow() << " col: " << lexer.getCol() << " " << " false " << std::endl;
  //   } else {
  //     std::cout << " current token is not support: " << token << std::endl;
  //     assert(false);
  //   }
  //   lexer.consumerCurrnetToken();
  // }
  // assert(token == tok_eof);
  Parser parser(filename);
  Json json = parser.parser_all();
  json.print();
  return 0;
}