#include "schema.hpp"
#include <asio.hpp>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <udp.hpp>

namespace rpc {
namespace udp {
Client::Client(std::string url) : url{url} {}

// TODO: if this is client implementation then it should implement Client
// interface
File Client::open(std::string filepath, std::string mode) {
  using udp = asio::ip::udp;

  try {
    schema::OpenRequest body;
    asio::io_context io_context;
    udp::resolver resolver{io_context};
    udp::endpoint endpoint =
        *resolver.resolve(udp::v4(), this->url, "rpc-filesystem");
    udp::socket socket{io_context};
    socket.open(udp::v4());

    std::array<std::uint8_t, 16> sendBuffer;
    socket.send_to(asio::buffer(sendBuffer), endpoint);

    std::array<std::uint8_t, 16> receiveBuffer;
    udp::endpoint senderEndpoint;
    std::size_t len =
        socket.receive_from(asio::buffer(receiveBuffer), senderEndpoint);
  } catch (std::exception &e) {
  }
}
} // namespace udp
} // namespace rpc