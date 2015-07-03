//#include "stdafx.h"
#include "ExchangeAPI.h"

using namespace std;

// There are repeating groups in fill message, find all trader_tag and keep counting the qty.
void Exchmessage::FindActiveTrader() 
{
	unordered_map<string, uint64_t> orderbook;
	for(auto it=order_fill.begin(); it!=order_fill.end(); it++) 
	{
		OrderFill *order = *it;
		vector<string> trader_tag = order->getTraderTag();
		vector<uint32_t> qty = order->getQty();

                for( int i=0; i<trader_tag.size(); i++) 
                {
		   if ( orderbook.find(trader_tag[i])==orderbook.end() )
			orderbook[trader_tag[i]] = qty[i];
		   else
			orderbook[trader_tag[i]] += qty[i];
                }
	}

	uint64_t maxqty = 0;
	for(auto it=orderbook.begin(); it!=orderbook.end(); it++)
	{
		if ( it->second > maxqty ) 
		{
			maxqty = it->second;
			most_active_trader_tag = it->first;
		}
	}
}

// Find the most liquility trader with the largest GFD volume in Order Entry Message. If Order Ack message rejects this order, volume is not counted.  
void Exchmessage::FindLiquidTrader() {
	unordered_map<uint64_t, OrderStatusEnum> ack_response;
	for(auto it=order_ack.begin(); it!=order_ack.end(); it++)
	{
		uint64_t client_id = (*it)->getClientId();
		if (ack_response.find(client_id)==ack_response.end())
			ack_response[client_id] = (*it)->getSatus();
		else
			cout << "Duplicated ID: " << client_id << endl;
	}

	unordered_map<string, uint64_t> entrybook;
	for(auto it=order_entry.begin(); it!=order_entry.end(); it++) 
	{
		string trader_tag = (*it)->getTraderTag();
		TimeForceEnum timeforce = (*it)->getTimeForce(); 
		uint64_t client_id = (*it)->getClientId();
		uint32_t qty = (*it)->getQty();

		bool sign = ack_response.find(client_id)!=ack_response.end() ? ack_response[client_id]==1 : false;
		if (sign && timeforce==TimeForceEnum::GFD) 
		{
			if (entrybook.find(trader_tag)==entrybook.end() )
				entrybook[trader_tag] = qty;
			else
				entrybook[trader_tag] += qty;
		}
	}

	uint64_t maxqty = 0;
	for(auto it=entrybook.begin(); it!=entrybook.end(); it++)
	{
		if ( it->second > maxqty ) 
		{
			maxqty = it->second;
			most_liquidity_trader_tag = it->first;
		}
	}
}

// Find volumn of trades per instrument. According Fill message order_id, find client_id in Ack message and check order_status, and then find instrument, side in Entry message. Assuming filled price equals price, 
// a trade occurs. Count the volume per instrument. By comparing the results with sample, both buy and sell volume are counted. 
void Exchmessage::FindVolPerInstrument() {
	unordered_map<uint32_t, uint64_t> ack_response;
	for(auto it=order_ack.begin(); it!=order_ack.end(); it++)
	{
		uint64_t client_id = (*it)->getClientId();
		uint32_t order_id = (*it)->getOrderId();
		bool sign = (*it)->getSatus()==1;
		if (sign && ack_response.find(order_id)==ack_response.end())
			ack_response[order_id] = client_id;
	}

	unordered_map<uint64_t, string> entrybook;
	for(auto it=order_entry.begin(); it!=order_entry.end(); it++)
	{
		uint64_t client_id = (*it)->getClientId();
	//	bool buy = (*it)->getSide()==1;
		if ( entrybook.find(client_id)==entrybook.end() )
		{
			entrybook[client_id] = (*it)->getInstrument();
		}
	}

	for(auto it=order_fill.begin(); it!=order_fill.end(); it++) 
	{
		uint32_t order_id = (*it)->getOrderId();
		uint32_t fill_qty = (*it)->getFillQty();
		if ( ack_response.find(order_id)!=ack_response.end() )
		{
            uint64_t client_id = ack_response[order_id];
			if ( entrybook.find(client_id) != entrybook.end() ) 
			{
				string instrument = entrybook[client_id];
				if ( vol_instrument.find(instrument) == vol_instrument.end() )
				{
					vol_instrument[instrument] = fill_qty;
				}
				else
				{
					vol_instrument[instrument] += fill_qty;
				}
			}
		}		
	}

}

// print the volume of trade per instrument
void Exchmessage::printVolTrade() {
	int len = vol_instrument.size();
        int idx = 0;
	for(auto&it : vol_instrument) 
	{
		cout << it.first << " : " << it.second;
		if (idx < len-1)
		   cout << ","; 
                idx++;
	}

	cout << endl;
}

// Divide the input message into separate packets and put into corresponding message type containers. 
// Find the most active trader, most liquidlity trader, and volume per instrument
void Exchmessage::Dividemessage() {
	uint64_t idx = 0;
	OrderEntry *oentry;
	OrderAck *oack;
	OrderFill *ofill;

	while(idx<file_size) 
	{
		Header *head = new Header (inmessage+idx);
                string marker = head->getMarker();
                
                if (marker!="ST") 
                {
                     cout << "Header marker wrong!" << endl;
                     return; 
                }

		OrderTypeEnum otype= head->getMsgtype();
		uint16_t mlen = head->getMsglen();		
		idx += 22;

		//cout << "Packet: " << total_packets << "\t" << otype << "\t" << mlen << endl;

                string endstr = "";
		switch (otype)
		{
		case OrderTypeEnum::Entry :
			oentry = new OrderEntry (inmessage+idx, mlen);
		        endstr = oentry->getEndStr();
                        if (endstr!="DBDBDBDB")
                        {
                           cout << "Entry parket END wrong!" << endl;
                           return;
                        } 	
                        order_entry.push_back(oentry);
			
			break;
		case OrderTypeEnum::Ack :
			oack = new OrderAck(inmessage+idx, mlen);
		        endstr = oack->getEndStr(); 
                        if (endstr!="DBDBDBDB") 
                        {
                            cout << "Ack packet END wrong!" << endl;
                            return;
                        }	
                        order_ack.push_back(oack);

			break;
		case OrderTypeEnum::Fill:
			ofill = new OrderFill(inmessage+idx, mlen);
		        endstr = ofill->getEndStr();
                        if (endstr!="DBDBDBDB")
                        {
                            cout << "Fill packet END wrong!" << endl;
                            return;
                        }	
                        order_fill.push_back(ofill);

			break; 
		}

        idx += mlen;
	total_packets++;
	}

	FindActiveTrader();
	FindLiquidTrader();
	FindVolPerInstrument();
}
