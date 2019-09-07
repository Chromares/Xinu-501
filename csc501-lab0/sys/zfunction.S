.text
	.globl zfunction
	.type zfuntion, @function

zfunction:
	movl 4(%esp), %eax
	andl $0xfffc01ff, %eax
	shrl $8, %eax
	orl $0xff000000, %eax
	ret
