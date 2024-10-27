#ifndef RPC_SCHEMA_HPP
#define RPC_SCHEMA_HPP

#include <cstdint>
#include <string>
#include <variant>

namespace rpc {
namespace schema {
using FILE = uint32_t;
using off_t = std::int64_t;
using mode_t = std::uint32_t;

struct OpenRequest final {
  std::string pathname;
  std::string mode;
};

struct ReadRequest final {
  FILE desc;
  std::uint64_t count;
};

struct WriteRequest final {
  FILE desc;
  std::uint64_t count;
};

struct LSeekRequest final {
  FILE desc;
  off_t offset;
};

struct ChmodRequest final {
  std::string pathname;
  std::uint32_t mode;
};

struct UnlinkRequest final {
  std::string pathname;
};

struct RenameRequest final {
  std::string oldpath;
  std::string newpath;
};

using RequestBody =
    std::variant<OpenRequest, ReadRequest, WriteRequest, LSeekRequest,
                 ChmodRequest, UnlinkRequest, RenameRequest>;

struct OpenResponse final {
  FILE file;
};

struct ReadResponse final {
  std::int64_t read;
};

struct WriteResponse final {
  std::int64_t written;
};

struct LSeekResponse final {
  off_t offset;
};

struct ChmodResponse final {
  std::int64_t result;
};

struct UnlinkResponse final {
  std::int64_t result;
};

struct RenameResponse final {
  std::int64_t result;
};

using ResponseBody =
    std::variant<OpenResponse, ReadResponse, WriteResponse, LSeekResponse,
                 ChmodResponse, UnlinkResponse, RenameResponse>;

enum class Code { OK, TIMEOUT, CONNECTION, INTERNAL, BAD_REQUEST };

struct Header {
  uint64_t id;
  uint64_t auth;
};

struct Request final {
  Header header;
  RequestBody body;
};

struct Response final {
  Header header;
  ResponseBody body;
};

} // namespace schema
} // namespace rpc

#endif // RPC_SCHEMA_HPP