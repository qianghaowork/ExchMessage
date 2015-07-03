// ExchangeMeg.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "ExchangeAPI.h"

using namespace std;

int main(int argc, char* argv[])
{
	Exchmessage *emeg = new Exchmessage (argv[1]);
	if (emeg->isFileopen() )
	{
		emeg->Dividemessage();
		uint64_t total_packets = emeg->getPacket();
		uint64_t order_entry_msg_count = emeg->getEntryNum();
		uint64_t order_ack_msg_count = emeg->getAckNum();
		uint64_t order_fill_msg_count = emeg->getFillNum();
		string most_active_trader_tag = emeg->getActiveTrader();
		string most_liquidity_trader_tag = emeg->getLiquidTrader();

		cout << total_packets << "," << order_entry_msg_count << "," << order_ack_msg_count << "," << order_fill_msg_count << "," << most_active_trader_tag << "," << most_liquidity_trader_tag << endl;

		emeg->printVolTrade();
	}
	else 
		cout << "Unable to open file " << argv[1];

	delete emeg;

//	getchar();
	return 0;
}


