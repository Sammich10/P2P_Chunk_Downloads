#include "dependencies.h"

#define SERVER_PORT 8080
#define CONFIG_FILE_PATH "./peernode.cfg"
#define DEFAULT_IP "127.0.0.1";
#define SERVER_BACKLOG 200
#define BUFSIZE 1024

using namespace std;

char server_ip[15] = "127.0.0.1";
int port = 8081;
int dl_attempts = 3;
string host_folder = "peer_files";

size_t total_dl_size = 0;
double total_dl_time = 0;

namespace fs = std::filesystem;

struct MessageHeader {
    uint32_t length; // Length of message data in bytes
};

struct file_info{//struct to store file information
	std::string file_name;
	int file_size;
};

std::vector<file_info> fileList;

struct peer_info{//struct to store registered peer information
	char ip[15];
	uint16_t port;
	std::vector<file_info> files;
};

typedef struct sockaddr SA;

void check(int n, const char *msg);

bool loadConfig(const char filepath[]){//function to load configuration parameters from config file
	char real_path[PATH_MAX+1];
	realpath(filepath,real_path);
	ifstream in(real_path);
	if(!in.is_open()){
		cout << "config file could not be opened\n";
        in.close();
		return false;
	}
	string param;
	string value;

	while(!in.eof()){
		in >> param;
		in >> value;

		if(param == "PORT"){
			port = stoi(value);
		}
        else if(param == "HOST_FOLDER"){
            host_folder = value;
        }
        else if(param == "SERVER_IP"){
            strcpy(server_ip,value.c_str());
        }
		
	}
	in.close();
	return true;
}

//function to create a socket and listen for connections
int slisten(int port){
    int opt = 1;

	int server_socket;
	sockaddr_in server_addr;

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

	//printf("Waiting for connections on port %d...\n",port);
    return server_socket;
}

//connect to server or peer and return socket
int sconnect(char IP[], int port){
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

//create a new socket and download a file specified
int downloadFile(int socket, char filename[]){
    
    int valread;
    char read_buffer[BUFSIZE] ={0};

    int sock = socket;
    
    MessageHeader h;
    h.length = strlen("download");
    send(sock, &h, sizeof(h), 0); //send message header
    if((send(sock, "download", strlen("download"), 0))<0){ //send download command to server
        printf("error sending download command to server\n");
        return -1;
    }

    h.length = strlen(filename);
    send(sock, &h, sizeof(h), 0); //send message header
    if((send(sock, filename, strlen(filename), 0))<0){ //send filename to server
        printf("error sending file name to server\n");
        return -1;
    }
    
    MessageHeader header;
    //receive message header
    valread = recv(sock, &header, sizeof(header),0);
    //receive message data
    valread = read(sock, read_buffer, header.length);
    if(valread < 0){
        printf("error reading response from server"); 
        close(sock);
        return -1;
    }
    //if file not found, tell user
    if(strncmp(read_buffer, "File not found", strlen("File not found")) == 0){
        printf("File not found: %s\n", filename);
        close(sock);
        return -1;
    }

    printf("Downloading file...\n");
    send(sock, "r", 1, 0);//signal to server we are ready to download

    //filename[strlen(filename)-1] = '\0';

    //create path to download folder
    std::string n_host_folder = host_folder + "/" + filename;

    FILE *fp = fopen(n_host_folder.c_str(), "w");
    //To time the speed of transfer
    clock_t start;
	double dur;
	start = clock();
	size_t bytes_recvd = 0;

    while((valread = read(sock, read_buffer, BUFSIZE))>0){
        //printf("%s",read_buffer);
        fwrite(read_buffer, valread,1,fp);
        bytes_recvd+=valread;
    }
    //record speed and size of transfer
    dur = (clock()-start)/((double)CLOCKS_PER_SEC);
    printf("Received %zu bytes in %.16f seconds\n",bytes_recvd, dur);
    total_dl_size +=bytes_recvd;
    total_dl_time +=dur;
    fclose(fp);


    if(valread < 0){
        printf("error reading response from server\n\n"); 
        close(sock);
        return -1;
    }

    
    close(sock);
    return 0;
}

//check if the upload directory exists, if not, create it
void checkUploadDirect(){
    struct stat st;
    char* d = new char[host_folder.length()];
    strcpy(d, host_folder.c_str());
    if(((stat(d,&st)) == 0) && (((st.st_mode) & S_IFMT) == S_IFDIR)){
        //directory exists
    }else{
        if(mkdir(d, S_IRWXU | S_IRWXG | S_IROTH)){
            printf("Failed to create directory %s\n",d);
            exit(EXIT_FAILURE);
        }
    }
}

//check if the download directory exists, if not, create it
void checkDownloadDirect(){
    struct stat st;
    char* d = new char[host_folder.length()];
    strcpy(d, host_folder.c_str());
    if(((stat(d,&st)) == 0) && (((st.st_mode) & S_IFMT) == S_IFDIR)){
        //directory exists
    }else{
        if(mkdir(d, S_IRWXU | S_IRWXG | S_IROTH)){
            printf("Failed to create directory %s\n",d);
            exit(EXIT_FAILURE);
        }
    }
}

void updateFileList(){
    std::string path = get_current_dir_name();
    path += "/" + host_folder;
    for(const auto & entry : fs::directory_iterator(path)){
        //std::cout << entry.path() << std::endl;
        file_info f;
        f.file_name = entry.path().filename();
        f.file_size = fs::file_size(entry.path());
        fileList.push_back(f);
    }
}

std::vector<file_info> getFileList(){
    updateFileList();
    return fileList;
}

//update the server with the files that are currently hosted, either delete or add
int update_hosted_files(int sock, int operation, char filename[]){

    std::vector<file_info> hostedFiles = getFileList();
    MessageHeader h;
    h.length = sizeof("update");
    check(send(sock, &h, sizeof(h), 0), "error sending message header"); //send message header
    check(send(sock, "update", strlen("update"), 0), "error sending update command to server"); //send update command to server

    h.length = sizeof(int);
    check(send(sock, &h, sizeof(h), 0), "error sending message header"); //send message header
    check(send(sock, &port, sizeof(port), 0), "error sending port to server");
    check(send(sock, &operation, sizeof(operation), 0), "error sending operation to server");
    
    if(operation == 0){//delete file
        h.length = strlen(filename);
        check(send(sock, &h, sizeof(h), 0), "error sending message header"); //send message header
        check(send(sock, filename, strlen(filename), 0), "error sending filename to server");
    }
    else if(operation == 1){//add file
        h.length = strlen(filename);
        std::string path = get_current_dir_name();
        path += "/" + host_folder + "/" + filename;
        int filesize = fs::file_size(path);
        check(send(sock, &h, sizeof(h), 0), "error sending message header"); //send message header
        check(send(sock, filename, strlen(filename), 0), "error sending filename to server");
        check(send(sock, &filesize, sizeof(filesize), 0), "error sending filesize to server");
    }
    
    return 0;
}

int register_peer(int port){

    std::vector<file_info> hostedFiles = getFileList();

    int sock = sconnect(server_ip, SERVER_PORT);

    check(sock, "error connecting to server");
    
    MessageHeader h;
    h.length = strlen("register");

    check(send(sock, &h, sizeof(h), 0), "error sending message header"); //send message header
    check(send(sock, "register", strlen("register"), 0), "error sending message body to server");
    
    h.length = sizeof(int);
    int num_hosted_files = hostedFiles.size();
    check(send(sock, &h, sizeof(h), 0), "error sending message header"); //send message header
    check(send(sock, &port, sizeof(port), 0), "error sending port to server");
    check(send(sock, &num_hosted_files, sizeof(num_hosted_files), 0), "error sending number of files to server");

    for(int i = 0; (unsigned long int)i < hostedFiles.size(); i++){
        h.length = hostedFiles[i].file_name.length();
        check(send(sock, &h, sizeof(h), 0), "error sending message header"); //send message header
        check(send(sock, hostedFiles[i].file_name.c_str(), h.length, 0), "error sending file name to server");//send file name to server

        int file_size = hostedFiles[i].file_size;
        check(send(sock, &file_size, sizeof(file_size), 0), "error sending file size to server");

    }

    close(sock);
    return 0;
}

void unregister_peer(){
    int sock = sconnect(server_ip, SERVER_PORT);

    check(sock, "error connecting to server");
    
    MessageHeader h;
    h.length = strlen("unregister");

    check(send(sock, &h, sizeof(h), 0), "error sending message header"); //send message header
    check(send(sock, "unregister", strlen("unregister"), 0), "error sending file name to server");
    h.length = sizeof(int);
    check(send(sock, &h, sizeof(h), 0), "error sending message header"); //send message header
    check(send(sock, &port, sizeof(port), 0), "error sending port to server");

    close(sock);
    return;
}

//download a file from a peer
void downloadFromPeer(std::vector<std::string> peers, char* file_name){
    int random_peer;
    //choose a random peer to download from
    if(peers.size() > 1){
        random_peer = (rand() % (peers.size()-1));
    }else{
        random_peer = 0;
    }
        
    std::string peer_ip = peers[random_peer].substr(0, peers[random_peer].find(":"));
    int peer_port = std::stoi(peers[random_peer].substr(peers[random_peer].find(":")+1, peers[random_peer].length()));

    printf("Connecting to peer %s:%d\n", peer_ip.c_str(), peer_port);

    int peer_sock = sconnect((char*)peer_ip.c_str(), peer_port);

    int dls = downloadFile(peer_sock, file_name);
    
    //if the download failed, remove the peer from the list and try again with a different peer
    if(dls == -1){
        printf("Error downloading file from peer %s:%d\n", peer_ip.c_str(), peer_port);
        peers.erase(peers.begin() + random_peer);
        if(peers.size() == 0){
            printf("No peers available to download file from\n");
            return;
        }
        downloadFromPeer(peers, file_name);
    }
    

}

std::vector <std::string> findPeers(char* file_name){
    int sock = sconnect(server_ip, SERVER_PORT);

    std::vector <std::string> valid_peers;
    
    MessageHeader h;
    h.length = strlen("find");

    check(send(sock, &h, sizeof(h), 0), "error sending message header"); //send message header
    check(send(sock, "find", strlen("find"), 0), "error sending find request to server");
    h.length = strlen(file_name);
    check(send(sock, &h, sizeof(h), 0), "error sending message header"); //send message header
    check(send(sock, file_name, strlen(file_name), 0), "error sending file name to server");

    int num_peers;
    check(recv(sock, &num_peers, sizeof(num_peers), 0), "error receiving number of peers from server");

    if(num_peers == 0){
        printf("No peers found for file %s\n", file_name);
    }else{
        printf("Found %d peers for file %s\n", num_peers, file_name);
        
        for(int i = 0; i < num_peers; i++){
            std::string peer_ip_port;
            char buffer[BUFSIZE] = {0};

            check(recv(sock, &h, sizeof(h), 0), "error receiving message header"); //receive message header
            check(recv(sock, &buffer, h.length, 0), "error receiving peer port from server");
            peer_ip_port = buffer;
            //printf("Peer %s\n",peer_ip_port.c_str());  
            valid_peers.push_back(peer_ip_port);
        }
    }
    close(sock);
    return valid_peers;
}

void handleConnection(int client_socket){
    char buffer[BUFSIZE] = {0};
    MessageHeader h;
    recv(client_socket, &h, sizeof(h), 0);
    recv(client_socket, buffer, h.length, 0);
    printf("Received message: %s\n", buffer);

    if(strcmp(buffer, "download") == 0){
        recv(client_socket, &h, sizeof(h), 0);
        recv(client_socket, buffer, h.length, 0);
        printf("Received file name: %s\n", buffer);
        std::string file_name = buffer;

        std::string path = get_current_dir_name();
        path += "/" + host_folder + "/" + file_name;

        FILE *fp = fopen(path.c_str(), "rb");

        if(fp == NULL){
            printf("File not found\n");
            h.length = strlen("File not found");
            send(client_socket, &h, sizeof(h), 0);
            send(client_socket, "File not found", h.length, 0);
            close(client_socket);
            //if the file is not found, update the server and tell it to remove the file from the list of hosted files
            update_hosted_files(sconnect(server_ip, SERVER_PORT), 1, (char*)file_name.c_str());
            return;
        }else{
            printf("File found\n");
            h.length = strlen("File found");
            send(client_socket, &h, sizeof(h), 0);
            send(client_socket, "File found", h.length, 0);
        }

        read(client_socket, buffer, 1);

        std::clock_t start;
        double dur;
        start = std::clock();
        size_t bytes_sent = 0;
        size_t bytes_read;
        while((bytes_read = fread(buffer, 1, BUFSIZE, fp)) > 0){
            bytes_sent += bytes_read;
            send(client_socket, buffer, bytes_read, 0);
        }

        dur = (std::clock() - start) / (double) CLOCKS_PER_SEC;
        printf("Sent %zu bytes in %.16f seconds\n", bytes_sent, dur);
        
        close(client_socket);

    }

    close(client_socket);
}

void listenForConnections(){
    int sock = slisten(port);
    struct sockaddr_in client_addr;
    int addr_size;
    check(sock, "error listening for connections");
    while(1){
        int client_sock = accept(sock, (sockaddr*)&client_addr, (socklen_t*)&addr_size);
        check(client_sock, "error accepting connection");
        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        std::thread t1(&handleConnection, client_sock);
        t1.detach();
    }
}

void check(int n, const char *msg){
	if(n<0){
		perror(msg);
        //unregister_peer();
		exit(1);
	}
}

void interruptHandler(int sig){
    printf("SIGNAL: %d Exiting...\n",sig);
    exit(0);
}

int main(int argc, char *argv[]){

    if(argc < 2){
        printf("Usage: <config file path>\nLoading default config file: %s\n", CONFIG_FILE_PATH);
        loadConfig(CONFIG_FILE_PATH);
    }else{
        loadConfig(argv[1]);
    }
    checkDownloadDirect();
    checkUploadDirect();
    
    atexit(unregister_peer);
    signal(SIGINT, interruptHandler);


    register_peer(port);
    std::thread t1(listenForConnections);
    t1.detach();

    std::string input;

    cout << "Enter 'exit' to exit" << endl;
    cout << "Enter a file name to find peers hosting it, and download it if there is a host" << endl;
    while(1){
        std::cin >> input;
        if(input != "exit"){
            vector <string> peers = findPeers((char*)input.c_str());
            if(peers.size() > 0){
                downloadFromPeer(peers, (char*)input.c_str());
                update_hosted_files(sconnect(server_ip, SERVER_PORT), 1, (char*)input.c_str());
            }
        }else if(input == "exit"){
            break;
        }
    }

    exit(0);
    return 0;
}