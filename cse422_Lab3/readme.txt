Lab 3: Multiplexed Interprocess Communication









*******************************TESTING********************************************

-----Edge Case: no line number at the start of line-----

Initially, we did not manage this edge case. However, when we are parsing the string on the client side,
we see that the difference betweent the first space and the start of the line is 0, indicating that there is no 
line number. Thus, when the difference is 0, we skip that line and don't add it to the list of ordered nodes.

The output file contains all but the string without the line number.

-----Edge Case: multiple of the same line number----

The implementation of the AVL tree by geeks for geeks does not allow the reuse of keys (as it should not) so when a line number is duplicated, then the second occurance does not appear in the output file.




