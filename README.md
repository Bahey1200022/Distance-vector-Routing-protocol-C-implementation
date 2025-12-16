
Distance Vector Routing Protocol (C)

This project implements a Distance Vector (DV) routing protocol in C, using UDP sockets and two concurrent threads:

Receiver Thread: handles incoming HELLO and DV messages

Sender Thread: periodically sends HELLO messages and DV updates

The implementation includes neighbor discovery, neighbor timeout detection, and Bellman–Ford distance vector updates.
