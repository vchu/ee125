#include "_MoteComm.h"


/*! Function that takes care of receiving data from the Mote and creates a
    process the received packet.
*/
void ReceiveFromMote(void* parm)
{
  TCHAR *m_sComPort = TEXT(MCOMM_PORT);

  //--------------------------------------------------------
	// SETTING THE PORT
  //--------------------------------------------------------
  BOOL     m_bPortReady;
	HANDLE   m_hCom;
	DCB      m_dcb;
	BOOL     bReadRC;
	DWORD iBytesRead;

	m_hCom = CreateFile(m_sComPort, 
			GENERIC_READ | GENERIC_WRITE,
			0, // exclusive access
			0,//NULL, // no security
			OPEN_EXISTING,
			NULL,//0, // no overlapped I/O
			0);//NULL); // null template 

	m_bPortReady = SetupComm(m_hCom, 128, 128); // set buffer sizes

	m_bPortReady = GetCommState(m_hCom, &m_dcb);
	m_dcb.BaudRate =115200;
	m_dcb.ByteSize = 8;
	m_dcb.Parity = NOPARITY;
	m_dcb.StopBits = ONESTOPBIT;
	m_dcb.fAbortOnError = TRUE;

	m_bPortReady = SetCommState(m_hCom, &m_dcb);

  //--------------------------------------------------------
  // READING DATA
  //--------------------------------------------------------
  unsigned char packet_data[MAX_PACKET_LEN];
  int packet_length;
  unsigned char c = 0;
  unsigned char c_prev = 0;

	unsigned char buf[MAX_PACKET_LEN];
	int bufSz;
	int moteID;
	int pktType;

	while (1) {
	  // Find packet start delimiter 7E 45 = 126 69
	  while(1) {
      bReadRC = ReadFile(m_hCom, &c, 1, &iBytesRead,NULL);
      if ((c == 69) && (c_prev == 126)){ break; }
	    c_prev = c;
	  }

    // Extracting rest of packet
	  packet_length = 0;
	  while(1)
    {
      bReadRC = ReadFile(m_hCom, &c, 1, &iBytesRead,NULL);
      if (c == 126){ break; }
	    else if (c_prev == 125)
      { // Special char 7D
	      if (c == 93)
        { // Saw sequence 7D 5D, convert to 7D
		      packet_data[packet_length-1] = 125;
	      }
        else if (c == 94)
        { // Saw sequence 7D 5E, convert to 7E
		      packet_data[packet_length-1] = 126;
	      }
	    }
      else
      {
	      packet_data[packet_length] = c;
	      packet_length++;
	    }
	    c_prev = c;

      if (packet_length >= MAX_PACKET_LEN){ break; }
	  }

		//for(int i = 0; i<packet_length; i++)
		//	printf(" %d",(int)packet_data[i]);
		//printf("\n");

	  // Parse packet
	  ParseMotePacket(packet_data, buf, bufSz, moteID, pktType);
		if(MCOMM_DISPLAY)
			printf("Mote: %d | PktType: %d | PktSize: %d\n",moteID+1,pktType,bufSz);

		// Process packet. This will probably have to be a thread.
		ProcessPacket(moteID,pktType,buf,bufSz);
  }

	CloseHandle(m_hCom);
}


/*! Function parsing the payload and other necessary packet from the mote
    message.

		\param msg Pointer to the mote message.
		\param buf Pointer to the payload to be extracted.
		\param bufSz Size of the payload.
		\param moteID Mote Id to be returned.
		\param pktType Type of packet to be returned.
*/
void ParseMotePacket(xshort* msg, xshort* buf, int& bufSz, int& moteID, int& pktType)
{
	moteID = (int)msg[4]-1;
	pktType = (int)msg[12];
	bufSz = (int)msg[20];
	memcpy(buf,msg+21,bufSz);
}

void SendStopSync() 
{
  TCHAR *m_sComPort = TEXT(MCOMM_PORT);

	// TODO: Add your control notification handler code here
	BOOL     m_bPortReady;
	HANDLE   m_hCom;
	DCB      m_dcb;
	BOOL     bWriteRC;
	DWORD iBytesWritten;

	m_hCom = CreateFile(m_sComPort, 
			GENERIC_READ | GENERIC_WRITE,
			0, // exclusive access
			0,//NULL, // no security
			OPEN_EXISTING,
			NULL,//0, // no overlapped I/O
			0);//NULL); // null template 

	m_bPortReady = SetupComm(m_hCom, 256, 256); // set buffer sizes

	m_bPortReady = GetCommState(m_hCom, &m_dcb);
	m_dcb.BaudRate = 115200;
	m_dcb.ByteSize = 8;
	m_dcb.Parity = NOPARITY;
	m_dcb.StopBits = ONESTOPBIT;
	m_dcb.fAbortOnError = TRUE;

	m_bPortReady = SetCommState(m_hCom, &m_dcb);

	char data[1000];
	data[ 0] = 0x7E;
	data[ 1] = 0x44;
	data[ 2] = 0x26;
	data[ 3] = 0x00;
	data[ 4] = 0xFF;
	data[ 5] = 0xFF;
	data[ 6] = 0x00;
	data[ 7] = 0x00;
	data[ 8] = 0x01;
	data[ 9] = 0x00;
	data[10] = 0x03;
	data[11] = 0x01;
	data[12] = 0xD8;
	data[13] = 0x5D;
	data[14] = 0x7E;

	bWriteRC = WriteFile(m_hCom, data,15,&iBytesWritten,NULL);
	Sleep(200);

	CloseHandle(m_hCom);
}
