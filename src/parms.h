
class EEParms: public EEUtil  //TODO
{
public:
	int  nodeId	= 0;
	long chk1  = 0x00010203;
	virtual ~EEParms(){}
	EEParms()
	{
		//EEProm::parent = &parentNode  ;
	}
	void setNodeId( long newVal)
	{
		if( newVal < 0 || nodeId == (int) newVal) return;
		nodeId = (int)newVal;
		changed = true;
	}
    void setup(  ) //
    {
		if( readLong(offsetof(EEParms, chk1)) == chk1  )
    	{
    		readAll();
    		nodeId = readInt(offsetof(EEParms, nodeId));
    		changed = false;
			#ifdef DEBUG
				Serial.println(F("EEProm.readAll"));
			#endif
    	}
		else
		{
			bootCount=0;
		}
    }

	void loop()
	{
		if(changed)
		{
			#ifdef DEBUG
				Serial.println(F("Parms.loop.changed"));
			#endif
			write(offsetof(EEParms, nodeId), nodeId);
			write(offsetof(EEParms, chk1), chk1);
			EEUtil::writeAll();
			changed = false;
			resync();
		}

//		EEProm::loop();
	}
	void resync()
	{
		#ifdef DEBUG
			Serial.println(F("parms.resync"));
		#endif
	}
};