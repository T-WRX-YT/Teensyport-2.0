#include "dataParser.h"
#include "globalVars.h"






// found on stack, wanky way of converting a string containing two integers received via serial into two different ints.  this sucks
void processOil (char *t) {
  char c;
  int  val;
  sscanf (t, "%c%d", &c, &val);

  switch (c) {
  case 'a':
      if (verbose) {
        Serial.print ("[VERBOSE] cmd a ");
        Serial.println (val);
      }
      oilTemperature = (val);
      break;

  case 'b':
      if (verbose) {
        Serial.print ("[VERBOSE] cmd b ");
        Serial.println (val);
      }
      // write the second value as oil pressure
      if (val < 0) { val = 0; }
      oilPressure = (val);
      break;

  default:
      //if (verbose) {
        Serial.print ("[VERBOSE] unknown cmd ");
        Serial.println (val);
      //}
      break;
  }
}

float calcFloatFull(unsigned char data[4], float multiplier) {
  	if (verbose) {
      Serial.print("[VERBOSE] Input data: ");
      for(int z = 0; z < 4; z++) {
        Serial.print(data[z], HEX);
      }
      Serial.println(); 
    }
  
  
	floatUnion converter; 
  
    for (int i = 0; i < 4; i++) {
      converter.byteArray[i] = data[i];
    }
  
  	float calc = converter.floatValue * multiplier;
  	return calc;  	
}

int calcTemp(unsigned char data) {
  	if (verbose) {
      Serial.print("[VERBOSE] Input data: ");
      Serial.println(data, HEX);
    }

	int calc = 32+((9*((data)-40))/5);
	return calc;		
}

float calcByteToFloat(unsigned char data, float multiplier) {
  	if (verbose) {
      Serial.print("[VERBOSE] Input data: ");
      Serial.println(data, HEX);
    }

	float calc = data * multiplier;
	return calc;
}

int calcIntFull(unsigned char data[2], float multiplier) { 	
  	if (verbose) {
      Serial.print("[VERBOSE] Input data: ");
      for(int z = 0; z < 2; z++) {
        Serial.print(data[z], HEX);
      }
      Serial.println();
    }

	floatUnion converter; 
  
    for (int i = 0; i < 2; i++) {
      converter.byteArray[i] = data[i];
    }
  
  	int calc = converter.intValue * multiplier;
  	return calc;
}


float calcTargetBoost(unsigned char data[2]) {
  if (verbose) {	
    Serial.print("[VERBOSE] Input data: ");
  	  for(int z = 0; z < 2; z++) {
  		  Serial.print(data[z], HEX);
      }
  	  Serial.println();
  }
  
	floatUnion converter; 

  converter.byteArray[0] = data[0];
  converter.byteArray[1] = data[1];
  converter.byteArray[2] = 0x00;
  converter.byteArray[3] = 0x00;

  float calc = converter.intValue;
  calc = (calc - 760) * 0.01933677;


	return calc;	
}


int calcAvcs(unsigned char data) {
  if (verbose) {
    Serial.print("[VERBOSE] Input data: ");
  	Serial.println(data, HEX);
  	Serial.println();
  }

	int calc = data - 50;
	return calc;
}

float calcTiming(unsigned char data) {
  if (verbose) {
    Serial.print("[VERBOSE] Input data: ");
  	Serial.println(data, HEX);
  	Serial.println();
  }
	
	float calc = (data - 128) / 2;
	return calc;
}

int calcByteToInt(unsigned char data) {
  	if (verbose) {
      Serial.print("[VERBOSE] Input data: ");
      Serial.println(data, HEX);
    }

	int calc = data;
	return calc;
}

float calcAfCorrection(unsigned char (data)) {
  if (verbose) {
    Serial.print("[VERBOSE] Input data: ");
  	Serial.println(data, HEX);
  	Serial.println();
  }

	float calc = ((data - 128) * 100) / 128;
	return calc;
}

float calcThrottle(unsigned char data) {
  if (verbose) {
    Serial.print("[VERBOSE] Input data: ");
  	Serial.println(data, HEX);
  }
	
	float calc = (data * 100) / 255;
  //if (calc <= 14) { calc = 0; } // account for idle throttle position
	return calc;
}	

float calcInjDutyCycle(unsigned char data) {
  if (verbose) {
    Serial.print("[VERBOSE] Input data: ");
  	Serial.println(data, HEX);
  	Serial.println();
  }

	float calc = (data * 256) / 1000;
	return calc;
}

float calcAfr(unsigned char data) {
  if (verbose) {
    Serial.print("[VERBOSE] Input data: ");
  	Serial.println(data, HEX);
  }

	float calc = (data / 128) * 14.7;
	return calc;
}
