#include "server.hpp"
#include <filesystem.hpp>
#include <filesystem>
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
                                     rpc::schema::off_t offset) {
  if (!this->files.contains(desc)) {
    return -1;
  }

  // TODO: add exception handling

  std::fstream& file = this->files[desc];
  file.seekg(offset);
  file.seekp(offset);

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

} // namespace filesystem
