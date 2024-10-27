#include <iostream>
#include <vector>

#include "asio/io_context.hpp"
#include <asio.hpp>
#include <udp.hpp>

int main() {
  try {
    rpc::udp::Server server{};

    server.setHandler(
        [](std::vector<std::uint8_t> input) -> std::vector<std::uint8_t> {
          for (const auto ch : input) {
            std::cout << (static_cast<std::uint32_t>(ch)) << "\n";
          }
          return {30, 40, 50};
        });

    server.run();

  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}