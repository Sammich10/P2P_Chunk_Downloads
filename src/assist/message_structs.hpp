#ifndef MSG_STRUCTS_HPP
#define MSG_STRUCTS_HPP

#include <vector>
#include <string>

struct file_info{//struct to store file information
	std::string file_name;
	int file_size;
};

struct peer_info{//struct to store registered peer information
    std::string peer_id;
	char ip[15];
	uint16_t port;
	std::vector<file_info> files;
};

struct hosting_peer{
    std::string peer_id;
    char ip[15];
    uint16_t port;
};

struct super_peer_info{//struct to store registered super peer information
    std::string super_peer_id;
    char ip[15];
    uint16_t port;
};

struct MessageHeader {
    uint32_t length; // Length of message data in bytes
};

struct Message{
    uint32_t length;
    std::string content;
};

#endif