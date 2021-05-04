Lab 3: Multiplexed Interprocess Communication









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


