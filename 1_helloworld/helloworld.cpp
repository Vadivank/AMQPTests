#include <iostream>

#include "AMQPHandler.h"

void send(const char* nameQueue, const char* message) {
    AMQPHandler handler("localhost", 5672);
    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);

    channel.onReady([&]() {
        channel.declareQueue(nameQueue);
        channel.publish("", nameQueue, message);
        std::cout << " [x] Sent 'Hello World!'" << std::endl;
        handler.quit();
    });

    handler.loop();
}

void receive(const char* nameQueue) {
    AMQPHandler handler("localhost", 5672);
    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);

    channel.declareQueue(nameQueue);

    channel.consume(nameQueue, AMQP::noack)
        .onReceived([](const AMQP::Message& message, uint64_t deliveryTag,
                       bool redelivered) {
            std::cout << " [" << deliveryTag << "] Received: " << message.body()
                      << std::endl;
        });
    

    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";


    handler.loop();
}

int main(void) {
    std::cout << "\nSending...\n";
    send("test1", "This is AMQP message, mazafaka!");
    
    std::cout << "\nReceiving...\n";
    receive("test1");

    return 0;
}