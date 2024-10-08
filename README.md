# C-SOCKET
A starting point for designing a low-level server-client communication “socket” in C, setting up client-server connections and constructing structures for data transmission.

## DESCRIPTIONS OF THE USING FUNCTIONS

       socket()  creates  an  endpoint for communication and returns a file descriptor that refers to that endpoint.  The
       file descriptor returned by a successful call will be the lowest-numbered file descriptor not currently  open  for
       the process.

       The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.
       The htons() function converts the unsigned short integer hostshort from host byte order to network byte order.

       The  accept()  system call is used with connection-based socket types (SOCK_STREAM, SOCK_SEQPACKET).  It extracts the first
       connection request on the queue of pending connections for the listening socket, sockfd, creates a  new  connected  socket,
       and  returns  a new file descriptor referring to that socket.  The newly created socket is not in the listening state.  The
       original socket sockfd is unaffected by this call.

       When  a  socket  is  created  with socket(2), it exists in a name space (address family) but has no address assigned to it.
       bind() assigns the address specified by addr to the socket referred to by the file descriptor  sockfd.   addrlen  specifies
       the size, in bytes, of the address structure pointed to by addr.  Traditionally, this operation is called “assigning a name
       to a socket”.

       The  connect()  system call connects the socket referred to by the file descriptor sockfd to the address specified by addr.
       The addrlen argument specifies the size of addr.  The format of the address in addr is determined by the address  space  of
       the socket sockfd; see socket(2) for further details.

       The  recv(),  recvfrom(), and recvmsg() calls are used to receive messages from a socket.  They may be used to receive data
       on both connectionless and connection-oriented sockets.  This page first describes common  features  of  all  three  system
       calls, and then describes the differences between the calls.

       struct sockaddr_in {
           sa_family_t     sin_family;     /* AF_INET */
           in_port_t       sin_port;       /* Port number */
           struct in_addr  sin_addr;       /* IPv4 address */
       };

## BUILD
       gcc -o tcp_server TCPServer.c
       gcc -o tcp_client TCPClient.c

