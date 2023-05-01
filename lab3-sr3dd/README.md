# CSE 5542 - Lab 3

## 1. How to run

```
make
./drone3 <portnumber>
```

## 2. Drone

The drone program acts as a client/server which receives network messages while also sending out user input to each client specified in the configuration file.

### 2.1 Server

Flow: 

1. listens on a port number (specified by user as a command line argument)
2. receives a message
3. validates message
   1. no duplicate keys
   2. message contains mandatory keys: version, port, and location
   3. port key value matches port of the server
   4. version key value is 3 (for this lab)
4. if message is valid
   1. prints the parsed message string as key-value pairs
   2. stores the message as a node in a linked list on server


### 2.2 Client

Flow:

1. Receives a string input from STDIN (user)
2. If the string is not empty
   1. Sends it to each client address in the configuration file


### 2.3 User Input

User input is expected to be the message payload that will be sent out to all other drones in the network.

*Implementation detail*:

1. the "location:value" will automatically be appended to the user's input string 
and therefore not expected from user. Note that user providing that will likely cause our payload to be 
discarded by other drones in the network as the message will end up having duplicate 'location' keys/

## 3. Guidelines and Assumptions

### Server/client Specific
1. To run the program you will type ./drone3 <portnumber>
2. Location is an integer => possible values: 1-50
3. Messages should be received from command prompt and from the network with NO LIMIT on how many are received 
4. Config.file formatting:
   - space delimited. example: one line -> *server_IP* *port_number* *location*\n
   - What if you get a line that has an extra “token”? Ex: 127.0.0.1 4200 90 12323
     - Ignore the bad token/ignore line (do not die)
   - What if there are duplicate config lines? (Implementation detail)

### Protocol Specific

1. Our current version = 3 (version:3)
   - Anything other than version 3, we ignore/delete
   - Print “bad version”
2. Payload must have a port number, version, and a location key-value pair in what is sent to the drone. 
   - Should not crash if not specified
   - Discarded/deleted/not printed
   - Print “message doesn’t have a port/version/location key-value” (Implementation detail)
3. Keys are unique within a message 
   - example: (version:1 version:2 IS INVALID)
   - Message with a duplicate key received -> invalid message -> say message is invalid or something similar
4. Keys not in specification (not port,version, or location) should be accepted and printed along with the others 
   - example: ttl:4200
5. The 'msg' key in a payload
   - At most one 'msg' key in a payload.
   - If exists, value must be surrounded by “". (Like in previous labs)

---

