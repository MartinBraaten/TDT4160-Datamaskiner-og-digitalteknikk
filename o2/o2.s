.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO
.include "sys-tick_constants.s" // Register-adresser og konstanter for SysTick

.text
	.global Start

Start:

//  Systick base + (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk)
	LDR R0, =SYSTICK_BASE
	LDR R1, [R0]
	ADD R1, R1, 0b110
	STR R1, [R0]

//  Setting up clockcycles
	LDR R2, =FREQUENCY/10
	LDR R1, =SYSTICK_BASE + SYSTICK_LOAD
	STR R2, [R1]

//  Systick value
	LDR R0, =SYSTICK_BASE + SYSTICK_VAL
	MOV R1, #0
	STR R1, [R0]

//  GPIO interrupt button setup
	LDR R0, =GPIO_BASE + GPIO_EXTIPSELH
	LDR R2, [R0]
	AND R2, ~(0b1111 << 4)
	ORR R2, 0b0001 << 4
	STR R2, [R0]


//  Extifall
	LDR R0, =GPIO_BASE + GPIO_EXTIFALL
	LDR R1, [R0]
	ORR R1, 1 << BUTTON_PIN
	STR R1, [R0]


//  Reset interrupt flag
	LDR R1, =GPIO_BASE
	LDR R2, =GPIO_IFC
	ADD R2, R2, R1
	LDR R3, =1<<9
	STR R3, [R2]

//  Enable interrupt
	LDR R0, =GPIO_BASE+GPIO_IEN
	LDR R1, [R0]
	ORR R1, 1 << BUTTON_PIN
	STR R1, [R0]

	B Poll

Poll:
	B Poll


.global SysTick_Handler
.thumb_func
SysTick_Handler:
// 	counting tenths
	LDR R3, =tenths
	LDR R0, [R3]
	ADD R0, #1
	CMP R0, #10
	BNE SysTick_Tenths
	LDR R0, =0

// 	counting seconds
	LDR R4, =seconds
	LDR R1, [R4]
	ADD R1, #1
// 	Led blinking every second
	LDR R6, =GPIO_BASE + LED_PORT * PORT_SIZE + GPIO_PORT_DOUTTGL
	LDR R7, =1<<LED_PIN
	STR R7, [R6]

	CMP R1, #60
	BNE SysTick_Seconds
	LDR R1, =0

// 	counting minutes
	LDR R5, =minutes
	LDR R2, [R5]
	ADD R2, #1

SysTick_Minutes:
	STR R2, [R5]
SysTick_Seconds:
	STR R1, [R4]
SysTick_Tenths:
	STR R0, [R3]
	BX LR

.global GPIO_ODD_IRQHandler
.thumb_func
GPIO_ODD_IRQHandler:
// 	set off clock
	LDR R4, =SYSTICK_BASE + SYSTICK_CTRL
	LDR R5, [R4]
	EOR R5, R5, #1
	STR R5, [R4]

// 	resetting the interrupt flag
	LDR R4, =GPIO_BASE+GPIO_IFC
	LDR R5, =1<<9
	STR R5, [R4]

	BX LR



NOP // Behold denne på bunnen av fila

