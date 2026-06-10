.data
print_fmt: .string "%ld \n"

.text

.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq $10, %rax
 movq %rax, -8(%rbp)
 movq -8(%rbp), %rax
 movq %rax, %r10
 movq $5, %rax
 cmpq %rax, %r10
 je case_1
 movq $10, %rax
 cmpq %rax, %r10
 je case_2
 jmp case_3
case_1:
 movq $50, %rax
 movq %rax, -16(%rbp)
 jmp endlbl_0
case_2:
 movq $100, %rax
 movq %rax, -16(%rbp)
 jmp endlbl_0
 jmp endlbl_0
case_3:
 movq $0, %rax
 movq %rax, -16(%rbp)
 jmp endlbl_0
endlbl_0:
 movq -16(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movq $0, %rax
 call printf@PLT
 movq $0, %rax
 jmp .end_main
.end_main:
 leave
 ret

.section .note.GNU-stack,"",@progbits
