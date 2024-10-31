#include "schema.hpp"
#include <marshalling.hpp>
#include <server.hpp>

#include <iostream>

namespace rpc {
namespace server {
rpc::protocol::handler Server::makeHandler() {
  return [this](std::vector<std::uint8_t> bytes) -> std::vector<std::uint8_t> {
    using namespace rpc::schema;
    auto request = rpc::marshalling::unmarshalRequest(bytes);
    Response response;
    response.id = request.header.id;
    response.code = schema::Code::OK;
    if (this->perms.contains(request.header.auth)) {
      auto perms = this->perms.at(request.header.auth);
      auto index = request.body.index();
      if (perms.size() <= index) {
        // return unauthorized
      }
      if (!perms[index]) {
        // return unauthorized
      }
    }
    if (auto it = std::get_if<OpenRequest>(&request.body)) {
      OpenResponse res;
      res.file = this->OpenHandler(it->pathname, it->mode);
      response.body = res;
    } else if (auto it = std::get_if<ReadRequest>(&request.body)) {
      ReadResponse res;
      res.read = this->ReadHandler(it->desc, it->count, res.bytes);
      response.body = res;
    } else if (auto it = std::get_if<WriteRequest>(&request.body)) {
      WriteResponse res;
      res.written = this->WriteHandler(it->desc, it->count, it->bytes);
      response.body = res;
    } else if (auto it = std::get_if<LSeekRequest>(&request.body)) {
      LSeekResponse res;
      res.offset = this->LSeekHandler(it->desc, it->offset, it->whence);
      response.body = res;
    } else if (auto it = std::get_if<ChmodRequest>(&request.body)) {
      ChmodResponse res;
      res.result = this->ChmodHandler(it->pathname, it->mode);
      response.body = res;
    } else if (auto it = std::get_if<UnlinkRequest>(&request.body)) {
      UnlinkResponse res;
      res.result = this->UnlinkHandler(it->pathname);
      response.body = res;
    } else if (auto it = std::get_if<RenameRequest>(&request.body)) {
      RenameResponse res;
      res.result = this->RenameHandler(it->oldpath, it->newpath);
      response.body = res;
    } else {
      throw std::invalid_argument("Invalid request type");
    }

    return rpc::marshalling::marshalResponse(response);
  };
}
} // namespace server
} // namespace rpc