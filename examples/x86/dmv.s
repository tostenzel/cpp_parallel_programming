        .globl	dmv
dmv:
        # args:
        #   rows:           %rdi
        #   cols:           %rsi
        #   A:              %rdx
        #   x:              %rcx
        #   b:              %r8
        # others:
        #   row:            %rax
        #   col:            %r10
        #   inc = cols*8:   %r9
        #   acc:            %xmm0
        #   tmp:            %xmm1
        xorq    %rax, %rax              # row = 0
        leaq    (,%rsi, 8), %r9         # inc = cols * 8

.loop_rows:
        cmpq    %rdi, %rax
        jge     .out                    # if row >= rows goto out

.body_rows:
        xorpd   %xmm0, %xmm0            # acc = 0
        xorq    %r10, %r10              # col = 0

.loop_cols:
        cmpq    %rsi, %r10
        jge     .continue_rows          # if col >= cols goto .continue_rows

.body_cols:
        movsd   (%rdx, %r10, 8), %xmm1  # tmp  = A[col * 8]
        mulsd   (%rcx, %r10, 8), %xmm1  # tmp *= x[col * 8]
        addsd   %xmm1, %xmm0            # acc += tmp

.continue_cols:
        incq    %r10
        jmp     .loop_cols

.continue_rows:
        movsd   %xmm0, (%r8, %rax, 8)   # b[row * 8]
        incq    %rax
        addq    %r9, %rdx               # A += inc
        jmp     .loop_rows

.out:
        retq
