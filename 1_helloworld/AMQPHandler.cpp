#include "AMQPHandler.h"
#include <Poco/Net/StreamSocket.h>
#include <thread>

namespace {
    class Buffer {
    public:
        explicit Buffer(size_t size)
            :  // only explicit constructor call is allowed
              charsToWrite(size, 0),
              usedBytes(0) {}

        size_t write(const char* data, size_t msgLength) {
            if (usedBytes == charsToWrite.size()) return 0;

            const size_t packageLength = (msgLength + usedBytes);
            size_t bytesToWrite = packageLength < charsToWrite.size()
                                      ? msgLength
                                      : charsToWrite.size() - usedBytes;
            memcpy(charsToWrite.data() + usedBytes, data, bytesToWrite);
            usedBytes += bytesToWrite;
            return bytesToWrite;
        }

        void drain() { usedBytes = 0; }

        [[nodiscard]] size_t available() const { return usedBytes; }

        [[nodiscard]] const char* data() const { return charsToWrite.data(); }

        void shl(size_t count) {
            assert(count < usedBytes);

            const size_t diff = usedBytes - count;
            std::memmove(charsToWrite.data(), charsToWrite.data() + count,
                         diff);
            usedBytes = usedBytes - count;
        }

    private:
        std::vector<char> charsToWrite;
        size_t usedBytes;
    };
}  // namespace

// handler data holder (socket, connection, flags)
struct AMQPHandlerImpl {
    AMQPHandlerImpl()
        : connected(false),
          connection(nullptr),
          quit(false),
          inputBuffer(AMQPHandler::BUFFER_SIZE),
          outBuffer(AMQPHandler::BUFFER_SIZE),
          tmpBuff(AMQPHandler::TEMP_BUFFER_SIZE) {}

    bool connected;
    AMQP::Connection* connection;
    bool quit;
    Buffer inputBuffer;
    Buffer outBuffer;
    std::vector<char> tmpBuff;

    Poco::Net::StreamSocket socket;
};

// handler constructor, host and port args, init list creates holder structure
// as an object is created - create a socket, connect, keep alive
AMQPHandler::AMQPHandler(const std::string& host, uint16_t port)
    : m_impl(new AMQPHandlerImpl) {
    const Poco::Net::SocketAddress address(host, port);
    m_impl->socket.connect(address);
    m_impl->socket.setKeepAlive(true);
}

// destructor closes handler, which closes socket
AMQPHandler::~AMQPHandler() {
    close();
}

void AMQPHandler::loop() {
    try {
        while (!m_impl->quit) {
            if (m_impl->socket.available() >
                0) {  // if we have something to read without being blocked
                size_t avail = m_impl->socket.available();
                if (m_impl->tmpBuff.size() < avail) {
                    m_impl->tmpBuff.resize(avail, 0);
                }

                m_impl->socket.receiveBytes(&m_impl->tmpBuff[0], avail);
                m_impl->inputBuffer.write(m_impl->tmpBuff.data(), avail);
            }
            if (m_impl->socket.available() < 0) {
                std::cerr << "SOME socket error!!!" << std::endl;
            }

            if (m_impl->connection && m_impl->inputBuffer.available()) {
                size_t count =
                    m_impl->connection->parse(m_impl->inputBuffer.data(),
                                              m_impl->inputBuffer.available());
                if (count == m_impl->inputBuffer.available()) {
                    m_impl->inputBuffer.drain();
                } else if (count > 0) {
                    m_impl->inputBuffer.shl(count);
                }
            }
            sendFromBufferToSocket();

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (m_impl->quit && m_impl->outBuffer.available()) {
            sendFromBufferToSocket();
        }
    } catch (const Poco::Exception& exc) {
        std::cerr << "Poco exception " << exc.displayText();
    }
}

// raise quit flag (soft closing)
void AMQPHandler::quit() {
    m_impl->quit = true;
}

void AMQPHandler::AMQPHandler::close() {
    m_impl->socket.close();
}

/**
 *  Method that is called by AMQP-CPP when data has to be sent over the
 *  network. You must implement this method and send the data over a
 *  socket that is connected with RabbitMQ.
 *
 *  Note that the AMQP library does no buffering by itself. This means
 *  that this method should always send out all data or do the buffering
 *  itself.
 */
void AMQPHandler::onData(AMQP::Connection* connection,
                         const char* data,
                         size_t size) {
    m_impl->connection = connection;  // knowing the connection
    const size_t written =
        m_impl->outBuffer.write(data, size);  // bufferize what we have
    if (written != size) {
        sendFromBufferToSocket();  // write directly to socket
        m_impl->outBuffer.write(data + written,
                                size - written);  // bufferize the remainder
    }
}

[[maybe_unused]] void AMQPHandler::onConnected(AMQP::Connection* connection) {
    m_impl->connected = true;
}

void AMQPHandler::onError(AMQP::Connection* connection, const char* message) {
    std::cerr << "AMQP error " << message << std::endl;
}

void AMQPHandler::onClosed(AMQP::Connection* connection) {
    std::cout << "AMQP closed connection" << std::endl;
    m_impl->quit = true;
}

bool AMQPHandler::isConnected() const {
    return m_impl->connected;
}

void AMQPHandler::sendFromBufferToSocket() {
    if (m_impl->outBuffer.available()) {
        m_impl->socket.sendBytes(m_impl->outBuffer.data(),
                                 m_impl->outBuffer.available());
        m_impl->outBuffer.drain();
    }
}