#ifndef RPC_CLIENT_HPP
#define RPC_CLIENT_HPP

#include <cstdint>
#include <functional>
#include <variant>

namespace rpc {

enum class Code { OK, TIMEOUT, CONNECTION, INTERNAL, BAD_REQUEST };

struct Request final {
  Code code;
  uint64_t id;
  uint64_t auth;
};

struct Read {};

struct Body {};

struct Response final {};

class Client {
  virtual Response MakeRequest(Request) = 0;
};

class Server {
  virtual void HandleRequest(std::function<Response(Request)>) = 0;
};

} // namespace rpc

#endif // RPC_CLIENT_HPP