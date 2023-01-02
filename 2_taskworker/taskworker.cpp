#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <iterator>
#include <thread>

#include "AMQPHandler.h"

void task() {
    const std::string msg = "Hello World!";

    AMQPHandler handler("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);

    AMQP::QueueCallback callback = [&](const std::string& name, int msgcount,
                                       int consumercount) {
        AMQP::Envelope env(msg.c_str(), msg.size());
        env.setDeliveryMode(2);
        channel.publish("", "task_queue", env);
        std::cout << " [x] Sent " << msg << "\n";
        handler.quit();
    };

    channel.declareQueue("task_queue", AMQP::durable).onSuccess(callback);
    handler.loop();
}

void worker() {
    AMQPHandler handler("localhost", 5672);
    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);

    // количество нераспакованных сообщений, которые могут существовать в
    // клиентском приложении
    channel.setQos(1);

    channel.declareQueue("task_queue", AMQP::durable);
    channel.consume("task_queue")
        .onReceived([&channel](const AMQP::Message& message,
                               uint64_t deliveryTag, bool redelivered) {
            std::string body = message.body();
            std::cout << " [x] Received " << body << std::endl;

            size_t count = std::count(body.cbegin(), body.cend(), '.');
            std::this_thread::sleep_for(std::chrono::seconds(count));

            std::cout << " [x] Done" << std::endl;
            channel.ack(deliveryTag);  // подтверждение получения сообщения
        });

    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";
    handler.loop();
}

int main(void) {
    // task();
    worker();

    return 0;
}