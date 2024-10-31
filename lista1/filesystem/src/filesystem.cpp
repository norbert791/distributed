#include "server.hpp"
#include <filesystem.hpp>

#include <filesystem>
#include <functional>
#include <ios>
#include <iostream>
#include <vector>

namespace filesystem {
rpc::schema::File Filesystem::open(std::string pathname,
                                   rpc::schema::mode_t mode) {
  std::filesystem::path path{pathname};
  std::filesystem::path root{this->root};
  root += path;
  rpc::schema::File desc = this->descCounter;
  descCounter++;

  std::fstream stream{};
  stream.open(root, static_cast<std::ios_base::openmode>(mode));

  if (!stream.good()) {
    return 0;
  }

  this->files.insert(std::make_pair(desc, std::move(stream)));

  return desc;
}

std::int64_t Filesystem::read(rpc::schema::File desc, std::uint64_t count,
                              std::vector<std::uint8_t>& v) {
  if (!this->files.contains(desc)) {
    return -1;
  }

  // TODO: add exception handling

  std::fstream& file = this->files[desc];
  file.read(reinterpret_cast<char*>(v.data()), count);

  return count;
}

std::int64_t Filesystem::write(rpc::schema::File desc, std::uint64_t count,
                               std::vector<std::uint8_t>& v) {
  if (!this->files.contains(desc)) {
    return -1;
  }

  // TODO: add exception handling

  std::fstream& file = this->files[desc];
  file.write(reinterpret_cast<char*>(v.data()), count);

  return count;
}

rpc::schema::off_t Filesystem::lseek(rpc::schema::File desc,
                                     rpc::schema::off_t offset,
                                     std::uint32_t whence) {
  if (!this->files.contains(desc)) {
    return -1;
  }

  // TODO: add exception handling

  std::fstream& file = this->files[desc];
  file.seekg(offset, static_cast<std::ios::seekdir>(whence));
  file.seekp(offset, static_cast<std::ios::seekdir>(whence));

  return offset;
}

std::int64_t Filesystem::chmod(std::string pathname, std::uint32_t mode) {
  std::filesystem::path path{pathname};
  std::filesystem::path root{this->root};
  root += path;

  // TODO: add exception handling

  std::filesystem::permissions(root, static_cast<std::filesystem::perms>(mode));

  return 0;
}

std::int64_t Filesystem::unlink(std::string pathname) {

  // TODO: add exception handling

  std::filesystem::remove(pathname);
  return 0;
}

std::int64_t Filesystem::rename(std::string oldpath, std::string newpath) {
  std::filesystem::rename(oldpath, newpath);

  // TODO: add exception handling

  return 0;
}

rpc::server::Handlers Filesystem::generateHandlers() {
  using namespace std::placeholders;
  return rpc::server::Handlers{
      .OpenHandler = std::bind(&Filesystem::open, this, _1, _2),
      .ReadHandler = std::bind(&Filesystem::read, this, _1, _2, _3),
      .WriteHandler = std::bind(&Filesystem::write, this, _1, _2, _3),
      .LSeekHandler = std::bind(&Filesystem::lseek, this, _1, _2, _3),
      .ChmodHandler = std::bind(&Filesystem::chmod, this, _1, _2),
      .UnlinkHandler = std::bind(&Filesystem::unlink, this, _1),
      .RenameHandler = std::bind(&Filesystem::rename, this, _1, _2),
  };
}

} // namespace filesystem
