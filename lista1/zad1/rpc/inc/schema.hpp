#ifndef RPC_SCHEMA_HPP
#define RPC_SCHEMA_HPP

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace rpc {
namespace schema {
using File = uint32_t;
using off_t = std::int64_t;
using mode_t = std::uint32_t;

struct OpenRequest final {
  std::string pathname;
  mode_t mode;
};

struct ReadRequest final {
  File desc;
  std::uint64_t count;
};

struct WriteRequest final {
  File desc;
  std::uint64_t count;
  std::vector<std::uint8_t> bytes;
};

struct LSeekRequest final {
  File desc;
  off_t offset;
  std::uint32_t whence;
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

struct CloseRequest final {
  File desc;
};

using RequestBody =
    std::variant<OpenRequest, ReadRequest, WriteRequest, LSeekRequest,
                 ChmodRequest, UnlinkRequest, RenameRequest, CloseRequest>;

struct OpenResponse final {
  File file;
};

struct ReadResponse final {
  std::int64_t read;
  std::vector<std::uint8_t> bytes;
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

struct CloseResponse final {
  std::int64_t result;
};

using ResponseBody =
    std::variant<OpenResponse, ReadResponse, WriteResponse, LSeekResponse,
                 ChmodResponse, UnlinkResponse, RenameResponse, CloseResponse>;

enum class Code : uint8_t {
  OK,
  TIMEOUT,
  CONNECTION,
  INTERNAL,
  BAD_REQUEST,
  FORBIDDEN,
};

struct Header {
  uint64_t auth;
  uint64_t id;
};

struct Request final {
  Header header;
  RequestBody body;
};

struct Response final {
  uint64_t id;
  Code code;
  ResponseBody body;
};

} // namespace schema
} // namespace rpc

#endif // RPC_SCHEMA_HPP