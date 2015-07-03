#ifndef _EXCHANGEAPI_H
#define _EXCHANGEAPI_H

#include <utility>
#include <stdio.h>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <fstream> 
#include <unordered_map>

using namespace std;

enum OrderTypeEnum { Entry = 1, Ack, Fill };
enum DirectionEnum { ToExch = 0, FromExch };
enum SideEnum { Buy = 1, Sell};
enum TimeForceEnum { IOC = 1, GFD};
enum OrderStatusEnum { Good = 1, Reject};
enum RejectCodeEnum { NoCode = 0, InvalidProduct, InvalidPrice, InvalidQty};

class Header{
private: 
	string marker;
	uint8_t msg_type;
	uint64_t sequence_id;
	uint64_t timestamp;
	uint8_t msg_direction;
	uint16_t msg_len;

public:
	Header(char* message) 
	{
		marker.assign(message, 2);
		memcpy(&msg_type, message+2, 1);
		memcpy(&sequence_id, message+3, 8);
		memcpy(&timestamp, message+11, 8);
		memcpy(&msg_direction, message+19, 1);
		memcpy(&msg_len, message+20, 2);
	}

	OrderTypeEnum getMsgtype() {
		return (OrderTypeEnum)msg_type;
	}
	uint16_t getMsglen() {
		return msg_len;
	}
        string getMarker() { return marker; }

};

class OrderEntry {
private:
	double price;
	uint32_t qty;
	string instrument;
	uint8_t side;
	uint64_t client_id;
	uint8_t time_in_force;
	string trader_tag;
	uint8_t firm_id;
	string firm;
        string endstr;      
public:
	OrderEntry(char* message, uint16_t msize) {
		int len = 0;
		uint64_t intprice; 
		memcpy(&intprice, message+len, 8);
		len += 8;
		price = (double) intprice / 10000;
		memcpy(&qty, message+len, 4);
		len += 4;
		instrument.assign(message+len, 10);
		len += 10;
		memcpy(&side, message+len, 1);
		len += 1;
		memcpy(&client_id, message+len, 8);
		len += 8;
		memcpy(&time_in_force, message+len, 1);
		len += 1;
		trader_tag.assign(message+len, 3);
		len += 3;
		memcpy(&firm_id, message+len, 1);
		len += 1;
		firm.assign(message+len, msize-len-8);
		len += msize-len-8;
		endstr.assign(message+len, 8);
	}

	string getTraderTag() { return trader_tag; }
	TimeForceEnum getTimeForce() { return (TimeForceEnum) time_in_force; }
	uint32_t getQty() { return qty; }
	uint64_t getClientId() { return client_id; }
	SideEnum getSide() { return (SideEnum)side; }
	string getInstrument() { return instrument; }
        string getEndStr() { return endstr; }
};

class OrderAck {
private:
	uint32_t order_id;
	uint64_t client_id;
	uint8_t order_status;
	uint8_t reject_code;
        string endstr; 
public:
	OrderAck(char* message, uint16_t msize) {
		int len = 0;
		memcpy(&order_id, message+len, 4);
		len += 4;
		memcpy(&client_id, message+len, 8);
		len += 8;
		memcpy(&order_status, message+len, 1);
		len += 1;
		memcpy(&reject_code, message+len, 1);
		len += 1;
		endstr.assign(message+len, 8);
	}

	uint64_t getClientId() { return client_id; }
	uint32_t getOrderId() { return order_id; }
	OrderStatusEnum getSatus() { return (OrderStatusEnum)order_status; }
        string getEndStr() { return endstr; }
};

class OrderFill {
private:
	uint32_t order_id;
	double fill_price;
	uint32_t fill_qty;
	uint8_t no_of_contras;
	vector<uint8_t> firm_id;
	vector<string> trader_tag;
	vector<uint32_t> qty;
        string endstr;
public: 
	OrderFill(char *message, uint16_t msize) {
		int len = 0;
		memcpy(&order_id, message+len, 4);
		len += 4;
		uint64_t fprice;
		memcpy(&fprice, message+len, 8);
		fill_price = (double)fprice/10000;
		len += 8;
		memcpy(&fill_qty, message+len, 4);
		len += 4;
		memcpy(&no_of_contras, message+len, 1);
		len += 1;
                for(int i=0; i<no_of_contras; i++) 
                {
                   uint8_t tmp_id; 
                   uint32_t tmp_qty;
                   string tmp_tag;
		   memcpy(&tmp_id, message+len, 1);
                   firm_id.push_back(tmp_id);
		   len += 1;
		   tmp_tag.assign(message+len, 3);
                   trader_tag.push_back(tmp_tag);
		   len += 3;
		   memcpy(&tmp_qty, message+len, 4);
                   qty.push_back(tmp_qty);
		   len += 4;
                }
		endstr.assign(message+len, 8);
	}
	vector<string> getTraderTag() { return trader_tag; }
	vector<uint32_t> getQty() { return qty; }
	uint32_t getOrderId() { return order_id;}
        uint32_t getFillQty() { return fill_qty;}
        string getEndStr() { return endstr; }
};

class Exchmessage{
private: 
	streampos file_size;
	char* inmessage;
	bool fileopen_sign;

	uint64_t total_packets;
	vector<OrderEntry*> order_entry;
	vector<OrderAck*> order_ack;
	vector<OrderFill*> order_fill;

	string most_active_trader_tag; 
	string most_liquidity_trader_tag;
	
	unordered_map<string,uint64_t> vol_instrument;

	void FindActiveTrader();
	void FindLiquidTrader();
	void FindVolPerInstrument();

public:
	Exchmessage(char* fname) {

		// open the binary file
        ifstream file (fname, ios::in|ios::binary|ios::ate);
		fileopen_sign = file.is_open();
		if (fileopen_sign) 
		{
			file_size = file.tellg();
			inmessage = new char [file_size];
			file.seekg (0, ios::beg);
			file.read (inmessage, file_size);
			file.close();
		}

		total_packets = 0;
		order_entry.clear();
		order_ack.clear();
		order_fill.clear();
		vol_instrument.clear();
	}

	void Dividemessage();
	void printVolTrade();

	bool isFileopen() {	return fileopen_sign; }

	uint64_t getPacket(){ return total_packets; }
	uint64_t getEntryNum() { return order_entry.size(); }
	uint64_t getAckNum() { return order_ack.size(); }
	uint64_t getFillNum() { return order_fill.size(); }
	string getActiveTrader() { return most_active_trader_tag; }
	string getLiquidTrader() { return most_liquidity_trader_tag; }

	~Exchmessage() {
		if (fileopen_sign) 
		{
			delete [] inmessage;
		}

        for(auto it=order_entry.begin(); it!=order_entry.end(); it++) 
			delete *it;
      	for(auto it=order_ack.begin(); it!=order_ack.end(); it++) 
			delete *it;
	for(auto it=order_fill.begin(); it!=order_fill.end(); it++) 
			delete *it;
	}
};

#endif
