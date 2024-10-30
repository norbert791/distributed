#ifndef RPC_PROTOCOL_HPP
#define RPC_PROTOCOL_HPP

#include <cstdint>
#include <functional>
#include <vector>

namespace rpc {
namespace protocol {

class Client {
public:
  virtual std::vector<std::uint8_t>
  makeRequest(const std::vector<std::uint8_t>&) = 0;
};

using handler =
    std::function<std::vector<std::uint8_t>(std::vector<std::uint8_t>)>;

class Server {
public:
  virtual void setHandler(handler) = 0;
};

} // namespace protocol

} // namespace rpc

#endif // RPC_PROTOCOL_HPP