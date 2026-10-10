.intel_syntax noprefix

.text
.global _start
.type _start, @function
.extern main

_start:
    xor ebp, ebp;
    mov rdi, [rsp];
    lea rsi, [rsp + 8];
    and rsp, -16;
    call main;
    mov rdi, rax;
    mov rax, 60;
    syscall;
    ud2;

.size _start, .-_start

.section .note.GNU-stack,"",@progbits
