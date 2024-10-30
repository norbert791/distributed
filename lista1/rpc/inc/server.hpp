#ifndef RPC_SERVER_HPP
#define RPC_SERVER_HPP

#include <functional>
#include <memory>

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
  std::function<off_t(schema::File desc, off_t offset)> LSeekHandler;
  std::function<std::int64_t(std::string pathname, std::uint32_t mode)>
      ChmodHandler;
  std::function<std::int64_t(std::string pathname)> UnlinkHandler;
  std::function<std::int64_t(std::string oldpath, std::string newpath)>
      RenameHandler;
};

class Server : public Handlers {
public:
  Server(Handlers config, std::shared_ptr<rpc::protocol::Server> server)
      : Handlers(config), server{server} {
    this->server->setHandler(this->makeHandler());
  }

private:
  std::shared_ptr<rpc::protocol::Server> server;
  rpc::protocol::handler makeHandler();
};

} // namespace server
} // namespace rpc

#endif // RPC_SERVER_HPP