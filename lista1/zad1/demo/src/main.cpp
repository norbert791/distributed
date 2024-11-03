#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <stop_token>
#include <thread>

#include <client.hpp>
#include <filesystem.hpp>
#include <server.hpp>
#include <udp.hpp>

int main() {
  // Init
  filesystem::Filesystem fsystem{"/tmp"};
  rpc::server::Handlers handlers{fsystem.generateHandlers()};
  std::uint64_t token = 123;
  //   std::unordered_map<std::uint64_t, std::array<bool, 7>> users;
  std::array<bool, 8> perms;
  perms.fill(true);
  std::unordered_map<std::uint64_t, std::array<bool, 8>> users{{token, perms}};
  auto protoServ = std::make_shared<rpc::udp::Server>();
  rpc::server::Server serv{handlers, protoServ, users};

  rpc::client::Client client{123,
                             std::make_shared<rpc::udp::Client>("localhost")};

  std::jthread thread{[protoServ]() { protoServ->run(); }};
  std::stop_callback callback(thread.get_stop_token(),
                              [protoServ]() { protoServ->stop(); });

  // TODO: hide static_casts inside methods

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  auto file = client.open(
      "file", static_cast<rpc::schema::mode_t>(std::ios::in | std::ios::out));
  std::vector<std::uint8_t> bytes{40, 50, 10, 13, 50};
  std::cout << "file opened with desc " << file << "\n";
  auto r = client.write(file, bytes.size(), bytes);
  std::cout << r << " bytes written\n";
  client.lseek(file, 2, static_cast<std::uint32_t>(std::ios::beg));
  std::cout << "seeked\n";
  std::vector<std::uint8_t> respBytes{};
  auto num = client.read(file, 3, respBytes);
  std::cout << num << " bytes read\n";
  for (const auto num : respBytes) {
    std::cout << " " << static_cast<std::uint32_t>(num);
  }
  std::cout << "\n";
  auto status = client.close(file);
  std::cout << "file closed with code: " << status << "\n";
  client.chmod("file", static_cast<std::uint32_t>(std::filesystem::perms::all));
  std::cout << "chmod\n";
  client.rename("file", "newfile");
  std::cout << "rename\n";
  client.unlink("newfile");
  std::cout << "unlink\n";
}