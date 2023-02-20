#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "1337"

namespace THINGIII {



    bool DPMEMPICK()
    {
        WSADATA wsaData;
        SOCKET ConnectSocket = INVALID_SOCKET;
        struct addrinfo* result = NULL,
            * ptr = NULL,
            hints;
        const char* sendbuf = "True";
        char recvbuf[DEFAULT_BUFLEN];
        int iResult;
        int recvbuflen = DEFAULT_BUFLEN;

        // Validate the parameters


        // Initialize Winsock
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            ////("WSAStartup failed with error: %d\n", iResult);
            return false;
        }

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        // Resolve the server address and port
        iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
            ////("getaddrinfo failed with error: %d\n", iResult);
            WSACleanup();
            return false;
        }

        // Attempt to connect to an address until one succeeds
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

            // Create a SOCKET for connecting to server
            ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                ptr->ai_protocol);
            if (ConnectSocket == INVALID_SOCKET) {
                ////("socket failed with error: %ld\n", WSAGetLastError());
                WSACleanup();
                return false;
            }

            // Connect to server.
            iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (iResult == SOCKET_ERROR) {
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
                continue;
            }
            break;
        }

        freeaddrinfo(result);

        if (ConnectSocket == INVALID_SOCKET) {
            ////("Unable to connect to server!\n", 0);
            WSACleanup();
            return false;
        }

        // Send an initial buffer
        iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            ////("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return false;
        }

        //("Bytes Sent: %ld\n", iResult);

        // shutdown the connection since no more data will be sent
        iResult = shutdown(ConnectSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            ////("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return false;
        }

        // Receive until the peer closes the connection

        do {

            iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

            if (iResult > 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                return true;
            }
            else if (iResult == 0) {
                exit(-1);
                return false;
            }


        } while (iResult > 0);

        // cleanup
        closesocket(ConnectSocket);
        WSACleanup();

        return false;
    }
}
