/*
 * Copyright 2013 AppNexus, Inc.
 *
 * This is unpublished proprietary source code of AppNexus, Inc.
 * The copyright notice above does not evidence any actual or
 * intended publication of such source code.
 *
 * Redistribution of this material is strictly prohibited.
 */

/*
 * Copyright 2010, 2011 Samy Al Bahra.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "../asm.h"

/*
 * size_t strlen(const char *);
 */
ENTRY(sse2_strlen):
	pxor     %xmm0, %xmm0
	movl     %edi, %esi
	movl     %edi, %ecx
	andq     $~15, %rdi	
	movdqa   (%rdi), %xmm1
	andl     $15, %ecx
	pcmpeqb	 %xmm0, %xmm1
	pmovmskb %xmm1, %eax

	/* Shift out padding bytes and check if we have already found a match. */
	shrl     %cl, %eax
	testl    %eax, %eax
	jnz	 8f
	leaq     16(%rdi), %rdi

1:	/*
	 * At this point the source address is 16-byte aligned. Check by 16-byte
	 * chunks naively, we are guaranteed not to cross a page boundary.
	 */
	movdqa   (%rdi), %xmm1
	leaq     16(%rdi), %rdi
	pcmpeqb  %xmm0, %xmm1
	pmovmskb %xmm1, %eax
	testl    %eax, %eax
	jz	 1b
.align 4, 0x90
	bsfl     %eax, %eax
	subl     %esi, %edi
	addl     %edi, %eax
	subl     $16, %eax
	ret
8:
	bsfl     %eax, %eax
	ret

