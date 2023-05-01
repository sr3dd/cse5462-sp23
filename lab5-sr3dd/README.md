# CSE 5462 - Lab 5

Protocol and Lab specific guidelines [here](#3-guidelines-and-assumptions)

## 1. How to run

```
make
./drone5 <portnumber>
```

The program will then prompt the user to enter the number of rows and columns in the drone network grid:

```
Enter rows: m
Enter columns: n
```

If the location of the server read from the config file is out of the grid as per given dimensions, then the program 
will notify and exit.

## 2. Drone

The drone program acts as a client/server which receives network messages while also sending out user input to each client specified in the configuration file.

### 2.1 Server

Receives message from network and checks
1. 'toPort' value of message => gives port to which message originally addressed
2. 'location' value of message => gives location of drone which sent the message
3. 'TTL' value of message => gives time-to-live, which lets our server know if message is valid/can be retransmitted

Based on the above 3 values, the server:
1. stores and prints the message
2. discards the message
3. retransmits the message to other drones in the config file
    
### 2.2 Client

The client sends outgoing messages based on:
1. user input
2. network message not deemed to be correctly addressed and still valid (TTL > 0)
   
### 2.3 User Input

1. User input is expected to be the message payload that will be sent out to all other drones in the network.

2. ```fromPort:<value> location:<value> TTL:<value>``` will automatically be appended to the user's input string 
and therefore not expected from user. Note that user providing that will likely cause our payload to be 
discarded by other drones in the network as the message will end up having duplicate keys.

1. User is also expected to input the dimensions of the drone network grid which lets our server know if a transmitting 
client is in range or not to be heard.

## 3. Guidelines and Assumptions

### Server/client Specific
1. To run the program you will type ./drone3 <portnumber>
2. Location is an integer => possible values: >= 1
3. Messages should be received from command prompt and from the network with NO LIMIT on how many are received 
4. Config.file formatting:
   - space delimited. example: one line -> *server_IP* *port_number* *location*\n
   - What if you get a line that has an extra “token”? Ex: 127.0.0.1 4200 90 12323
     - Ignore the bad token/ignore line (do not die)
   - What if there are duplicate config lines? (Implementation detail)

### Protocol Specific

1. Need to have the following in your outbound message:
   1. toPort:number 
   2. version:5 
   3. fromPort:number 
   4. location:number 
   5. msg: " some_text_here "
   6. TTL:number 
2. The fromPort, TTL, and location are NOT provided on the command line, your program MUST fill those in.
   - location is the location of the sender 
   - fromPort is the port of the sender
   - TTL will be 6 but should be a #define in your code
3. Actions to take when you get a message:
   1. If the version is wrong, ignore and do nothing
   2. If the message doesn’t have the other required fields, ignore and do nothing
   3. If the toPort matches your port number, AND the location is in range (distance  of 2 or less) AND TTL >= 0, print the message
   4. If the toPort matches your port number but the location is a distance of more than 2 away, ignore the message.
   5. If the toPort does NOT match your location, but the message is within a distance of 2, and TTL > 0
      - Decrement the TTL
      - Change the location to be YOUR location
      - Leave all other fields the same
      - Forward the message to all your peers
4. What if you get keys not in specification, eg: length:4200 => Print the extra keys and values
5. Expected output same as output of Lab 4.

---
