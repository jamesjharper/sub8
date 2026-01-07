# Sub8 — Sub-Byte Bit Packing & B5 String Encoding

Sub8 is a **C++ library for packing data at the bit level**, designed for **severely size-constrained transmission systems** such as **LoRa / LPWAN**, embedded radios, and custom binary protocols.

---

## When Should You Use This?

Sub8 is a structured data format, requiring both send and transmit to have prior knowledge of the transmitted package. 
This library is **not general-purpose serialization**. It exists to solve one problem extremely well:

> **Transmit as little data as possible, predictably and deterministically.**

# 1. Simple Examples

## Example 1: Packing Small Integers

Suppose you want to transmit:

- a 3-bit enum
- a 1-bit flag
- a 6-bit value

```cpp

FixedBitWriter<64> bw; 


sub8::BitWriter writer(buffer, sizeof(buffer));

writer.write_bits(5, 3);   // enum (0–7)
writer.write_bool(true);   // 1 bit
writer.write_bits(42, 6);  // value (0–63)

writer.finalize();
