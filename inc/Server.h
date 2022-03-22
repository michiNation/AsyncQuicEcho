#pragma once

#include "EchoHandler.h"
#include "LogQuicStats.h"

#include <glog/logging.h>
#include <quic/common/test/TestUtils.h>
#include <quic/server/QuicServer.h>
#include <quic/server/QuicServerTransport.h>
#include <quic/server/QuicSharedUDPSocketFactory.h>


class EchoServerTransportFactory : public quic::QuicServerTransportFactory {
 public:
  ~EchoServerTransportFactory() override {
    while (!echoHandlers_.empty()) {
      auto& handler = echoHandlers_.back();
      handler->getEventBase()->runImmediatelyOrRunInEventBaseThreadAndWait(
          [this] {
            // The evb should be performing a sequential consistency atomic
            // operation already, so we can bank on that to make sure the writes
            // propagate to all threads.
            echoHandlers_.pop_back();
          });
    }
  }

  EchoServerTransportFactory() = default;

  quic::QuicServerTransport::Ptr make(
      folly::EventBase* evb,
      std::unique_ptr<folly::AsyncUDPSocket> sock,
      const folly::SocketAddress&,
      quic::QuicVersion,
      std::shared_ptr<const fizz::server::FizzServerContext> ctx) noexcept
      override {
    CHECK_EQ(evb, sock->getEventBase());
    auto echoHandler = std::make_unique<EchoHandler>(evb);
    auto transport = quic::QuicServerTransport::make(
        evb, std::move(sock), echoHandler.get(), echoHandler.get(), ctx);
    echoHandler->setQuicSocket(transport);
    echoHandlers_.push_back(std::move(echoHandler));
    return transport;
  }

  std::vector<std::unique_ptr<EchoHandler>> echoHandlers_;

 private:
};

class EchoServer {
 public:
  explicit EchoServer(const std::string& host = "127.0.0.1", uint16_t port = 6666);
  void start();

 private:
  std::string host_;
  uint16_t port_;
  folly::EventBase eventbase_;
  std::shared_ptr<quic::QuicServer> server_;
};

class Clock {
  public:
  virtual ~Clock() = default;
  std::chrono::system_clock::time_point getCurrentTime() const {
      return std::chrono::system_clock::time_point();
  }
};