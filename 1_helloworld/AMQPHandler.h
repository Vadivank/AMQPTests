//
// Согласно документации на используемую библиотеку AMQP-CPP необходимо перед
// началом использования описать класс-обработчик протокола (реализацию),
// поскольку сама библиотека не может знать, какие именно действия необходимо
// выполнять при возникающих в очереди событиях. Поэтому в инструкции
// описывается класс с пустыми реализациями функций обратного вызова (callbacks)
//

#ifndef AMQPSENDER_AMQPHANDLER_H
#define AMQPSENDER_AMQPHANDLER_H

#include <amqpcpp.h>
#include <cassert>
#include <memory>

class AMQPHandlerImpl;
class AMQPHandler : public AMQP::ConnectionHandler {
public:
    static constexpr size_t BUFFER_SIZE = 8 * 1024 * 1024;   // 8Mb
    static constexpr size_t TEMP_BUFFER_SIZE = 1024 * 1024;  // 1Mb

    AMQPHandler(const std::string& host, uint16_t port);
    virtual ~AMQPHandler();

    void loop();  // цикл ожидания ответа от очереди
    void quit();  // soft disconnection
    bool isConnected() const;

private:
    AMQPHandler(const AMQPHandler&) = delete;             // deny copying
    AMQPHandler& operator=(const AMQPHandler&) = delete;  // deny pointers

    void close();
    // AMQP-CPP callbacks
    virtual void onData(AMQP::Connection* connection,
                        const char* data,
                        size_t size);
    [[maybe_unused]] virtual void onConnected(
        AMQP::Connection* connection);  // for TCP handler
    virtual void onError(AMQP::Connection* connection, const char* message);
    virtual void onClosed(AMQP::Connection* connection);
    // actual data sender
    void sendFromBufferToSocket();

private:
    // single data holder
    std::shared_ptr<AMQPHandlerImpl> m_impl;
};

#endif  // AMQPSENDER_AMQPHANDLER_H
