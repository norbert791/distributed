#ifndef RPC_MARSHALLING_HPP
#define RPC_MARSHALLING_HPP

#include <vector>

#include "schema.hpp"

namespace rpc {
namespace marshalling {
std::vector<std::uint8_t> marshalRequest(schema::Request);
schema::Request unmarshalRequest(std::vector<std::uint8_t>);
std::vector<std::uint8_t> marshalResponse(schema::Response);
schema::Response unmarshalResponse(std::vector<std::uint8_t>);
} // namespace marshalling
} // namespace rpc

#endif // RPC_MARSHALLING_HPP