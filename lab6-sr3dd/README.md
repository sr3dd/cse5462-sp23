# CSE 5462 - Lab 6

Protocol and Lab specific guidelines [here](#3-guidelines-and-assumptions)

## 1. How to run

```
make
./drone6 <portnumber>
```

The program will then prompt the user to enter the number of rows and columns in the drone network grid:

```
Enter rows: m
Enter columns: n
```

If the location of the server read from the config file is out of the grid as per given dimensions, then the program 
will notify and exit.

## 2. Drone

The drone program acts as a client/server which receives network messages while also sending out user input to each client specified in the configuration file. In Lab 6, the drone functionality is extended to record the path travelled by a message in transit as well as the ability of the receiver to send back ACKs to the sender and for the sender to track the delivery of messages based on these ACKs.

### 2.1 Server

Receives messages from network. Messages are of two types:
1. REG
2. ACK (contain a type:ACK key-value pair in the message payload)

Messages also have mandatory keys: version, toPort, fromPort, send-path, TTL, location

Based on the key-value pairs, the server:
1. stores, prints and acknowledges a message received for the first time
2. Duplicate messages are acknowledged and discarded
3. prints and tracks the ACK status of outgoing messages based on received ACKs
4. Duplicate ACKs are discarded

Location and TTL rules apply as they did in previous labs (4 and 5)
    
### 2.2 Client

The client component of the drone handles outgoing messages. Outgoing messages can be:
1. User input which is sent to the correct address
2. Messages in range which are forwarded to all other drones
   
### 2.3 User Input

The user should provide these key-value pairs to create a valid outgoing message payload:
1. version
2. toPort
3. msg

## 3. Guidelines and Assumptions

Introduction of 2 new concepts.

1. we will have the receiver (destination) of the message ACK the message. This implies that we will have to have a sequence number in the message. This will be key value of seqNumber:n (where n is an integer). 
2. The first time the receiver gets a message, it will print out the message like in lab 5. It will then send an ACK back to the sender. If the receiver gets the message again, it will print out the sequence number, fromPort and a message saying “duplicate”. It will then ACK the message again. 
3. We have a new message type called ACK. This will be key value of type:ACK
4. The second concept we will introduce is the concept of tracking the nodes (port numbers) from sender to receiver. 
5. This will be included in every message (whether it is an ACK or not). We will call this the “send-path”. 
6. Every hop along the way will add its portNumber to the sendpath, separated by a comma.
7. The destination will print out the send-path along with the message. This will be ```send-path:port1,port2```

Assumptions:

1. Each toPort-fromPort will start at seq number 1
2. There will be a maximum of 100 messages between any toPort-fromPort pair (don’t have to check for this, it will help when creating data structures)
3. The maximum message length will be 200 bytes (this includes all fields in the message)

---
