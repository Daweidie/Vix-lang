	.text
	.file	"VixModule"
	.globl	fib
	.p2align	4, 0x90
	.type	fib,@function
fib:
	pushq	%r14
	pushq	%rbx
	pushq	%rax
	movl	%edi, %r14d
	xorl	%ebx, %ebx
	cmpl	$2, %edi
	jge	.LBB0_3
	movl	%r14d, %ecx
	jmp	.LBB0_2
.LBB0_3:
	xorl	%ebx, %ebx
	.p2align	4, 0x90
.LBB0_4:
	leal	-1(%r14), %edi
	callq	fib@PLT
	leal	-2(%r14), %ecx
	addl	%eax, %ebx
	cmpl	$4, %r14d
	movl	%ecx, %r14d
	jae	.LBB0_4
.LBB0_2:
	addl	%ecx, %ebx
	movl	%ebx, %eax
	addq	$8, %rsp
	popq	%rbx
	popq	%r14
	retq
.Lfunc_end0:
	.size	fib, .Lfunc_end0-fib

	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	pushq	%rax
	movl	$40, %edi
	callq	fib@PLT
	leaq	.Lfmt_i32_nl(%rip), %rdi
	movl	%eax, %esi
	xorl	%eax, %eax
	callq	printf@PLT
	xorl	%eax, %eax
	popq	%rcx
	retq
.Lfunc_end1:
	.size	main, .Lfunc_end1-main

	.type	.Lfmt_i32_nl,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.Lfmt_i32_nl:
	.asciz	"%d\n"
	.size	.Lfmt_i32_nl, 4

	.section	".note.GNU-stack","",@progbits
