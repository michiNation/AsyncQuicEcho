#pragma once
#include <quic/api/QuicSocket.h>
#include <quic/client/QuicClientTransport.h>
#include <quic/common/BufUtil.h>
#include <quic/common/test/TestClientUtils.h>
#include <quic/common/test/TestUtils.h>
#include <quic/fizz/client/handshake/FizzClientQuicHandshakeContext.h>
#include <quic/samples/echo/LogQuicStats.h>
#include <folly/io/async/ScopedEventBaseThread.h>
class MyClient : public quic::QuicSocket::ConnectionCallback,
                 public quic::QuicSocket::ReadCallback,
                 public quic::QuicSocket::WriteCallback
{
public:
    MyClient(const std::string &host, uint16_t port);

    void readAvailable(quic::StreamId streamId) noexcept override;

    void readError(
        quic::StreamId streamId,
        std::pair<quic::QuicErrorCode, folly::Optional<folly::StringPiece>>
            error) noexcept override;

    void onNewBidirectionalStream(quic::StreamId id) noexcept override;

    void onNewUnidirectionalStream(quic::StreamId id) noexcept override;

    void onStopSending(
        quic::StreamId id,
        quic::ApplicationErrorCode /*error*/) noexcept override;

    void onConnectionEnd() noexcept override;

    void onConnectionSetupError(std::pair<quic::QuicErrorCode, std::string> error) noexcept override;

    void onConnectionError(std::pair<quic::QuicErrorCode, std::string> error) noexcept override;

    void onTransportReady() noexcept override;

    void onStreamWriteReady(quic::StreamId id, uint64_t maxToSend) noexcept;

    void onStreamWriteError(
        quic::StreamId id,
        std::pair<quic::QuicErrorCode, folly::Optional<folly::StringPiece>>
            error) noexcept override;

    void start();
    std::string getString();

    ~MyClient() override = default;

    void setUpConnection();

    void reConnect();

private:

    void sendBytes(quic::StreamId id, quic::BufQueue data, bool eof);
    void sendMessage(quic::StreamId id, quic::BufQueue &data);
    void sendMessage2(quic::StreamId id, std::string string);
    std::string mHost;
    uint16_t mPort;
    std::shared_ptr<quic::QuicClientTransport> mQuicClient;
    std::map<quic::StreamId, quic::BufQueue> mPendingOutput;
    std::map<quic::StreamId, uint64_t> mRecvOffsets;
    folly::fibers::Baton mStartDone;
    std::map<std::string, quic::StreamId> appToStreamID;

    folly::ScopedEventBaseThread networkThread{"EchoClientThread"};
};
