
# < Hierarchical peer-to-peer system implementation in C++ >

[Description](#Description)

[Installation/Set-Up](#Installation/Set-Up)

[Usage](#Usage)

[Evaluation](#Evaluation)

### NOTE!
To run the scripts included, you'll probably have to use "chmod +x script_name.sh"

## Description
This project includes 2 separate but functionally related programs: 
- a super peer program that acts as an index server that tracks peers connected to it, as well as other super peers in the overlay network,

The super peer can have its IP and PORT specified through a configuration file, as well as all the connected super peers IPs and ports to form the overlay network. The super peer uses a thread pool to handle incoming connections. When a new connection is accepted, its socket is added to a queue, which is then dequeued and handled by a worker thread the worker thread reads the data that was sent, which it is expecting to be some command. This communication is standardized to consist of a header which tells the receiver the length of the message, followed by the message. The super peer accepts a handful of different commands: Query, QueryHit, register, update, and unregister are the most important ones. Register will add the peer to the super peers struct which contains information on all connected peers, such as the peer's name, it's IP and port, and the files it hosts. Conversely, unregister removes that peer from the super peer. Update will update the super peer with all the files that are currently in the peers hosting directory. The super peer can receive a Query for a file, where it will search through it's connected peers and see if any of them have the requested file. Regardless of if the file is found, the super peer will forward the Query to all its connected super peers. These Queries are handled by a separate thread pool that is dedicated to processing and forwarding QueryMessages, using a queue. If a queried file is found in a super peer's list of peers, a QueryHit message is generated. This QueryHit carries the same globally unique ID as the query for the file. This QueryHit message is propagated back through the chain of senders to the original sender via usage of a map that tracks QueryMessage ID's and the upstream peer from which it came. There is a separate thread pool dedicated to handling QueryHit messages. Altogether, these 3 thread pools make up the functionality of a super peer node. 

- a weak peer to host files and download files

The weak peer first connects to the super peer and registers itself on the network, sending the relevant information including the port it listens on and the files it hosts. The port the peer listens on, the super peer's IP and port, as well as the directory name of the folder containing files for the peer to host are all specified through a config file. This configuration file is specified as the first argument when the program is run. The second argument is either a comma separated list of files containing the names of files to download, or a flag to signal the program to run in interactive "-i" or host only "-h" mode. The weak peer can query the super peer for files, and if the file is found it will eventually receive a QueryHit for that file. The QueryHit contains the file name and the location of the host in it. The weak peer adds this information to a queue that contains pairs of file names and their host's locations. The peer has a dedicated thread pool for dequeueing and downloading files. In addition, it created a new thread to handle incoming connections to it. Since the weak peer doesn't receive as much traffic, there is not a thread pool to handle incoming connections, instead a new thread is simply created to handle it.

## Installation/Set-Up

To use the program(s), make sure you have g++ compiler installed, and that the program(s) are being executed in a linux environment. From the root directory of the project, type "make" into the terminal to compile the code and generate the executable binaries. They will be named "peernode.o" and "superpeer.o".

# Dependencies

The full list of dependencies can be viewed in "src/dependencies"

## Usage

To generate sample cfg files, directories, and files for the peers to host, use the ./init.sh script. It accepts 3 arguments: the type of overlay network to set up, the number of super peers, the number of weak peers per super peer.

    ./init.sh a2a 4 5

    ./init.sh tree 5 3

Will generate configs and folders for an all-to-all overlay network with 4 super peers, each of which have 5 connected weak peers.

You can start a single instance of a super peer by typing 

    ./superpeer.o peer_configs/super_peers/superpeer1.cfg
    
into your terminal. There should be output indicating that the server is waiting for connections.

Open another terminal and navigate to the same directory, and type in  

    ./peernode.o peer_configs/weak_peers/peer1.cfg 

You can open more terminals and manually create as many super peers and weak peers as you like. It is recommended for testing that you create the number of weak and super peers that you initialized the network for.

To make things easier, you can use the ./testdep.sh script, which is used the same way as ./init.sh but it will deploy all the peers in host only mode, except for 1 which you can interact with in the terminal. You can query files hosted by other peers by typing the file name and hitting enter.

    ./testdep.sh a2a 3 3 

## Evaluation and Scripts

In addition, there are two testing scripts included to perform automated testing. The first, ./test.sh is used in the same format as ./init.sh and ./testdep.sh. It will deploy the super peers and peers, and have the peers automatically download 5 files from another random peer, update the server, and then download 5 more files from another random peer. After some time passes and the files are all downloaded, the ./getresults.sh script will get all the query response times from the logs generated and put them in the results directory. The ./perform all tests automatically performs a series of tests with differing numbers of super peers, connected weak peers, and overlay network topologies. It will gather the query response times for each and put them in a csv file in the results directory. 