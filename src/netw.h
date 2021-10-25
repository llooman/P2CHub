 void localSetVal(int id, long val)
{

	Serial.print("set "); Serial.print(id);Serial.print(" to "); Serial.println(val);

	switch(id )
	{

	default:
		eeParms.setVal( id,  val);
		parentNode.setVal( id,  val);
		break;
	}
}

int upload(int id)
{
	int ret = 0;

	switch( id )
	{
 	case 2:
		upload(id, NODE_TYPE );   
		break;			
	case 8:
		upload(id, JL_VERSION );   
		break;

	#ifdef VOLTAGE_PIN
		case 11: upload(id, vin.val);    		break;
	#endif

	#ifdef KAMER_TEMP_PIN
		case 12: tempKamer.upload(); break;
		case 20: tempKamer.upload(); break;
	#endif

	default:
		if( 1==2
		 ||	parentNode.upload(id)>0
		 ||	childNodes.upload(id)>0
		 ||	eeParms.upload(id)>0
		){}
		break;
	}
	return ret;
}

 

int upload(int id, long val) { return upload(id, val, millis()); }
int upload(int id, long val, unsigned long timeStamp)
{
	return parentNode.txUpload(id, val, timeStamp);
}
int uploadError(int id, long val)
{
	#ifdef DEBUG
		Serial.print("err-");Serial.print(id);Serial.print(": ");Serial.println(val);
	#endif	
	return parentNode.txError(id, val);
}


 


int handleChildReq(RxItem *rxItem)
{
	#ifdef DEBUG
		// childNodes.debug("child<", rxItem);
	#endif

	if(rxItem->data.msg.node==2 || rxItem->data.msg.node == parentNode.nodeId)
	{
		return localRequest	(rxItem);
	}

	#ifdef DEBUG
		// childNodes.debug("upward", rxItem);
		//return 0;
	#endif

	return parentNode.putTxBuf(rxItem);


}



int handleParentReq( RxItem *rxItem)   
{
	#ifdef DEBUG
		parentNode.debug("Prnt<", rxItem);
	#endif

	if( rxItem->data.msg.node == 2 
	 || rxItem->data.msg.node == parentNode.nodeId
	){
		return localRequest(rxItem);
	}

	if(parentNode.nodeId==0)
	{
		#ifdef DEBUG
			parentNode.debug("skip", rxItem);
		#endif
		return 0;
	}

	#ifdef DEBUG
		parentNode.debug("forward", rxItem);
	#endif

	#ifndef SKIP_CHILD_NODES
		return childNodes.putTxBuf( rxItem );
	#endif

	return 0;
}



int localRequest(RxItem *rxItem)
{
	#ifdef DEBUG
		parentNode.debug("local", rxItem);
	#endif

	int ret=0;
	int subCmd;
	int val;		

	switch ( rxItem->data.msg.cmd)
	{
	case 'L':
		switch (rxItem->data.msg.id)
		{
		case 1:
			trace(); 
			break;
		
		// case '0':parentNode.resetPayLoad( );   break;

		}
		break;

	case 'l':
	    subCmd = rxItem->data.msg.val & 0x000000ff ;
    	val = rxItem->data.msg.val >> 8;
		switch (rxItem->data.msg.id)
		{
		case 1:
			parentNode.localCmd(subCmd, val);
			break;
		case 2:
			childNodes.localCmd(subCmd, val);
			break;	
		// case 'x':childNodes.tw_restart();  break;
		// 	break;
 		// case 2:
		// 	mhz19b.localCmd(subCmd, val);
		// 	break;
		}
		break; 

	case 's':
	case 'S':
		localSetVal(rxItem->data.msg.id, rxItem->data.msg.val);
		upload(rxItem->data.msg.id);
		break;
	case 'r':
	case 'R':
		upload(rxItem->data.msg.id);
		break;
	case 'b':
		// eeParms.resetBootCount();
 		upload(3);
		break;
	case 'B':
		// wdt_enable(WDTO_15MS);
		// while(true){
		// 	delay(500);
		// 	asm volatile ("  jmp 0");
		// }
		break;

	default:
		eeParms.handleRequest(rxItem);
		break;
	}

	return ret;
}
