#include "dependencies.hpp"
#include "assist/message_utils.hpp"
#include "assist/message_structs.hpp"

Message msg;

int main(){
    msg.content = "Hello World";
    msg.length = msg.content.size();

    char buffer[1024] = {0};
    serialize_message(msg, buffer);

    Message msg2;

    deserialize_message(buffer, msg2);

    return 0;
}