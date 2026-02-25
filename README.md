# DistMemStorage
Distributive Memory Storage

Authors: Sebastian Manza and Declan Murray

We chose to implement a distributive memory storage reminiscient of Chords. For any set value input by the client, the request is sent to whatever node the client is connected to. The key for this request is then hashed. Each storage node has a hash range created by checking the least significant bit of the hash. If the key is within the nodes hash range, it knows it belongs to it, and safely stores it in a hash table on that node. If it does not belong to it, it simply forwards it to the next storage node, which notes that it it is supposed to have it. Since both nodes know what hashes they need to have in their own table, they are consistent in their beliefs about who has what data. Therefore any write will only change the value on one server. Get requests work in a very similar manner, forwarding the request to the necessary node, who then returns the response to the requester, whether that be another node or the client. If it was another node, that node passes it back again, to the client. Our system is fairly tolerant of bad inputs and failures, though we ask any prospective clients to not check this too thoroughly, as it may not be perfect. 

We note that we built this system to be easily scalable. More nodes can easily be added to the network with very few changes (namely that of the listed names of storage nodes). Nodes would then connect in a loop, and the hash range would be divided up amongst the 2 or 3 least significant bits, rather than just one. Multiple clients should be able to connect at once to either different or the same storage nodes, and send get and set requests concurrently.

The program can be run by the following set of commands:

```
make
./runStorage.sh  # in one terminal
./client 8080    # in second terminal

# For testing
make
./runStorage.sh   # in one terminal
./test 8080       # in second terminal
```

Requests should be written of the form:
get:key or 
set:key:value
