#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <variant>
#include <vector>

#include <marshalling.hpp>
#include <schema.hpp>

namespace rpc {
namespace marshaling {
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
void writeBytes(std::vector<std::uint8_t>::iterator& it, T obj) {
  auto arr = toBytes(obj);
  it = std::copy(arr.begin(), arr.end(), it);
}

void writeString(std::vector<std::uint8_t>::iterator& it, std::string str) {
  std::uint64_t len = str.size();
  auto data = str.data();

  writeBytes(it, len);
  it = std::copy(data, data + len, it);
}

template <typename T>
void readBytes(std::vector<std::uint8_t>::iterator& it, T& obj) {
  std::array<std::uint8_t, sizeof(obj)> arr;
  std::copy(it, it + sizeof(obj), arr.data());
  it = it + sizeof(obj);

  fromBytes(arr, obj);
}

void readString(std::vector<std::uint8_t>::iterator& it, std::string& result) {
  std::uint64_t len;
  readBytes(it, len);

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

  writeBytes(resultIt, header.auth);
  writeBytes(resultIt, header.id);

  if (auto body = std::get_if<OpenRequest>(&req.body)) {
    writeBytes(resultIt, Type::OPEN);
    writeString(resultIt, body->pathname);
    writeString(resultIt, body->mode);
  } else if (auto body = std::get_if<ReadRequest>(&req.body)) {
    writeBytes(resultIt, Type::READ);
    writeBytes(resultIt, body->desc);
    writeBytes(resultIt, body->count);
  } else if (auto body = std::get_if<WriteRequest>(&req.body)) {
    writeBytes(resultIt, Type::WRITE);
    writeBytes(resultIt, body->desc);
    writeBytes(resultIt, body->count);
  } else if (auto body = std::get_if<LSeekRequest>(&req.body)) {
    writeBytes(resultIt, Type::LSEEK);
    writeBytes(resultIt, body->desc);
    writeBytes(resultIt, body->offset);
  } else if (auto body = std::get_if<ChmodRequest>(&req.body)) {
    writeBytes(resultIt, Type::CHMOD);
    writeString(resultIt, body->pathname);
    writeBytes(resultIt, body->mode);
  } else if (auto body = std::get_if<UnlinkRequest>(&req.body)) {
    writeBytes(resultIt, Type::UNLINK);
    writeString(resultIt, body->pathname);
  } else if (auto body = std::get_if<RenameRequest>(&req.body)) {
    writeBytes(resultIt, Type::RENAME);
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

  auto header = resp.header;
  auto resultIt = result.begin();

  writeBytes(resultIt, header.auth);
  writeBytes(resultIt, header.id);

  if (auto body = std::get_if<OpenResponse>(&resp.body)) {
    writeBytes(resultIt, Type::OPEN);
    writeBytes(resultIt, body->file);
  } else if (auto body = std::get_if<ReadResponse>(&resp.body)) {
    writeBytes(resultIt, Type::READ);
    writeBytes(resultIt, body->read);
  } else if (auto body = std::get_if<WriteResponse>(&resp.body)) {
    writeBytes(resultIt, Type::WRITE);
    writeBytes(resultIt, body->written);
  } else if (auto body = std::get_if<LSeekResponse>(&resp.body)) {
    writeBytes(resultIt, Type::LSEEK);
    writeBytes(resultIt, body->offset);
  } else if (auto body = std::get_if<ChmodResponse>(&resp.body)) {
    writeBytes(resultIt, Type::CHMOD);
    writeBytes(resultIt, body->result);
  } else if (auto body = std::get_if<UnlinkResponse>(&resp.body)) {
    writeBytes(resultIt, Type::UNLINK);
    writeBytes(resultIt, body->result);
  } else if (auto body = std::get_if<RenameResponse>(&resp.body)) {
    writeBytes(resultIt, Type::RENAME);
    writeBytes(resultIt, body->result);
  }

  return result;
}

schema::Request unmarshalRequest(std::vector<std::uint8_t> bytes) {
  using namespace schema;
  Request result;

  auto it = bytes.begin();

  readBytes(it, result.header.auth);
  readBytes(it, result.header.id);

  Type type;
  readBytes(it, type);
  schema::RequestBody body;

  switch (type) {
  case Type::OPEN: {
    schema::OpenRequest req{};
    readString(it, req.pathname);
    readString(it, req.mode);
    body = req;
    break;
  }
  case Type::READ: {
    schema::ReadRequest req{};
    readBytes(it, req.desc);
    readBytes(it, req.count);
    body = req;
    break;
  }
  case Type::WRITE: {
    schema::WriteRequest req{};
    readBytes(it, req.desc);
    readBytes(it, req.count);
    body = req;
    break;
  }
  case Type::LSEEK: {
    schema::LSeekRequest req{};
    readBytes(it, req.desc);
    readBytes(it, req.offset);
    body = req;
    break;
  }
  case Type::CHMOD: {
    schema::ChmodRequest req{};
    readBytes(it, req.pathname);
    readBytes(it, req.mode);
    body = req;
    break;
  }
  case Type::UNLINK: {
    schema::UnlinkRequest req{};
    readBytes(it, req.pathname);
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

  readBytes(it, result.header.auth);
  readBytes(it, result.header.id);

  Type type;
  readBytes(it, type);
  schema::ResponseBody body;

  switch (type) {
  case Type::OPEN: {
    schema::OpenResponse res{};
    readBytes(it, res.file);
    body = res;
    break;
  }
  case Type::READ: {
    schema::ReadResponse res{};
    readBytes(it, res.read);
    body = res;
    break;
  }
  case Type::WRITE: {
    schema::WriteResponse res{};
    readBytes(it, res.written);
    body = res;
    break;
  }
  case Type::LSEEK: {
    schema::LSeekResponse res{};
    readBytes(it, res.offset);
    body = res;
    break;
  }
  case Type::CHMOD: {
    schema::ChmodResponse res{};
    readBytes(it, res.result);
    body = res;
    break;
  }
  case Type::UNLINK: {
    schema::UnlinkResponse res{};
    readBytes(it, res.result);
    body = res;
    break;
  }
  case Type::RENAME: {
    schema::RenameResponse res{};
    readBytes(it, res.result);
    body = res;
    break;
  }
  default:
    throw std::invalid_argument("Invalid message type");
  }

  result.body = body;

  return result;
}

} // namespace marshaling

} // namespace rpc