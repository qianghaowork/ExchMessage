# ExchMessage
An implemtation of the simplified version of exchange message. It could be compiled under Linux and Windows. It parsed a binary file which records the order message between traders and Exchange. 

The member functions summarized some information about the trades, such as how many packets are there in the stream?
How many message exist for message type? Who was the most active trader? (measured as the trader largest filled volume)
Who provided the most liquidity? (measured as the trader with the largest GFD volume entered into the market) Volume of trades per instrument?

Each message contains a header, some message specific fields, and a termination string

Order Entry Message
This message is sent from the trader to the exchange to indicate that a trader wishes to buy or sell an instrument at the specified price and quantity

Order Acknowledgement Message
This message is sent from the exchange to the trader in response to an 'Order Entry Message' indicating if the order was accepted or rejected

Order Fill Message
This message is sent from the exchange to the trader to indicate that part or all of an order has been filled

////////////////////////////////////////////////
The tricky part is multiple trader information in order fill message as belows. I only consider the first trader and therefore got the wrong answer for active trader. 

==> repeating group
firm_id(uint8_t)
trader_tag(char[3])
qty(uint32_t)
