#include "chibicc.h"

// スタックに変数のアドレスをpushする
static void gen_lvar(Node *node) {
  if (node->kind != ND_LVAR)
    error("expected a variable");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

static void gen(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %ld\n", node->val);
      return;
    case ND_LVAR:
      gen_lvar(node);
      printf("  pop rax\n");        // 変数のアドレスをpop
      printf("  mov rax, [rax]\n"); // 変数の値を取得
      printf("  push rax\n");       // 変数の値をpush
      return;
    case ND_ASSIGN:
      gen_lvar(node->lhs);
      gen(node->rhs);
      printf("  pop rdi\n");        // 右辺の値をpop
      printf("  pop rax\n");        // 変数のアドレスをpop
      printf("  mov [rax], rdi\n"); // 変数に右辺の値を代入
      printf("  push rdi\n");       // 右辺の値をpush
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LAT:
      printf("  cmp rdi, rax\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LET:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LAE:
      printf("  cmp rdi, rax\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LEE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}

void codegen(Node *node) {
  gen(node);
}