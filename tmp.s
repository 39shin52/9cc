.intel_syntax noprefix
.globl main
main:
 push 0
 push 10
 pop rdi
 pop rax
 sub rax, rdi
 push rax
 push 20
 pop rdi
 pop rax
 add rax, rdi
 push rax
 pop rax
 ret
