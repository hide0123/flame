#pragma once

#include "AST.h"

namespace metro {

class Lexer {

public:
  Lexer(SourceStorage const& source);

  TokenVector Lex();

private:
  bool check() const;
  char peek();
  void pass_space();
  bool match(std::string_view);

  bool eat(char c) {
    if (this->peek() == c) {
      this->position++;
      return true;
    }

    return false;
  }

  bool eat(std::string_view s) {
    if (this->match(s)) {
      this->position += s.length();
      return true;
    }

    return false;
  }

  std::string_view trim(i64 len) {
    return std::string_view(this->source.data.data() + this->position,
                            len);
  }

  SourceStorage const& source;
  i64 position;
  i64 length;
};

} // namespace metro

namespace metro::parser {

class Parser {

public:
  Parser(TokenVector tokens);

  // ASTPointer Ident();

  ASTPointer Factor();

  ASTPointer IndexRef();
  ASTPointer Unary();

  ASTPointer Mul();
  ASTPointer Add();
  ASTPointer Shift();
  ASTPointer Compare();
  ASTPointer BitCalc();
  ASTPointer LogAndOr();
  ASTPointer Assign();

  ASTPointer Expr();
  ASTPointer Stmt();

  ASTPointer Top();

  ASTPtr<AST::Program> Parse();

private:
  bool check() const;

  bool eat(std::string_view str);
  void expect(std::string_view str, bool keep_position = false);

  bool match(std::string_view s) {
    return this->cur->str == s;
  }

  bool match(TokenKind kind) {
    return this->cur->kind == kind;
  }

  bool match(std::pair<TokenKind, std::string_view> pair) {
    return this->match(pair.first) && this->match(pair.second);
  }

  template <class T, class U, class... Args>
  bool match(T&& t, U&& u, Args&&... args) {
    if (!this->check() || !this->match(std::forward<T>(t))) {
      return false;
    }

    auto save = this->cur;
    this->cur++;

    auto ret =
        this->match(std::forward<U>(u), std::forward<Args>(args)...);

    this->cur = save;
    return ret;
  }
  static ASTPtr<AST::Expr> new_expr(ASTKind k, Token& op,
                                    ASTPointer lhs, ASTPointer rhs) {
    return AST::Expr::New(k, op, lhs, rhs);
  }

  static ASTPtr<AST::Expr> new_assign(ASTKind kind, Token& op,
                                      ASTPointer lhs,
                                      ASTPointer rhs) {
    return new_expr(ASTKind::Assign, op, lhs,
                    new_expr(kind, op, lhs, rhs));
  }

  TokenIterator insert_token(Token tok) {
    this->cur = this->tokens.insert(this->cur, tok);
    this->end = this->tokens.end();

    return this->cur;
  }

  TokenIterator expectIdentifier();

  ASTPtr<AST::TypeName> expectTypeName();

  TokenVector tokens;
  TokenIterator cur, end, ate;
};

} // namespace metro::parser