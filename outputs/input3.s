.data
print_fmt: .string "%ld \n"

.text

.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq $1, %rax
 movq %rax, -8(%rbp)
while_0:
 movq -8(%rbp), %rax
 pushq %rax
 movq $10, %rax
 movq %rax, %rcx
 popq %rax
 cmpq %rcx, %rax
 movq $0, %rax
 setl %al
 movzbq %al, %rax
 cmpq $0, %rax
 je endlbl_0
 movq $1, %rax
 movq %rax, -16(%rbp)
while_1:
 movq -16(%rbp), %rax
 pushq %rax
 movq $10, %rax
 movq %rax, %rcx
 popq %rax
 cmpq %rcx, %rax
 movq $0, %rax
 setl %al
 movzbq %al, %rax
 cmpq $0, %rax
 je endlbl_1
 movq -8(%rbp), %rax
 pushq %rax
 movq -16(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 pushq %rax
 movq $8, %rax
 movq %rax, %rcx
 popq %rax
 cmpq %rcx, %rax
 movq $0, %rax
 sete %al
 movzbq %al, %rax
 cmpq $0, %rax
 je else_2
 jmp endlbl_1
 jmp endif_2
else_2:
endif_2:
 movq -16(%rbp), %rax
 pushq %rax
 movq $1, %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movq %rax, -16(%rbp)
 jmp while_1
endlbl_1:
 movq -8(%rbp), %rax
 pushq %rax
 movq $6, %rax
 movq %rax, %rcx
 popq %rax
 cmpq %rcx, %rax
 movq $0, %rax
 sete %al
 movzbq %al, %rax
 cmpq $0, %rax
 je else_3
 jmp endlbl_0
 jmp endif_3
else_3:
endif_3:
 movq -8(%rbp), %rax
 pushq %rax
 movq $100, %rax
 movq %rax, %rcx
 popq %rax
 imulq %rcx, %rax
 pushq %rax
 movq -16(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movq $0, %rax
 call printf@PLT
 movq -8(%rbp), %rax
 pushq %rax
 movq $1, %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movq %rax, -8(%rbp)
 jmp while_0
endlbl_0:
 movq $0, %rax
 jmp .end_main
.end_main:
 leave
 ret

.section .note.GNU-stack,"",@progbits
