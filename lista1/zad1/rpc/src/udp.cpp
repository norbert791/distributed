#include "asio/io_context.hpp"
#include "asio/ip/address_v4.hpp"
#include <asio.hpp>
#include <chrono>
#include <cstdint>
#include <exception>
#include <future>
#include <iostream>
#include <stdexcept>
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

    std::vector<std::uint8_t> result(1024, 0);
    udp::endpoint senderEndpoint;
    auto status = std::async(std::launch::async, [&]() {
                    socket.receive_from(asio::buffer(result), senderEndpoint);
                  }).wait_for(std::chrono::seconds{5});
    if (status == std::future_status::ready) {
      std::cout << "client received\n";
      return result;
    }
    if (status != std::future_status::timeout) {
      throw std::logic_error("unknown status");
    }
    std::vector<std::uint8_t> result2(1024, 0);
    status = std::async(std::launch::async, [&]() {
               socket.receive_from(asio::buffer(result2), senderEndpoint);
             }).wait_for(std::chrono::seconds{15});
    if (status == std::future_status::ready) {
      std::cout << "client received\n";
      return result;
    }
    if (status != std::future_status::timeout) {
      throw std::logic_error("unknown status");
    }
    throw std::runtime_error("timeout");
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