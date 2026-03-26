
#include "./../code_gen/generated/example.h" 
#include <catch2/catch_all.hpp>

// Generated code under test.
// Your build should add the generated folder to include paths.
#include "example.h"

#include <cstdint>

using namespace sub8;

namespace {

users::name::space::MessageItem make_item(uint16_t f1, uint32_t f2, float f3) {
  users::name::space::MessageItem it{};
  it.feild_1 = f1;
  it.feild_2 = f2;
  it.feild_3 = f3;
  return it;
}

} // namespace

TEST_CASE("generated/example.h: HelloRequestMessage roundtrip") {
  UnboundedBitWriter bw;

  users::name::space::HelloRequestMessage tx{};
  tx.say_it_n_time = 10;
  tx.hello_phrase = "hello how are you";

  write_or_throw(bw, tx);

  UnboundedBitReader br(bw.storage(), bw.size());
  auto rx = read_or_throw<users::name::space::HelloRequestMessage>(br);

  REQUIRE(rx.say_it_n_time == tx.say_it_n_time);
  REQUIRE(rx.hello_phrase.value() == tx.hello_phrase.value());
}

TEST_CASE("generated/example.h: MessageItem roundtrip") {
  UnboundedBitWriter bw;

  users::name::space::MessageItem tx = make_item(1u, 45u, 1.5f);

  write_or_throw(bw, tx);

  UnboundedBitReader br(bw.storage(), bw.size());
  auto rx = read_or_throw<users::name::space::MessageItem>(br);

  REQUIRE(rx.feild_1 == tx.feild_1);
  REQUIRE(rx.feild_2 == tx.feild_2);

  // bfloat16 quantization can bite; pick a value that should round-trip cleanly.
  REQUIRE(static_cast<float>(rx.feild_3.value()) == Catch::Approx(static_cast<float>(tx.feild_3.value())));
}

TEST_CASE("generated/example.h: HelloResponseMessage (string + array) roundtrip") {
  UnboundedBitWriter bw;

  users::name::space::HelloResponseMessage tx{};
  tx.response_phrase = "hi";

  users::name::space::MessageItem a = make_item(1u, 2u, 1.5f);
  users::name::space::MessageItem b = make_item(3u, 4u, 2.0f);

  // Array< ObjectElement<MessageItem>, 0..5, Delimited > supports init-list when STL initializer_list is enabled.
  tx.list = HelloResponseMessage_list_T3{a, b};

  write_or_throw(bw, tx);

  UnboundedBitReader br(bw.storage(), bw.size());
  auto rx = read_or_throw<users::name::space::HelloResponseMessage>(br);

  REQUIRE(rx.response_phrase.value() == tx.response_phrase.value());
  REQUIRE(rx.list.size() == 2);

  REQUIRE(rx.list[0].feild_1 == tx.list[0].feild_1);
  REQUIRE(rx.list[0].feild_2 == tx.list[0].feild_2);
  REQUIRE(static_cast<float>(rx.list[0].feild_3.value()) == Catch::Approx(static_cast<float>(tx.list[0].feild_3.value())));

  REQUIRE(rx.list[1].feild_1 == tx.list[1].feild_1);
  REQUIRE(rx.list[1].feild_2 == tx.list[1].feild_2);
  REQUIRE(static_cast<float>(rx.list[1].feild_3.value()) == Catch::Approx(static_cast<float>(tx.list[1].feild_3.value())));
}

TEST_CASE("generated/example.h: HelloMessage variant roundtrip") {
  // Prefer a bounded writer sized by the generated MaxPossibleSize.
  auto bw = make_bounded_writer_for<users::name::space::HelloMessage>();

  // 1) hello_request
  users::name::space::HelloRequestMessage req{};
  req.say_it_n_time = 3;
  req.hello_phrase = "ping";

  users::name::space::HelloMessage tx_req{};
  REQUIRE(tx_req.set_hello_request(req) == BitFieldResult::Ok);
  write_or_throw(bw, tx_req);

  // 2) hello_response
  users::name::space::HelloResponseMessage resp{};
  resp.response_phrase = "pong";
  resp.list = HelloResponseMessage_list_T3{make_item(10u, 11u, 1.5f)};

  users::name::space::HelloMessage tx_resp{};
  REQUIRE(tx_resp.set_hello_response(resp) == BitFieldResult::Ok);
  write_or_throw(bw, tx_resp);

  // Read back
  BoundedBitReader br(bw.storage(), bw.size());

  auto rx_req = read_or_throw<users::name::space::HelloMessage>(br);
  REQUIRE(rx_req.is_hello_request());
  REQUIRE(rx_req.get_hello_request() != nullptr);
  REQUIRE(rx_req.get_hello_request()->say_it_n_time == req.say_it_n_time);
  REQUIRE(rx_req.get_hello_request()->hello_phrase.value() == req.hello_phrase.value());

  auto rx_resp = read_or_throw<users::name::space::HelloMessage>(br);
  REQUIRE(rx_resp.is_hello_response());
  REQUIRE(rx_resp.get_hello_response() != nullptr);
  REQUIRE(rx_resp.get_hello_response()->response_phrase.value() == resp.response_phrase.value());
  REQUIRE(rx_resp.get_hello_response()->list.size() == 1);
  REQUIRE(rx_resp.get_hello_response()->list[0].feild_1 == resp.list[0].feild_1);
  REQUIRE(rx_resp.get_hello_response()->list[0].feild_2 == resp.list[0].feild_2);
}

TEST_CASE("generated/example.h: bounded writer reports insufficient buffer") {
  // Intentionally too small.
  BoundedBitWriter<1> bw;

  users::name::space::HelloRequestMessage tx{};
  tx.say_it_n_time = 1;
  tx.hello_phrase = "this will not fit";

  auto r = write_field(bw, tx);
  REQUIRE(r != BitFieldResult::Ok);
  REQUIRE((r == BitFieldResult::ErrorInsufficentBufferSize || r == BitFieldResult::ErrorOversizedLength));
}
