.hello:
    .string "Hello World!"

    .globl  main
main:
    leaq    .hello(%rip), %rdi
    call    puts
    xorl    %eax, %eax
    ret
