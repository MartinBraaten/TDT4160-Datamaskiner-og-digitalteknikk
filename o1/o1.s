.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO

.text
	.global Start
	
Start:

	// Finding the address like in the lecture video

	// Address for the output for the led port (port E)
	LDR R0, =PORT_E // load the port value in R0
	LDR R1, =PORT_SIZE // load port size into R1
	MUL R0, R0, R1 // multiplying them together
	LDR R1, =GPIO_BASE // redefining R1 to base value
	ADD R0, R0, R1 // Adding base address to R0
	LDR R1, =GPIO_PORT_DOUTSET // Redefining R1 to the address of the output of led
	ADD R0, R0, R1 // Finding the complete address of the output of LED
	// The STR R0, [R0, R1] in the lecture video did not work. Used ADD instead

	// Address for clear output led port (port E) (equivalent procedure as above)
	LDR R1, =PORT_E
	LDR R2, =PORT_SIZE
	MUL R1, R1, R2
	LDR R2, =GPIO_BASE
	ADD R1, R1, R2
	LDR R2, =GPIO_PORT_DOUTCLR // clear output
	ADD R1, R1, R2

	// Address for input from button (port B)
	LDR R2, =PORT_B
	LDR R3, =PORT_SIZE
	MUL R2, R2, R3
	LDR R3, =GPIO_BASE
	ADD R2, R2, R3
	LDR R3, =GPIO_PORT_DIN // Input from button
	ADD R2, R2, R3



	MOV R4, #1  // loaded with 0b0000000001 (1 bit)
	LSL R5, R4, #LED_PIN // left shifted two places (0b0000000100) for LED pin.
	LSL R6, R4, #BUTTON_PIN // Left shifted nine places (0b1000000000) for button pin


	Loop:
		LDR R4, [R2]		// Get input from button
		AND R4, R4, R6		// remove the unwanted values
		CMP R4, R6     		// Compare button input with 1 bit
		BEQ On				// If input = 1 bit, go to on. Else go to off
		Off:
			STR R5, [R0]		// Turn LED on
			B Loop				// Go back to start of loop
		On:
			STR R5, [R1]		// Turn LED off
			B Loop				// Go back to start of loop


NOP // Behold denne på bunnen av fila

