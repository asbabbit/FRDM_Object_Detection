
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL46Z4.h"
#include "fsl_debug_console.h"

#define LEFT_MOTOR 0
#define RIGHT_MOTOR 1
#define CW 0
#define CCW 1
#define BREAK 0
#define STOP 1

static volatile unsigned int time_now_ms = 0;
static volatile unsigned int ping_start = 0;

// Setup for motors, encoders, and switches
void motors_setup() {
	SIM->SCGC5 |= (1 << 10 | 1 << 11);

	SIM->SCGC6 |= (1 << 26); // Clock Enable TPM2
	SIM->SOPT2 |= (0x2 << 24); // Set TPMSRC to OSCERCLK
	TPM2->CONTROLS[0].CnV = 600; //Channel 0 is PTB2
	TPM2->CONTROLS[1].CnV = 600; //Channel 1 is PTB3
	TPM2->MOD = 1000;

	PORTB->PCR[0] &= ~0x703;
	PORTB->PCR[1] &= ~0x703;
	PORTB->PCR[2] &= ~0x703;
	PORTB->PCR[3] &= ~0x703;
	PORTC->PCR[1] &= ~0x703;
	PORTC->PCR[2] &= ~0x703;

	TPM2->CONTROLS[0].CnSC |= (1 << 3) | (1 << 5); //Set to Edge PWM
	TPM2->CONTROLS[1].CnSC |= (1 << 3) | (1 << 5);

	TPM2->SC |= 1 << 3;



	PORTB->PCR[0] |= 1 << 8;
	PORTB->PCR[1] |= 1 << 8;
	PORTC->PCR[1] |= 1 << 8;
	PORTC->PCR[2] |= 1 << 8;

	PORTB->PCR[2] |= 3 << 8;
	PORTB->PCR[3] |= 3 << 8;

	GPIOB->PDDR |= 0xF;
	GPIOC->PDDR |= 0x6;
}

void SW1_setup_interrupt() {
	SIM->SCGC5 |= (1<<11);  // Enable Port C Clock
	PORTC->PCR[3] &= ~0xF0703; // Clear First
	PORTC->PCR[3] |= 0xF0703 & ((0xA << 16) | (1 << 8) | 0x3 ); // Set MUX bits, enable pullups, interrupt on falling edge
	GPIOC->PDDR &= ~(1 << 3); // Setup Pin 3 Port C as input

	NVIC_EnableIRQ(31);
}

void PORTC_PORTD_IRQHandler(void) { // Location defined in startup/startup_MKL46Z4.c
	PORTC->PCR[3] |= (1 << 24); // Clear Interrupt Flag!
	GPIOD->PTOR |= (1 << 5); // Toggle!

	int distance;
	int prevDist;
	delay_seconds(2);
	while(1){
		setServo(0);
		distance = getDistance();
		if(distance > 15){
			motor_control(LEFT_MOTOR, CW, 880);
			motor_control(RIGHT_MOTOR, CCW, 880);
			delay_ms(500);
			motor_break(LEFT_MOTOR, STOP);
			motor_break(RIGHT_MOTOR, STOP);
			delay_ms(500);
		}else {
			motor_break(RIGHT_MOTOR, STOP);
			motor_control(LEFT_MOTOR, CW, 880);
			delay_ms(500);
			motor_break(LEFT_MOTOR, STOP);
			motor_break(RIGHT_MOTOR, STOP);
			delay_ms(500);
		}
		setServo(90);
		distance = getDistance();
		if(distance > 15){
			motor_break(LEFT_MOTOR, STOP);
			motor_control(RIGHT_MOTOR, CCW, 880);
			delay_ms(500);
			motor_break(LEFT_MOTOR, STOP);
			motor_break(RIGHT_MOTOR, STOP);
			delay_ms(500);
		}else {
			motor_break(RIGHT_MOTOR, STOP);
			motor_control(LEFT_MOTOR, CW, 880);
			delay_ms(500);
			motor_break(LEFT_MOTOR, STOP);
			motor_break(RIGHT_MOTOR, STOP);
			delay_ms(500);
		}
	}
}



void ultrasensor_setup(void) {
	SIM->SCGC5 |= (1 << 9 | 1 << 12);

	PORTA->PCR[13] &= ~0x703;
	PORTD->PCR[2] &= ~0x703;

	PORTA->PCR[13] |= 1 << 8;
	PORTD->PCR[2] |= 1 << 8;

	GPIOA->PDDR &= ~(1 << 13); //PTA13 as input
	GPIOD->PDDR |= (1 << 2); //PTD2 as output

	SIM->SCGC6 |= (1 << 25); // Clock enable TPM1
	SIM->SOPT2 |= (0x2 << 24); // TPM Clock Source OSCERCLK
	TPM1->MOD = 10000;

	TPM1->SC |= (1 << 7) | (1 << 3) | 3;
}

void servo_setup(void) {
	SIM->SCGC6 |= (1 << 25); // Clock Enable TPM1
	SIM->SOPT2 |= (0x2 << 24); // Set TPMSRC to OSCERCLK
	PORTA->PCR[12] &= ~0x703;
	PORTA->PCR[12] |= 3 << 8; // Drive this pin with TMP1 (ALT3)

	TPM1->CONTROLS[0].CnV = 1300; //Channel 0 is PTA12, CnV value of 500 = 0 degrees, 1000 = , 2500 = 180 deg
	TPM1->MOD = 10000;
	TPM1->CONTROLS[0].CnSC |= (1 << 3) | (1 << 5); //Set to Edge PWM CHECK for edge aligned high true pulses
	TPM1->CONF |= (0x3 << 5);
	TPM1->SC |= (1 << 3 | 3); //Start clock and prescaler =8

	SIM->SCGC5 |= (1 << 9);



	GPIOA->PDDR |= 1 << 12;
}

int getDistance(void){
	int start = 0;
	int end =0;
	int elapsed=0;
	int distance =0;
	int overflow_counter=0;

	GPIOD->PDOR |= (1<<2); // set PTD2 high

	//wait for 10 microseconds
	//count nops
	for(int i=0; i< 480; i++){
		__asm volatile ("nop");
	}

	GPIOD->PDOR &= ~(1<<2); // set PTD2 low

	while(!(GPIOA->PDIR & (1<<13))){ }// wait for echo to go high
	start = TPM1->CNT;	//tStart is the time when echo goes high
	TPM1->SC |= 1<<7; //clear TPM1 TOF flag

	while(GPIOA->PDIR & (1<<13)){ // wait for echo to go low
		if(TPM1->STATUS & 1<<8){ //if TPM1 Overflow flag is set
			overflow_counter++;
			TPM1->SC |= 1<<7; //clear TPM1 TOF flag
		}
	}
	end = TPM1->CNT;

	if(end < start || overflow_counter >1){
		elapsed = ((overflow_counter*10000)-start) +end;
	}
	else
		elapsed = end - start;

	//convert elapsed time to centimeters
	distance = elapsed/58;
	return distance;
}

void setServo(int degree){
	int usec;
	if(degree>180){
		degree=180;
	}
	if(degree<0){
		degree=0;
	}
	if(degree==0){
		usec = 100;
	}
	else{
		usec = 328.57 * 0.043 * degree;
	}
	TPM1->CONTROLS[0].CnV = usec;
}



void delay_ms(unsigned short delay_t){
	SIM->SCGC6 |= (1 << 24); //Clock enable TPM0
	SIM->SOPT2 |= (0x2 << 24); //TPM clock source OSCERCLK
	TPM0->CONF |= (0x1 << 17); // Set rest for overflow
	TPM0->SC = (1 << 7) | (0x7);  //Reset TOF, PRescalar 128

	TPM0->MOD = delay_t*62 + delay_t/2;

	TPM0->SC |= 0x01 << 3; //start clock

	while(!(TPM0->SC & 0x80)){}
	return;
}

void delay_seconds(float itime) {


	   int icompTime = 0;

	   for(icompTime=itime*10000000;icompTime!=0;icompTime--){

	   }
}



void motor_break(char motor, char stp_brk) {
	if((motor == LEFT_MOTOR) & (stp_brk == BREAK)) {
		GPIOB->PDOR |= 0x3;
	}

	if((motor == RIGHT_MOTOR) & (stp_brk == BREAK)) {
		GPIOC->PDOR |= 0x6;
	}

	if((motor == LEFT_MOTOR) & (stp_brk == STOP)) {
		GPIOB->PDOR &= ~0x3;
	}

	if((motor == RIGHT_MOTOR) & (stp_brk == STOP)) {
		GPIOC->PDOR &= ~0x6;
	}
}

void motor_control(char motor, char direction, int speed) {

	if(motor == LEFT_MOTOR) {
		TPM2->CONTROLS[0].CnV = speed; //Channel 0 is PTB2 (left motor)
		if(direction == CW) {
			GPIOB->PDOR &= ~(1<<1);
			GPIOB->PDOR |= (1<<0);
		}

		if(direction == CCW) {
			GPIOB->PDOR &= ~0x1;
			GPIOB->PDOR |= 0x2;
		}
	}

	if(motor == RIGHT_MOTOR) {
		TPM2->CONTROLS[1].CnV = speed; //Channel 1 is PTB3 (right motor)
		if(direction == CW) {
			GPIOC->PDOR &= ~0x4;
			GPIOC->PDOR |= 0x2;
		}

		if(direction == CCW) {
			GPIOC->PDOR &= ~0x2;
			GPIOC->PDOR |= 0x4;
		}
	}
}

int main(void)
{

  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
  	/* Init FSL debug console. */
    BOARD_InitDebugConsole();

    SW1_setup_interrupt();
//	sw2_setup();
    motors_setup();

	servo_setup();
	ultrasensor_setup();



	while(1){
		__asm volatile ("nop");
	}
	return 0;
}
