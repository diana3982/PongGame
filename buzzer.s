/* 
*This is assembly code for the msp430,
* it is supposed to switch the frequency of the buzzer.
* This will manipulate the sound.
*/

	.data
win:	.word 2			;default case 2
	
	.text
cases:	.word case0 //player 1 wins
	.word case1 //player 2 wins
	.word case2 //default
	
winner:	mov &win, r12
	mov cases(r12), r0 	;find case r12

case0:	mov #4000, r12
	call #buzzer_advance_frequency ;buzzer_advancfrequency(4000)
	brk break

case1:	mov #6000, r12
	call #buzzer_advance_frequency ;buzzer_advance_frequency(6000)
	brk break

case2:	brk break

brk:
	pop(r0)
