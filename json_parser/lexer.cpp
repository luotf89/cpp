#include "lexer.h"

Lexer::Lexer(std::string &filepath) : fr(filepath, std::ios::in | std::ios::binary) {
  assert(fr.is_open());
  fr.seekg(0, std::ios::end);
  int64_t file_size = fr.tellg();
  fr.seekg(0, std::ios::beg);
  std::cout << "file size: " << file_size << std::endl;
  buffer = (char *)malloc(file_size);
  fr.read(buffer, file_size + 2);
  fr.close();
  buffer[file_size] = '\n';
  buffer[file_size + 1] = EOF;
  std::cout << "buffer : " << buffer << std::endl;
  start = buffer;
  begin = nullptr;
  end = nullptr;
  curr_tok = getNextToken();
}

Lexer::~Lexer() {
  if (buffer) {
    free(buffer);
  }
}

Token Lexer::getNextToken() {
  while(begin == nullptr || *begin == '\n' || (*begin == '\t' || *begin == ' ')) {
    if (begin == nullptr || *begin == '\n') {
      getNextLine();
      row++;
      col = 0;
    }
    while(*begin == '\t' || *begin == ' ') {
      begin++;
      col++;
      assert(begin <= end);
    }
  }
  if (*end == EOF) {
    return tok_eof;
  }
  assert(begin != nullptr);
  if(*begin == Token::tok_double_quotation) {
    str.clear();
    begin++;
    col++;
    while(*begin != tok_double_quotation && begin != end ) {
      str += *begin;
      begin++;
      col++;
    }
    assert(*begin == tok_double_quotation);
    begin++;
    col++;
    return tok_string;
  } else if (*begin == tok_bracket_open) {
    begin++;
    col++;
    return tok_bracket_open;
  } else if (*begin == tok_bracket_close) {
    begin++;
    col++;
    return tok_bracket_close;
  } else if (*begin == tok_sbracket_open) {
    begin++;
    return tok_sbracket_open;
  } else if (*begin == tok_sbracket_close) {
    begin++;
    col++;
    return tok_sbracket_close;
  } else if (*begin == tok_comma) {
    begin++;
    col++;
    return tok_comma;
  } else if (*begin == tok_colon) {
    begin++;
    col++;
    return tok_colon;
  } else if (isalnum(*begin)) {
    str.clear();
    str += *begin;
    begin++;
    col++;
    while(isalnum(*begin) || *begin == '.') {
      str += *begin;
      begin++;
      col++;
    }
    if (str == "false") {
      return tok_false;
    }
    if (str == "true") {
      return tok_true;
    }
    if (str == "null") {
      return tok_null;
    }
    assert(isdigit(str[0]));
    for (size_t i = 1; i < str.size(); i++) {
      assert(isdigit(str[i]) || str[i] == '.');
    }
    number = std::stod(str);
    return tok_number;
  } else {
    assert(false);
    return tok_eof;
  }
}

void Lexer::getNextLine() {
  begin = start;
  while(*start != EOF && *start != '\n') {
    start++;
  }
  end = *start == '\n' ? start++ : start;
}