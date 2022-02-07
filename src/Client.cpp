
#include "Client.h"
#include "LogQuicStats.h"
MyClient::MyClient(const std::string &host, uint16_t port)
    : mHost(host), mPort(port) {}

void MyClient::readAvailable(quic::StreamId streamId) noexcept
{
    auto readData = mQuicClient->read(streamId, 0);
    if (readData.hasError())
    {
        LOG(ERROR) << "EchoClient failed read from stream=" << streamId
                   << ", error=" << (uint32_t)readData.error();
    }
    auto copy = readData->first->clone();
    if (mRecvOffsets.find(streamId) == mRecvOffsets.end())
    {
        mRecvOffsets[streamId] = copy->length();
    }
    else
    {
        mRecvOffsets[streamId] += copy->length();
    }
    LOG(INFO) << "Client received data=" << copy->moveToFbString().toStdString()
              << " on stream=" << streamId;
}

void MyClient::readError(
    quic::StreamId streamId,
    std::pair<quic::QuicErrorCode, folly::Optional<folly::StringPiece>>
        error) noexcept
{

    LOG(ERROR) << "EchoClient failed read from stream=" << streamId
               << ", error=" << quic::toString(error);
    // A read error only terminates the ingress portion of the stream state.
    // Your application should probably terminate the egress portion via
    // resetStream
    quic::ApplicationErrorCode errorcode;
    mQuicClient->resetStream(streamId, errorcode);
    LOG(ERROR) << "Error after reset " << quic::toString(errorcode);
}

void MyClient::onNewBidirectionalStream(quic::StreamId id) noexcept
{
    LOG(INFO) << "EchoClient: new bidirectional stream=" << id;
    mQuicClient->setReadCallback(id, this);
}

void MyClient::onNewUnidirectionalStream(quic::StreamId id) noexcept
{
    LOG(INFO) << "EchoClient: new unidirectional stream=" << id;
    mQuicClient->setReadCallback(id, this);
}

void MyClient::onStopSending(
    quic::StreamId id,
    quic::ApplicationErrorCode /*error*/) noexcept
{
    VLOG(10) << "EchoClient got StopSending stream id=" << id;
}

void MyClient::onConnectionEnd() noexcept
{
    LOG(INFO) << "EchoClient connection end";
}

void MyClient::onConnectionSetupError(std::pair<quic::QuicErrorCode, std::string> error) noexcept
{
    onConnectionError(std::move(error));
}

void MyClient::reConnect(){
    folly::SocketAddress addr(mHost.c_str(), mPort);
    auto evb = networkThread.getEventBase();


    evb->runInEventBaseThreadAndWait([&]
                                     {
    auto sock = std::make_unique<folly::AsyncUDPSocket>(evb);
     auto fizzClientContext =
          quic::FizzClientQuicHandshakeContext::Builder()
              .setCertificateVerifier(quic::test::createTestCertificateVerifier())
              .build();

    auto newTransport = quic::QuicClientTransport::newClient(evb,std::move(sock),fizzClientContext);
    mQuicClient = newTransport;
    mQuicClient->setHostname(mHost.c_str());
    mQuicClient->addNewPeerAddress(addr);
      

    quic::TransportSettings settings;
    mQuicClient->setTransportSettings(settings);
          
    mQuicClient->setTransportStatsCallback(
    std::make_shared<LogQuicStats>("client"));
    mQuicClient->start(this);
                                     });
}

void MyClient::onConnectionError(std::pair<quic::QuicErrorCode, std::string> error) noexcept
{
    LOG(ERROR) << "EchoClient error: " << toString(error.first)
               << "; errStr=" << error.second;


    mStartDone.post();
}

void MyClient::onTransportReady() noexcept
{
    LOG(INFO) << "Transport is ready";

    mStartDone.post();
}

void MyClient::onStreamWriteReady(quic::StreamId id, uint64_t maxToSend) noexcept
{
    LOG(INFO) << "EchoClient socket is write ready with maxToSend="
              << maxToSend;
    sendMessage(id, mPendingOutput[id]);
}

void MyClient::onStreamWriteError(
    quic::StreamId id,
    std::pair<quic::QuicErrorCode, folly::Optional<folly::StringPiece>>
        error) noexcept
{

    LOG(ERROR) << "EchoClient write error with stream=" << id
               << " error=" << toString(error);
}



void MyClient::start()
{
   // folly::ScopedEventBaseThread networkThread("EchoClientThread");
    auto evb = networkThread.getEventBase();
    folly::SocketAddress addr(mHost.c_str(), mPort);

    evb->runInEventBaseThreadAndWait([&]
                                     {
      auto sock = std::make_unique<folly::AsyncUDPSocket>(evb);
      auto fizzClientContext =
          quic::FizzClientQuicHandshakeContext::Builder()
              .setCertificateVerifier(quic::test::createTestCertificateVerifier())
              .build();
              
      mQuicClient = std::make_shared<quic::QuicClientTransport>(
          evb, std::move(sock), std::move(fizzClientContext));
      mQuicClient->setHostname(mHost.c_str());
      mQuicClient->addNewPeerAddress(addr);
      

      quic::TransportSettings settings;
      mQuicClient->setTransportSettings(settings);
          
      mQuicClient->setTransportStatsCallback(
          std::make_shared<LogQuicStats>("client"));

      LOG(INFO) << "EchoClient connecting to " << addr.describe();
      
      mQuicClient->start(this); });
      
    mStartDone.wait();

    std::string message = "Hello";    

    // loop until Ctrl+D
    //while (std::getline(std::cin, message))
    //{
        if (message.empty())
        {
            //continue;
        }
        if(message.compare("stop") == 0){
            mQuicClient->closeGracefully();
        }
        else if(message.compare("start") == 0){
            reConnect();
        }
        else if(message.compare("switch") == 0){
            


        }
        else if(message.compare("exit") == 0){
            mQuicClient->closeGracefully();
            //break;
        }
        else{
                    evb->runInEventBaseThreadAndWait([=]
                                         {
                                             // create new stream for each message
                                             auto search = appToStreamID.find(message);
                                             quic::StreamId streamId;
                                             if (appToStreamID.end() != search)
                                             {
                                                 streamId = search->second;
                                                 LOG(INFO) << "Use old stream=" << streamId << " is bi :" << mQuicClient->isBidirectionalStream(streamId);
                                             }
                                             else
                                             {
                                                 LOG(INFO) << "Use new stream ";
                                                 streamId = mQuicClient->createBidirectionalStream().value();
                                                 appToStreamID[message] = streamId;
                                                 mQuicClient->setReadCallback(streamId, this);
                                             }

                                             sendMessage2(streamId, message);
                                         });

        }

    mQuicClient->closeGracefully();
    //}
    LOG(INFO) << "EchoClient stopping client";
}

std::string MyClient::getString()
{
    return mHost;
}

void MyClient::sendMessage2(quic::StreamId id, std::string string)
{
    auto res = mQuicClient->writeChain(id, folly::IOBuf::copyBuffer(string), false);

    if (res.hasError())
    {
        LOG(ERROR) << "EchoClient writeChain error=" << uint32_t(res.error());
    }
    else
    {
        LOG(INFO) << "Sent " << string;
    }
}

void MyClient::sendMessage(quic::StreamId id, quic::BufQueue &data)
{
    auto message = data.move();
    auto res = mQuicClient->writeChain(id, message->clone(), true);
    auto str = message->moveToFbString().toStdString();

    LOG(INFO) << "Receided " << str;
    if (res.hasError())
    {
        LOG(ERROR) << "EchoClient writeChain error=" << uint32_t(res.error());
    }
    else
    {
        // auto str = message->moveToFbString().toStdString();
        LOG(INFO) << "EchoClient wrote \"" << str << "\""
                  << ", len=" << str.size() << " on stream=" << id;
        // sent whole message
        mPendingOutput[id].move();
    }
}

void MyClient::setUpConnection(){
       // folly::ScopedEventBaseThread networkThread("EchoClientThread");
    auto evb = networkThread.getEventBase();
    folly::SocketAddress addr(mHost.c_str(), mPort);

    evb->runInEventBaseThreadAndWait([&]
                                     {
      auto sock = std::make_unique<folly::AsyncUDPSocket>(evb);
      auto fizzClientContext =
          quic::FizzClientQuicHandshakeContext::Builder()
              .setCertificateVerifier(quic::test::createTestCertificateVerifier())
              .build();
              
      mQuicClient = std::make_shared<quic::QuicClientTransport>(
          evb, std::move(sock), std::move(fizzClientContext),12, 0UL, false);

      mQuicClient->setHostname(mHost.c_str());
      mQuicClient->addNewPeerAddress(addr);
      

      quic::TransportSettings settings;
      mQuicClient->setTransportSettings(settings);
          
      mQuicClient->setTransportStatsCallback(
          std::make_shared<LogQuicStats>("client"));

      LOG(INFO) << "EchoClient connecting to " << addr.describe();
      mQuicClient->start(this); });
}