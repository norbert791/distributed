#include "schema.hpp"
#include <gtest/gtest.h>
#include <marshalling.hpp>
#include <variant>

TEST(rpc_marshalling, request) {
  using namespace rpc;

  // ReadRequest
  {
    schema::ReadRequest body{
        .desc = 5,
        .count = 12,
    };
    schema::Request req{
        .header = {.auth = 12, .id = 15},
        .body = body,
    };
    auto bytes = marshaling::marshalRequest(req);
    auto unmarshaled = marshaling::unmarshalRequest(bytes);

    EXPECT_EQ(req.header.id, unmarshaled.header.id);
    EXPECT_EQ(req.header.auth, unmarshaled.header.auth);
    auto ptr = std::get_if<schema::ReadRequest>(&req.body);
    ASSERT_TRUE(ptr);
    EXPECT_EQ(ptr->count, body.count);
    EXPECT_EQ(ptr->desc, body.desc);
  }

  // RenameRequest
  {
    schema::RenameRequest body{
        .oldpath = "path1",
        .newpath = "path2",
    };
    schema::Request req{
        .header = {.auth = 4, .id = 60},
        .body = body,
    };
    auto bytes = marshaling::marshalRequest(req);
    auto unmarshaled = marshaling::unmarshalRequest(bytes);

    EXPECT_EQ(req.header.id, unmarshaled.header.id);
    EXPECT_EQ(req.header.auth, unmarshaled.header.auth);
    auto ptr = std::get_if<schema::RenameRequest>(&req.body);
    ASSERT_TRUE(ptr);
    EXPECT_EQ(ptr->oldpath, body.oldpath);
    EXPECT_EQ(ptr->newpath, body.newpath);
  }
}