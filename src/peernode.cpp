#include "dependencies.h"

#define SERVER_PORT 8060
#define CONFIG_FILE_PATH "./test/peerconfigtest.cfg"
#define DEFAULT_IP "127.0.0.1";
#define SERVER_BACKLOG 200
#define BUFSIZE 1024

using namespace std;

//peer node global variables
string peer_id = "peer0";
string ip = "127.0.0.1";
int port = 8081;
string host_folder = "test/peer_files";
char super_peer_ip[15] = "127.0.0.1";
int super_peer_port = 8060;
int dl_attempts = 3;

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

struct Message{
    std::string length;
    std::string content;
};

struct peer_info{//struct to store registered peer information
	char ip[15];
	uint16_t port;
	std::vector<file_info> files;
};

struct QueryMessage{
    std::string message_id;
    std::string file_name;
    int ttl;
    std::string origin_peer_id;
    std::string origin_ip_address;
    int origin_port;
};

struct QueryHitMessage{
    std::string message_id;
    std::string file_name;
    std::string peer_id;
    std::string ip_address;
    int port;
};

typedef struct sockaddr SA;

std::shared_mutex directoryMutex;
std::mutex mtx;

void check(int n, const char *msg);
int sconnect(char IP[], int port);

/*** Load the config parameters from the config file ***/

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

        if(param == "PEER_ID"){
            peer_id = value;
        }
        else if(param == "IP"){
            ip = value;
        }
		else if(param == "PORT"){
			port = stoi(value);
		}
        else if(param == "HOST_FOLDER"){
            host_folder = value;
        }
        else if(param == "SUPER_PEER_ADDR"){
            strcpy(super_peer_ip,value.c_str());
        }
        else if(param == "SUPER_PEER_PORT"){
            super_peer_port = stoi(value);
        }
        else if(param == "DOWNLOAD_ATTEMPTS"){
            dl_attempts = stoi(value);
        }
		
	}
	in.close();
	return true;
}

/*** Helper functions for peer ***/

void check(int n, const char *msg, bool fatal){//function to check for errors, print error message and exit if fatal
	if(n<0){
		perror(msg);
	}
    if(n < 0 && fatal){
        perror(msg);
        exit(1);
    }
}

void checkUploadDirect(){//check if the upload directory exists, if not, create it
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

void checkDownloadDirect(){//check if the download directory exists, if not, create it
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

void updateFileList(){//update the list of files in the upload directory
    std::string path = get_current_dir_name();
    path += "/" + host_folder;
    fileList.clear();

    std::shared_lock<std::shared_mutex> lock(directoryMutex);

    for(const auto & entry : fs::directory_iterator(path)){
        //std::cout << entry.path() << std::endl;
        file_info f;
        f.file_name = entry.path().filename();
        f.file_size = fs::file_size(entry.path());
        fileList.push_back(f);
    }
}

std::vector<file_info> getFileList(){//return the list of files in the upload directory
    updateFileList();
    return fileList;
}

std::string generateQueryName() {//generate a unique query name for each query
    std::uniform_int_distribution<> dis(0, 15);
    std::stringstream ss;
    std::random_device rd;
    std::mt19937 gen(rd());
    ss << std::hex;
    for (int i = 0; i < 5; ++i) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 5; ++i) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 5; ++i) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis(gen) % 4 + 8;
    for (int i = 0; i < 5; ++i) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 12; ++i) {
        ss << dis(gen);
    }
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    std::stringstream name_ss;
    name_ss << ss.str() << "_" << std::setfill('0') << std::setw(20) << time;

    return name_ss.str();
}

std::vector<uint8_t> serializeQueryMessage(QueryMessage& message) {
    std::vector<uint8_t> buffer;
    uint32_t message_id_length = message.message_id.size();
    buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&message_id_length), reinterpret_cast<uint8_t*>(&message_id_length) + sizeof(message_id_length));
    buffer.insert(buffer.end(), message.message_id.begin(), message.message_id.end());
    uint32_t file_name_length = message.file_name.size();
    buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&file_name_length), reinterpret_cast<uint8_t*>(&file_name_length) + sizeof(file_name_length));
    buffer.insert(buffer.end(), message.file_name.begin(), message.file_name.end());
    buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&message.ttl), reinterpret_cast<uint8_t*>(&message.ttl) + sizeof(message.ttl));
    uint32_t origin_peer_id_length = message.origin_peer_id.size();
    buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&origin_peer_id_length), reinterpret_cast<uint8_t*>(&origin_peer_id_length) + sizeof(origin_peer_id_length));
    buffer.insert(buffer.end(), message.origin_peer_id.begin(), message.origin_peer_id.end());
    uint32_t origin_ip_address_length = message.origin_ip_address.size();
    buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&origin_ip_address_length), reinterpret_cast<uint8_t*>(&origin_ip_address_length) + sizeof(origin_ip_address_length));
    buffer.insert(buffer.end(), message.origin_ip_address.begin(), message.origin_ip_address.end());
    buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&message.origin_port), reinterpret_cast<uint8_t*>(&message.origin_port) + sizeof(message.origin_port));
    
    return buffer;
}

QueryMessage deserializeQueryMessage(std::vector<uint8_t>& buffer) {
    QueryMessage message;
    
    uint32_t offset = 0;
    
    uint32_t message_id_length = *reinterpret_cast<const uint32_t*>(&buffer[offset]);
    offset += sizeof(message_id_length);
    message.message_id.assign(reinterpret_cast<const char*>(&buffer[offset]), message_id_length);
    offset += message_id_length;
    uint32_t file_name_length = *reinterpret_cast<const uint32_t*>(&buffer[offset]);
    offset += sizeof(file_name_length);
    message.file_name.assign(reinterpret_cast<const char*>(&buffer[offset]), file_name_length);
    offset += file_name_length;
    message.ttl = *reinterpret_cast<const int*>(&buffer[offset]);
    offset += sizeof(message.ttl);
    uint32_t origin_peer_id_length = *reinterpret_cast<const uint32_t*>(&buffer[offset]);
    offset += sizeof(origin_peer_id_length);
    message.origin_peer_id.assign(reinterpret_cast<const char*>(&buffer[offset]), origin_peer_id_length);
    offset += origin_peer_id_length;
    uint32_t origin_ip_address_length = *reinterpret_cast<const uint32_t*>(&buffer[offset]);
    offset += sizeof(origin_ip_address_length);
    message.origin_ip_address.assign(reinterpret_cast<const char*>(&buffer[offset]), origin_ip_address_length);
    message.origin_port = *reinterpret_cast<const int*>(&buffer[offset]);
    offset += sizeof(message.origin_port);
    
    return message;
}

std::vector<uint8_t> serializeQueryHitMessage(QueryHitMessage& message) {
    std::vector<uint8_t> buffer;

    uint32_t message_id_length = message.message_id.size();
    buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&message_id_length), reinterpret_cast<uint8_t*>(&message_id_length) + sizeof(message_id_length));
    buffer.insert(buffer.end(), message.message_id.begin(), message.message_id.end());
    uint32_t file_name_length = message.file_name.size();
    buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&file_name_length), reinterpret_cast<uint8_t*>(&file_name_length) + sizeof(file_name_length));
    buffer.insert(buffer.end(), message.file_name.begin(), message.file_name.end());
    uint32_t peer_id_length = message.peer_id.size();
    buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&peer_id_length), reinterpret_cast<uint8_t*>(&peer_id_length) + sizeof(peer_id_length));
    buffer.insert(buffer.end(), message.peer_id.begin(), message.peer_id.end());
    uint32_t ip_address_length = message.ip_address.size();
    buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&ip_address_length), reinterpret_cast<uint8_t*>(&ip_address_length) + sizeof(ip_address_length));
    buffer.insert(buffer.end(), message.ip_address.begin(), message.ip_address.end());
    buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&message.port), reinterpret_cast<uint8_t*>(&message.port) + sizeof(message.port));

    return buffer;
}

QueryHitMessage deserializeQueryHitMessage(const std::vector<uint8_t>& buffer) {
    QueryHitMessage message;
    
    uint32_t offset = 0;
    uint32_t message_id_length = *reinterpret_cast<const uint32_t*>(&buffer[offset]);
    offset += sizeof(message_id_length);
    message.message_id.assign(reinterpret_cast<const char*>(&buffer[offset]), message_id_length);
    offset += message_id_length;
    uint32_t file_name_length = *reinterpret_cast<const uint32_t*>(&buffer[offset]);
    offset += sizeof(file_name_length);
    message.file_name.assign(reinterpret_cast<const char*>(&buffer[offset]), file_name_length);
    offset += file_name_length;
    uint32_t peer_id_length = *reinterpret_cast<const uint32_t*>(&buffer[offset]);
    offset += sizeof(peer_id_length);
    message.peer_id.assign(reinterpret_cast<const char*>(&buffer[offset]), peer_id_length);
    offset += peer_id_length;
    uint32_t ip_address_length = *reinterpret_cast<const uint32_t*>(&buffer[offset]);
    offset += sizeof(ip_address_length);
    message.ip_address.assign(reinterpret_cast<const char*>(&buffer[offset]), ip_address_length);
    offset += ip_address_length;
    message.port = *reinterpret_cast<const int*>(&buffer[offset]);
    offset += sizeof(message.port);
    
    return message;
}

/*** Socket functions for network communications ***/

int slisten(int port){//function to create a socket and listen for incoming connections on the specified port
    int opt = 1;

	int server_socket;
	sockaddr_in server_addr;

	check((server_socket = socket(AF_INET, SOCK_STREAM, 0)),"failed to create socket for listening",true);//create socket

	check(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)),"failed to set socket options",true);

	server_addr.sin_family = AF_INET;//configure server
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	check(bind(server_socket,(sockaddr*)&server_addr,sizeof(server_addr)),"failed to bind peer socket",true);//bind socket
	check(listen(server_socket, SERVER_BACKLOG),"peer failed to listen to connections",true);//listen for connections

	//printf("Waiting for connections on port %d...\n",port);
    return server_socket;
}

int sconnect(char IP[], int port){//connect to super-peer or another peer node and return the socket
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

/*** Core functions for the peer's operation ***/

int downloadFile(int socket, char filename[]){//download a file specified by it's name from a peer using the specified socket
    
    int valread;
    char read_buffer[BUFSIZE] ={0};

    int sock = socket;
    
    MessageHeader h;
    h.length = strlen("download");
    send(sock, &h, sizeof(h), 0); //send message header
    if((send(sock, "download", strlen("download"), 0))<0){ //send download command to server
        printf("error sending download command to server\n");
        close(sock);
        return -1;
    }

    h.length = strlen(filename);
    send(sock, &h, sizeof(h), 0); //send message header
    if((send(sock, filename, strlen(filename), 0))<0){ //send filename to server
        printf("error sending file name to server\n");
        close(sock);
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
    //lock directory

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

void downloadFromPeer(std::vector<std::string> peers, char* file_name){//download a file from a peer given a list of peers that have the file, and the file name
    //choose a random peer to download from
    int random_peer;
    if(peers.size() > 1){
        random_peer = (rand() % (peers.size()-1));
    }else{
        random_peer = 0;
    }
    //get the ip and port of the peer
    std::string peer_ip = peers[random_peer].substr(0, peers[random_peer].find(":"));
    int peer_port = std::stoi(peers[random_peer].substr(peers[random_peer].find(":")+1, peers[random_peer].length()));

    printf("Connecting to peer %s:%d\n", peer_ip.c_str(), peer_port);
    //connect to the peer
    int peer_sock = sconnect((char*)peer_ip.c_str(), peer_port);
    //download the file
    int dls = downloadFile(peer_sock, file_name);
    
    //if the download failed, remove the peer from the list and try again with a different peer
    if(dls == -1){
        printf("Error downloading file from peer %s:%d\n", peer_ip.c_str(), peer_port);
        peers.erase(peers.begin() + random_peer);
        if(peers.size() == 0){
            //if there are no more peers to try, exit
            printf("No peers available to download file from\n");
            return;
        }
        downloadFromPeer(peers, file_name);
    }
    

}

void findPeers(char* file_name){//query the super peer that acts as an indexing server for the specified file name
    int sock = sconnect(super_peer_ip, SERVER_PORT);

    std::vector <std::string> valid_peers;
    
    MessageHeader h;
    h.length = strlen("find");
    check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
    check(send(sock, "find", strlen("find"), 0), "error sending find request to super peer",false);

    QueryMessage qm;
    qm.message_id = generateQueryName();
    qm.ttl = 5;
    qm.file_name = file_name;
    qm.origin_peer_id = peer_id;
    qm.origin_ip_address = ip;
    qm.origin_port = port;

    std::vector<uint8_t> msg = serializeQueryMessage(qm);
    //h.length = msg.size();
    //check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
    check(send(sock, msg.data(), msg.size(), 0), "error sending query message to super peer",false);

    //clock_t start = clock(); //start timer
    /*
    
    std::string qid = generateQueryName();
    h.length = qid.length();
    check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
    check(send(sock, qid.c_str(), qid.length(), 0), "error sending query id to super peer",false);
    h.length = strlen(file_name);
    check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
    check(send(sock, file_name, strlen(file_name), 0), "error sending file name to server",false);
    int ttl = 5;
    h.length = sizeof(ttl);
    check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
    check(send(sock, &ttl, sizeof(ttl), 0), "error sending ttl to server",false);
    std::string ipp = ip + ":" + std::to_string(port);
    h.length = ipp.length();
    check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
    check(send(sock, ipp.c_str(), ipp.length(), 0), "error sending ip:port to server",false);
    */

   

    /*
    int num_peers;
    check(recv(sock, &num_peers, sizeof(num_peers), 0), "error receiving number of peers from server",false);

    if(num_peers == 0){
        printf("No peers found for file %s\n", file_name);
    }else{
        printf("Found %d peer(s) for file %s\n", num_peers, file_name);
        
        for(int i = 0; i < num_peers; i++){
            std::string peer_ip_port;
            char buffer[BUFSIZE] = {0};

            check(recv(sock, &h, sizeof(h), 0), "error receiving message header",false); //receive message header
            check(recv(sock, &buffer, h.length, 0), "error receiving peer port from server",false);
            peer_ip_port = buffer;
            //printf("Peer %s\n",peer_ip_port.c_str());  
            valid_peers.push_back(peer_ip_port);
        }
    }

    clock_t end = clock(); //end timer
    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);

    printf("Time taken to find peers: %f seconds\n",time_taken);

    return valid_peers;
    */
    close(sock);
}


int update_hosted_files_all(int sock){
    std::vector<file_info> hostedFiles = getFileList();
    MessageHeader h;
    h.length = strlen("update");
    check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
    check(send(sock, "update", strlen("update"), 0), "error sending update command to server",false); //send update command to server

    h.length = sizeof(int);
    int size = hostedFiles.size();
    check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
    check(send(sock, &port, sizeof(port), 0), "error sending port to server",false);
    check(send(sock, &size, sizeof(size), 0), "error sending number of files to server",false);

    for(int i = 0;(long unsigned int)i < hostedFiles.size(); i++){
        h.length = hostedFiles[i].file_name.length();
        check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
        check(send(sock, hostedFiles[i].file_name.c_str(), hostedFiles[i].file_name.length(), 0), "error sending file name to server",false);
        check(send(sock, &hostedFiles[i].file_size, sizeof(hostedFiles[i].file_size), 0), "error sending file size to server",false);
    }
    return 0;
}

//update the server with the files that are currently hosted, either delete or add
int update_hosted_files(int sock, int operation, char filename[]){

    std::vector<file_info> hostedFiles = getFileList();
    MessageHeader h;
    h.length = strlen("single_update");
    check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
    check(send(sock, "single_update", strlen("single_update"), 0), "error sending update command to server",false); //send update command to server

    h.length = sizeof(int);
    check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
    check(send(sock, &port, sizeof(port), 0), "error sending port to server",false);
    check(send(sock, &operation, sizeof(operation), 0), "error sending operation to server",false);
    
    if(operation == 0){//delete file
        h.length = strlen(filename);
        check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
        check(send(sock, filename, strlen(filename), 0), "error sending filename to server",false);
    }
    else if(operation == 1){//add file
        h.length = strlen(filename);
        std::string path = get_current_dir_name();
        path += "/" + host_folder + "/" + filename;
        std::shared_lock<std::shared_mutex> lock(directoryMutex);
        int filesize = fs::file_size(path);
        check(send(sock, &h, sizeof(h), 0), "error sending message header",false); //send message header
        check(send(sock, filename, strlen(filename), 0), "error sending filename to server",false);
        check(send(sock, &filesize, sizeof(filesize), 0), "error sending filesize to server",false);
    }
    
    return 0;
}

//register the peer with the server
int register_peer(int port){

    std::vector<file_info> hostedFiles = getFileList();

    int sock = sconnect(super_peer_ip, SERVER_PORT);

    check(sock, "error connecting to super peer for registration",true);
    
    MessageHeader h;
    h.length = strlen("register");

    check(send(sock, &h, sizeof(h), 0), "error sending message header",true); //send message header
    check(send(sock, "register", strlen("register"), 0), "error sending message body to server",true);
    
    h.length = sizeof(int);
    int num_hosted_files = hostedFiles.size();
    check(send(sock, &h, sizeof(h), 0), "error sending message header",true); //send message header
    check(send(sock, peer_id.c_str(), peer_id.length(), 0), "error sending peer id to server",true);
    check(send(sock, &h, sizeof(h), 0), "error sending message header",true); //send message header
    check(send(sock, &port, sizeof(port), 0), "error sending port to server",true);
    check(send(sock, &num_hosted_files, sizeof(num_hosted_files), 0), "error sending number of files to server",true);

    for(int i = 0; (unsigned long int)i < hostedFiles.size(); i++){
        h.length = hostedFiles[i].file_name.length();
        check(send(sock, &h, sizeof(h), 0), "error sending message header",true); //send message header
        check(send(sock, hostedFiles[i].file_name.c_str(), h.length, 0), "error sending file name to server",true);//send file name to server
        int file_size = hostedFiles[i].file_size;
        check(send(sock, &file_size, sizeof(file_size), 0), "error sending file size to server",true);

    }

    close(sock);
    return 0;
}

void unregister_peer(){
    int sock = sconnect(super_peer_ip, SERVER_PORT);

    check(sock, "error connecting to super peer for unregistration",true);
    
    MessageHeader h;
    h.length = strlen("unregister");

    check(send(sock, &h, sizeof(h), 0), "error sending message header",true); //send message header
    check(send(sock, "unregister", strlen("unregister"),0), "error sending file name to server",true);
    h.length = sizeof(int);
    check(send(sock, &h, sizeof(h), 0), "error sending message header",true); //send message header
    check(send(sock, &port, sizeof(port), 0), "error sending port to server",true);

    close(sock);
    return;
}

void handleConnection(int client_socket){
    char buffer[BUFSIZE] = {0};
    MessageHeader h;
    recv(client_socket, &h, sizeof(h), 0);
    recv(client_socket, buffer, h.length, 0);
    //printf("Received message: %s\n", buffer);

    if(strcmp(buffer, "QueryHit")==0){
        printf("Received QueryHit\n");
        std::vector<uint8_t> serialized_data;
        QueryHitMessage qhm;
        char buffer[BUFSIZE];
        int bytes_read;
        while((bytes_read = recv(client_socket, buffer, BUFSIZE, 0)) > 0){
            serialized_data.insert(serialized_data.end(), buffer, buffer + bytes_read);
        }

        std::vector<uint8_t> serialized_vector(serialized_data.begin(),serialized_data.end());
        qhm = deserializeQueryHitMessage(serialized_vector);
        printf("Received QueryHit from %s\n", qhm.peer_id.c_str());
    }

    if(strcmp(buffer, "download") == 0){
        recv(client_socket, &h, sizeof(h), 0);
        recv(client_socket, buffer, h.length, 0);
        printf("Received download request for file name: %s\n", buffer);
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
            update_hosted_files(sconnect(super_peer_ip, SERVER_PORT), 1, (char*)file_name.c_str());
            return;
        }else{
            //printf("File found\n");
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
    check(sock, "error listening for connections",true);
    while(1){
        int client_sock = accept(sock, (sockaddr*)&client_addr, (socklen_t*)&addr_size);
        check(client_sock, "error accepting connection",true);
        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        std::thread t1(&handleConnection, client_sock);
        t1.detach();
    }
}

void interruptHandler(int sig){
    printf("SIGNAL: %d Exiting...\n",sig);
    exit(0);
}

int main(int argc, char *argv[]){

    if(argc < 2){
        printf("Usage: <config file path>\nLoading default config parameters: %s\n", CONFIG_FILE_PATH);
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
        if(input == "wait"){
            std::cin >> input;
            std::this_thread::sleep_for(std::chrono::seconds(atoi(input.c_str())));
            cout<<"Waited for "<<input<<" seconds"<<endl;
            continue;
        }
        else if(input == "update"){
            update_hosted_files_all(sconnect(super_peer_ip, SERVER_PORT));
        }
        else if(input != "exit"){
            findPeers((char*)input.c_str());
            
        }else if(input == "exit"){
            break;
        }
    }

    exit(0);
    return 0;
}