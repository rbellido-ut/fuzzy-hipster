

#include "util.h"
using namespace std;
size_t Communication::count_ = 0;

Communication::Communication(const Communication& src)
	:readBuffer_(src.readBuffer_), writeBuffer_(src.writeBuffer_), bytesSent_(src.bytesSent_),
	bytesRecvd_(src.bytesRecvd_), readSocket_(src.readSocket_), writeSocket_(src.writeSocket_)
{ count_++; }


bool Communication::sendPacket()
{

	return true;

}

bool Communication::recvPacket()
{

	return true;
}

ostream& operator<< (ostream& os, const Communication& cs)
{
	return os;
}

