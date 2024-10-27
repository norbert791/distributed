#include "schema.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#include <iterator>
#include <marshaling.hpp>
#include <stdexcept>
#include <variant>
#include <vector>

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
  std::copy(it, it + sizeof(obj), arr);
  it = it + sizeof(obj);

  fromBytes(arr, obj);
}

template <typename T>
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

} // namespace marshaling

} // namespace rpc