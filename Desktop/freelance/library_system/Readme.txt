a. Full Name:
b. Student ID:
c. Optional: Not attempted
d. Files:
    a. serverM: this file is responsible for creating tcp and udp server for main server, this file is responsible for verification of user as well as exchage of
    data between main to library servers as well as main to client servers. on the start of the saerver members data will be populatedbin unordered hash map for verification function.
    this files contains multiple helper functions for detailed understating refer to the comments above each function in the file.
    b. serverL: this file is responsible for building of literature udp server and on bootup sends the books data to tha main function also loads all trhe data in
    in unordered map for future querying. this file contains multiple helper function for detailed understanding of function refer to comments above each function
    c. serverH: this file is responsible for building of history udp server and on bootup sends the books data to tha main function also loads all trhe data in
    in unordered map for future querying. this file contains multiple helper function for detailed understanding of function refer to comments above each function
    d. serverS: this file is responsible for building of science udp server and on bootup sends the books data to tha main function also loads all trhe data in
    in unordered map for future querying. this file contains multiple helper function for detailed understanding of function refer to comments above each function
    e. client: this file is responsible for user interaction and populates the user query to the respective servers, for which it contains multiple helper functionsfor detailed query of each function refer to the comment section
    f. members.txt: Stores encrypted username and password with ',' as delimiter.
    g. science.txt: Stock keeping unit (SKU) of Science books in the library.
    h. literature.txt: SKU of Literature books in the library.
    i. history.txt: SKU of History books in the library.
    j. Makefile: Uses two flags: one to specify which compiler to use to avoid faliure due to environment inconsistency and another to specify that lpthread is to be used (since we are using multi-threading)
e. FOrmat of messages exchanged:
g. Idiosyncrasy: All edge cases and test cases mentioned in the assignment doc is passing and working.
h. I have referred to the following sources and upon learning from them I wrote my own code and logic. The links of all the resources I referred to are:
    a. https://cs.colby.edu/maxwell/courses/tutorials/maketutor/
    b. https://www.geeksforgeeks.org/socket-programming-cc/
    c. https://www.geeksforgeeks.org/handling-multiple-clients-on-server-with-multithreading-using-socket-programming-in-c-cpp/
    d. Beej’s Guide
    e. https://cplusplus.com/doc/tutorial/files/
    f. https://cplusplus.com/doc/tutorial/typecasting/
    g. https://www.linuxhowtos.org/C_C++/socket.htm
    h. https://www.linuxhowtos.org/manpages/2/bind.htm
    i. https://www.linuxhowtos.org/manpages/3/getaddrinfo.htm
    j. https://www.javatpoint.com/multithreading-in-cpp-with-examples