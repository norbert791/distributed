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

  rpc::server::File open(std::string pathname, rpc::server::mode_t mode);
  std::int64_t read(rpc::server::File desc, std::vector<std::uint8_t>& v,
                    std::uint64_t count);
  std::int64_t write(rpc::server::File desc, std::vector<std::uint8_t>& v,
                     std::uint64_t count);
  rpc::server::off_t lseek(rpc::server::File desc, rpc::server::off_t offset);
  std::int64_t chmod(std::string pathname, std::uint32_t mode);
  std::int64_t unlink(std::string pathname);
  std::int64_t rename(std::string oldpath, std::string newpath);

private:
  const std::filesystem::path root;

  std::unordered_map<rpc::server::File, std::fstream> files{};
  // TODO: improve desc assignment
  rpc::server::File descCounter = 1;
};
}; // namespace filesystem

#endif