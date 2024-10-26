#ifndef RPC_API_HPP
#define RPC_API_HPP

#include <functional>
#include <memory>

#include "schema.hpp"

namespace rpc {
namespace api {

class Client {
public:
  virtual schema::Response makeRequest(schema::Request) = 0;
};

class Server {
public:
  virtual void
      handleRequest(std::function<schema::Response(schema::Request)>) = 0;
};

using File = schema::FILE;

class Filesystem {
public:
  Filesystem(std::unique_ptr<Client> client);

  File open(std::string filename, std::string mode);
};

} // namespace api

} // namespace rpc

#endif // RPC_API_HPP