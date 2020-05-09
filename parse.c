#include "9cc.h"

// production rules
static Node *program();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

static Token *currentToken;
static Node *code[100];

static Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node *new_num_node(long val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

static Node *new_ident_node(char var) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  node->offset = (var - 'a' + 1) * 8;
  return node;
}

static int get_number(Token *token) {
  if (token->kind != TK_NUM)
    error_at(token->str, "expected a number");
  return token->val;
}

static bool equal(Token *token, char *op) {
  return strlen(op) == token->len &&
         !strncmp(token->str, op, token->len);
}

// 次のtokenが期待しているcharのとき、tokenを1つ進めて
// trueを返す。それ以外はfalseを返す。
bool consume(char *op) {
  if (!equal(currentToken, op))
    return false;
  currentToken = currentToken->next;
  return true;
}

// program = stmt*
static Node *program() {
  int i = 0;
  while (currentToken->kind != TK_EOF)
    code[i++] = stmt();
  code[i] = NULL;
}

// stmt = expr ";"
static Node *stmt() {
  Node *node = expr();

  if (consume(";"))
    return node;
  else
    error_at(currentToken->str, "Not ';'");
}

// expr = assign
static Node *expr() {
  return assign();
}

// assign = equality ("=" assign)?
static Node *assign() {
  Node *node = equality();

  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">=" add | ">" add)*
static Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LET, node, relational());
    else if (consume("<="))
      node = new_node(ND_LEE, node, relational());
    else if (consume(">"))
      node = new_node(ND_LAT, node, relational());
    else if (consume(">="))
      node = new_node(ND_LAE, node, relational());
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
static Node *add(){
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-")? primary
static Node *unary() {
  if (consume("+"))
    return primary();
  else if (consume("-"))
    return new_node(ND_SUB, new_num_node(0), primary());
  else
    return primary();
}

// primary = num | ident | "(" expr ")"
static Node *primary() {
  Node *node;

  if (consume("(")) {
    node = expr();
    if (consume(")"))
      return node;
    else
      error_at(currentToken->str, "Not ')'");
  }

  if (currentToken->kind == TK_NUM)
    node = new_num_node(get_number(currentToken));
  else if (currentToken->kind == TK_IDENT)
    node = new_ident_node(currentToken->str[0]);

  currentToken = currentToken->next;
  return node;
}

Node **parse(Token *token) {
  currentToken = token;
  program();
  if (currentToken->kind != TK_EOF)
    error_at(currentToken->str, "extra token");
  return code;
}
