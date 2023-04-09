#include "dependencies.hpp"
#include "assist/message_utils.hpp"
#include "assist/message_structs.hpp"
#include "assist/queues.hpp"

#define PORT 8059
#define BUFSIZE 1024
#define SERVER_BACKLOG 250
#define CONFIG_FILE_PATH "./test/superpeerconfigtest.cfg"
#define THREAD_POOL_SIZE 20
#define QUERY_MESSAGES_THREAD_POOL_SIZE 3
#define MAX_CONNECTIONS 25

std::string super_peer_id;
std::string ip = "127.0.0.1";
int port = PORT;
int accepted_connections = 0;
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;
std::thread conn_thread_pool[THREAD_POOL_SIZE];
//Vectors to store registered peers and super peers
std::vector<peer_info> peers;//vector to store registered peers
std::vector<super_peer_info> super_peers;//vector to store registered super peers
//Mutexes and condition variables
std::mutex mtx;
std::mutex peer_mutex;
std::mutex sconnect_mtx;
std::condition_variable cv;
std::condition_variable qcv;
//Queues
SocketQueue socket_queue;

/*** Helper functions for super peer ***/

bool loadConfig(const char filepath[]){//function to load configuration parameters from config file
	char real_path[PATH_MAX+1];
	realpath(filepath,real_path);
	std::ifstream in(real_path);
	if(!in.is_open()){
		std::cout << "config file could not be opened\n";
		return false;
	}
	std::string param;
	std::string value;

	while(!in.eof()){
		in >> param;
		in >> value;

        if(param == "IP"){
            ip = value;
        }
        else if(param == "PEER_ID"){
            super_peer_id = value;
        }

		else if(param == "PORT"){
			port = stoi(value);
        }		
	}
	in.close();

	return true;
}

void check(int n, const char *msg){
    std::string err = super_peer_id + ":" + msg;
	if(n<0){
		perror(err.c_str());
	}
}
/*****************************************************************************************************************/

/*** Socket functions for super peer ***/

//connect to IP and port and return the socket
int sconnect(char IP[], int port){
    std::lock_guard<std::mutex> lock(sconnect_mtx);
    
    int sock = 0, client_fd;
    struct sockaddr_in server_addr;
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("error creating socket\n");
        return -1;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, IP, &server_addr.sin_addr) <=0){
        printf("\nInvalid address or address not supported\n");
        return -1;
    }
    if((client_fd = connect(sock, (SA*)&server_addr, sizeof(server_addr))) < 0){
        printf("Connection to server on port %d failed!\n",port);
        return -1;
    }
    //printf("Connection to server on port %d successful\n",port);
    return sock;
}

/*** Core functions for super peer ***/

//function to update a peer's files, essentially a re-registration function
int update_all_peer_files(int client_socket){
	char buffer[BUFSIZE] = {0};
    peer_info info;
    if((recv(client_socket, buffer, BUFSIZE, 0))<0){
        printf("Error receiving peer info for update\n");
        return -1;
    }
    deserialize_peer_info(buffer, info);
    for(auto it = peers.begin(); it != peers.end(); it++){
        if(strcmp(it->ip,info.ip) == 0 && it->port == info.port && it->peer_id == info.peer_id){
            peer_mutex.lock();
            it->files = info.files;
            peer_mutex.unlock();
            printf("Peer %s:%d updated\n",info.ip,info.port);
            close(client_socket);
            return 0;
        }
    }
    printf("%s at %s:%d updated\n",info.peer_id.c_str(),info.ip,info.port);
    /*
    printf("Files hosted:");
    for (int i = 0; (long unsigned int)i < info.files.size(); i++)
    {
        printf(" %s, size: %d", info.files[i].file_name.c_str(),info.files[i].file_size);
    }
    */
    close(client_socket);
    return 0;
}

//function to register peer with super peer
int register_peer(int client_socket){
	char buffer[BUFSIZE] = {0};
    peer_info info;
    if((recv(client_socket, buffer, BUFSIZE, 0))<0){
        printf("Error receiving peer info for registration");
        return -1;
    }
    deserialize_peer_info(buffer, info);
    if(peers.size() > 0){
        for(int i=0;(long unsigned int)i < peers.size();i++){
            if(strcmp(peers[i].ip,info.ip) == 0 && peers[i].port == info.port){
                printf("Peer %s:%d already registered",info.ip,info.port);
                close(client_socket);
                return -1;
            }
        }
    }
    peer_mutex.lock();
    peers.push_back(info);
    peer_mutex.unlock();
    printf("%s %s:%d registered\n",info.peer_id.c_str(),info.ip,info.port);
    /*
    printf("Files hosted:");
    for (int i = 0; (long unsigned int)i < info.files.size(); i++)
    {
        printf(" %s, size: %d", info.files[i].file_name.c_str(),info.files[i].file_size);
    }
    */
    close(client_socket);
    return 0;
}

//function to unregister peer with super peer
int unregister_peer(int client_socket){
	char buffer[BUFSIZE] = {0};
    peer_info info;
    if((recv(client_socket, buffer, BUFSIZE, 0))<0){//receive peer info
        printf("Error receiving peer info for unregistration");
        return -1;
    }
    deserialize_peer_info(buffer, info);
    /*
    //print the files the peer is hosting (for testing purposes)
    printf("Files hosted:");
    for (int i = 0; (long unsigned int)i < info.files.size(); i++)
    {
        printf(" %s, size: %d", info.files[i].file_name.c_str(),info.files[i].file_size);
    }
    */
    close(client_socket);
    //check if peer is registered and unregister it
	for(int i=0;(long unsigned int)i<peers.size();i++){
		if(strcmp(peers[i].ip,info.ip)==0 && peers[i].port==info.port){
            peer_mutex.lock();
			peers.erase(peers.begin()+i);
			printf("%s at %s:%d unregistered successfully\n",info.peer_id.c_str(),info.ip,info.port);
            peer_mutex.unlock();
			close(client_socket);
			return 0;
		}
	}
    //if peer is not registered
    printf("%s %s:%d not found\n",info.peer_id.c_str(),info.ip,info.port);
    return -1;
}

//check all registered weak peers for a file, return a vector of peer_info structs containing info of the weak peers that have the file
std::vector<peer_info> queryWeakPeers(std::string file_name){
	std::vector<peer_info> result;
    peer_mutex.lock();
	for(int i=0;(long unsigned int)i<peers.size();i++){
		for(int j=0;(long unsigned int)j<peers[i].files.size();j++){
			if(peers[i].files[j].file_name == file_name){
				result.push_back(peers[i]);
			}
		}
	}
    peer_mutex.unlock();
	return result;
}

int queryFile(int cs){

    

    std::vector<peer_info> peersWithFile = queryWeakPeers("test.txt");
}

//function to handle incoming connections and determine what kind of command is being given
void handle_connection(int client_socket){
    Message msg;
    char buffer[BUFSIZE] = {0};
    read(client_socket, buffer, BUFSIZE);
    deserialize_message(buffer, msg);

	if(strncmp(msg.content.c_str(),"register",8)==0){//register peer
        send(client_socket, "OK", 2, 0);
		//printf("Registering peer\n");
		register_peer(client_socket);
	}
	else if(strncmp(msg.content.c_str(),"unregister",10)==0){//unregister peer
        send(client_socket,"OK",2,0);
		//printf("Unregistering peer\n");
		unregister_peer(client_socket);
	}
    else if(strncmp(msg.content.c_str(),"Query",5)==0){//query from super peer"
        send(client_socket, "OK", 2, 0);
        //printf("Querying super-peers for file\n");
        queryFile(client_socket);        
    }
	else if(strncmp(msg.content.c_str(),"update",6)==0){
        send(client_socket, "OK", 2, 0);
		//printf("Updating peer files\n");
		update_all_peer_files(client_socket);
	}
	else if(strncmp(msg.content.c_str(),"list",4)==0){//list peers
		printf("Listing peers and their files\n");
		//TODO
	}
	else if(strncmp(msg.content.c_str(),"single_add",13)==0){
		//add_peer_file(client_socket);
	}
    else if(strncmp(msg.content.c_str(),"single_delete",13)==0){
        //delete_peer_file(client_socket);
    }
	else{//invalid command
        send(client_socket, "Invalid command", 15, 0);
		printf("Invalid command: %s\n",buffer);
	}

    accepted_connections--;

	fflush(stdout);
	try {
        close(client_socket);
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << '\n';
    }
	//printf("connection closed\n");
    return;
}

void thread_pool_function(){
    while(true){
        int *client_socket;
        std::unique_lock<std::mutex> lock(mtx);
        if((client_socket = socket_queue.dequeue()) == NULL){
            cv.wait(lock);
        }
        lock.unlock();
        if(client_socket != NULL){
            handle_connection(*client_socket);
        }
    }
    return;
}

void handle_sigpipe(int sig) {
    void *array[10];
    size_t size;
    // get void*'s for all entries on the stack
    size = backtrace(array, 10);
    // print out all the frames to stderr
    fprintf(stderr, "%s Error: signal %d: %s\n",super_peer_id.c_str(),sig, strsignal(sig));
    backtrace_symbols_fd(array, size, STDOUT_FILENO);
    //exit(1);
}

int main(int argc, char *argv[]){//main function
	if(argc < 2){
		printf("Usage: %s <config file path>\n",argv[0]);
		loadConfig(CONFIG_FILE_PATH);
	}else{
		loadConfig(argv[1]);
	}

    signal(SIGPIPE, handle_sigpipe);

	int opt = 1;

	int server_socket, client_socket, addr_size;
	sockaddr_in server_addr, client_addr;

	check((server_socket = socket(AF_INET, SOCK_STREAM, 0)),"socket creation failed");//create socket

	if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){//set socket options
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	server_addr.sin_family = AF_INET;//configure server
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	check(bind(server_socket,(sockaddr*)&server_addr,sizeof(server_addr)),"bind failed");//bind socket
	check(listen(server_socket, SERVER_BACKLOG),"listen failed");//listen for connections

	printf("Waiting for connections on port %d...\n",port);

    //create thread pool for handling connections
    for(int i = 0; i < THREAD_POOL_SIZE; i++){
        conn_thread_pool[i] = std::thread(&thread_pool_function);
    }
	while(true){//accept connections
		addr_size = sizeof(sockaddr_in);
		check((client_socket = accept(server_socket, (sockaddr*)&client_addr,(socklen_t*)&addr_size)),"failed to accept client connection");//accept connection
        accepted_connections++;

        if(accepted_connections > MAX_CONNECTIONS){
            printf("TOO MANY CONNECTIONS\n");
        }

        int *client = (int*)malloc(sizeof(int));
        *client = client_socket;

        std::unique_lock<std::mutex> lock(mtx);
        socket_queue.enqueue(client);
        lock.unlock();
        cv.notify_one();
    }
	return 0;
}


