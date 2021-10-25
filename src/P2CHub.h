
#define JL_VERSION 10030
// #define DEBUG
/*

2021-08-26 0.01.00 start with pico-i2c and pico-bme280 
*/



/* select one of below configurations */
#define SERIAL_TO_TWIS
// #define TWI_TO_SERIALS
 
#include "arduino.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
// #include "hardware/spi.h"
// #include "pico/binary_info.h"
#include "DS18B20.h"
#include <NetwSerial.h> 
#include <NetwPico.h>
#include <MyTimers.h>
#include <EEUtil.h>

//#define SERIAL_CONSOLE 77
//#include <NetwSerial.h>
//NetwSerial console;

//#define SPI_CONSOLE 77
//#include <NetwSPI.h>
//NetwSPI  console ;

// !!!!!   set 11 or 12 if conn stay 0



//#define NODE_ID 0    //  nodeID in eeprom !!!!!!
#define TRACE_SEC 3

// #define NODE_11 11

#define NODE_ID 13
#define NODE_TYPE 11

#if NODE_ID == NODE_11
#define KAMER_TEMP_PIN 5
// #define RC_SWITCH_PIN 10
#endif

 
#if defined (TWI_TO_SERIALS)
	NetwPico parentNode(  D4    );
	NetwSerial childNodes;
	#define TWI_PARENT				// replaced by ISR_PARENT_TWI
	#define ISR_PARENT_TWI
#endif

// NetwPico parentNode(  D2    ); // SDA D2 defaults to port 1 ! same as: NetwPico parentNode( 1, D2, D3   );
// NetwPico parentNode(  D2, D3, D12, D13    ); // SDA D2 defaults to port 1 ! same as: NetwPico parentNode( 1, D2, D3   );
// NetwPico hubNode( 0, D12, D13  );
// !! serialmon to CR/NL    nano  com9  optiboot  {S,13,50,13,1}
  
#if defined (SERIAL_TO_TWIS)
	/* SERIAL > TWI (RPI TWI_HEAD)  */	
	NetwSerial parentNode;
	NetwPico childNodes(  D4    );
	#define TWI_CHILDS				// replaced by ISR_CHILDS_TWI
	#define ISR_CHILDS_TWI
#endif


#define TIMERS_COUNT 3
MyTimers myTimers(TIMERS_COUNT);
#define TIMER_TRACE 0
#define TIMER_TEST 1
#define TIMER_UPLOAD_ON_BOOT 2 


/*  TWI > SERIAL  (DIMMER_HUB) COM9   */
//#define NODE_ID 18
//#define HN_SLAVE 18
//#define TWI_PARENT
//#include <NetwTWI.h>
//NetwTWI parentNode;
//#define UPLOAD_ID 9
//#define SERIAL_CHILDS
//#include <NetwSerial.h>
//NetwSerial childNodes;


/*  TCP > TWI_REPEATER   MEGA HUB COM21  */
//extern HardwareSerial Serial;  // for MEGA
//#define TCPIP_PARENT
//#define NODE_ID 10
//#include <NetwTCP.h>
//NetwTCP parentNode;
//#define TWI_CHILDS
//#define TWI_ADDR 9
//#include <NetwTWI.h>
//NetwTWI childNodes;
//
//#define SERIAL_CONSOLE 77
//#include <NetwSerial.h>
//NetwSerial console;


/*  TWI > SPI (DIMMER_HUB)   */
//#define NODE_ID 19
//#define HN_SLAVE 19
//#define TWI_PARENT
//#include <NetwTWI.h>
//NetwTWI parentNode;
//#define SPI_CHILDS
//#include <NetwSPI.h>
//NetwSPI childNodes;
//#define SPI_MASTER false
//#define SPI_PIN 9


/* SERIAL > SPI (TEST SPI_IHEAD)  */
//#define NODE_ID 19
//#define SERIAL_PARENT
//#include <NetwSerial.h>
//NetwSerial parentNode;
//#define SPI_CHILDS
//#include <NetwSPI.h>
//NetwSPI childNodes;
 


/*

TODO send Child send and read stats
2021-09-22 1.0.30 debuged timming issue fixed NetwBase volatile unsigned long timers 

2021-01-09 refactor to latest

2019-04-29 reset millis()
2017-05-05 dimmer hub TWI_FREQ 200000L
2017-05-04 nano's TWI_FREQ 200000L

2017-03-31 refactor everything parent and child

Serial.flush() ;

2017-02-13 TWI_FREQ 100000L
 	 	   NetwSerial

2017-02-12 replace Netw by NetwBase
           upgrade NetwI2C to NetwBase
		   TWI_FREQ 25000L
		   optimize eeparm.loop

2017-02-11 //bootPeriode3
		   //hang if sentHealthCount > 3
           NetwBase for I2C
           parentNW.TwoWire::setTimeout(1);
           if(req->data.cmd == 'S') msgSize = sizeof(Set);

2017-01-15 new Parms

2017-01-12 add bootCount
           retry
		   HomeNetH.h 2017-01-10  refactor bootTimer
           new EEProm class replacing EEParms and Parms
		   System::ramFree()
		   set netw.netwTimer   moved to NETW class
		   HomeNetH.h 2017-01-10  netw.sendData retry on send error
		   ArdUtils.h 2017-01-12 fix signatureChanged

2017-01-08 HomeNetH.h 2017-01-08 volatile

2017-01-07 add netw.execParms(cmd, id, value) in handleRpiCmd.localExeParms
           HomeNetH.h 2017-01-06
 *
 * */
 
int 	uploadOnBootCount=0;
 

#ifdef KAMER_TEMP_PIN
	DS18B20 tempKamer(KAMER_TEMP_PIN);
#endif

void localSetVal(int id, long val);
int  upload(int id, long val, unsigned long timeStamp);
int  upload(int id, long val) ;
int  upload(int id);
int  uploadError(int id, long val);
int  handleParentReq( RxItem *rxItem) ;
int localRequest(RxItem *rxItem);
void trace( ); 