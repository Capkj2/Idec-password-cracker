#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
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
//invoke function to xor all bytes of password packet
static inline unsigned int value(char c)
{
    if (c >= '0' && c <= '9') { return c - '0';      }
    if (c >= 'a' && c <= 'f') { return c - 'a' + 10; }
    if (c >= 'A' && c <= 'F') { return c - 'A' + 10; }
    return -1;
}

string data(int& cnt, int Pad )
{


    // Count from 0 to X
    string pre, st, hs, packet;
    //int cnt  = 0;
    //while (cnt < 2)
    //	{
    //	Sleep(10);
    	stringstream ss;
    	if (cnt >= 10 && cnt < 100 )	{ Pad = Pad + 1; }
    	else if (cnt >= 100 && cnt < 1000 )	{ Pad = Pad + 2; }
    	else if (cnt >= 1000 && cnt < 10000 )	{ Pad = Pad + 3; }
    	else if (cnt >= 10000 && cnt < 100000 )	{ Pad = Pad + 4; }
    	else if (cnt >= 100000 && cnt < 1000000 )	{ Pad = Pad + 5; }
    	else if (cnt >= 1000000 && cnt < 10000000 )	{ Pad = Pad + 6; }
    	else if (cnt >= 10000000 && cnt <= 99999999 )	{ Pad = Pad + 7; }
    	ss << setw(Pad) << setfill('0') << cnt;	//get the number and pad 0's to 'Pad' char
    	//ss << cnt;								//  No padded 0's
    	string s = ss.str();
    	stream2hex(s, hs); //convert to hex
    	//cout <<"bs= " << s << endl;
    	//istringstream ("00") >> hex >> hs;
    	//cout <<"s_hex= " << hs << endl;
    	//string packet;
		istringstream("0546463057560000000000000000") >> hex >> packet;	//include preamble
    	//cout <<endl <<"packet= " << packet << endl;
    	int hsl = hs.length();
    	int Psl = packet.length();
    	Psl = Psl-hsl;
    	//cout << "before replace" << packet << endl;
    	packet.replace(packet.begin()+(12),packet.begin()+12+(hsl),hs.begin(),hs.end());	//append hex stream to preamble
    	//cout << "appended" << packet << endl;

    	/*The following was in the function ( string str_xor(string &packet) )
    	 * but I couldn't get it to return the values, so I just stuffed
    	 * the code here in Main. Of course I added a second for loop in order
    	 * to process the High Byte and Low Byte separately.
    	 */
    	//cout << "so far so good " << endl;
    	//static char const alphabet[] = "0123456789ABCDEF"; // Not sure what this did
    	//unsigned int result;								// or this
    	//result.reserve(packet.length());
    	//cout << "packetlength is " << dec << packet.length() << endl;
    	//cout << "going in deep" << endl;
    	unsigned int cksmHB = 0;
    	unsigned int cksmLB = 0;
    	unsigned int byte = 0;
    	for	(std::size_t i = 0; i < packet.length(); i = i+2){
    		//cout <<endl << "password value is " << hex << packet << endl;
    		byte = value(packet [i]);
    		//cout << "next operation is "<< cksmHB << " xor " << byte << endl;
    		cksmHB ^= byte;
    		//cout << "The high byte checksum is "<< hex  << cksmHB << endl;
            //assert(cksmHB < sizeof alphabet);
            //result.push_back(cksmHB);
        }
    	for (std::size_t j = 1; j < packet.length(); j = j+2){
    	   	//cout << "packet value is " << hex << packet << endl;
    	   	byte = value(packet [j]);
    	   	//cout << "next operation is "<< cksmLB << " xor " << byte << endl;
    	   	cksmLB ^= byte;
    		//cout << "The low byte checksum is "<< hex  << cksmLB << endl;
    		//assert(cksmLB < sizeof alphabet);
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
    	//cout << "generated password string" << packet << setw(2) << uppercase << hex  << endl;
    	++cnt;
    	//}



return packet;
}
