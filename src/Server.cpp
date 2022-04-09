#include "Server.h"
#include <glog/logging.h>

#include <quic/common/test/TestUtils.h>
#include <quic/samples/echo/EchoHandler.h>
#include <quic/samples/echo/LogQuicStats.h>
#include <quic/server/QuicServer.h>
#include <quic/server/QuicServerTransport.h>
#include <quic/server/QuicSharedUDPSocketFactory.h>



EchoServer::EchoServer(const std::string& host, uint16_t port)
      : host_(host), port_(port), server_(quic::QuicServer::createQuicServer()) {
    server_->setQuicServerTransportFactory(
        std::make_unique<EchoServerTransportFactory>());
    server_->setTransportStatsCallbackFactory(
        std::make_unique<LogQuicStatsFactory>());
    auto serverCtx = quic::test::createServerCtx();
    serverCtx->setClock(std::make_shared<fizz::SystemClock>());
    server_->setFizzContext(serverCtx);
  }

  void EchoServer::start() {

    // Create a SocketAddress and the default or passed in host.
    folly::SocketAddress addr1(host_.c_str(), port_);
    addr1.setFromHostPort(host_, port_);
    server_->start(addr1, 0);
    LOG(INFO) << "Echo server started at: " << addr1.describe();
    eventbase_.loopForever();
  }




