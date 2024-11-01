#ifndef RPC_SERVER_HPP
#define RPC_SERVER_HPP

#include <functional>
#include <memory>
#include <unordered_map>

#include "asio/io_context.hpp"
#include "protocol.hpp"
#include "schema.hpp"

namespace rpc {
namespace server {

using openHandler =
    std::function<std::uint32_t(std::string path, std::string mode)>;

// TODO: Change type to server config

struct Handlers {
  std::function<schema::File(std::string path, mode_t mode)> OpenHandler;
  std::function<std::int64_t(schema::File desc, std::uint64_t,
                             std::vector<uint8_t>&)>
      ReadHandler;
  std::function<std::int64_t(schema::File desc, std::uint64_t count,
                             std::vector<std::uint8_t>&)>
      WriteHandler;
  std::function<off_t(schema::File desc, off_t offset, std::uint32_t whence)>
      LSeekHandler;
  std::function<std::int64_t(std::string pathname, std::uint32_t mode)>
      ChmodHandler;
  std::function<std::int64_t(std::string pathname)> UnlinkHandler;
  std::function<std::int64_t(std::string oldpath, std::string newpath)>
      RenameHandler;
};

using Permissions = std::array<bool, 7>;

class Server : public Handlers {
public:
  Server(Handlers handlers, std::shared_ptr<rpc::protocol::Server> server,
         std::unordered_map<std::uint64_t, Permissions> users)
      : Handlers(handlers), server{server}, perms{users} {
    this->server->setHandler(this->makeHandler());
  }

private:
  std::shared_ptr<rpc::protocol::Server> server;
  std::unordered_map<std::uint64_t, Permissions> perms;
  rpc::protocol::handler makeHandler();
  asio::io_context ctx;
};

} // namespace server
} // namespace rpc

#endif // RPC_SERVER_HPP