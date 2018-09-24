/* Data Network Client for Project 1
 * Developed by Austen Rozanski
 * 
 * Created: 09/06/2018
 * Last Updated: 09/16/2018
 */

/* Description:
 * 
 * This project is a client that sends that data within pcap files to a server using the UDP protocol. 
 * This project uses an external library (WINPCAP) to extract the contents of pcap files and store them
 * inside of a u_char array. A link to the library is provided below. 
 * 
 * This program was developed inside of visual studio 2017 community edition.
 */

#include <iostream>
#include <WS2tcpip.h>
#include <vector>
#include <pcap.h>
#include <string>

// Include the Winsock library file
#pragma comment (lib, "ws2_32.lib")

using namespace std;

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
		cout << "Can't start Winsock! " << wsOk;
		return;
	}

	////////////////////////////////////////////////////////////
	// CONNECT TO THE SERVER
	////////////////////////////////////////////////////////////

	// Create a hint structure for the server
	sockaddr_in server;
	server.sin_family = AF_INET; // AF_INET = IPv4 addresses
	server.sin_port = htons(54000); // Little to big endian conversion
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr); // Convert from string to byte array

	// Socket creation, note that the socket type is datagram
	SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);

	////////////////////////////////////////////////////////////
	// PCAP FILE PROCESSING
	////////////////////////////////////////////////////////////

	// Use a vector to store all the characters read in from the pcap file.
	vector<u_char> dataVector = vector<u_char>();

	// The file to be read from
	string file = "C:\\pcapFiles\\Project1input.pcap";

	// array to store errors
	char errbuff[PCAP_ERRBUF_SIZE];

	// Open the file
	pcap_t * pcap = pcap_open_offline(file.c_str(), errbuff);

	// Variables to store data from each frame read in
	struct pcap_pkthdr *header; // stores pcap header for current frame
	const u_char *pcapData; // stores pcap data for current frame

	// Read in all frames from the pcap file, storing only the data and not the headers
	// into the dataVector.
	while (int returnValue = pcap_next_ex(pcap, &header, &pcapData) >= 0)
	{
		for (u_int i = 0; (i < header->caplen); i++)
		{
			dataVector.push_back(pcapData[i]);
		}
	}

	// Output the data inside dataVector
	for (int i = 0; i < dataVector.size(); i++) {
		if ((i % 16) == 0) printf("\n");
		printf("%.2x ", dataVector[i]);
	}

	// convert the vector to a u_char (unsigned char) array. Because the vector is still pointing to
	// the addresses of the values inside this array, there is no need for manual garbage collection
	// at the end.
	u_char * dataToSend = &dataVector[0];

	////////////////////////////////////////////////////////////
	// SENDING DATA
	////////////////////////////////////////////////////////////

	// Send the dataToSend u_char array.
	int sendOk = sendto(out, (const char *)dataToSend, dataVector.size(), 0, (sockaddr*)&server, sizeof(server));

	// Output error if data failed to send
	if (sendOk == SOCKET_ERROR)
	{
		cout << "That didn't work! " << WSAGetLastError() << endl;
	}

	// Close the socket
	closesocket(out);

	// Close down Winsock
	WSACleanup();
	system("PAUSE");
}
