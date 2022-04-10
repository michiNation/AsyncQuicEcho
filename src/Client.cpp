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
    LOG_("readError" + error.message);
    quic::ApplicationErrorCode errorcode;
    sw->DisconnectedEvent("StreamReset: " + streamId);
    mQuicClient->resetStream(streamId, errorcode);
}

void MyClient::onNewBidirectionalStream(quic::StreamId id) noexcept
{
    LOG_("NewBidirectionalStream " + std::to_string(id));
    sw->ConnectedEvent("NewSteam: " + id);
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
    sw->DisconnectedEvent("Disconnected");
    mStartDone.post();
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
    //sendMessage(id, mPendingOutput[id]);
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
       // LOG(INFO) << "Sent " << string;
    }
}

void MyClient::sendMessage(quic::StreamId id, quic::BufQueue &data)
{
    auto message = data.move();
    auto res = mQuicClient->writeChain(id, message->clone(), false);
    //auto str = message->moveToFbString().toStdString();

    if (res.hasError())
    {
        LOG_("EchoClient writeChain error=" + uint32_t(res.error()));
    }
    else
    {
        //mPendingOutput[id].move();
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
    //LOG_("Readavailability");
    if(this->testType != TESTTYPE::STARTDOWNLOADCLOSE){
        auto readData = mQuicClient->read(streamId, 0);
        if (readData.hasError())
        {
            LOG_("EchoClient failed read from stream=" + std::to_string(streamId) + ", error=" + std::to_string((uint32_t)readData.error()));
        }
        auto copy = readData->first->clone();
        std::string msg = copy->moveToFbString().toStdString();
        //LOG_("Received: " + msg);
     }
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
                //LOG_("Read from Stream: " + std::to_string(streamId));
                std::lock_guard<std::mutex> guard(it->second.mutex);
                it->second.isReceived = true;
                //sw->CreateLogEntry("RTT-QUIC-0PL", ("StartFireClose-" + std::to_string(streamId)),it->second.start,sw->getCurrentTime());
                sw->CreateLogEntry("RTT-QUIC-5PL", ("StartFireClose-" + std::to_string(streamId)),it->second.start,sw->getCurrentTime());
            }
            break;
        }
        case TESTTYPE::STARTDOWNLOADCLOSE:
        {
            if(!startDownload){
                LOG_("Read 9 bytes");
                auto readData = mQuicClient->read(streamId, 9);
                auto copy = readData->first->clone();
                const uint8_t *buf = copy->data();
                std::string str(&buf[0],&buf[9]);
                std::cout << str << std::endl;
                fileSize = std::stoi(str);
                LOG_("Start Download - Filesize: " + std::to_string(fileSize) + " Received Bytes: " + std::to_string(copy->length()));
                downloadFile = std::make_unique<FileAbstraction>(false);
                downloadFile->LodeFile("../Files/Files_1/VideoFileDownload.MOV");
                //downloadFile->LodeFile("../Files/Files_1/BigFile1GBdw.zip");
                startDownload = true;
                sw->ReceivedEvent("StartDownload");
            }else{
                
                auto readData = mQuicClient->read(streamId, 0);
                auto copy = readData->first->clone();

                LOG_("Next Chunk of: " + std::to_string(copy->length()) + " Bytessum: " + std::to_string(bytesReceived));
                bytesReceived += copy->length();
                downloadFile->WriteBytes((const char*)copy->data(), copy->length());
                LOG_("Written to file");
            }

            if(bytesReceived >= fileSize){
                std::lock_guard<std::mutex> guard(mutex);
                finishedDownload = true;
                LOG_("Finished Download. Bytessum: " + std::to_string(bytesReceived));
                sw->Stop();
                sw->CreateLogEntry("RTT-QUIC", "Downloaded");
            }
            return;
        }
          case TESTTYPE::STARTFIREDOWNLOADCLOSE:
        {
            break;
            
        }
    }
        sw->ReceivedEvent(("Received" + std::to_string(streamId)));
}

void MyClient::start(std::string ip, uint16_t port, TESTTYPE testtype,uint16_t instances ,uint16_t loops)
{
    LOG_("Start: Port(" + std::to_string(port) +")");
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
      //settings.dataPathType = quic::DataPathType::ContinuousMemory;
      //settings.attemptEarlyData = true;
      mQuicClient->setTransportSettings(settings);
          
      mQuicClient->setTransportStatsCallback(
          std::make_shared<LogQuicStats>("client"));
    
    sw->Connect();
    mQuicClient->start(this, this); });
    mStartDone.wait();
    sw->ConnectedEvent("Connected");
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
            
            {
                std::lock_guard<std::mutex> guard(mapMutex);
                streamId = mQuicClient->createBidirectionalStream().value();     
                mQuicClient->setReadCallback(streamId, this);
                streamMutexMap[streamId]; //using default constructor of mutexBool struct
            }

            int x = 0;
            while(x < loops){
                auto& mb = streamMutexMap[streamId];
                sw->SendEvent(message + std::to_string(streamId));
                mb.start = sw->getCurrentTime();
                sendMessage2(streamId, message);
                waitFor([&](){
                std::lock_guard<std::mutex> guard(mb.mutex);
                return mb.isReceived;}, 1, 5000);
                x++;
                {
                    std::lock_guard<std::mutex> guard(mb.mutex);
                    mb.isReceived = false;
                }
            }
        };
        std::vector<std::thread> threads;
        for(int i = 0; i < instances; i++){
            threads.push_back(std::thread(func));
        }
        for (std::thread & th : threads)
        {
            // If thread Object is Joinable then Join that thread.
            if (th.joinable())
                th.join();
        }         
        mQuicClient->closeGracefully(); 
        sw->Disconnect();
        sw->CreateConnectionTrackingEntry("QUIC-StartStopConnection5%", "nothing");
        break;
    }
    case TESTTYPE::STARTDOWNLOADCLOSE:
    {
        sw->Start();
        sw->SendEvent("StartDownload");
        quic::StreamId streamId;          
        streamId = mQuicClient->createBidirectionalStream().value();  
        mQuicClient->setReadCallback(streamId, this);
        LOG_("Send start Download");
        sendMessage2(streamId, "Download");
        LOG_("Wait...");
        waitFor([=](){
            std::lock_guard<std::mutex> guard(mutex);
            return finishedDownload;}, 100, (1000 * 60 * 30)); //timeout 30 min

        mQuicClient->closeGracefully();
        downloadFile->CloseFile();
        LOG_("Done...");
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