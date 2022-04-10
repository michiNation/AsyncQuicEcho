#pragma once
#include <quic/api/QuicSocket.h>
#include <folly/io/async/EventBase.h>
#include <quic/common/BufUtil.h>
#include "FileAbstraction.h"
#include <iostream>
class EchoHandler : public quic::QuicSocket::ConnectionSetupCallback,
                    public quic::QuicSocket::ConnectionCallback,
                    public quic::QuicSocket::ReadCallback,
                    public quic::QuicSocket::WriteCallback {
 public: 
  using StreamData = std::pair<quic::BufQueue, bool>;



  explicit EchoHandler(folly::EventBase* evbIn) : evb(evbIn) {
    std::cout << fa->GetFileSize() << std::endl; 
  }

  void setQuicSocket(std::shared_ptr<quic::QuicSocket> socket) {
    sock = socket;
  }

  void onNewBidirectionalStream(quic::StreamId id) noexcept override {
/*     LOG(INFO) << "Got bidirectional stream id=" << id; */
    sock->setReadCallback(id, this);
  }

  void onNewUnidirectionalStream(quic::StreamId id) noexcept override {
/*     LOG(INFO) << "Got unidirectional stream id=" << id; */
    sock->setReadCallback(id, this);
  }

  void onStopSending(
      quic::StreamId id,
      quic::ApplicationErrorCode error) noexcept override {
 /*    LOG(INFO) << "Got StopSending stream id=" << id << " error=" << error; */
  }

  void onConnectionEnd() noexcept override {
    LOG(INFO) << "Socket closed";
  }

  
  void onConnectionSetupError(quic::QuicError error) noexcept override {
    onConnectionError(std::move(error));
  }

 void onConnectionError(quic::QuicError error) noexcept override {
    LOG(ERROR) << "Socket error=" << toString(error.code) << " "
               << error.message;
  }

  void onTransportReady() noexcept override {
    LOG(INFO) << "onTransportReady" << sock->getPeerAddress().describe();
  }

  void readAvailable(quic::StreamId id) noexcept override {
   /*  LOG(INFO) << "read available for stream id=" << id; */

    auto res = sock->read(id, 0);
    if (res.hasError()) {
      LOG(ERROR) << "Got error=" << toString(res.error());
      return;
    }
    if (input_.find(id) == input_.end()) {
      input_.emplace(id, std::make_pair(quic::BufQueue(), false));
    }
    quic::Buf data = std::move(res.value().first);
    bool eof = res.value().second;
    auto dataLen = (data ? data->computeChainDataLength() : 0);

    std::string str = "";
    if(dataLen != 0){
      auto s = data->cloneAsValue();
      str = s.moveToFbString().toStdString();
    }

    if(str.compare("Download") == 0){
      std::cout << "Start Download Requested" << std::endl;
/*       auto f = [&,id](){this->Download(id);};
      std::thread a{f};
      a.detach(); */
      input_[id].first.append(std::move(data));
      input_[id].second = eof;
      Download(id);
    } 
    else{
    /*  LOG(INFO) << "Got len=" << dataLen << " eof=" << uint32_t(eof)
                << " total=" << input_[id].first.chainLength() + dataLen
                << " data="
                << ((data) ? data->clone()->moveToFbString().toStdString()
                          : std::string()); */
      input_[id].first.append(std::move(data));
      input_[id].second = eof;
      //if (eof) {
        echo(id, input_[id]);
      //}
    }
}

  void Download(quic::StreamId id){

    auto size = fa->GetFileSize();
    std::string si = std::to_string(size);
    std::cout << si << std::endl;
    auto echoedData = folly::IOBuf::copyBuffer(si);
    auto res = sock->writeChain(id, std::move(echoedData), false, nullptr); // no EOF because we need stream 17.03.22
    if (res.hasError()) {
      LOG(ERROR) << "write error=" << quic::toString(res.error());
    } else {
      // echo is done, clear EOF
      
    }
    //prep stuff
    std::cout << "Start Download" << std::endl;
    uint16_t CHUNCKSIZE = 1200;
    std::vector<uint8_t> v(CHUNCKSIZE);

   
        //loop and read CHUNCKSIZE bytes from the file and send them to the client
        while(size > 0){
            //todo check why download is not working after a few packages
            if(size >= CHUNCKSIZE){
                size = size - CHUNCKSIZE;
            }
            else{
                CHUNCKSIZE = size;
                size = 0;
            }
            v = fa->ReadBytes(CHUNCKSIZE);
            auto res = sock->writeChain(id, folly::IOBuf::copyBuffer(v), false, nullptr); // no EOF because we need stream 17.03.22
            //sock->
            if (res.hasError()) {
              std::cout << "write error=" << quic::toString(res.error());
            }else{
              std::cout << "Size: " + std::to_string(size) + " sent: " + std::to_string(v.size()) << std::endl;
            }
        }
        std::cout << "Download finished " << std::endl;
  }

    void echo(quic::StreamId id, StreamData& data) {
/*     if (!data.second) {
      // only echo when eof is present
      return;
    } */
    auto echoedData = folly::IOBuf::copyBuffer("");
    echoedData->prependChain(data.first.move());
    auto res = sock->writeChain(id, std::move(echoedData), false, nullptr); // no EOF because we need stream 17.03.22
    if (res.hasError()) {
      LOG(ERROR) << "write error=" << quic::toString(res.error());
    } else {
      // echo is done, clear EOF
      data.second = false;
    }
  }

   void readError(quic::StreamId id, quic::QuicError error) noexcept override {
    std::cout << "ReadError: " << error.message() << std::endl;
  }



  void onStreamWriteReady(quic::StreamId id, uint64_t maxToSend) noexcept
      override {
/*     LOG(INFO) << "socket is write ready with maxToSend=" << maxToSend; */
   // echo(id, input_[id]);
  }

  void onStreamWriteError(quic::StreamId id, quic::QuicError error) noexcept
      override {
   
    //LOG(ERROR) << "write error with stream=" << id
    //           << " error=" << toString(error);
  }

  folly::EventBase* getEventBase() {
    return evb;
  }

  folly::EventBase* evb;
  std::shared_ptr<quic::QuicSocket> sock;

 private:
  std::map<quic::StreamId, StreamData> input_;
  std::unique_ptr<FileAbstraction> fa = std::make_unique<FileAbstraction>(true);
};
