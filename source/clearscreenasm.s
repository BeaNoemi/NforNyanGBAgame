
.THUMB						@ turn on thumb
.ALIGN  2			
.GLOBL  clearscreen		@ name of the function


.THUMB_FUNC			
					
clearscreen:		@ function start
					@ void clearscreen(unsigned short *BGMapBase)

push { r4-r7, lr }			@ push r4-r7 and link register onto stack. 


		mov r5, #0			@ this register will hold our iteration value (0 to 1024)
		mov r6,	#1			@ --this sets r6 to 1024 ( set r6 to 1 and then 0 << 10 which is 1024 )
		lsl r6, r6, #10		@ /
		

for_loop:
		mov		r4, #0			@ set r4 to 0 (value for the empty tile
		strh	r4, [r0]		@ put the value of r4 into the memory location that r0 points to (BG0MapBase)
		add		r0, r0, #2		@ move r0 by 2 bytes to the next memory location 
		add		r5, r5, #1		@ add 1 to the iteration value (r5)
		cmp		r5, r6			@ compare iteration value(r5) with the last value of the loop (r6)
		bne		for_loop		@ if it's negative branch back to the beginning of loop
		

pop { r4-r7 }		@ pop first 4 values from stack back into r4-r7, and also
pop { r3 }			@ pop the next value from stack (stored value for lr) into some unused register, e.g. r3 - we cannot overwrite lr so we have to do it via a normal register
bx r3				@ "branch and exchange" (return) back to C, using the previous value for lr stored in r3
@ ==================================







