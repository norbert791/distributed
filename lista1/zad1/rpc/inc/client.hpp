#ifndef RPC_CLIENT_HPP
#define RPC_CLIENT_HPP

#include <cstdint>
#include <limits>
#include <memory>
#include <random>

#include <protocol.hpp>
#include <schema.hpp>

namespace rpc {
namespace client {
class Client {
public:
  Client(std::uint64_t auth, std::shared_ptr<protocol::Client> client)
      : auth{auth}, client{client} {}

  schema::File open(std::string pathname, schema::mode_t mode);
  std::int64_t read(schema::File desc, std::uint64_t count,
                    std::vector<std::uint8_t>& v);
  std::int64_t write(schema::File desc, std::uint64_t count,
                     std::vector<std::uint8_t>& v);
  schema::off_t lseek(schema::File desc, schema::off_t offset,
                      std::uint32_t whence);
  std::int64_t chmod(std::string pathname, std::uint32_t mode);
  std::int64_t unlink(std::string pathname);
  std::int64_t rename(std::string oldpath, std::string newpath);
  std::int64_t close(schema::File desc);

private:
  const std::uint64_t auth{0};
  std::uniform_int_distribution<std::uint64_t> uniform{
      1, std::numeric_limits<uint64_t>::max()};
  std::default_random_engine rng{std::random_device{}()};
  const std::shared_ptr<protocol::Client> client;
  schema::ResponseBody sendBody(schema::RequestBody);
};

} // namespace client
} // namespace rpc

#endif // RPC_CLIENT_HPP