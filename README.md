## Introduction and Main Goal
- The main goal of this project consisted in implementing a (key, value) pair storage service (along the lines of the java.util.Map interface of the Java API) similar to the same one used by Amazon to support its Web services In this sense, the data structures used to store this information are a simple linked list and a hash table, given their high efficiency at the search level.

## Description of the Implementation Stages
**The project had 4 implementation stages:**
- In **step 1**, data structures were defined and several functions were implemented to deal with
with data manipulation by implementing a hash table from scratch. It was also built
a module for data serialization, which aimed to highlight the
need to serialize data for communication in distributed applications.
- In **step 2** a simple client-server system was implemented, in which the server was
responsible for maintaining a hash table and the client responsible for communicating with the
server to perform operations on the table. We used **Protocol Buffers from
Google** to automate the serialization and de-serialization of data, based on a
sdmessage.proto file with the definition of the message to be used in communication,
for both requests and responses.
- In **step 3** a concurrent system was created that accept and process orders from multiple
clients simultaneously through the use of multiple threads. This system also guaranteed the
management of competition in access to data shared on the server. Additionally, we added a functionality to obtain relevant server statistics.
- In **step 4** the system started to support fault tolerance through server state replication, following the Chain Replication Model, using **ZooKeeper** service to coordenate.
