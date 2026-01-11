#pragma once

#include <fstream>

template<typename Container> inline std::string to_binary_string(const Container &buf) {
  if (buf.size() == 0) {
    return "{ }";
  }

  std::string out;
  out.reserve(buf.size() * 12);
  out.append("{ 0b");

  for (std::size_t i = 0; i < buf.size(); ++i) {
    uint8_t b = static_cast<uint8_t>(buf.at(i));

    for (int bit = 7; bit >= 0; --bit) {
      out.push_back((b & (1u << bit)) ? '1' : '0');
    }

    if (i + 1 < buf.size()) {
      out.append(", 0b");
    }
  }

  out.append(" }");
  return out;
}

static inline uint32_t bits_from_float(float v) {
  uint32_t b;
  std::memcpy(&b, &v, sizeof(b));
  return b;
}

static inline uint64_t bits_from_float(double v) {
  uint64_t b;
  std::memcpy(&b, &v, sizeof(b));
  return b;
}

static inline float float_from_bits(uint32_t u) {
  float f;
  std::memcpy(&f, &u, sizeof(f));
  return f;
}

static inline float float16_subnormal_value(uint16_t frac /* 1..1023 */) {
  return std::ldexp(1.0f, -24) * static_cast<float>(frac);
}

#define MAX_BITS(n) ((n) >= 64 ? ~0ULL : ((1ULL << (n)) - 1ULL))

static void require_strings_match(const std::string &a, const std::string &b) {
  if (a == b) {
    REQUIRE(a == b);  // keep normal reporting
    return;
  }

  const size_t asz = a.size();
  const size_t bsz = b.size();
  const size_t m = std::min(asz, bsz);

  size_t diff = m;
  for (size_t i = 0; i < m; ++i) {
    if (a[i] != b[i]) {
      diff = i;
      break;
    }
  }

  INFO("Strings differ!");

  INFO("  Index of first difference: " << diff);
  INFO("  a.size(): " << asz);
  INFO("  b.size(): " << bsz);
  INFO("  a[diff] = " << int(uint8_t(a[diff])) << " ('" << a[diff] << "')");
  INFO("  b[diff] = " << int(uint8_t(b[diff])) << " ('" << b[diff] << "')");
  INFO("  string is truncated: " << (diff == m ? "true" : "false"));

  // Optional context window
  const int window = 16;
  const size_t start = diff > window ? diff - window : 0;
  const size_t end = diff + window;

  INFO("Context around diff: [" << start << " <-> " << end << "]");
  INFO("a: " << a.substr(start, std::min(end, asz) - start));
  INFO("b: " << b.substr(start, std::min(end, bsz) - start));

  REQUIRE(false);  // force failure now that info has been printed
}
