[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/niGavbkL)


# **Embedded Pong Game with Wireless IoT Control**

**Platform:** ATmega328PB Xmini + ESP32 Feather S2

**Interfaces:** Joystick, LEDs, Buzzer, DC Motor, Blynk IoT

**Display:** LCD

---

## **Project Overview**

This project implements a complete embedded version of the classic **Pong** game using an ATmega328PB microcontroller, external hardware peripherals, and wireless IoT control via an ESP32 and Blynk.

The system integrates:

* Real-time game graphics on an LCD
* Physical user input through a joystick
* Sound feedback using a buzzer
* Visual feedback using LEDs
* Mechanical feedback using a brushed DC motor
* Wireless control through a mobile application

The result is a fully interactive Pong system that supports both  **wired and wireless gameplay** .

---

## **System Architecture**

The system is built around two microcontrollers:

**ATmega328PB Xmini**

* Handles game logic
* Paddle and ball motion
* LCD rendering
* Buzzer, LED, and motor control

**ESP32 Feather S2**

* Handles Wi-Fi communication
* Interfaces with the Blynk mobile app
* Sends paddle control signals wirelessly

A logic-level shifter is used between the ESP32 (3.3 V logic) and the ATmega (5 V logic).

---

## **Hardware Components**

* ATmega328PB Xmini
* ESP32 Feather S2
* Joystick
* Two LEDs with resistors
* Active or passive buzzer
* Brushed DC motor with flag
* MOSFET with flyback diode
* LCD display
* External 5 V power supply for the motor

---

## **Game Features**

### **Game Modes**

* Player vs Computer
* Two-player mode

A home screen allows selection of the desired game mode before gameplay begins.

---

### **Paddles**

* Two paddles are placed on the left and right edges of the screen
* Paddles move along the vertical axis
* Motion is controlled using one axis of the joystick or wirelessly via Blynk
* Paddle motion is bounded by the screen limits

---

### **Ball**

* The ball starts at the center of the screen
* It moves in a random direction at a random speed
* The ball bounces off:
  * Top and bottom walls
  * Both paddles

---

### **Scoring**

* A scoreboard is displayed on the LCD
* A player scores when the opponent fails to return the ball
* The game resets when a player reaches the selected point limit

---

## **Feedback System**

### **LED Feedback**

* One LED flashes whenever a player scores

### **Buzzer Feedback**

The buzzer generates sound when:

* The ball hits a wall
* The ball hits a paddle
* The ball misses a paddle

### **DC Motor Feedback**

* A DC motor spins a flag when a player wins
* PWM is used to create a smooth acceleration and deceleration animation

---

## **Wireless Control**

Player 1’s paddle can be controlled wirelessly using the **Blynk mobile app** running on the ESP32.

The wireless control:

* Works alongside or instead of the joystick
* Communicates with the ATmega through a logic-level shifter
* Allows real-time paddle movement from a smartphone

---

## **Motor Control Circuit**

The brushed DC motor is driven using a MOSFET and flyback diode for protection.

PWM from the ATmega controls motor speed to create animation when a player wins.
