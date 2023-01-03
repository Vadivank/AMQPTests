#include <iostream>
#include "AMQPHandler.h"

void receiveLogs() {
    AMQPHandler handler("localhost", 5672);
    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);

    auto receiveMessageCallback = [](const AMQP::Message& message,
                                     uint64_t deliveryTag, bool redelivered) {
        std::cout << " [x] " << message.body() << std::endl;
        std::cout << " [x] " << message.bodySize() << std::endl;
        std::cout << " [x] " << message.contentEncoding() << std::endl;
    };

    auto callback = [&](const std::string& name, int msgcount,
                        int consumercount) {
        channel.bindQueue("logs", name, "");
        channel.consume(name, AMQP::noack).onReceived(receiveMessageCallback);
    };

    channel.declareExchange("logs", AMQP::fanout).onSuccess([&]() {
        channel.declareQueue(AMQP::exclusive).onSuccess(callback);
    });

    std::cout << " [*] Waiting for messages. To exit press CTRL-Cn";
    handler.loop();
}

void emitLogs() {
    const std::string msg = "info: Hello World!";

    AMQPHandler handler("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    channel.declareExchange("logs", AMQP::fanout).onSuccess([&]() {
        channel.publish("logs", "", msg);
        std::cout << " [x] Sent " << msg << std::endl;
        handler.quit();
    });

    handler.loop();
}

int main() {
    emitLogs();
    // receiveLogs();


    return 0;
}