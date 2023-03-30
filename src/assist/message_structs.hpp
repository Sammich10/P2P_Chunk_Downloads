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

struct super_peer_info{//struct to store registered super peer information
    std::string super_peer_id;
    char ip[15];
    uint16_t port;
};

struct MessageHeader {
    uint32_t length; // Length of message data in bytes
};

struct Message{
    std::string length;
    std::string content;
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

#endif