#ifndef MESSAGE_UTILS_HPP
#define MESSAGE_UTILS_HPP

#include "message_structs.hpp"
#include "message_utils.cpp"

std::vector<uint8_t> serializeQueryMessage(QueryMessage& message);
QueryMessage deserializeQueryMessage(std::vector<uint8_t>& buffer);
std::vector<uint8_t> serializeQueryHitMessage(QueryHitMessage& message);
QueryHitMessage deserializeQueryHitMessage(const std::vector<uint8_t>& buffer);
void serialize_peer_info(peer_info& info, char* buffer);
void deserialize_peer_info(char* buffer, peer_info& info);



#endif