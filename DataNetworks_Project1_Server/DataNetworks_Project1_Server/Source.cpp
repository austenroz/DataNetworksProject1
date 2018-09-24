/* Data Network Server for Project 1
 * Developed by Austen Rozanski
 *
 * Created: 09/06/2018
 * Last Updated: 09/16/2018
 */

 /* Description:
  *
  * This project is a server that recieves data from a client using the UDP protocol. The data must come in the form of Ethernet Frames
  * using IPV4. Once the data is received, the server will output information from each of the ethernet frames sent. This includes all the
  * data from the headers of the ethernet frame, and the IPV4 header. The hex will also be output after the header data along with its
  * associated char representation.
  *
  * This program was developed inside of visual studio 2017.
  */

#include <iostream>
#include <WS2tcpip.h>
#include <vector>
#include <iomanip>

// Include the Winsock library file
#pragma comment (lib, "ws2_32.lib")

using namespace std;

////////////////////////////////////////////////////////////
// GET DIGIT METHODS
////////////////////////////////////////////////////////////

// getHexDigit Method
// Description:
//     Method takes an int value in base 10 and returns a specified digit in base 16.
//     For example, value = 31 (base 10) = 0x1f. The digitNumber specifies which digit
//     to return starting with 0 on the far right side of the hex number. If digitNumber
//     equals 0, the above example would return 15 and if digitNumber = 1, it would return
//     1.
// Parameters:
//     int value - the value to convert to hex and find a digit in
//     int digitNumber - the number of digits to count to starting from the right

int getHexDigit(int value, int digitNumber) {
	return (value >> (4 * digitNumber)) & 0x000F;
}

// return the int value of the binary digit specified by digitNumber
// digitNumber starts on the right at index 0 and increments to the left.
// Example: value = 1110= 14, digitNumber = 0, will return 0

// getBinaryDigit Method
// Description:
//     Method takes an int value in base 10 and returns a specified digit in base 2.
//     For example, value = 10 (base 10) = 1010 (binary). The digitNumber specifies which
//     digit to return starting with 0 on the far right side of the binary number. If
//     digitNumber = 0, the above example would return 0. If digitNumber = 3, the above
//     example would return 1. 
// Parameters:
//     int value - the value to convert to binary to find a digit in
//     int digitNumber - the number of digits to count to starting from the right
int getBinaryDigit(int value, int digitNumber) {
	return ((value >> (1 * digitNumber)) % 2 == 0) ? (0) : (1);
}

////////////////////////////////////////////////////////////
// Main Method
////////////////////////////////////////////////////////////

void main()
{
	////////////////////////////////////////////////////////////
	// INITIALIZE WINSOCK
	////////////////////////////////////////////////////////////

	// Create a WORD that states we are using WinSock version 2.
	WORD version = MAKEWORD(2, 2);

	// Start WinSock
	WSADATA data;
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0)
	{
		// Not ok! Get out quickly
		cout << "Can't start Winsock! " << wsOk;
		return;
	}

	////////////////////////////////////////////////////////////
	// SOCKET CREATION AND BINDING
	////////////////////////////////////////////////////////////

	// Create a socket, notice that it is a user datagram socket (UDP)
	SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);

	// Create a server hint structure for the server
	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY; // Us any IP address available on the machine
	serverHint.sin_family = AF_INET; // Address format is IPv4
	serverHint.sin_port = htons(54000); // Convert from little to big endian

	// Try and bind the socket to the IP and port
	if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
	{
		cout << "Can't bind socket! " << WSAGetLastError() << endl;
		return;
	}

	////////////////////////////////////////////////////////////
	// MAIN LOOP SETUP AND ENTRY
	////////////////////////////////////////////////////////////

	sockaddr_in client; // Use to hold the client information (port / ip address)
	int clientLength = sizeof(client); // The size of the client information

	// Buffer to store the data coming in from the client
	char buf[1024];

	// Enter a loop
	while (true)
	{
		ZeroMemory(&client, clientLength); // Clear the client structure
		ZeroMemory(buf, 1024); // Clear the receive buffer

		// Wait for message
		int bytesIn = recvfrom(in, buf, 1024, 0, (sockaddr*)&client, &clientLength);
		if (bytesIn == SOCKET_ERROR)
		{
			cout << "Error receiving from client " << WSAGetLastError() << endl;
			continue;
		}

		// Display message and client info
		char clientIp[256]; // Create enough space to convert the address byte array
		ZeroMemory(clientIp, 256); // to string of characters

		// Convert from byte array to chars
		inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);

		// stores the current index being read in the buf array (data from client)
		int packetIndex = 0;

		// stores the beginning of the current frame. Used when outputting all the hex at the end
		int outputOctetsStart = 0;

		while (packetIndex < bytesIn) {
			// Each frame must be at least 34 chars long
			if (bytesIn - packetIndex >= 34) {

				////////////////////////////////////////////////////////////
				// Retrieve Data For Headers
				////////////////////////////////////////////////////////////

				///////////////////////////////
				// Frame Variables for Output
				///////////////////////////////

				// Information from Ethernet Header
				int ether_packetSize;
				unsigned char ether_destMacAddr[6];
				unsigned char ether_sourceMacAddr[6];
				unsigned char ether_type[2];
				// Information from IP Header
				int ip_version;
				int ip_headerLength;
				unsigned char ip_typeOfService;
				int ip_totalLength;
				int ip_id;
				//int ip_flagHex;
				bool ip_flags[3];
				int ip_fragmentOffset;
				unsigned char ip_flagsOctets[2];
				int ip_timeToLive;
				int ip_protocol;
				unsigned char ip_checksum[2];
				int ip_sourceAddr[4];
				int ip_destAddr[4];

				////////////////////////////////
				// Retreive Ethernet Header Data
				////////////////////////////////

				// Destination MAC Address
				for (int i = 0; i < 6; i++) {
					ether_destMacAddr[i] = (unsigned char)buf[packetIndex];
					packetIndex++;
				}
				// Source MAC Address
				for (int i = 0; i < 6; i++) {
					ether_sourceMacAddr[i] = (unsigned char)buf[packetIndex];
					packetIndex++;
				}
				// EtherType
				for (int i = 0; i < 2; i++) {
					ether_type[i] = (unsigned char)buf[packetIndex];
					packetIndex++;
				}
				// Assign frame Size when ip header's total length is assigned

				//////////////////////////////
				// Retrieve IP Header Data
				//////////////////////////////

				// Version and Header Length
				ip_version = getHexDigit((int)((unsigned char)buf[packetIndex]), 1);
				ip_headerLength = 4 * getHexDigit((int)((unsigned char)buf[packetIndex]), 0);
				packetIndex++;

				// Type of Service
				ip_typeOfService = (unsigned char)buf[packetIndex];
				packetIndex++;

				// Total Length
				ip_totalLength = (256 * (int)((unsigned char)buf[packetIndex])) + ((int)((unsigned char)buf[packetIndex+1]));
				ether_packetSize = ip_totalLength + 14;
				packetIndex += 2;

				// Identification
				ip_id = (256 * (int)((unsigned char)buf[packetIndex])) + ((int)((unsigned char)buf[packetIndex+1]));
				packetIndex += 2;

				// Flags and Fragment Offset
				// flags = first 3 bits, fragment = next 13 bits
				int tmp_flagsOctet = (int)((unsigned char)buf[packetIndex]);
				int tmp_flagsHex = getHexDigit(tmp_flagsOctet, 1);
				ip_flags[0] = getBinaryDigit(tmp_flagsHex, 3);
				ip_flags[1] = getBinaryDigit(tmp_flagsHex, 2);
				ip_flags[2] = getBinaryDigit(tmp_flagsHex, 1);
				// fragment offset = int value of second octet + 16^2 * (second hex in first octet) + 2^12 * (fourth bit in first hex in first octet)
				ip_fragmentOffset = (int)((unsigned char)buf[packetIndex + 1]) + (256 * getHexDigit(tmp_flagsOctet, 0)) + (4096 * getBinaryDigit(tmp_flagsHex, 0));
				ip_flagsOctets[0] = buf[packetIndex];
				ip_flagsOctets[1] = buf[packetIndex + 1];

				packetIndex += 2;

				// Time to Live
				ip_timeToLive = (int)((unsigned char)buf[packetIndex]);
				packetIndex++;

				// Protocol
				ip_protocol = (int)((unsigned char)buf[packetIndex]);
				packetIndex++;

				// Header Checksum
				ip_checksum[0] = (unsigned char)buf[packetIndex];
				ip_checksum[1] = (unsigned char)buf[packetIndex + 1];
				packetIndex += 2;

				// Source IP Address
				for (int i = 0; i < 4; i++) {
					ip_sourceAddr[i] = (int)((unsigned char)buf[packetIndex]);
					packetIndex++;
				}

				// Distination IP Address
				for (int i = 0; i < 4; i++) {
					ip_destAddr[i] = (int)((unsigned char)buf[packetIndex]);
					packetIndex++;
				}

				// Options and Padding
				// options size = HeaderLength - 20 or until first option terminator
				// padding length = HeaderLength - 20 - optionssize
				packetIndex += (ip_headerLength - 20);

				// Additional Data
				// size = total length - IHL
				packetIndex += (ip_totalLength - ip_headerLength);


				////////////////////////////////////////////////////////////
				// Output Data from above
				////////////////////////////////////////////////////////////

				// Ether Header
				cout << "Ether:  ----- Ether Header -----" << endl;
				cout << "\tEther:  " << endl;

				// packet size
				cout << "\tEther:  " << left << setfill(' ') << setw(14) << "Packet Size" << ": " << dec << ether_packetSize << " bytes" << endl;

				// destination mac address
				cout << "\tEther:  " << left << setfill(' ') << setw(14) << "Destination" << ": " << right << setfill('0') << setw(2) << hex << (int)ether_destMacAddr[0];
				for (int i = 1; i < 6; i++) {
					cout << "-" << setfill('0') << setw(2) << hex << (int)ether_destMacAddr[i];
				}
				cout << endl;

				// source mac address
				cout << "\tEther:  " << left << setfill(' ') << setw(14) << "Source" << ": " << right << setfill('0') << setw(2) << hex << (int)ether_sourceMacAddr[0];
				for (int i = 1; i < 6; i++) {
					cout << "-" << setfill('0') << setw(2) << hex << (int)ether_sourceMacAddr[i];
				}
				cout << endl;

				// ethertype
				cout << "\tEther:  " << left << setfill(' ') << setw(14) << "EtherType" << ": " << right << setfill('0') << setw(2) << hex << (int)ether_type[0] << setfill('0') << setw(2) << (int)ether_type[1];
				if ((int)ether_type[0] == 8 && (int)ether_type[1] == 0) {
					cout << " (IP)" << endl;
				}
				else if ((int)ether_type[0] == 0 && (int)ether_type[1] == 0) {
					cout << " (ARP)" << endl;
				}
				else {
					cout << " (UNKNOWN)" << endl;
				}
				cout << "\tEther: " << endl << endl;

				// IP Header
				cout << "\tIP:  ----- IP Header -----" << endl;
				cout << "\tIP:  " << endl;

				// version
				cout << "\tIP:  Version = " << dec << ip_version << endl;

				// header length
				cout << "\tIP:  Header length = " << dec << ip_headerLength << " bytes" << endl;

				// type of service
				// get the binary digits in the hex stored in ip_typeOfService
				int tmp_tosHex[2];
				tmp_tosHex[0] = getHexDigit((int)ip_typeOfService, 0);
				tmp_tosHex[1] = getHexDigit((int)ip_typeOfService, 1);
				int tmp_precedence = getBinaryDigit(tmp_tosHex[1], 1) + (2 * getBinaryDigit(tmp_tosHex[1], 2)) + (4 * getBinaryDigit(tmp_tosHex[1], 3));
				int tmp_tosBits[5];
				for (int i = 0; i < 4; i++) {
					tmp_tosBits[i] = getBinaryDigit(tmp_tosHex[0], i);
				}
				tmp_tosBits[4] = getBinaryDigit(tmp_tosHex[1], 0);
				//output the type of service and details
				cout << "\tIP:  Type of service = 0x" << setfill('0') << setw(2) << hex << (int)ip_typeOfService << endl;
				if (tmp_tosBits[4] == 0) {
					cout << "\tIP:  \t...0 .... = normal delay" << endl;
				}
				else {
					cout << "\tIP:  \t...1 .... = low delay" << endl;
				}
				if (tmp_tosBits[4] == 0) {
					cout << "\tIP:  \t.... 0... = normal throughput" << endl;
				}
				else {
					cout << "\tIP:  \t....	1... = high throughput" << endl;
				}
				if (tmp_tosBits[4] == 0) {
					cout << "\tIP:  \t.... .0.. = normal reliability" << endl;
				}
				else {
					cout << "\tIP:  \t.... .1.. = high reliability" << endl;
				}

				// total length
				cout << "\tIP:  Total length = " << dec << ip_totalLength << " octets" << endl;

				// identification
				cout << "\tIP:  Identificiation = " << dec << ip_id << endl;

				// flags
				cout << "\tIP:  Flags = 0x" << hex << setw(2) << (int)ip_flagsOctets[0];
				cout << setw(2) << (int)ip_flagsOctets[1] << endl;
				if (ip_flags[1] == 0) {
					cout << "\tIP:  \t.0.. .... = do not fragment" << endl;
				}
				else {
					cout << "\tIP:  \t.1.. .... = fragment" << endl;
				}
				if (ip_flags[2] == 0) {
					cout << "\tIP:  \t..0. .... = last fragment" << endl;
				}
				else {
					cout << "\tIP:  \t..1. .... = last fragment" << endl;
				}

				// fragment offset
				cout << "\tIP:  Fragment offset = " << dec << ip_fragmentOffset << " bytes" << endl;

				// time to live
				cout << "\tIP:  Time to live = " << dec << ip_timeToLive << " seconds/hops" << endl;

				// protocol
				cout << "\tIP:  Protocol = " << dec << ip_protocol;
				if (ip_protocol == 6) {
					cout << " (TCP)" << endl;
				}
				else if (ip_protocol == 17) {
					cout << " (UDP)" << endl;
				}
				else {
					cout << " (UNKNOWN)" << endl;
				}

				// header checksum
				cout << "\tIP:  Header checksum = " << setfill('0') << setw(2) << hex << (int)ip_checksum[0] << setfill('0') << setw(2) << (int)ip_checksum[1] << endl;

				// source ip address
				cout << "\tIP:  Source address = " << dec << ip_sourceAddr[0];
				for (int i = 1; i < 4; i++) {
					cout << "." << dec << ip_sourceAddr[i];
				}
				cout << endl;

				// destination ip address
				cout << "\tIP:  Destination address = " << dec << ip_destAddr[0];
				for (int i = 1; i < 4; i++) {
					cout << "." << dec << ip_destAddr[i];
				}
				cout << endl;

				// options
				if (ip_headerLength > 20) {
					cout << "\tIP:  Some Options";
				}
				else {
					cout << "\tIP:  No Options" << endl;
				}
				cout << "\tIP: " << endl << endl;

				////////////////////////////////////////////////////////////
				// Output all hex analyzed for current frame
				////////////////////////////////////////////////////////////
				int curCol = 1; // index of the current column
				int curRow = 0; // index of the current row
				int i = outputOctetsStart; // the index of the beginning of the frame in the buf array

				cout << "0000 ";

				// loop through all the chars until the end of the frame has been reached, outputting each
				for (; i < packetIndex; i++) {

					// once at the end of the current row, output the char representation of the hex in current row
					// and then move on to the next row.
					if (curCol % 17 == 0) {
						for (int j = i - 16; j < i; j++) {
							if ((int)((unsigned char)buf[j]) >= 33 && (int)((unsigned char)buf[j]) <= 126) {
								cout << dec << (unsigned char)buf[j];
							}
							else {
								cout << ".";
							}
						}
						cout << endl;
						curRow += 16;
						cout << setfill('0') << setw(4) << hex << curRow << " ";
						curCol++;
					}

					// output the hex for the current char
					cout << setfill('0') << setw(2) << hex << (int)((unsigned char)buf[i]) << " ";
					curCol++;
				}

				// for the final row, add whitespace to reach the right side where char representation is output
				cout << setfill(' ') << setw( (17 - (curCol%17))*3) << " ";

				// output the char respresentation for final row
				for (int j = i - (curCol%17)+1; j < i; j++) {
					if ((int)((unsigned char)buf[j]) >= 33 && (int)((unsigned char)buf[j]) <= 126) {
						cout << dec << (unsigned char)buf[j];
					}
					else {
						cout << ".";
					}
				}

				// assign the beginning of the next frame
				outputOctetsStart = packetIndex;

				cout << endl << endl;
			}
			else {
				cout << "Some packets remaining";
			}
		}


	}

	// Close socket
	closesocket(in);

	// Shutdown winsock
	WSACleanup();
}