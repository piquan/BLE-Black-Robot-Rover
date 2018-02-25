// Erin and Joel's Robot
//
// This is code that's based on the Adafruit Adabox 002; see
// https://learn.adafruit.com/adabox002/ .  I've cleaned it up,
// changed it to my own whims, and added many new bugs.
//
// The Adafruit copyright notice is as follows:
//
//   This is an example for our nRF51822 based Bluefruit LE modules
//
//   Modified to drive a 3-wheeled BLE Robot Rover! by http://james.devi.to
//
//   Pick one up today in the Adafruit shop!
//
//   Adafruit invests time and resources providing this open source code,
//   please support Adafruit and open-source hardware by purchasing
//   products from Adafruit!
//
//   MIT license, check LICENSE for more information
//   All text above, and the splash screen below must be included in
//   any redistribution

#include <Arduino.h>
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>

#include "BluefruitConfig.h"

#include <Adafruit_MotorShield.h>

//
// Utilities
//

// A small helper
void
error(const __FlashStringHelper* err)
{
    Serial.println(err);
    while (1);
}

//
// Motor
//

// Create the motor shield object with the default I2C address.
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

// Connect 2 DC motors to port M3 & M4.
Adafruit_DCMotor *L_MOTOR = AFMS.getMotor(3);
Adafruit_DCMotor *R_MOTOR = AFMS.getMotor(4);

// Set the forward, reverse, and turning speeds.
#define ForwardSpeed                255
#define ReverseSpeed                255
#define TurningSpeed                100

//
// Bluetooth
//

// Name your RC here
String BROADCAST_NAME = "Adafruit Black Robot Rover";

String BROADCAST_CMD = String("AT+GAPDEVNAME=" + BROADCAST_NAME);

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ,
                             BLUEFRUIT_SPI_RST);

// Function prototypes for functions in packetparser.cpp.
uint8_t readPacket(Adafruit_BLE* ble, uint16_t timeout);
float parsefloat(uint8_t* buffer);
void printHex(const uint8_t* data, const uint32_t numBytes);

// The packet buffer.
extern uint8_t packetbuffer[];

// The transmit buffer.
char buf[60];

void
BLEsetup()
{
    Serial.print(F("Initializing the Bluefruit LE module: "));

    if (!ble.begin(VERBOSE_MODE)) {
        error(F("Couldn't find Bluefruit.  Make sure it's in "
                "command mode, and check wiring."));
    }
    Serial.println(F("Ok."));

    // Perform a factory reset to make sure everything is in a known state.
    Serial.println(F("Performing a factory reset."));
    if (!ble.factoryReset()) {
        error(F("Couldn't factory reset."));
    }
    // Convert the name change command to a char array.
    BROADCAST_CMD.toCharArray(buf, 60);

    // Change the broadcast device name.
    if (ble.sendCommandCheckOK(buf)) {
        Serial.println("Name changed.");
    }
    delay(250);

    // Reset to take effect.
    if (ble.sendCommandCheckOK("ATZ")) {
        Serial.println("Resetting.");
    }
    delay(250);

    // Confirm the name change.
    ble.sendCommandCheckOK("AT+GAPDEVNAME");

    // Disable command echo from Bluefruit.
    ble.echo(false);

    Serial.println("Requesting Bluefruit info:");
    // Print Bluefruit information.
    ble.info();

    Serial.println(F("Please use Adafruit Bluefruit LE app to "
                     "connect in Controller mode"));
    Serial.println(F("Then activate/use the sensors, color picker, "
                     "game controller, etc!"));
    Serial.println();

    ble.verbose(false); // Debug info is a little annoying after this point!

    // Wait for a connection.
    while (!ble.isConnected()) {
        delay(500);
    }

    // Set Bluefruit to DATA mode.
    Serial.println(F("***********************"));
    Serial.println(F("Switching to DATA mode."));
    ble.setMode(BLUEFRUIT_MODE_DATA);
    Serial.println(F("***********************"));
}

//
// Music
//

#define toneC      1911
#define toneC1     1804
#define toneD      1703
#define toneEb     1607
#define toneE      1517
#define toneF      1432
#define toneF1     1352
#define toneG      1276
#define toneAb     1204
#define toneA      1136
#define toneBb     1073
#define toneB      1012
#define tonec       955
#define tonec1      902
#define toned       851
#define toneeb      803
#define tonee       758
#define tonef       716
#define tonef1      676
#define toneg       638
#define toneab      602
#define tonea       568
#define tonebb      536
#define toneb       506

#define tonep         0

int speaker = A1;
long vel = 20000;
boolean hasplayed = false;

int melod[] = {
    tonec, toneG, toneE, toneA, toneB, toneBb, toneA, toneG, tonee,
    toneg, tonea, tonef, toneg, tonee, tonec, toned, toneB
};
int ritmo[] = { 18, 18, 18, 12, 12, 6, 12, 8, 8, 8, 12, 6, 12, 12, 6, 6, 6 };

void
tocar(int tom, long tempo_value)
{
    long tempo_gasto = 0;
    while (tempo_gasto < tempo_value) {
        digitalWrite(speaker, HIGH);
        delayMicroseconds(tom / 2);

        digitalWrite(speaker, LOW);
        delayMicroseconds(tom / 2);
        tempo_gasto += tom;
    }
}

//
// Controller
//

bool isMoving = false;
unsigned long lastPress = 0;

bool
readController()
{
    uint8_t maxspeed;

    // Buttons
    if (packetbuffer[1] == 'B') {

        uint8_t buttnum = packetbuffer[2] - '0';
        boolean pressed = packetbuffer[3] - '0';

        if (pressed) {
            if (buttnum == 1) {
                if (hasplayed == true) {
                    return;
                }
                for (int i = 0; i < 17; i++) {
                    int tom = melod[i];
                    int tempo = ritmo[i];
                    long tvalue = tempo * vel;

                    tocar(tom, tvalue);
                    delayMicroseconds(1000);
                }

                hasplayed = true;
            }

            if (buttnum == 2) {

            }

            if (buttnum == 3) {

            }

            if (buttnum == 4) {

            }

            if (buttnum == 5) {
                isMoving = true;
                L_MOTOR->run(FORWARD);
                R_MOTOR->run(FORWARD);
                maxspeed = ForwardSpeed;
            }

            if (buttnum == 6) {
                isMoving = true;
                L_MOTOR->run(BACKWARD);
                R_MOTOR->run(BACKWARD);
                maxspeed = ReverseSpeed;
            }

            if (buttnum == 7) {
                isMoving = true;
                L_MOTOR->run(RELEASE);
                R_MOTOR->run(FORWARD);
                maxspeed = TurningSpeed;
            }

            if (buttnum == 8) {
                isMoving = true;
                L_MOTOR->run(FORWARD);
                R_MOTOR->run(RELEASE);
                maxspeed = TurningSpeed;
            }

            lastPress = millis();

            // Speed up the motors.
            for (int speed = 0; speed < maxspeed; speed += 5) {
                L_MOTOR->setSpeed(speed);
                R_MOTOR->setSpeed(speed);
                delay(5);	// 250ms total to speed up
            }
        } else {
            isMoving = false;
            // Slow down the motors.
            for (int speed = maxspeed; speed >= 0; speed -= 5) {
                L_MOTOR->setSpeed(speed);
                R_MOTOR->setSpeed(speed);
                delay(5);	// 50ms total to slow down
            }
            L_MOTOR->run(RELEASE);
            R_MOTOR->run(RELEASE);
            hasplayed = false;
        }
    }
}

//
// Entry points
//

// Sets up the hardware and the Bluetooth LE module.  This function is
// called automatically on startup.
void
setup(void)
{
    Serial.begin(9600);

    AFMS.begin();        // Create with the default frequency, 1.6KHz.

    // Configure the motors.
    L_MOTOR->setSpeed(0);
    L_MOTOR->run(RELEASE);

    R_MOTOR->setSpeed(0);
    R_MOTOR->run(RELEASE);

    Serial.begin(115200);
    Serial.println(F("Adafruit Bluefruit Robot Controller Example"));
    Serial.println(F("-------------------------------------------"));

    // Initialize the module.
    BLEsetup();

    pinMode(speaker, OUTPUT);
}

void
loop(void)
{
    // Read new packet data.
    uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);

    readController();
}
