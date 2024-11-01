#include "asio/io_context.hpp"
#include "asio/ip/address_v4.hpp"
#include <asio.hpp>
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

    std::cout << "client sent\n";
    socket.send_to(asio::buffer(data), endpoint);

    std::vector<std::uint8_t> result(24, 0);
    udp::endpoint senderEndpoint;
    socket.receive_from(asio::buffer(result), senderEndpoint);
    std::cout << "client received\n";

    return result;
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
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

  if (this->running) {
    return;
  }
  this->running = true;

  try {
    asio::io_context io_context;

    udp::socket socket(io_context,
                       udp::endpoint(asio::ip::address_v4{{127, 0, 0, 1}}, 13));

    while (this->running) {
      std::vector<std::uint8_t> buffer(240, 0);
      udp::endpoint remote_endpoint;
      // TODO: add timeout and retry
      socket.receive_from(asio::buffer(buffer), remote_endpoint);
      std::cout << "server received\n";

      std::vector<std::uint8_t> message = handler(buffer);

      std::error_code ignored_error;
      socket.send_to(asio::buffer(message), remote_endpoint, 0, ignored_error);
      std::cout << "server sent\n";
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

void Server::stop() {
  this->running = false;
  this->ctx.stop();
}

} // namespace udp
} // namespace rpc