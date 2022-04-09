#pragma once
#include <quic/api/QuicSocket.h>
#include <quic/client/QuicClientTransport.h>
#include <quic/common/BufUtil.h>
#include <quic/common/test/TestClientUtils.h>
#include <quic/common/test/TestUtils.h>
#include <quic/fizz/client/handshake/FizzClientQuicHandshakeContext.h>
#include <quic/samples/echo/LogQuicStats.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include "TypesAndHelpers.h"
#include "StopWatch.h"
#include "FileAbstraction.h"

class MyClient : public quic::QuicSocket::ConnectionSetupCallback, 
                 public quic::QuicSocket::ConnectionCallback,
                 public quic::QuicSocket::ReadCallback,
                 public quic::QuicSocket::WriteCallback
{
public:
    MyClient();

    void readAvailable(quic::StreamId streamId) noexcept override;

    void readError(
        quic::StreamId streamId,
        quic::QuicError error) noexcept override;

    void onNewBidirectionalStream(quic::StreamId id) noexcept override;

    void onNewUnidirectionalStream(quic::StreamId id) noexcept override;

    void onStopSending(
        quic::StreamId id,
        quic::ApplicationErrorCode /*error*/) noexcept override;

    void onConnectionEnd() noexcept override;

    void onConnectionSetupError(quic::QuicError error) noexcept override;

    void onConnectionError(quic::QuicError error) noexcept override;

    void onTransportReady() noexcept override;

    void onStreamWriteReady(quic::StreamId id, uint64_t maxToSend) noexcept;

    void onStreamWriteError(
        quic::StreamId id,quic::QuicError error) noexcept override;

    void start(std::string ip, uint16_t port, TESTTYPE testtype,uint16_t instances = 1, uint16_t loops = 1);
    std::string getString();

    ~MyClient() override = default;

    void setUpConnection();

    void reConnect();

private:

    void sendBytes(quic::StreamId id, quic::BufQueue data, bool eof);
    void sendMessage(quic::StreamId id, quic::BufQueue &data);
    void sendMessage2(quic::StreamId id, std::string string);

    bool logging = true;
    void LOG_(std::string logstring){
        if(logging){
            std::cout << logstring << std::endl;
        }
    };

    std::string mHost;
    uint16_t mPort;
    std::shared_ptr<quic::QuicClientTransport> mQuicClient;
    //std::map<quic::StreamId, quic::BufQueue> mPendingOutput;
   // std::map<quic::StreamId, uint64_t> mRecvOffsets;
    folly::fibers::Baton mStartDone;
    std::map<std::string, quic::StreamId> appToStreamID;

    folly::ScopedEventBaseThread networkThread{"EchoClientThread"};

    std::mutex mutex;
    bool isconnected = false;
    std::mutex received;
    bool isreceived = false;
    quic::StreamId usecaseStream;
    std::unique_ptr<StopWatch> sw = std::make_unique<StopWatch>();
    TESTTYPE testType = TESTTYPE::KEYBOARD;
    struct mutexBool{
        std::mutex mutex;
        bool isReceived = false;
        Timepoint start = Timepoint();
    };
    std::map<quic::StreamId,mutexBool> streamMutexMap;

    std::mutex mapMutex;


    //download stuff
    uint32_t bytesReceived = 0;
    uint32_t fileSize = 0;
    bool finishedDownload = false;
    bool startDownload = false;
    std::unique_ptr<FileAbstraction> downloadFile;
};
