#ifndef RPC_UDP_HPP
#define RPC_UDP_HPP

#include <vector>

#include "protocol.hpp"
#include "schema.hpp"
#include <asio.hpp>

namespace rpc {
namespace udp {

using File = schema::File;

class Client : public protocol::Client {
public:
  Client(std::string url) : url{url} {};

  virtual std::vector<std::uint8_t>
  makeRequest(const std::vector<std::uint8_t>& data) override;

private:
  const std::string url;
};

class Server : public protocol::Server {
public:
  void run();
  void stop();

  virtual void setHandler(
      std::function<std::vector<std::uint8_t>(std::vector<std::uint8_t>)>)
      override;

private:
  protocol::handler handler;
  bool running{};
  asio::io_context ctx;
};

} // namespace udp
} // namespace rpc
#endif // RPC_UDP_HPP