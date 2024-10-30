#ifndef RPC_SERVER_HPP
#define RPC_SERVER_HPP

#include <functional>

#include "schema.hpp"

namespace rpc {
namespace server {

using File = schema::FILE;
using off_t = schema::off_t;
using mode_t = schema::mode_t;

using openHandler =
    std::function<std::uint32_t(std::string path, std::string mode)>;

// TODO: Change type to server config

struct ServerConfig {
  std::function<std::uint32_t(std::string path, std::string mode)> OpenHandler;
  std::function<std::int64_t(File desc, std::uint64_t)> ReadHandler;
  std::function<std::int64_t(File desc, std::uint64_t count)> WriteHandler;
  std::function<off_t(File desc, off_t offset)> LSeekHandler;
  std::function<std::int64_t(std::string pathname, std::uint32_t mode)>
      ChmodHandler;
  std::function<std::int64_t(std::string pathname)> UnlinkHandler;
  std::function<std::int64_t(std::string oldpath, std::string newpath)>
      RenameHandler;
};

} // namespace server
} // namespace rpc

#endif // RPC_SERVER_HPP