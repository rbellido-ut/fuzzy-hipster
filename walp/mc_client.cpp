#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE     1024
#define MAXADDRSTR  16

#define TIMECAST_ADDR "234.5.6.7"
#define TIMECAST_PORT 8910

char achMCAddr[MAXADDRSTR] = TIMECAST_ADDR;
u_long	lMCAddr;
u_short nPort              = TIMECAST_PORT;
SYSTEMTIME *lpstSysTime, stSysTime;

int main(int argc, char *argv[])
{
  int				nRet, nIP_TTL = 2;
  BOOL				fFlag;
  SOCKADDR_IN		local_address, server;
  struct ip_mreq	stMreq;         /* Multicast interface structure */
  SOCKET			hSocket;
  char				achInBuf [BUFSIZE];
  WSADATA			stWSAData;

  printf("------------------------------------------------------\n");
  printf(" TimeCastClnt - multicast time client\n");
  printf("------------------------------------------------------\n");

  // Init WinSock
  nRet = WSAStartup(0x0202, &stWSAData);
  if (nRet)
  {
      printf ("WSAStartup failed: %d\r\n", nRet);
      exit(1);
  }

  // Get a datagram socket
  hSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if (hSocket == INVALID_SOCKET)
  {
    printf ("socket() failed, Err: %d\n", WSAGetLastError());
    WSACleanup();
    exit(1);
  }

  fFlag = TRUE;
  nRet = setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&fFlag, sizeof(fFlag));
  if (nRet == SOCKET_ERROR)
  {
    printf ("setsockopt() SO_REUSEADDR failed, Err: %d\n",
      WSAGetLastError());
  }

  // Name the socket (assign the local port number to receive on)
  local_address.sin_family      = AF_INET;
  local_address.sin_addr.s_addr = htonl(INADDR_ANY);
  local_address.sin_port        = htons(nPort);

  nRet = bind(hSocket, (struct sockaddr*)&local_address, sizeof(local_address));
  if (nRet == SOCKET_ERROR)
  {
      printf ("bind() port: %d failed, Err: %d\n", nPort, 
      WSAGetLastError());
  }

  // Join the multicast group so we can receive from it
  stMreq.imr_multiaddr.s_addr = inet_addr(achMCAddr);
  stMreq.imr_interface.s_addr = INADDR_ANY;

  nRet = setsockopt(hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq));
  if (nRet == SOCKET_ERROR)
  {
    printf("setsockopt() IP_ADD_MEMBERSHIP address %s failed, Err: %d\n",
				achMCAddr, WSAGetLastError());
  } 

  printf("Now waiting for time updates from the TimeCast server\n");
  printf("  multicast group address: %s, port number: %d\n",
			achMCAddr, nPort);

  lpstSysTime = (SYSTEMTIME *)achInBuf;
  for (;;)
  {
    int addr_size = sizeof(struct sockaddr_in);

    nRet = recvfrom(hSocket, achInBuf, BUFSIZE, 0, (struct sockaddr*)&server, &addr_size);
    if (nRet < 0)
	{
      printf ("recvfrom() failed, Error: %d\n", WSAGetLastError());
      WSACleanup();
      exit(1);
    }

    printf(
     "UTC Time %02d:%02d:%02d:%03d on %02d-%02d-%d from %s:%d\n",
      lpstSysTime->wHour,   lpstSysTime->wMinute, 
      lpstSysTime->wSecond, lpstSysTime->wMilliseconds,
      lpstSysTime->wMonth,  lpstSysTime->wDay,  lpstSysTime->wYear,
      inet_ntoa(server.sin_addr), 
      ntohs(server.sin_port));

    // Set the local time using the system (UTC) time from server
    nRet = SetSystemTime(lpstSysTime);
    if (!nRet)
	{
        printf ("SetLocalTime() failed, Err: %d\n", 
            GetLastError());
    }
	else
	{
      GetLocalTime(&stSysTime);
      printf (
      " Local Time reset to %02d-%02d-%02d at %02d:%02d:%02d:%03d\n",
        stSysTime.wMonth,  stSysTime.wDay,  stSysTime.wYear,
        stSysTime.wHour,   stSysTime.wMinute, 
        stSysTime.wSecond, stSysTime.wMilliseconds);
    }
  }

  // Leave the multicast group 
  stMreq.imr_multiaddr.s_addr = inet_addr(achMCAddr);
  stMreq.imr_interface.s_addr = INADDR_ANY;

  nRet = setsockopt(hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq));
  if (nRet == SOCKET_ERROR)
  {
    printf (
      "setsockopt() IP_DROP_MEMBERSHIP address %s failed, Err: %d\n",
      achMCAddr, WSAGetLastError());
  } 

  closesocket(hSocket);

  WSACleanup();

  return 0;
}
