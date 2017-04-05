.text
    .globl go
go:
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    call _main
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    retq
_main:
    movq $123, %rdi
    movq $321, %rsi
    movq $10, %rdx
    movq $_f_ret, -8(%rsp)
    subq $8, %rsp
    jmp _product_sum
_f_ret:
    call print
    retq
_product_sum:
    movq $0, %rcx
    movq $0, %rax
_loop_start:
    movq $1, %r8
    imulq %rdi, %r8
    imulq %rsi, %r8
    addq %r8, %rax
    inc %rdi
    inc %rsi
    inc %rcx
    cmpq %rcx, %rsi
    je _loop_end
    jmp _loop_start
_loop_end:
    salq $1, %rax
    inc %rax
    retq
