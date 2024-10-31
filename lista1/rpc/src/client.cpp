#include "marshalling.hpp"
#include "schema.hpp"
#include <client.hpp>
#include <stdexcept>

namespace rpc {
namespace client {
schema::ResponseBody Client::sendBody(schema::RequestBody body) {
  using namespace schema;
  Request req{.header =
                  Header{.auth = this->auth, .id = this->uniform(this->rng)},
              .body = body};
  auto bytes = this->client->makeRequest(marshalling::marshalRequest(req));
  auto resp = marshalling::unmarshalResponse(bytes);
  if (resp.id != req.header.id) {
    // TODO: improve handling
    throw std::invalid_argument("bad return code");
  }

  return resp.body;
}

schema::File Client::open(std::string pathname, schema::mode_t mode) {
  schema::OpenRequest req{.pathname = pathname, .mode = mode};
  schema::ResponseBody resp = this->sendBody(req);
  auto result = std::get<schema::OpenResponse>(resp);
  return result.file;
}

std::int64_t Client::read(schema::File desc, std::uint64_t count,
                          std::vector<std::uint8_t>& v) {
  schema::ReadRequest req{.desc = desc, .count = count};
  schema::ResponseBody resp = this->sendBody(req);
  auto result = std::get<schema::ReadResponse>(resp);
  v = result.bytes;
  return result.read;
}

std::int64_t Client::write(schema::File desc, std::uint64_t count,
                           std::vector<std::uint8_t>& v) {
  schema::WriteRequest req{.desc = desc, .count = count, .bytes = v};
  schema::ResponseBody resp = this->sendBody(req);
  auto result = std::get<schema::WriteResponse>(resp);
  return result.written;
}

schema::off_t Client::lseek(schema::File desc, schema::off_t offset,
                            std::uint32_t whence) {
  schema::LSeekRequest req{.desc = desc, .offset = offset, .whence = whence};
  schema::ResponseBody resp = this->sendBody(req);
  auto result = std::get<schema::LSeekResponse>(resp);
  return result.offset;
}

schema::off_t Client::chmod(std::string pathname, std::uint32_t mode) {
  schema::ChmodRequest req{.pathname = pathname, .mode = mode};
  schema::ResponseBody resp = this->sendBody(req);
  auto result = std::get<schema::ChmodResponse>(resp);
  return result.result;
}

std::int64_t Client::unlink(std::string pathname) {
  schema::UnlinkRequest req{.pathname = pathname};
  schema::ResponseBody resp = this->sendBody(req);
  auto result = std::get<schema::UnlinkResponse>(resp);
  return result.result;
}

std::int64_t Client::rename(std::string oldpath, std::string newpath) {
  schema::RenameRequest req{.oldpath = oldpath, .newpath = newpath};
  schema::ResponseBody resp = this->sendBody(req);
  auto result = std::get<schema::RenameResponse>(resp);
  return result.result;
}

} // namespace client
} // namespace rpc