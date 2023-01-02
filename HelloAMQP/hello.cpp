#include <fstream>
#include <iostream>
#include <string>

#include "AMQPHandler.h"

#define FILE_BUFFER_SIZE 32768

void send(const char* _nameQueue, const char* _message) {
    AMQPHandler handler("localhost", 5672);
    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);

    const char* nameQueue{_nameQueue};
    const char* message{_message};

    channel.onError([&](const char*) { std::cout << "Error" << std::endl; });

    channel.onReady([&]() {
        // channel.declareQueue("imgpng");
        // char buffer[FILE_BUFFER_SIZE];
        // std::ifstream fin("img.png", std::ios::in | std::ios::binary);
        // int size;
        // int sent = 0;
        // do {
        //     fin.read(buffer, FILE_BUFFER_SIZE);
        //     size = fin.gcount();
        //     char wr[size];
        //     std::copy(buffer, buffer + size, wr);
        //     if (size == 0) break;
        //     std::cout << size << std::endl;
        //     channel.publish("", "imgpng", wr);
        //     sent++;
        // } while (size > 0);
        // fin.close();
        // std::cout << sent << std::endl;

        channel.declareQueue(_nameQueue);
        channel.publish("", _nameQueue, _message);

        handler.quit();
    });
    handler.loop();
}

void receive(const char* _nameQueue) {
    AMQPHandler handler("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    channel.declareQueue(_nameQueue);
    channel.consume(_nameQueue, AMQP::noack)
        .onReceived([](const AMQP::Message& message, uint64_t deliveryTag,
                       bool redelivered) {
            if (message.bodySize())
                std::cout << " [x] Received: " << message.body() << std::endl;
            // else handler.quit();
        });

    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";

    handler.loop();
}

int main() {
    printf("Start programm...\n");

    for (int i = 0; i < 5; ++i) {
        send("vadim", std::to_string(i).c_str());
    }

    receive("vadim");

    printf("Close programm...\n");
    return 0;
}
