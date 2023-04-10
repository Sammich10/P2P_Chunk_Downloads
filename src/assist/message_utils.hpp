#ifndef MESSAGE_UTILS_HPP
#define MESSAGE_UTILS_HPP

#include "message_structs.hpp"
#include "message_utils.cpp"

void serialize_peer_info(peer_info& info, char* buffer);
void deserialize_peer_info(char* buffer, peer_info& info);
void deserialize_message(char* buffer, Message& msg);
void serialize_message(Message& msg, char* buffer);
std::string md5(const std::string& file_path);

#endif