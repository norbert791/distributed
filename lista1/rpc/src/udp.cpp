#include "asio/io_context.hpp"
#include "asio/ip/address_v4.hpp"
#include "schema.hpp"
#include <asio.hpp>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <udp.hpp>
#include <vector>

namespace rpc {
namespace udp {

std::vector<std::uint8_t>
Client::makeRequest(const std::vector<std::uint8_t>& data) {
  using udp = asio::ip::udp;

  try {
    asio::io_context io_context;
    udp::resolver resolver{io_context};
    udp::endpoint endpoint =
        udp::endpoint(asio::ip::address_v4{{127, 0, 0, 1}}, 13);
    udp::socket socket{io_context};
    socket.open(udp::v4());

    socket.send_to(asio::buffer(data), endpoint);

    std::vector<std::uint8_t> result(24, 0);
    udp::endpoint senderEndpoint;
    socket.receive_from(asio::buffer(result), senderEndpoint);

    return result;
  } catch (std::exception& e) {
    // TODO: better handling
    std::cout << e.what();
  }

  return {};
}

void Server::setHandler(
    std::function<std::vector<std::uint8_t>(std::vector<std::uint8_t>)>
        handler) {
  this->handler = handler;
}

void Server::run() {
  using udp = asio::ip::udp;

  try {
    asio::io_context io_context;

    udp::socket socket(io_context,
                       udp::endpoint(asio::ip::address_v4{{127, 0, 0, 1}}, 13));

    for (;;) {
      std::vector<std::uint8_t> buffer(24, 0);
      udp::endpoint remote_endpoint;
      socket.receive_from(asio::buffer(buffer), remote_endpoint);

      std::vector<std::uint8_t> message = handler(buffer);

      std::error_code ignored_error;
      socket.send_to(asio::buffer(message), remote_endpoint, 0, ignored_error);
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

} // namespace udp
} // namespace rpc