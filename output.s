.section .data
.LCfmt:
	.string "%d\n"

.comm x,4096,16
.comm y,4096,16
.comm t1,4096,16
.comm t2,4096,16
.comm result,4096,16
.comm arr,4096,16
.comm t3,4096,16
.comm t4,4096,16
.comm t5,4096,16
.comm t6,4096,16
.comm t7,4096,16
.comm t8,4096,16
.comm i,4096,16
.comm t9,4096,16
.comm t11,4096,16
.comm t10,4096,16

.section .text
.globl main
.extern printf
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $10, %eax
	movl %eax, x(%rip)
	movl $3, %eax
	movl %eax, y(%rip)
	movl y(%rip), %eax
	pushq %rax
	movl $2, %eax
	movl %eax, %ecx
	popq %rax
	imull %ecx, %eax
	movl %eax, t1(%rip)
	movl x(%rip), %eax
	pushq %rax
	movl t1(%rip), %eax
	movl %eax, %ecx
	popq %rax
	addl %ecx, %eax
	movl %eax, t2(%rip)
	movl t2(%rip), %eax
	movl %eax, result(%rip)
	movl result(%rip), %eax
	movl %eax, arr+0(%rip)
	movl x(%rip), %eax
	pushq %rax
	movl y(%rip), %eax
	movl %eax, %ecx
	popq %rax
	subl %ecx, %eax
	movl %eax, t3(%rip)
	movl t3(%rip), %eax
	movl %eax, arr+4(%rip)
	movl x(%rip), %eax
	pushq %rax
	movl y(%rip), %eax
	movl %eax, %ecx
	popq %rax
	cmpl %ecx, %eax
	setg %al
	movzbl %al, %eax
	movl %eax, t4(%rip)
	movl t4(%rip), %eax
	testl %eax, %eax
	je L1
	movl arr+0(%rip), %eax
	movl %eax, t5(%rip)
	movl arr+4(%rip), %eax
	movl %eax, t6(%rip)
	movl t5(%rip), %eax
	pushq %rax
	movl t6(%rip), %eax
	movl %eax, %ecx
	popq %rax
	addl %ecx, %eax
	movl %eax, t7(%rip)
	movl t7(%rip), %eax
	movl %eax, result(%rip)
	jmp L2
L1:
	movl x(%rip), %eax
	pushq %rax
	movl y(%rip), %eax
	movl %eax, %ecx
	popq %rax
	imull %ecx, %eax
	movl %eax, t8(%rip)
	movl t8(%rip), %eax
	movl %eax, result(%rip)
L2:
	movl $0, %eax
	movl %eax, i(%rip)
L3:
	movl i(%rip), %eax
	pushq %rax
	movl $5, %eax
	movl %eax, %ecx
	popq %rax
	cmpl %ecx, %eax
	setl %al
	movzbl %al, %eax
	movl %eax, t9(%rip)
	movl t9(%rip), %eax
	testl %eax, %eax
	je L4
	movl result(%rip), %eax
	pushq %rax
	movl i(%rip), %eax
	movl %eax, %ecx
	popq %rax
	addl %ecx, %eax
	movl %eax, t11(%rip)
	movl t11(%rip), %eax
	movl %eax, result(%rip)
	movl i(%rip), %eax
	pushq %rax
	movl $1, %eax
	movl %eax, %ecx
	popq %rax
	addl %ecx, %eax
	movl %eax, t10(%rip)
	movl t10(%rip), %eax
	movl %eax, i(%rip)
	jmp L3
L4:
	movl result(%rip), %eax
	movl %eax, %esi
	leaq .LCfmt(%rip), %rdi
	xorl %eax, %eax
	call printf@PLT
	movl result(%rip), %eax
	jmp .Lreturn

.Lreturn:
	leave
	ret
.section .note.GNU-stack,"",@progbits
