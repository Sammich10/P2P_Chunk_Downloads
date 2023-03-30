#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "message_structs.hpp"

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
    offset+= origin_ip_address_length;
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
