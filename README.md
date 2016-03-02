Application-Layer-File-Transfer-Protocol
========================================

Application Level file sharing protocol with support for upload/download and indexed searching.


**How to Use:**

1)Copy the code onto two different machines or two different folders on the same machine

2)Start two different sessions, compile the code ( gcc Protocol.c -o client), and run it (./client)

3) The program asks you to enter the port on which it will listen, enter a value (eg: 5000)

4) Choose the mode (O for TCP and 1 for UDP)

5) Enter the server address to which you want to connect ( Eg: 6000)

Note: Now the program tries to connect with the server listening on this port and will wait till it establishes a connection

6) Once connection is established, the program will work and the 2 network clients can start listening for requests waiting to share files.
