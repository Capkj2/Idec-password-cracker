#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;


// Convert string of chars to its representative string of hex numbers
void stream2hex(const std::string str, std::string& hexstr, const bool capital = true){
	hexstr.resize(str.size() * 2);
	static const char a = capital ? 0x40 : 0x60;
	for (size_t i = 0; i < str.size(); i++){
            char c = (str[i] >> 4) & 0xF;
            hexstr[i * 2] = c > 9 ? (c - 9) | a : c | '0';
            hexstr[i * 2 + 1] = ((str[i] & 0xF) > 9) ? ((str[i] - 9) & 0xF) | a : (str[i] & 0xF) | '0';
        }
}
//////////////////////// Generate Packet /////////////////////////
// This Function is called to generate the packet string needed to send to the PLC.
// when called it will build the packet and increment by one each time it is called.
// The increment count is handled by a global (Count) and passed here as (cnt)
string data(int& cnt, int Pad )
{
        string pre, st, hs, packet;
       	stringstream ss;
    	if (cnt >= 10 && cnt < 100 )		{ Pad = Pad + 1; }
    	else if (cnt >= 100 && cnt < 1000 )	   { Pad = Pad + 2; }
    	else if (cnt >= 1000 && cnt < 10000 )    { Pad = Pad + 3; }
    	else if (cnt >= 10000 && cnt < 100000 )	   { Pad = Pad + 4; }
    	else if (cnt >= 100000 && cnt < 1000000 ) 	 { Pad = Pad + 5; }
    	else if (cnt >= 1000000 && cnt < 10000000 )	   { Pad = Pad + 6; }
    	else if (cnt >= 10000000 && cnt <= 99999999 )    { Pad = Pad + 7; }
    	ss << setw(Pad) << setfill('0') << cnt;	//get the number and pad 0's to 'Pad' char
    	//ss << cnt;								//  No padded 0's
    	string s = ss.str();
    	stream2hex(s, hs); //convert to hex
    	istringstream("0546463057560000000000000000") >> hex >> packet;	//include preamble
    	int hsl = hs.length();
    	int Psl = packet.length();
    	Psl = Psl-hsl;
    	packet.replace(packet.begin()+(12),packet.begin()+12+(hsl),hs.begin(),hs.end());	//append hex stream to preamble

    	//////////////////////// Checksum calculation /////////////////////////
    	// to calculate the checksum of the string xor the high nibble and
    	// the low nibble of each character before the mask (0x30) separately
    	// next, the high nibble result is xor'ed by 0x33 (mask 0x30 xor 0x3)
    	// next the low nibble result is xor'ed by 0x30 (mask byte).
    	// If the result is greater than 0x39, add 0x07 to the result.

    	unsigned int cksmHB = 0;
    	unsigned int cksmLB = 0;
    	unsigned int byte = 0;
    	for	(size_t i = 0; i < packet.length(); i = i+2)
    	{
    		byte = (packet [i]);
    		cksmHB ^= byte;
    	 }
    	for (size_t j = 1; j < packet.length(); j = j+2)
    	{
    	   	byte = (packet [j]);
    	   	cksmLB ^= byte;
    	}
    	cksmHB ^= 0x33;
    	cksmLB ^= 0x30;
    	if (cksmLB > 0x39){
    		cksmLB += 0x07;
    	}

    	stringstream ckassm;
    	unsigned int mask = 0x30;
    	unsigned int eof = 0x0d;
    	ckassm << hex <<setfill('0') << setw(2) << mask << hex << cksmHB << hex <<cksmLB << hex << eof;
    	string CKsm = ckassm.str();
    	packet.append(CKsm);
    	++cnt;

    return packet;
}
