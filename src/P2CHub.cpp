#include "P2CHub.h"
 
#include "parms.h" 
EEParms eeParms;
#include "netw.h" 

// void setup1() {
//   	pinMode(MY_LED, OUTPUT);


// 	// hubNode.onReceive( handleParentReq2);
// 	// hubNode.onError(uploadError2);
// 	// hubNode.onUpload(upload2);
// 	// hubNode.nodeId = 9;
// 	// hubNode.isParent = false;
// 	// hubNode.pullUp = false;
// 	// hubNode.autoPing = 0;

// 	myTimers.nextTimer(TIMER_LED, 1);
// 	// hubNode.begin(9);
// }

// void loop1(){

// 	if(myTimers.isTime(TIMER_LED)){

// 		gpio_put(MY_LED, !digitalRead(MY_LED));		 

// 		myTimers.nextTimer(TIMER_LED, 3);
// 		// hubNode.trace("hup");

// 		// hubNode.txCmd('S', 45, 77, 0);
// 		// hubNode.txCmd('R', 45, 8);

// 	}

// 	// hubNode.loop();

// }

  
void setup() // TODO
{
// *******************************
  	Serial.begin(115200);  //9600  115200
	// while(!Serial) {} // Wait
 
	eeParms.onUpload(upload);
	eeParms.setup( );

	#ifdef DEBUG
		Serial.println(F("DEBUG P2CHub...")); Serial.flush();
	#else
		Serial.println(F("P2CHub..."));Serial.flush();
	#endif

	int nodeId = NODE_ID;
 
	if(nodeId==0)					//  !!!!!   default node in eeprom
		nodeId=eeParms.nodeId;
	else
		eeParms.setNodeId(nodeId);


	parentNode.onReceive( handleParentReq);
	parentNode.onError(uploadError);
	parentNode.onUpload(upload);
	parentNode.nodeId = nodeId;
	parentNode.isParent = true;					// default false

	childNodes.nodeId = parentNode.nodeId;
	childNodes.onReceive( handleChildReq );

	#if defined (TWI_TO_SERIALS)

		parentNode.begin();

		#ifdef  __AVR_ATmega2560__
			childNodes.setup(0,115200);	
		#endif

		#ifdef DEBUG
			Serial.println(F("twi>ser's"));
		#endif
	#endif

	#if defined (SERIAL_TO_TWIS)

		childNodes.begin(0x09);

		#ifdef  __AVR_ATmega2560__
			parentNode.setup(0,115200);	
		#endif

		#ifdef DEBUG
			Serial.println(F("ser>twi's"));
		#endif	
	#endif
 
	#ifdef KAMER_TEMP_PIN
	tempKamer.tempMin = 1000;
	tempKamer.sensorFrequency_s = 7;
	// tempKamer.loop(3);
	tempKamer.onUpload(upload, 12);
	tempKamer.onError(uploadError);
	// tempKamer.setSyncInterval(eeParms.samplePeriode_sec);
	#endif 



	myTimers.nextTimer(TIMER_TRACE, 1);
	myTimers.nextTimer(TIMER_UPLOAD_ON_BOOT, 0);
	// myTimers.nextTimer(TIMER_TEST, 6);



	// wdt_reset();
	// wdt_enable(WDTO_8S);
}




void loop() // TODO
{
	eeParms.loop();

	childNodes.loop();

	parentNode.loop();

    #if defined(SERIAL_CONSOLE) || defined(SPI_CONSOLE)
		// console.loop();
	#endif

	#ifdef KAMER_TEMP_PIN
		tempKamer.loop();
	#endif


	if( parentNode.isReady() 
	 && ! parentNode.isTxFull()
	){
		if( myTimers.isTime(TIMER_UPLOAD_ON_BOOT)
		){
			switch( uploadOnBootCount )
			{
			case 1:
			    if(millis()<60000) upload(1);
				break;    // boottimerr
 
			case 4: upload(3); break; 	// bootcount
			case 5: upload(50); break; 	// buildin led
			#ifdef KAMER_TEMP_PIN
				case 6: upload(12); break;
			#endif			
			case 7: upload(2); break;   // NodeType
			case 8: upload(8); break;	// version 

			case 30: myTimers.timerOff(TIMER_UPLOAD_ON_BOOT); break;			
			}

			uploadOnBootCount++;
			myTimers.nextTimerMillis(TIMER_UPLOAD_ON_BOOT, TWI_SEND_ERROR_INTERVAL);
		}

		// if( myTimers.isTime(TIMER_UPLOAD_HUMIDITY_ABS)) {
		// 	 upload(DOUCH_HUMIDITY_ABS_ID);
		// 	 myTimers.nextTimer(TIMER_UPLOAD_HUMIDITY_ABS);
		// }
		// if( myTimers.isTime(TIMER_UPLOAD_HUMIDITY_MEDIAN)) {
		// 	 upload(DOUCH_HUMIDITY_MEDIAN_ID);
		// 	 myTimers.nextTimer(TIMER_UPLOAD_HUMIDITY_MEDIAN);
		// }
		// if( myTimers.isTime(TIMER_UPLOAD_FAN)) {
		// 	 upload(FAN_ID);
		// 	 myTimers.nextTimer(TIMER_UPLOAD_FAN);
		// }
	}
 
	#ifdef DEBUG		
		if( myTimers.isTime(TIMER_TRACE)){ trace();}
	#endif

	if( myTimers.isTime(TIMER_TEST)
	 && parentNode.isTxEmpty()
	){ 
		myTimers.nextTimer(TIMER_TEST, 5);
		// tempKamer.trace("temp");
		childNodes.txCmd('R', 45, 8);
	}

	// wdt_reset();
	delay(0);

	 	
}  //end  loop


void trace() // TODO
{
	myTimers.nextTimer(TIMER_TRACE, TRACE_SEC);

	Serial.print("@ ");
	Serial.print( millis()/1000 );
	Serial.print (": ");

	//Serial.print(F(", childActiveErr="));	 Serial.print(activeError);
	Serial.print(F(", nodeId="));	 Serial.print(eeParms.nodeId);
//	Serial.print(F(", ramFree="));Serial.print(System::ramFree()  );
	//Serial.print(F(", TWBR=")); Serial.print(TWBR);
	Serial.println();


	//console.trace("console");
	//if(force)
	{
		parentNode.trace("prnt");
		childNodes.trace("childs");
	//	console.trace("con");
		// childNodes.debugTxBuf("txBufC");
	}


	#ifdef KAMER_TEMP_PIN
		tempKamer.trace("temp");
	#endif
	Serial.flush() ;

}
