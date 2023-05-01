# CSE 5462 - Lab 7

Protocol and Lab specific guidelines [here](#3-guidelines-and-assumptions)

## 1. How to run

```
make
./drone7 <portnumber>
```

The program will then prompt the user to enter the number of rows and columns in the drone network grid:

```
Enter rows: m
Enter columns: n
```

If the location of the server read from the config file is out of the grid as per given dimensions, then the program 
will notify and exit.

## 2. Drone

The program emulates a drone in the real world - it has a location and an address. Assuming that multiple instances of such drones exists in a grid to make up a swarm, each drone can reach another only if they are both in range of each other and messages sent between the two are correctly addressed.

Messages sent from one drone to another are acknowledged in kind and so the sender knows exactly what has been sent out and which of those have been received while a receiver knows what has been received and which sender has already been notified of it.

In this lab, we introduce the concept of movement. Now, a drone program does not permanently use the location assigned to it in the configuration but can be moved on the receipt of a MOV type message from another drone.

We also make an effort to reduce the flooding of messages in the network by making sure a message is only sent out to those drones which have not already seen a particular message as recorded in the "send-path" of the message.

### 2.1 Server

Receives messages from network. Messages are of three types:
1. REG
2. ACK (contain a type:ACK key-value pair in the message payload)
3. MOV (contains a move:x key-value pair in the message payload)

Regular messages have mandatory keys: version, toPort, fromPort, send-path, TTL, location
Move messages have mandatory keys: version, fromPort, toPort, move (TTL is optional since no forwarding)

Based on the key-value pairs, the server:
1. stores, prints and acknowledges a message received for the first time
2. Duplicate messages are acknowledged and discarded
3. prints and tracks the ACK status of outgoing messages based on received ACKs
4. Duplicate ACKs are discarded
5. Classifies a MOV command message, appends the move-specific keys required and sends it out
6. MOV messages are not forwarded and can only be processed by the toPort drone
7. MOVE messages causes the drone to update its location to x where message has key-value pair 'move:x'

Location and TTL rules apply as they did in previous labs.
    
### 2.2 Client

The client component of the drone handles outgoing messages. Outgoing messages can be:
1. User input which is sent to the correct address
2. Regular Messages in range which are forwarded to all other drones
   
### 2.3 User Input

For sending out a REG message, a user should input only msg and toPort:
```
toPort:43015 msg:"Hello, this is a regular message!"
```
and other key value pairs required to make up the message (version, TTL, fromPort, location)
 will be appended automatically.

 Similarly, for sending out a MOV message, a user should only input toPort and move:
 ```
 toPort: 43007 move:7
 ```
 and other key-value pairs will be auto-appended (version, fromPort, TTL).

**DO NOT** provide other key-value pairs in STDIN as that will cause the resulting 
message to contain duplicate keys and therefore become invalid at the receiver side.

## 3. Guidelines and Assumptions

1. MOV type messages are not forwarded
2. MOV type messages are received regardless of distance and drone location
3. Only forward to those ports which are not already present in the send-path of the message
4. Print out the send-path even for duplicate messages for validation of the above feature

---
