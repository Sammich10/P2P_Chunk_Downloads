#include "dependencies.h"

#define PORT 8080
#define BUFSIZE 1024
#define SERVER_BACKLOG 200
#define CONFIG_FILE_PATH "./server.cfg"

int port = PORT;

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

struct file_info{//struct to store file information
	std::string file_name;
	int file_size;
};

struct peer_info{//struct to store registered peer information
	char ip[15];
	uint16_t port;
	std::vector<file_info> files;
};

std::vector<peer_info> peers;//vector to store registered peers

struct MessageHeader {
    uint32_t length; // Length of message data in bytes
};

bool loadConfig(const char filepath[]){//function to load configuration parameters from config file
	char real_path[PATH_MAX+1];
	realpath(filepath,real_path);
	std::ifstream in(real_path);
	if(!in.is_open()){
		std::cout << "config file could not be opened\n";
		return false;
	}
	std::string param;
	int value;

	while(!in.eof()){
		in >> param;
		in >> value;

		if(param == "PORT"){
			port = value;
		}
		
	}
	in.close();

	return true;
}

void check(int n, const char *msg){
	if(n<0){
		perror(msg);
		exit(1);
	}
}

int update_peer_files(int client_socket, struct sockaddr_in client_addr){//function to update peer files
	MessageHeader h;
	peer_info p;
	
	int peer_port;
	int operation;

	recv(client_socket, &h, sizeof(h), 0); //receive message header
	recv(client_socket, &peer_port, h.length, 0); //receive peer port
	recv(client_socket, &operation, h.length, 0); //receive operation

	strcpy(p.ip,inet_ntoa(client_addr.sin_addr));
	p.port = peer_port;

	if(peers.size() < 1){
		printf("Peer %s:%d not registered\n",p.ip,p.port);
		return 0;
	}

	for(int i=0;(long unsigned int)i < peers.size();i++){
		if(strcmp(peers[i].ip,p.ip) == 0 && peers[i].port == p.port){
			if(operation == 0){//delete a file
				char file_name_buffer[BUFSIZE] = {0};
				recv(client_socket, &h, sizeof(h), 0); //receive message header
				recv(client_socket, &file_name_buffer, h.length, 0); //receive message body
				std::string file_name = file_name_buffer;
				for(int j=0;(long unsigned int)j < peers[i].files.size();j++){
					if(peers[i].files[j].file_name == file_name){
						peers[i].files.erase(peers[i].files.begin()+j);
						printf("Peer %s:%d updated\nFile %s deleted",p.ip,p.port,file_name.c_str());
						return 0;
					}
				}
			}
			else if(operation == 1){//add a file
				file_info f;
				char file_name_buffer[BUFSIZE] = {0};
				recv(client_socket, &h, sizeof(h), 0); //receive message header
				recv(client_socket, &file_name_buffer, h.length, 0); //receive message body
				recv(client_socket, &f.file_size, sizeof(f.file_size), 0); //receive message body
				std::string file_name = file_name_buffer;
				f.file_name = file_name;
				for(int j = 0; (long unsigned int)j<peers[i].files.size();j++){
					if(peers[i].files[j].file_name == file_name){
						printf("Peer %s:%d already hosting file %s",p.ip,p.port,file_name.c_str());
						return 0;
					}
				}
				peers[i].files.push_back(f);
				printf("Peer %s:%d updated\nFile %s added",p.ip,p.port,file_name.c_str());
				return 0;
				}
		}
	}
	return 0;
}

int register_peer(int client_socket, struct sockaddr_in client_addr){//function to register peer
	
	MessageHeader h;
	peer_info p;
	
	int peer_port;
	int num_files_int;

	recv(client_socket, &h, sizeof(h), 0); //receive message header
	recv(client_socket, &peer_port, h.length, 0); //receive peer port
	recv(client_socket, &num_files_int, h.length, 0); //receive number of files

	strcpy(p.ip,inet_ntoa(client_addr.sin_addr));
	p.port = peer_port;

	if(peers.size() > 0){
		for(int i=0;(long unsigned int)i < peers.size();i++){
			if(strcmp(peers[i].ip,p.ip) == 0 && peers[i].port == p.port){
				printf("Peer %s:%d already registered\n",p.ip,p.port);
				return 1;
			}
		}
	}

	peers.push_back(p);
	
	for(int i=0;i<num_files_int;i++){
		file_info f;
		char file_name_buffer[BUFSIZE] = {0};
		
		recv(client_socket, &h, sizeof(h), 0); //receive message header
		recv(client_socket, &file_name_buffer, h.length, 0); //receive message body
		recv(client_socket, &f.file_size, sizeof(f.file_size), 0); //receive file size

		f.file_name = file_name_buffer;
		peers.back().files.push_back(f);
	}

	printf("Peer %s:%d registered successfully\n",p.ip,p.port);
	return 0;
}

int unregister_peer(int client_socket, struct sockaddr_in client_addr){//function to unregister peer
	MessageHeader h;
	peer_info p;
	
	int peer_port;

	recv(client_socket, &h, sizeof(h), 0); //receive message header
	recv(client_socket, &peer_port, h.length, 0); //receive peer port

	strcpy(p.ip,inet_ntoa(client_addr.sin_addr));
	p.port = peer_port;

	for(int i=0;(long unsigned int)i<peers.size();i++){
		if(strcmp(peers[i].ip,p.ip)==0 && peers[i].port==p.port){
			peers.erase(peers.begin()+i);
			printf("Peer %s:%d unregistered successfully\n",p.ip,p.port);
			return 0;
		}
	}

	printf("Peer %s:%d not found\n",p.ip,p.port);
	return 0;
}

std::vector<peer_info> queryFile(std::string file_name){//function to query file
	std::vector<peer_info> result;
	for(int i=0;(long unsigned int)i<peers.size();i++){
		for(int j=0;(long unsigned int)j<peers[i].files.size();j++){
			if(peers[i].files[j].file_name == file_name){
				result.push_back(peers[i]);
			}
		}
	}
	return result;
}

int findFile(int client_socket){//function to find file
	MessageHeader h;
	char file_name_buffer[BUFSIZE] = {0};
	std::string file_name;
	std::vector<peer_info> result;

	recv(client_socket, &h, sizeof(h), 0); //receive message header
	recv(client_socket, &file_name_buffer, h.length, 0); //receive message body

	file_name = file_name_buffer;
	result = queryFile(file_name);

	int num_peers = result.size();
	send(client_socket, &num_peers, sizeof(num_peers), 0); //send number of peers

	for(int i=0;(long unsigned int)i<result.size();i++){
		std::string colon = ":";
		std::string peer_info = result[i].ip + colon + std::to_string(result[i].port);
		h.length = peer_info.length();
		send(client_socket, &h, sizeof(h), 0); //send message header
		send(client_socket, peer_info.c_str(), h.length, 0); //send message body
	}
	return 0;
}

void handle_connection(int client_socket, struct sockaddr_in client_addr){//function to handle connections
	char buffer[BUFSIZE] = {0};
	//size_t bytes_read;
	//char real_path[PATH_MAX+1];
	MessageHeader h;
    //receive message header
	recv(client_socket, &h, sizeof(h), 0);
    //receive message body
	recv(client_socket, buffer, h.length, 0);

	printf("Message received: %s\n",buffer);

	if(strncmp(buffer,"register",8)==0){//register peer
		printf("Registering peer\n");
		register_peer(client_socket,client_addr);

	}
	else if(strncmp(buffer,"unregister",10)==0){//unregister peer
		printf("Unregistering peer\n");
		unregister_peer(client_socket,client_addr);

	}
	else if(strncmp(buffer,"list",4)==0){//list peers
		printf("Listing peers and their files\n");
		//TODO
	}
	else if(strncmp(buffer,"find",4)==0){//get file
		printf("Querying peers for file\n");
		findFile(client_socket);
	}
	else if(strncmp(buffer,"update",6)==0){//exit
		update_peer_files(client_socket,client_addr);
	}
	else{//invalid command
		printf("Invalid command\n");
	}

	fflush(stdout);
	close(client_socket);
	//printf("connection closed\n");
}

int main(int argc, char *argv[]){//main function
	if(argc < 2){
		printf("Usage: %s <config file path>\n",argv[0]);
		loadConfig(CONFIG_FILE_PATH);
	}else{
		loadConfig(argv[1]);
	}

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

	while(true){//accept connections
		addr_size = sizeof(sockaddr_in);
		check((client_socket = accept(server_socket, (sockaddr*)&client_addr,(socklen_t*)&addr_size)),"failed to accept client connection");//accept connection

		//printf("Connection successful!\n");

		std::thread tr(&handle_connection, client_socket, client_addr);//create thread to handle connection
		tr.detach();
	}
	return 0;
}


