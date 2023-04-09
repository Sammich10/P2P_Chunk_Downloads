#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "message_structs.hpp"

void serialize_peer_info(peer_info& info, char* buffer) {
    int offset = 0;

    memcpy(buffer + offset, info.peer_id.c_str(), info.peer_id.size() + 1);
    offset += info.peer_id.size() + 1;
    memcpy(buffer + offset, info.ip, 15);
    offset += 15;
    uint16_t port_n = htons(info.port);
    memcpy(buffer + offset, &port_n, sizeof(port_n));
    offset += sizeof(port_n);
    uint32_t num_files_n = htonl(info.files.size());
    memcpy(buffer + offset, &num_files_n, sizeof(num_files_n));
    offset += sizeof(num_files_n);
    for (const auto& file : info.files) {
        memcpy(buffer + offset, file.file_name.c_str(), file.file_name.size() + 1);
        offset += file.file_name.size() + 1;

        int file_size_n = htonl(file.file_size);
        memcpy(buffer + offset, &file_size_n, sizeof(file_size_n));
        offset += sizeof(file_size_n);
    }
}

void deserialize_peer_info(char* buffer, peer_info& info) {
    int offset = 0;

    info.peer_id = buffer + offset;
    offset += info.peer_id.size() + 1;
    memcpy(info.ip, buffer + offset, 15);
    offset += 15;
    uint16_t port_n;
    memcpy(&port_n, buffer + offset, sizeof(port_n));
    info.port = ntohs(port_n);
    offset += sizeof(port_n);
    uint32_t num_files_n;
    memcpy(&num_files_n, buffer + offset, sizeof(num_files_n));
    uint32_t num_files = ntohl(num_files_n);
    offset += sizeof(num_files_n);
    for (uint32_t i = 0; i < num_files; i++) {
        file_info file;

        file.file_name = buffer + offset;
        offset += file.file_name.size() + 1;

        int file_size_n;
        memcpy(&file_size_n, buffer + offset, sizeof(file_size_n));
        file.file_size = ntohl(file_size_n);
        offset += sizeof(file_size_n);

        info.files.push_back(file);
    }
}

void serialize_hosting_peers(std::vector<hosting_peer> peers, char* buffer){
    int offset = 0;
    uint32_t num_peers_n = htonl(peers.size());
    memcpy(buffer+offset, &num_peers_n, sizeof(num_peers_n));
    offset += sizeof(num_peers_n);
    for(const auto& peer : peers){
        memcpy(buffer+offset, peer.peer_id.c_str(), peer.peer_id.size()+1);
        offset += peer.peer_id.size()+1;
        memcpy(buffer+offset, peer.ip, 15);
        offset += 15;
        uint16_t port_n = htons(peer.port);
        memcpy(buffer+offset, &port_n, sizeof(port_n));
        offset += sizeof(port_n);
    }
}

void deserialize_hosting_peers(char* buffer, std::vector<hosting_peer> peers){
    int offset = 0;
    uint32_t num_peers_n = ntohl(*reinterpret_cast<const uint32_t*>(&buffer[offset]));
    offset += sizeof(num_peers_n);
    for(uint32_t i = 0; i < num_peers_n; i++){
        hosting_peer peer;
        peer.peer_id = buffer+offset;
        offset += peer.peer_id.size()+1;
        memcpy(peer.ip, buffer+offset, 15);
        offset += 15;
        uint16_t port_n = ntohs(*reinterpret_cast<const uint16_t*>(&buffer[offset]));
        peer.port = port_n;
        offset += sizeof(port_n);
        peers.push_back(peer);
    }
}

void serialize_message(Message& msg, char* buffer){
    int offset = 0;
    uint32_t message_len = htonl(msg.length);
    memcpy(buffer+offset, &message_len, sizeof(message_len));
    offset += sizeof(msg.length);
    memcpy(buffer+offset, msg.content.c_str(), msg.content.size()+1);
    return;
}

void deserialize_message(char* buffer, Message& msg) {
    int offset = 0;
    msg.length = ntohl(*reinterpret_cast<const uint32_t*>(&buffer[offset]));
    if(msg.length > sizeof(buffer)-sizeof(uint32_t)){
        perror("Message length longer than buffer");
        return;
    }
    offset += sizeof(msg.length);
    msg.content.assign(reinterpret_cast<const char*>(&buffer[offset]), msg.length);
    offset += msg.length;
}

