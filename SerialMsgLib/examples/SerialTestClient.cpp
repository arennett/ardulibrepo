#include "Arduino.h"
#include <SoftwareSerial.h>



#include <SerialMsg.h>

#include <tools.h>


// receive data on PIN 10,11 max 16 Bytes
SerialRx serialRx(new SoftSerialPort(10,11), 16);
void callBack(byte* pData, size_t size);

//The setup function is called once at startup of the sketch
void setup() {
	Serial.begin(9600);
	serialRx.begin(9600);

	// for non waited callback
	serialRx.setUpdateCallback(callBack);

}

// The loop function is called in an endless loop

size_t dataSize;
byte* rpDat;

void loop() {

	// for nonwaited receiving, readNext is called in every loop
	// also you have to set a callback function, see setup
	serialRx.readNext();


	// waited receiving
	MPRINTLN("waiting ...");
	if (serialRx.waitOnMessage(rpDat, dataSize, 5000)) {
		MPRINTSVAL("receive message on wait ...size: ", dataSize);
		for (size_t i = 0; i < dataSize; i++) {
			MPRINT(i);
			MPRINTS(" : ");MPRINTLN(rpDat[i]);
		}

	} else {
		MPRINTLN("timeout ...");
	}
	MPRINTFREE
	;
}

void callBack(byte* pData, size_t size) {

	MPRINTSVAL("receive message on non wait  ...size: ", size);

	for (size_t i = 0; i < size; i++) {
		MPRINT(i);
		MPRINTS(" : ");MPRINTLN(pData[i]);
	}

}
