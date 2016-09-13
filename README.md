# ExchMessage

The project is a C++ implementation of the simplified version of parsing exchange message from NYSE and NASDAQ. It could be compiled and executed under Linux and Windows. It reads from a binary file which contains the order messages between traders and Exchange, parses it into categories and finds the most active trader and liquid trader. 

##1) Descriptions

The binary file is divided into packets. How many packets are there in the stream? 

Each message contains a header, some message specific fields, and a termination string. How many messages are there and what type of each message? 

Each trader sends multiple orders to exchange with different prices and quantities. Exchange will reply to the trader whether the order is successful or not. The project will analyze the orders of each trader and give a summary of trader order situations. Who is the most active trader? Active trader is measured as the trader with largest filled volume. 
Who provided the most liquidity? It's measured as the trader with the largest GFD volume entered into the market. 

##2) Order Message Types

Order Entry Message
This message is sent from the trader to the exchange to indicate that a trader wishes to buy or sell an instrument at the specified price and quantity

Order Acknowledgement Message
This message is sent from the exchange to the trader in response to an 'Order Entry Message' indicating if the order was accepted or rejected

Order Fill Message
This message is sent from the exchange to the trader to indicate that part or all of an order has been filled

##3) Comments

It's straightforward to implement the functionality. However, the tricky part is that multiple trader information is existed in the order fill message as belows. At first, I only extract the first trader information and ignore others. I got wrong answers of active trader for some test cases. The project also includes checking the validility of termination code. More statistics of orders and traders information could be extended into this project. 

==> repeating group
firm_id(uint8_t)
trader_tag(char[3])
qty(uint32_t)

