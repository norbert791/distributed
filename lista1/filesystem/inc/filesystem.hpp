#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include <server.hpp>

namespace filesystem {

class Filesystem {

public:
  Filesystem(std::filesystem::path root) : root{root} {};

  rpc::schema::File open(std::string pathname, rpc::schema::mode_t mode);
  std::int64_t read(rpc::schema::File desc, std::uint64_t count,
                    std::vector<std::uint8_t>& v);
  std::int64_t write(rpc::schema::File desc, std::uint64_t count,
                     std::vector<std::uint8_t>& v);
  rpc::schema::off_t lseek(rpc::schema::File desc, rpc::schema::off_t offset,
                           std::uint32_t whence);
  std::int64_t chmod(std::string pathname, std::uint32_t mode);
  std::int64_t unlink(std::string pathname);
  std::int64_t rename(std::string oldpath, std::string newpath);

  rpc::server::Handlers generateHandlers();

private:
  const std::filesystem::path root;

  std::unordered_map<rpc::schema::File, std::fstream> files{};
  // TODO: improve desc assignment
  rpc::schema::File descCounter = 1;
};
}; // namespace filesystem

#endif