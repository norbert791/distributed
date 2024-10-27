#ifndef RPC_MARSHALING_HPP
#define RPC_MARSHALING_HPP

#include <cinttypes>
#include <vector>

#include "schema.hpp"

namespace rpc {
namespace marshaling {
std::vector<std::uint8_t> marshalRequest(schema::Request);
schema::Request unmarshalRequest(std::vector<std::uint8_t>);
std::vector<std::uint8_t> marshalResponse(schema::Response);
std::vector<std::uint8_t> unmarshalResponse(schema::Response);
} // namespace marshaling
} // namespace rpc

#endif // RPC_MARSHALING_HPP