/*
 * Main.cpp
 *
 *  Created on: Jul 2, 2016
 *      Author: K Johnson
 */



/***********************************************************************
 main.cpp - The main() routine for all the "Basic Winsock" suite of
    programs from the Winsock Programmer's FAQ.  This function parses
    the command line, starts up Winsock, and calls an external function
    called DoWinsock to do the actual work.

 This program is hereby released into the public domain.  There is
 ABSOLUTELY NO WARRANTY WHATSOEVER for this product.  Caveat hacker.
***********************************************************************/
#include "ws-util.h"

#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <time.h>
#include <fstream>

using namespace std;

// Comment this out to disable the shutdown-delay functionality.
#define SHUTDOWN_DELAY

//// Prototypes ////////////////////////////////////////////////////////

extern string data(int& cnt, int Pad);
u_long LookupAddress(const char* pcHost);
SOCKET EstablishConnection(u_long nRemoteAddr, u_short nPort);
bool SendPacket(SOCKET sd);
bool ReadReply(SOCKET sd, int Count);
char str2char( string src, char* tmpByte);
char FileName[80];
char Packet[18];
ofstream logfile;
int CreateFileName();
SOCKET sd;

//// Constants /////////////////////////////////////////////////////////

// Default port to connect to on the server
const int kDefaultServerPort = 2101;

// kBufferSize must be larger than the length of Packet.
const int kBufferSize = 1024;


#if defined(SHUTDOWN_DELAY)
// How long to wait after we do the echo before shutting the connection
// down, to give the user time to start other clients, for testing
// multiple simultaneous connections.
const int kShutdownDelay = 3;
#endif


//// main //////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
// Do we have enough command line arguments?
    if (argc < 2)
    {
        cerr <<endl << endl<< "\tusage: " << " <server-address> " <<
                "[server-port]" << endl;
        cerr << "\tIf you don't pass server-port, it defaults to " <<
                (kDefaultServerPort) << "." << endl;
        return 1;
    }

// Get host and (optionally) port from the command line
    const char* pcHost = argv[1];
    int nPort = kDefaultServerPort;
    if (argc >= 3)
    {
        nPort = atoi(argv[2]);
    }

    // Do a little sanity checking because we're anal.
    int nNumArgsIgnored = (argc - 3);
    if (nNumArgsIgnored > 0)
    {
        cerr << nNumArgsIgnored << " extra argument" <<
                (nNumArgsIgnored == 1 ? "" : "s") <<
                " ignored.  FYI." << endl;
    }

//Get user input
    int Attempts = '0', strtCnt = '0', zeroPad = '0';
    char yn;

    while (!((yn == 'Y') || (yn == 'y')))
    {
    	cout << "How many passwords would you like to try (1-99999999)?  ";
    	cin >> Attempts;
    	cout << string(2, '\n'); //Add 2 blank lines oon screen
    	while (Attempts < 1)
    	{
    		cout << "You need to try at least 1 password!" << endl
    			 << "Enter the number of passwords you would like to try (1-99999999).  ";
    		cin >> Attempts;
    		cout << string(2, '\n');	//Add 2 blank lines oon screen
    	}
    	cout << "Enter the starting number (1-" << Attempts-1 << ")  ";
    	cin >> strtCnt;
    	cout << string(2, '\n');
    	while (strtCnt >= Attempts)
    	{
    		cout << "The starting number must be " << Attempts-1 << " or lower."  << endl
    		     << "Enter a number from 1-  " << Attempts-1;
    		cin >> strtCnt;
    		cout << string(2, '\n');	//Add 2 blank lines on screen
    	}
    	/*	while (qty < 1)
    	{
    		cout << "You need to try at least 1 password!" << endl
        	<< "Enter a number from 1-" << val-1 << endl;
    		cin >> strtCnt;
    		cout << string(2, '\n');	//Add 2 blank lines on screen
    	}	*/
    	cout << "Enter the number of preceding Zeros (0-7)  ";
    	cin >> zeroPad;
    	if (zeroPad > 0)
    	{
    		zeroPad = zeroPad+1;
    	}
    	cout << string(2, '\n');	//Add 2 blank lines on screen

    	cout << "You have chosen to test " << Attempts-strtCnt << " Passwords " << endl
    			<< "The range is " << strtCnt << " to " << Attempts << endl <<endl
				<< "Enter Y to continue N to return   ";
    	cin >> yn;
    	cout << string(2, '\n');	//Add 2 blank lines on screen
    }
    if (CreateFileName()==1)
    {
    	logfile.open (FileName);
    	if(logfile.is_open())
    	{
    		cout << "Logfile created successfully" << endl;
    		logfile << FileName << "\n";
    		logfile.close();
    	}
    }


// Start Winsock up
    WSAData wsaData;
    int nCode;
    if ((nCode = WSAStartup(MAKEWORD(1, 1), &wsaData)) != 0)
    {
    	cerr << "WSAStartup() returned error code " << nCode << "." << endl;
    	return 255;
    }

// Find the server's address
    cout << "Looking up address..." << flush;
    u_long nRemoteAddress = LookupAddress(pcHost);
    if (nRemoteAddress == INADDR_NONE)
    {
    	cerr << endl << WSAGetLastErrorMessage("lookup address") <<	endl;
    	return 3;
    }
    in_addr Address;
    memcpy(&Address, &nRemoteAddress, sizeof(u_long));
    cout << inet_ntoa(Address) << ":" << nPort << endl;

// Connect to the server
    cout << "Connecting to remote host... " << flush;
	SOCKET sd = EstablishConnection(nRemoteAddress, htons(nPort));
	if (sd == INVALID_SOCKET)
	{
		cerr << endl << WSAGetLastErrorMessage("connect to server") << endl;
		return 3;
	}
	cout << "connected, socket " << sd << "." << endl;
	cout << string(2, '\n');	//Add 2 blank lines on screen
	cout << "\t Working...\tPress 'Esc' to quit: \n" << endl;




//Send the password out on the wire and check return, we can do the loop here
// 0 for fail 1 for succeed.
    int Count =strtCnt ;
    int start_s=clock();
    int k = 0;
    bool loop = true;

    while ((loop) && (Count <= Attempts)) // && (input != 'Q'||'q'))
    {
    	string datapacket = data(Count, zeroPad);// Call the password generator

        //Get the packet ready to go out on the wire
        //
        str2char(datapacket, Packet);
        SendPacket(sd);
        loop = ReadReply(SOCKET(sd), Count);
        if (k == 0)
        {
        	cout << "\t" << Count - strtCnt <<  " passwords checked so far " << flush;
       	}

        else if (k == 500)
  	   	{
        	cout << "\r\tcurrent password = "<< (Count-1) << "\t\t" << (Count-1)- strtCnt << " passwords checked so far ";
        	logfile.open(FileName, ios::out | ios::app );
			logfile << "Last checked " <<(Count-1)<< "\n";
        	logfile.close();
        	k=0;
		}
        k = k+1;
        if (GetAsyncKeyState(VK_ESCAPE))
            	{
            		break;
            	}
    }
       int stop_s=clock();
    cout << "\nTested passwords " << strtCnt << "-" << (Count - 1) << " in "
    	 << (stop_s-start_s)/float(CLOCKS_PER_SEC) << " Seconds" << endl;

    if(loop == false)
    {
    	cout << "\n Your password is " << (Count - 1) << endl;
    	cout << string(2, '\n');	//Add 2 blank lines on screen
    	logfile.open(FileName, ios::out | ios::app );
    	logfile << "Your password is " <<(Count-1)<< "\n";
    	logfile.close();
    }
    else
    {
    	cout << "No valid password found" << endl
    		 <<	"Last Password tested = " << (Count - 1) << endl;
    	logfile.open(FileName, ios::out | ios::app );
    	logfile << "No valid password found \nLast Password tested =  " <<(Count-1)<< "\n"
    			<< (Count-1)- strtCnt << " passwords checked" << "in "
    	    	<< (stop_s-start_s)/float(CLOCKS_PER_SEC) << " Seconds" << endl;
    	logfile.close();
    	cout << string(2, '\n');	//Add 2 blank lines on screen
    }

        //cout << "result is " << setw(2) << uppercase << hex  << packet << endl;


#if defined(SHUTDOWN_DELAY)
    // Delay for a bit, so we can start other clients.  This is strictly
    // for testing purposes, so you can convince yourself that the
    // server is handling more than one connection at a time.
    //cout << "Will shut down in " << kShutdownDelay <<
            //" seconds... (one dot per second): " << flush;
    for (int i = 0; i < kShutdownDelay; ++i) {
        Sleep(1000);
        //cout << '.' << flush;
    }
    cout << endl;
#endif

    // Shut connection down
    cout << "Shutting connection down..." << flush;
    if (ShutdownConnection(sd)) {
        cout << "Connection is down." << endl;
    }
    else {
        cout << endl << WSAGetLastErrorMessage("Shutdown connection") <<
                endl;
    }
    //cout << "All done!" << endl;

    // Shut Winsock back down and take off.
    WSACleanup();
    return 0;
}	//		END OF MAIN
	//


/////////////////////// CreateFileName /////////////////////////////////
//Creates a filename for our log file with the date hour and minute. ///

int CreateFileName()
{
     time_t t = time(0);   // get time now
     struct tm * now = localtime( & t );
     //char buffer[80];
     strftime (FileName,80,"passwords %m-%d_%H%M.txt",now);
     std::string name (FileName);
     std::cout<<"Created password file "<< (FileName)<< std::endl;
     std::string tempfile;
     return 1;
}

///////////////////////// LookupAddress //////////////////////////////////
// Given an address string, determine if it's a dotted-quad IP address	//
// or a domain address.  If the latter, ask DNS to resolve it.  In		//
// either case, return resolved IP address.  If we fail, we return		//
// INADDR_NONE. //////////////////////////////////////////////////////////															//

u_long LookupAddress(const char* pcHost)
{
    u_long nRemoteAddr = inet_addr(pcHost);
    if (nRemoteAddr == INADDR_NONE) {
        // pcHost isn't a dotted IP, so resolve it through DNS
        hostent* pHE = gethostbyname(pcHost);
        if (pHE == 0) {
            return INADDR_NONE;
        }
        nRemoteAddr = *((u_long*)pHE->h_addr_list[0]);
    }
    return nRemoteAddr;
}

////////////////////////// EstablishConnection ///////////////////////////
// Connects to a given address, on a given port, both of which must be	//
// in network byte order.  Returns newly-connected socket if we succeed,//
// or INVALID_SOCKET if we fail.//////////////////////////////////////////										//

SOCKET EstablishConnection(u_long nRemoteAddr, u_short nPort)
{
    // Create a stream socket
    SOCKET sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd != INVALID_SOCKET) {
        sockaddr_in sinRemote;
        sinRemote.sin_family = AF_INET;
        sinRemote.sin_addr.s_addr = nRemoteAddr;
        sinRemote.sin_port = nPort;
        if (connect(sd, (sockaddr*)&sinRemote, sizeof(sockaddr_in)) ==
                SOCKET_ERROR) {
            sd = INVALID_SOCKET;
        }
    }

    return sd;
}

//// SendPacket //////////////////////////////////////////////
// Sends the packet to the PLC.  Returns true on success, 	//
// false otherwise.///////////////////////////////////////////

bool SendPacket(SOCKET sd)
{
    // Send the string to the PLC
	if (send(sd, Packet, sizeof Packet, 0) != SOCKET_ERROR) {
		return true;
	}
	else {
        return false;
    }
}

//////////////////////// ReadReply ///////////////////////////////////
// Read the reply packet and check it for sanity.  Returns True if  //
// no password match, returns False on a match.   ////////////////////

bool ReadReply(SOCKET sd, int counts)
{
    // Read reply from server and compare the data to known values

    char acReadBuffer[kBufferSize];
    unsigned int nTotalBytes = 0;
    char ValidPass[7]={0x06,0x30,0x31,0x30,0x33,0x37,0x0D};
    char InvalidPass[9]={0x06,0x30,0x31,0x32,0x30,0x35,0x33,0x30,0x0D};
    char BadCheck[9]={0x15,0x30,0x31,0x30,0x31,0x30,0x32,0x35,0x0D};
    bool loops;
    while (nTotalBytes < sizeof acReadBuffer[kBufferSize])
    {
    	int nNewBytes = recv(sd, acReadBuffer + nTotalBytes,
        kBufferSize - nTotalBytes, 0);
        if (nNewBytes == SOCKET_ERROR) {
        return -1;
    }
    else if (nNewBytes == 0)
    {
    	cerr << "Connection closed by peer." << endl;
    	return 0;
	}
	nTotalBytes += nNewBytes;
    }
    // Check data for sanity
    if (strncmp(acReadBuffer, ValidPass, nTotalBytes) == 0)
    {
		cout << "\n\n\tWe found the password!" << endl;
		loops = false;
	}
	else if(strncmp(acReadBuffer, BadCheck, nTotalBytes) == 0)
	{
        cerr << "\nBad checksum received from server. " << endl
        	 << "Password " << (counts - 1) << "=\t";
        printf("%x %x %x %x %x %x %x %x %x",acReadBuffer[0],acReadBuffer[1],acReadBuffer[2],acReadBuffer[3],acReadBuffer[4],acReadBuffer[5],acReadBuffer[6],acReadBuffer[7],acReadBuffer[8]);
    	logfile.open(FileName, ios::out | ios::app );
    	logfile << "Bad checksum \nPassword " <<(counts-1) << "\t" << acReadBuffer;
    	logfile.close();
        loops = true;
    }
	else if(strncmp(acReadBuffer, InvalidPass, nTotalBytes) == 0)
		{
	        loops = true;
	    }
    return loops;
}

//////////////////////// str2char ////////////////////////////////////////
// Convert the Hexidecimal encoded string to a Hex encoded char array	//
// returns the array in the return parameter you must have an apropriately
// sized char array declared prior to sending the pointer ////////////////

char str2char( string src, char* tmpByte)
{
	int i, n;
    char temp[2];
    for(i = 0; i < 18; i++)
    {
    	strncpy(temp, &src[2*i], 2);
    	sscanf(temp, "%02x", &n);
        tmpByte[i] = (char)n;
    }
    return 0;
}


