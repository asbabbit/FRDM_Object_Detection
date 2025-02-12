# FRDM_Object_Detection
Objective

The goal of this project is to implement an obstacle avoidance system using an ultrasonic sensor and a servo motor. The robot detects obstacles in its path and makes navigation decisions based on sensor input. If an obstacle is detected while moving forward, it turns right. If an object is detected on the left, the robot turns right.
Hardware Components

    Microcontroller: FRDM board
    Motors: 2 DC motors with motor driver
    Servo Motor: Controls sensor scanning direction
    Ultrasonic Sensor: Detects obstacles
    Additional Components: Jumper wires, power supply

Implementation Details
Motor Setup

    Enable clock gating for PORTB and PORTC.
    Configure TPMSRC to OSCERCLK.
    Set the duty cycle for motor control using TPM0.
    Assign PortB3 and PortB2 as motor control outputs.
    Start the clock using PCR1 and PCR2.

Switch (SW1) Configuration

    Enable PORTC clock.
    Configure pull-ups and set an interrupt on the falling edge.
    Set PCR3 with the appropriate bitmask to enable the interrupt.
    Implement an IRQHandler to trigger on switch press.

Ultrasonic Sensor Configuration

    Enable clock gating for PORTD and PORTA.
    Configure PortA13 as input (echo signal) and PortD2 as output (trigger signal).
    Enable TPM1 and set its clock source to OSCERCLK.

Servo Motor Configuration

    Enable clock gating for TPM1 and configure it with OSCERCLK.
    Set PortA12 as the servo control output using TPM1.
    Configure duty cycle, prescaler, and modulation values for precise servo positioning.
    Start the clock using TPM1->SC.

Control Algorithm
Interrupt Service Routine (PortC IRQHandler)

    Set servo to 0°
        If distance > threshold, move forward.
        Else, turn left.
    Set servo to 90°
        If distance > threshold, turn left.
        Else, turn right.

Distance Measurement (Get Distance Function)

    Set PTD2 (Trigger) HIGH and then LOW after a short delay.
    Wait for echo signal (PTA13) to go HIGH and record tStart.
    Clear TPM1 TOF flag.
    Wait for echo signal to go LOW and compute travel time.
    If TPM1 overflow flag is set, clear it to prevent timing errors.

Servo Motor Control (Set Servo Function)

    Constrain angle values between 0° and 180°.
    Adjust duty cycle proportionally:
        0° → 100% duty cycle
        Other angles → degree × scaling factor
