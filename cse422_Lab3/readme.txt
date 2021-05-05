Lab 3: Multiplexed Interprocess Communication

Tara Renduchintala: trenduchitnala@wustl.edu
Anderson Gonzalez: anderson.g@wustl.edu

***********************SERVER IMPLEMENTATION**************************
---Data structures within the Server-----

1) Linked List to hold pointer to each file
	-- We created a linked list to hold a pointer to each file so that we could iterate through the linked list and ensure that each fragment file got opened and read. This allowed us to make sure that all of the fragment files were passed to the clients. 

	-- The linked list also helped us keep track of what files were opened and what files failed as well as closing files when they were done being used.

2) AVL Tree Linked List to hold the sorted lines when recieved
	-- After the lines from the client were read from the socket, they were then placed into an AVL tree according to its line number. Once the message "done" was recieved, the empty the events portion of that socket so that it would stop monitoring. Then, when the number of receptions (which is incremented on reciept of "done") is the same as the number of fragment files, the server uses the AVL Tree and writes the ordered nodes to the file. 

	-- The AVL tree does dynamic storing, therefore the tree is always balanced at time of writing. 


--- Use of Polling -----
In order to monitor the sockets and events, we initially tried to use epoll(). However, after much struggle, we decided to sacrifice the efficiency and use poll() instead. Poll allowed us to monitor all of the sockets as well as pay attention to any events that occured.

To correctly use poll we had to cretae a server socket that would listen to connections. Once the number of connections was the same as the number of fragment files, the server would stop listening for connections and only focus on recieving. The server socket was created at the port number provided as an input argument. 

----Getting the IP Address---
Since we wanted to be able to use the connection remotely as well as locally, we ensured the IP address that would be printed out was always the external IP address. That way, we were never displaying the local loopback address (which would be of no use to a remote client).


***************************CLIENT IMPLEMENTATION*********************************
---Data Structures Used---
1) Again, we used an AVL tree to store the sorted nodes and send them back in a sorted order. This implementation required the same logic as what was used in the server function. The client would recieve the line, parse it, store the line number as the key and the content within the node and then order the tree such that it is balanced. That way, when sending back the lines, they were in a ordered fashion.

---Checking Arguments---
The client would check for a valid port address and a valid IP address. It would return an error if the IP address was not valid as well as the port. It would also print out a usage method if the port was not a number. 

---Use of Select---
We used the Select() IO because it was easier to manage what event should be triggered as we could trigger it ourselves. When the reading was complete, we were able to trigger the write functionality. It worked well with the poll as each client was supposed to do one thing.

---Remote Client---
We wrote the client in such a way that it could be executed remotely using the IP Address of the server that was provided. It could also be ran locally.


***********************BUILD INSTRUCTIONS*************************
Run the compileFiles.sh file. It should compile the programs such that they can be run by the computer. When we ran the compileFiles.sh file with Wall, we recieved no warnings. 


Enable the sh file to be executable: 
chmod +x compileFiles.sh

Run the compilation:
./compileFiles.sh

When running, ensure that the avl_tree.h is in the same folder as the server and client files as they both need them. Therefore, if the client is being run on a machine that is not the same as the server, ensure the avl_tree.h file is also included there. 
*******************************TESTING********************************************

-----Edge Case: no line number at the start of line-----

Initially, we did not manage this edge case. However, when we are parsing the string on the client side,
we see that the difference betweent the first space and the start of the line is 0, indicating that there is no 
line number. Thus, when the difference is 0, we skip that line and don't add it to the list of ordered nodes.

The output file contains all but the string without the line number.



-----Edge Case: multiple of the same line number----

The implementation of the AVL tree by geeks for geeks does not allow the reuse of keys (as it should not) so when a line number is duplicated, then the second occurance does not appear in the output file. Therefore, the output would only consist of the "well-formed" lines.



-----Edge Case: no space between line number and test----

Initially, the program was taking this line number and converting it to an int which would result in multiple nodes being a 0. We changed the implementation such that is skips a line that has no space between the line number and the text.



-----Reuse same port immediatly after closing old program-----

Even though we closed the port, it seemd as though the program still owned it. Therefore, we set the socket to be set to be reusable. Once we made the port reusable, we were able to have multiple runs on the same port. 



-----Closing port on error----
Initially, when the program returned an error, we did not close the ports properly which led to binding the address to produce an error. Therefore, we properly closed to socket such that when an error occured, the server socket was properly closed. 

----Hanging Program----
The program somtimes would hang after the connection was made. After investigation, we found out it was because we didn't FD_ZERO our read_fds in the client. Once we zero'd that out, it stopped hanging.

----Invalid server address---
If the server address is not valid, then the client returns an error indicating that the
server address was not valid.

---Checking Remote Connection---
Initially when we were running remotely, there was an error on select. It turns out we needed to initlaize our timeout to 2 seconds, which we had set to 0 in the beginning. Once that was set, we were able to communicate with the socket. Furthermore, we had to get the "wlan0" IP address to be able to get the external IP. 


******COMMAND LINE TESTING*******

---Invalid input file---
When the program encounters an invalid input file, it prints out an error message indicating that there was an error opening the file.

---Invalid framgent file---
when the program encounters an invalid fragment file it prints out an error message indicating that there was an error opening the fragment file.

---Invalid Port Number---
When the program encounters an invalid prot number, it prints out an error message indicating an invalid port number and then outputs the usage message.

---Wrong number of paramters---
When the program encounters a wrong number of parameters, it prints out the usage message.

***************************KNOWN LIMITATIONS********************************
Sometimes, there are additional characters being printed out and we are unsure why that was happening. It was only for certain lines and we couldn't
find a general solution as to why it was happening.

We also can't run the surver on something that does not have "wlan0" (the Pi does and we assumed we wouldn't be expected to run the Pi elsewhere). As noted above, when we tried to use the method the Professor described in class, it did not display an exterenal IP Address that could be used by client not on the same machine. i


********************DEVELOPMENT EFFORT********************************
We spent about 35-40 hours on this lab. Definitely the hardest one of the three. 
