#include "Arduino.h"
#include <digitalWriteFast.h>  // library for high performance reads and writes by jrraines
#include <SPI.h>
#include <Ethernet.h>

#include <ArdOSC.h>                               // see http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1267553811/0
															 // and http://code.google.com/p/digitalwritefast/
 
// It turns out that the regular digitalRead() calls are too slow and bring the arduino down when
// I use them in the interrupt routines while the motor runs at full speed creating more than
// 40000 encoder ticks per second per motor.
 
// Quadrature encoders
// Left encoder
#define c_LeftEncoderInterrupt 0
#define c_LeftEncoderPinA 2
#define c_LeftEncoderPinB 3
#define LeftEncoderIsReversed
volatile bool _LeftEncoderBSet;
volatile long _LeftEncoderTicks = 0;
unsigned int calcDegrees;
unsigned int lastReportedPos = 1;
byte myMac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte myIp[]  = { 192, 168, 0, 177 }; 
int destPort=12000;
byte destIp[]  = { 192, 168, 0, 2 };

OSCClient client;
OSCMessage global_mes;
 
 
void setup()
{
	Serial.begin(115200);

 
	// Quadrature encoders
	// Left encoder
	pinMode(c_LeftEncoderPinA, INPUT);      // sets pin A as input
	digitalWrite(c_LeftEncoderPinA, LOW);  // turn on pullup resistors
	pinMode(c_LeftEncoderPinB, INPUT);      // sets pin B as input
	digitalWrite(c_LeftEncoderPinB, LOW);  // turn on pullup resistors
	attachInterrupt(c_LeftEncoderInterrupt, HandleLeftMotorInterruptA, RISING);
 

}
 
void loop()
{
 if(_LeftEncoderTicks>1024){
	 _LeftEncoderTicks=0;
 }
 if(_LeftEncoderTicks<0){
	 _LeftEncoderTicks=1024;
 }
 calcDegrees=map(_LeftEncoderTicks, 0, 1024, 0, 360);
 
	if (lastReportedPos != calcDegrees) {
		lastReportedPos = calcDegrees; 
		global_mes.setAddress(destIp,destPort);
		global_mes.beginMessage("/washingline/spin");
		global_mes.addArgInt32(calcDegrees);
		client.send(&global_mes);
		global_mes.flush(); //object data clear
	}
}
 
// Interrupt service routines for the left motor's quadrature encoder
void HandleLeftMotorInterruptA()
{
	// Test transition; since the interrupt will only fire on 'rising' we don't need to read pin A
	_LeftEncoderBSet = digitalReadFast(c_LeftEncoderPinB);   // read the input pin
 
	// and adjust counter + if A leads B
	#ifdef LeftEncoderIsReversed
		_LeftEncoderTicks -= _LeftEncoderBSet ? -1 : +1;
	#else
		_LeftEncoderTicks += _LeftEncoderBSet ? -1 : +1;
	#endif
}
 

