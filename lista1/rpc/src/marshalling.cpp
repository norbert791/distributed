#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <variant>
#include <vector>

#include <marshalling.hpp>
#include <schema.hpp>

namespace rpc {
namespace marshalling {
namespace {
enum class Type : std::uint8_t {
  OPEN,
  READ,
  WRITE,
  LSEEK,
  CHMOD,
  UNLINK,
  RENAME,
};

// TODO: concept/check for POD?

template <typename T>
constexpr std::array<std::uint8_t, sizeof(T)> toBytes(T obj) {
  std::array<std::uint8_t, sizeof(T)> result{};
  std::uint8_t* ptr = result.data();
  std::uint8_t* objPtr = reinterpret_cast<uint8_t*>(&obj);

  std::copy(objPtr, objPtr + sizeof(obj), ptr);

  return result;
}

template <typename T>
constexpr void fromBytes(std::array<std::uint8_t, sizeof(T)> arr, T& result) {
  std::uint8_t* ptr = arr.data();
  std::uint8_t* objPtr = reinterpret_cast<uint8_t*>(&result);

  std::copy(ptr, ptr + sizeof(result), objPtr);
}

} // namespace

template <typename T>
void writeScalar(std::vector<std::uint8_t>::iterator& it, const T obj) {
  auto arr = toBytes(obj);
  it = std::copy(arr.begin(), arr.end(), it);
}

void writeString(std::vector<std::uint8_t>::iterator& it,
                 const std::string str) {
  std::uint64_t len = str.size();
  auto data = str.data();

  writeScalar(it, len);
  it = std::copy(data, data + len, it);
}

template <typename T>
void readScalar(std::vector<std::uint8_t>::iterator& it, T& obj) {
  std::array<std::uint8_t, sizeof(obj)> arr;
  std::copy(it, it + sizeof(obj), arr.data());
  it = it + sizeof(obj);

  fromBytes(arr, obj);
}

void readString(std::vector<std::uint8_t>::iterator& it, std::string& result) {
  std::uint64_t len;
  readScalar(it, len);

  std::vector<char> vec(len, 0);
  std::copy(it, it + len, vec.begin());
  result = std::string(vec.begin(), vec.end());
  it = it + len;
}

std::vector<std::uint8_t> marshalRequest(schema::Request req) {
  using namespace schema;
  std::vector<std::uint8_t> result(sizeof(req) + sizeof(Type), 0);

  auto header = req.header;
  auto resultIt = result.begin();

  writeScalar(resultIt, header.auth);
  writeScalar(resultIt, header.id);

  if (auto body = std::get_if<OpenRequest>(&req.body)) {
    writeScalar(resultIt, Type::OPEN);
    writeString(resultIt, body->pathname);
    writeScalar(resultIt, body->mode);
  } else if (auto body = std::get_if<ReadRequest>(&req.body)) {
    writeScalar(resultIt, Type::READ);
    writeScalar(resultIt, body->desc);
    writeScalar(resultIt, body->count);
  } else if (auto body = std::get_if<WriteRequest>(&req.body)) {
    writeScalar(resultIt, Type::WRITE);
    writeScalar(resultIt, body->desc);
    writeScalar(resultIt, body->count);
    resultIt = std::copy(body->bytes.begin(), body->bytes.begin() + body->count,
                         resultIt);
  } else if (auto body = std::get_if<LSeekRequest>(&req.body)) {
    writeScalar(resultIt, Type::LSEEK);
    writeScalar(resultIt, body->desc);
    writeScalar(resultIt, body->offset);
    writeScalar(resultIt, body->whence);
  } else if (auto body = std::get_if<ChmodRequest>(&req.body)) {
    writeScalar(resultIt, Type::CHMOD);
    writeString(resultIt, body->pathname);
    writeScalar(resultIt, body->mode);
  } else if (auto body = std::get_if<UnlinkRequest>(&req.body)) {
    writeScalar(resultIt, Type::UNLINK);
    writeString(resultIt, body->pathname);
  } else if (auto body = std::get_if<RenameRequest>(&req.body)) {
    writeScalar(resultIt, Type::RENAME);
    writeString(resultIt, body->oldpath);
    writeString(resultIt, body->newpath);
  } else {
    throw std::invalid_argument("Unknown type");
  }

  return result;
}

std::vector<std::uint8_t> marshalResponse(schema::Response resp) {
  using namespace schema;
  std::vector<std::uint8_t> result(sizeof(resp) + sizeof(Type), 0);

  auto resultIt = result.begin();

  writeScalar(resultIt, resp.id);
  writeScalar(resultIt, resp.code);

  if (auto body = std::get_if<OpenResponse>(&resp.body)) {
    writeScalar(resultIt, Type::OPEN);
    writeScalar(resultIt, body->file);
  } else if (auto body = std::get_if<ReadResponse>(&resp.body)) {
    writeScalar(resultIt, Type::READ);
    writeScalar(resultIt, body->read);
    resultIt = std::copy(body->bytes.begin(), body->bytes.begin() + body->read,
                         resultIt);
  } else if (auto body = std::get_if<WriteResponse>(&resp.body)) {
    writeScalar(resultIt, Type::WRITE);
    writeScalar(resultIt, body->written);
  } else if (auto body = std::get_if<LSeekResponse>(&resp.body)) {
    writeScalar(resultIt, Type::LSEEK);
    writeScalar(resultIt, body->offset);
  } else if (auto body = std::get_if<ChmodResponse>(&resp.body)) {
    writeScalar(resultIt, Type::CHMOD);
    writeScalar(resultIt, body->result);
  } else if (auto body = std::get_if<UnlinkResponse>(&resp.body)) {
    writeScalar(resultIt, Type::UNLINK);
    writeScalar(resultIt, body->result);
  } else if (auto body = std::get_if<RenameResponse>(&resp.body)) {
    writeScalar(resultIt, Type::RENAME);
    writeScalar(resultIt, body->result);
  }

  return result;
}

schema::Request unmarshalRequest(std::vector<std::uint8_t> bytes) {
  using namespace schema;
  Request result;

  auto it = bytes.begin();

  readScalar(it, result.header.auth);
  readScalar(it, result.header.id);

  Type type;
  readScalar(it, type);
  schema::RequestBody body;

  switch (type) {
  case Type::OPEN: {
    schema::OpenRequest req{};
    readString(it, req.pathname);
    readScalar(it, req.mode);
    body = req;
    break;
  }
  case Type::READ: {
    schema::ReadRequest req{};
    readScalar(it, req.desc);
    readScalar(it, req.count);
    body = req;
    break;
  }
  case Type::WRITE: {
    schema::WriteRequest req{};
    readScalar(it, req.desc);
    readScalar(it, req.count);
    req.bytes = std::vector<std::uint8_t>(req.count, 0);
    std::copy(it, it + req.count, req.bytes.begin());
    it += req.count;
    body = req;
    break;
  }
  case Type::LSEEK: {
    schema::LSeekRequest req{};
    readScalar(it, req.desc);
    readScalar(it, req.offset);
    readScalar(it, req.whence);
    body = req;
    break;
  }
  case Type::CHMOD: {
    schema::ChmodRequest req{};
    readScalar(it, req.pathname);
    readScalar(it, req.mode);
    body = req;
    break;
  }
  case Type::UNLINK: {
    schema::UnlinkRequest req{};
    readScalar(it, req.pathname);
    body = req;
    break;
  }
  case Type::RENAME: {
    schema::RenameRequest req{};
    readString(it, req.oldpath);
    readString(it, req.newpath);
    body = req;
    break;
  }
  default:
    throw std::invalid_argument("Invalid message type");
  }

  result.body = body;

  return result;
}

schema::Response unmarshalResponse(std::vector<std::uint8_t> bytes) {
  using namespace schema;
  Response result;

  auto it = bytes.begin();

  readScalar(it, result.id);
  readScalar(it, result.code);

  Type type;
  readScalar(it, type);
  schema::ResponseBody body;

  switch (type) {
  case Type::OPEN: {
    schema::OpenResponse res{};
    readScalar(it, res.file);
    body = res;
    break;
  }
  case Type::READ: {
    schema::ReadResponse res{};
    readScalar(it, res.read);
    res.bytes = std::vector<std::uint8_t>(res.read, 0);
    std::copy(it, it + res.read, res.bytes.begin());
    it += res.read;
    body = res;
    break;
  }
  case Type::WRITE: {
    schema::WriteResponse res{};
    readScalar(it, res.written);
    body = res;
    break;
  }
  case Type::LSEEK: {
    schema::LSeekResponse res{};
    readScalar(it, res.offset);
    body = res;
    break;
  }
  case Type::CHMOD: {
    schema::ChmodResponse res{};
    readScalar(it, res.result);
    body = res;
    break;
  }
  case Type::UNLINK: {
    schema::UnlinkResponse res{};
    readScalar(it, res.result);
    body = res;
    break;
  }
  case Type::RENAME: {
    schema::RenameResponse res{};
    readScalar(it, res.result);
    body = res;
    break;
  }
  default:
    throw std::invalid_argument("Invalid message type");
  }

  result.body = body;

  return result;
}

} // namespace marshalling

} // namespace rpc