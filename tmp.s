.intel_syntax noprefix
.globl main
main:
 push 3
 push 5
 pop rdi
 pop rax
 add rax, rdi
 push rax
 push 2
 pop rdi
 pop rax
 cqo
 idiv rax, rdi
 push rax
 pop rax
 ret
