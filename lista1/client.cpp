
#include <iostream>

#include <asio.hpp>
#include <udp.hpp>

int main() {
  rpc::udp::Client client{"localhost"};

  auto result = client.makeRequest({5, 10, 15, 20});
  for (const auto it : result) {
    std::cout << (static_cast<std::uint32_t>(it)) << "\n";
  }
}