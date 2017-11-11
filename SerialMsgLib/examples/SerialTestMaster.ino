#include "Arduino.h"
#include <tools.h>
#include <SerialMsg.h>

SerialTx stxSerial (new SoftSerialPort(10,11));

byte data[] = {55,99,88,44}; // some data

void setup()
{
	 Serial.begin(9600);
	 MPRINTLN("setup SerialTestMaster");
	 stxSerial.begin(9600);
}


void loop()
{
	stxSerial.sendData(data,sizeof data);
	delay(1000);

}





