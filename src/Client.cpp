#include "Client.h"
#include "LogQuicStats.h"
MyClient::MyClient() {}



void MyClient::readError(
    quic::StreamId streamId,
    quic::QuicError error) noexcept
{

    // A read error only terminates the ingress portion of the stream state.
    // Your application should probably terminate the egress portion via
    // resetStream
    LOG_("readError");
    quic::ApplicationErrorCode errorcode;
    mQuicClient->resetStream(streamId, errorcode);
}

void MyClient::onNewBidirectionalStream(quic::StreamId id) noexcept
{
    LOG_("NewBidirectionalStream " + std::to_string(id));
    mQuicClient->setReadCallback(id, this);
}

void MyClient::onNewUnidirectionalStream(quic::StreamId id) noexcept
{
    mQuicClient->setReadCallback(id, this);
}

void MyClient::onStopSending(
    quic::StreamId id,
    quic::ApplicationErrorCode /*error*/) noexcept
{
    LOG_("Stop sending request " + std::to_string(id));
}

void MyClient::onConnectionEnd() noexcept
{
    LOG_("OnConnectionEnd");
}

void MyClient::onConnectionSetupError(quic::QuicError error) noexcept
{
    LOG_("onConnectionSetupError");
    onConnectionError(std::move(error));
}

void MyClient::reConnect()
{
    LOG_("reConnect");
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
    mQuicClient->start(this, this); });
}

void MyClient::onConnectionError(quic::QuicError error) noexcept
{
   LOG_("EchoClient error: " + error.message);

   mStartDone.post();
}

void MyClient::onTransportReady() noexcept
{
    LOG_("Transport is ready");
    mStartDone.post();
}

void MyClient::onStreamWriteReady(quic::StreamId id, uint64_t maxToSend) noexcept
{
    LOG_("EchoClient socket is write ready with maxToSend=" + maxToSend); 
    sendMessage(id, mPendingOutput[id]);
}

void MyClient::onStreamWriteError(
    quic::StreamId id, quic::QuicError error) noexcept
{

   LOG_("EchoClient write error with stream=" + std::to_string(id) + " error=" + toString(error));
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
      LOG_("EchoClient writeChain error=" + uint32_t(res.error()));
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

    if (res.hasError())
    {
        LOG_("EchoClient writeChain error=" + uint32_t(res.error()));
    }
    else
    {
        mPendingOutput[id].move();
    }
}

void MyClient::setUpConnection()
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
          evb, std::move(sock), std::move(fizzClientContext),12, 0UL, false);

      mQuicClient->setHostname(mHost.c_str());
      mQuicClient->addNewPeerAddress(addr);
      

      quic::TransportSettings settings;
      mQuicClient->setTransportSettings(settings);
          
      mQuicClient->setTransportStatsCallback(
          std::make_shared<LogQuicStats>("client"));

    LOG_("EchoClient connecting to " + addr.describe());
      mQuicClient->start(this, this); });
}

void MyClient::readAvailable(quic::StreamId streamId) noexcept
{
    auto readData = mQuicClient->read(streamId, 0);
    if (readData.hasError())
    {
        LOG_("EchoClient failed read from stream=" + std::to_string(streamId) + ", error=" + std::to_string((uint32_t)readData.error()));
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
    std::string msg = copy->moveToFbString().toStdString();
    //then check usecase 
    switch (this->testType)
    {
        case TESTTYPE::KEYBOARD:
        {
            break;
        }
        case TESTTYPE::STARTFIRECLOSE:
        case TESTTYPE::STARTLOOPCLOSE:
        {
            
            auto it = streamMutexMap.find(streamId);
            if(it != streamMutexMap.end()){
                LOG_("Read from Stream: " + std::to_string(streamId));
                std::lock_guard<std::mutex> guard(it->second.mutex);
                it->second.isReceived = true;
                //sw->Stop();
                sw->CreateLogEntry("RTT", ("StartFireClose" + std::to_string(streamId)),it->second.start,sw->getCurrentTime());
            }
            break;
        }
          case TESTTYPE::STARTDOWNLOADCLOSE:
        {
            break;
            
        }
          case TESTTYPE::STARTFIREDOWNLOADCLOSE:
        {
            break;
            
        }
    }
     sw->ReceivedEvent((msg + std::to_string(streamId)));
}

void MyClient::start(std::string ip, uint16_t port, TESTTYPE testtype,uint16_t instances ,uint16_t loops)
{
    LOG_("Start: Port(" + std::to_string(port));
    sw->CreateFile(("QUICTest_" + sw->GetUtcString()) ,getStringfromTesttype(static_cast<int>(testtype)),"QUIC");
    mHost = ip;
    mPort = port;
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
      
      mQuicClient->start(this, this); });

    mStartDone.wait();
    this->testType = testtype;
    switch (testtype)
    {
    case TESTTYPE::KEYBOARD:
    {

        std::string message = "Hello";

        // loop until Ctrl+D
        while (std::getline(std::cin, message))
        {
            if (message.empty())
            {
                // continue;
            }
            if (message.compare("stop") == 0)
            {
                mQuicClient->closeGracefully();
            }
            else if (message.compare("start") == 0)
            {
                reConnect();
            }
            else if (message.compare("exit") == 0)
            {
                mQuicClient->closeGracefully();
                break;
            }
            else
            {
                evb->runInEventBaseThreadAndWait([=]
                                                 {
                                             // create new stream for each message
                                             auto search = appToStreamID.find(message);
                                             quic::StreamId streamId;
                                             if (appToStreamID.end() != search)
                                             {
                                                 streamId = search->second;
                                             /*     LOG(INFO) << "Use old stream=" << streamId << " is bi :" << mQuicClient->isBidirectionalStream(streamId); */
                                             }
                                             else
                                             {
                                               /*   LOG(INFO) << "Use new stream "; */
                                                 streamId = mQuicClient->createBidirectionalStream().value();
                                                 appToStreamID[message] = streamId;
                                                 mQuicClient->setReadCallback(streamId, this);
                                             }

                                             sendMessage2(streamId, message); });
            }
        }//end while
        mQuicClient->closeGracefully();
        break;
    }
    case TESTTYPE::STARTFIRECLOSE:
    case TESTTYPE::STARTLOOPCLOSE:
    {

        //send "hello from client"
        //wait for the response
        //close the connection
        std::string message = "HelloFromClient";
        auto func = [&](){
            quic::StreamId streamId;          
            /* LOG(INFO) << "Use new stream "; */
            {
                std::lock_guard<std::mutex> guard(mapMutex);
                streamId = mQuicClient->createBidirectionalStream().value();  
                streamMutexMap[streamId]; //using default constructor of mutexBool struct
            }
            int x = 0;
            while(x < loops){
                auto& mb = streamMutexMap[streamId];
                mQuicClient->setReadCallback(streamId, this);
                sw->SendEvent(message + std::to_string(streamId));
                mb.start = sw->getCurrentTime();
                sendMessage2(streamId, message);
                waitFor([&](){
                std::lock_guard<std::mutex> guard(mb.mutex);
                return mb.isReceived;}, 5, 5000);
                x++;
                {
                    std::lock_guard<std::mutex> guard(mb.mutex);
                    mb.isReceived = false;
                }
            }
        };
        std::vector<std::thread> threads;
        for(int i = 0; i < instances; i++){
            LOG(INFO) << "new thread " << i;
            threads.push_back(std::thread(func));
        }
        for (std::thread & th : threads)
        {
            // If thread Object is Joinable then Join that thread.
            if (th.joinable())
                th.join();
        }                    
        mQuicClient->closeGracefully();
        break;
    }
    case TESTTYPE::STARTDOWNLOADCLOSE:
    {
        //send start download
        //possible to start several loops? 
        break;
    }
    case TESTTYPE::STARTFIREDOWNLOADCLOSE:
    {
        //we need to streams 
        //one for seinding data 
        //one for looping the data 
        break;
    }
    default:
        break;
    }
    sw->CloseFile();
  /*   LOG(INFO) << "EchoClient stopping client"; */
}