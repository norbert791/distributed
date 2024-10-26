#ifndef RPC_UDP_HPP
#define RPC_UDP_HPP

#include "schema.hpp"

namespace rpc {
namespace udp {

using File = schema::FILE;

class Client {
public:
  Client(std::string url);

  File open(std::string filepath, std::string mode);

private:
  const std::string url;
};
} // namespace udp
} // namespace rpc
#endif // RPC_UDP_HPP