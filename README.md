
# <File Simple Peer-to-peer file sharing implementation in C++>

[Description](#Description)

[Installation/Set-Up](#Installation/Set-Up)

[Usage](#Usage)

[Evaluation](#Evaluation)

[TL;DR](#TL;DR)

### NOTE!
To run the scripts included, you'll probably have to use "chmod +x script_name.sh"

## Description
This project includes 2 separate but functionally related programs: 
- an index server that tracks peers on the network, and can find peers hosting a particular file

The server code sets up a socket to listen to incoming TCP connections and create a new thread to handle those connections. The server can listen on a user specified port from a configuration file. The location of the config file is passed as an argument to the program when you run it. Once a connection is established, the `handle_connection` function is called in a newly created thread, and it begins by reading the data sent on the client socket. The messages consists of a message length (specified via a "header" struct) followed by the message. The server recognizes a few commands. It can register a peer, where it gets the peer's IP address, the port the peer is listening to, and the list of files that the peer is hosting. Conversely, it can unregister a peer and remove it from the list of peers on the network. It can also query a file name, looking in it's list of peers for peers that are hosting that file, and returning to the requester a list of IP's and ports of peers hosting that file. Additionally, it can update the entire list of files a peer hosts, and it can add or remove a file from a the list of files a peer is hosting.


- a peer to host files and download files

The peer first connects to the indexing server and registers itself on the network, sending the port it listens on and the files it hosts. The port the peer listens on, the indexing server's IP and port, as well as the directory name of the folder containing files for the peer to host are all specified through a config file. It then waits for user input, assuming that any input is a file name (except for the "wait", "update", and "exit" commands) where it can query the server for a file and subsequently attempt to download the file from a random peer that is hosting it. If the peer for some reason does not have the file or something goes wrong, it will try another peer, until there are no more peers with that file to try. The user can manually update the list of files the peer is hosting by typing "update", in case the directory the peer is hosting gets changed during runtime. The user can tell the peer node to wait (basically just do nothing) by typing "wait" with some integer (ex: "wait 10"), or the user can tell the peer to "exit", in which case the peer unregisters itself from the index server and shuts down.

## Installation/Set-Up

To use the program(s), make sure you have g++ compiler installed, and that the program(s) are being executed in a linux environment. From the root directory of the project, type "make" into the terminal to compile the code and generate the executable binaries. They will be named "peernode.o" and "client.o".

# Dependencies

The full list of dependencies can be viewed in "src/dependencies"

## Usage

To generate sample cfg files, directories, and files for the peers to host, use the 

    ./init.sh

script. It will create 16 directories with 1 of each size file each, as well as 16 sample config files for the peer nodes.

You can start the server by typing 

    ./server.o peer_configs/server.cfg
    
into your terminal. There should be output indicating that the server is waiting for connections. The server will print out certain information indicating the commands it receives. 

Open another terminal and navigate to the same directory, and type in  

    ./peernode.o peer_configs/peer1.cfg 

Open 2 more terminals and type 

    ./peernode.o peer_configs/peer2.cfg

    ./peernode.o peer_configs/peer3.cfg

You now have 3 peer nodes running. You should see in the server terminal that it has registered 3 peer nodes. You can then type a filename into peer any peer's terminal, such as t32kb1.txt from peer 3. It will contact the server, get peer 1's information, and download the file. You should see the output in the respective terminals. You can now type "update" into peer 3's terminal. Now, if you request that same file from peer 3, you will see that there are now 2 peers hosting that file. 

## Evaluation

In addition, there are some scripts included that perform automated tests using the sample files and configs generated. The names should be pretty self-explanitory. In the "textXnodes.sh" scripts, X nodes are created, each of which downloads 10 files from two other nodes. After 5 files are downloaded, each node updates it's file list to the server. These tests generate log files in the "logs" folder, and generate results in the "results" folder.


## TL;DR

Run "make" in the root directory to compile the source files. In one terminal, run the 

    ./server.o
    
file (from the directory it is compiled into), do the same with 

    ./client.o

in another terminal. This will run the program interactively. Follow the prompts.

You can also run the "run.sh" script to start the server in the background and only interact with the client. Just make sure to kill the process when you're done using pkill. 

Alternatively, run the "clients_parallel.sh" or "clients_serial.sh" scripts with either an integer argument or no argument, and behold as the script generates N number of clients to download files
from the server, and conveniently aggregates the results in the "results" directory!

Or, run "perform_all_test.sh" script to automatically run tests using a varied number of clients, and aggregates the results in "results" for you!