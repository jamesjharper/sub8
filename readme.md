# Sub8

**Sub8** is an serialization/code-generation project for **very size-constrained binary protocols**.

It is designed for cases where conventional formats such as JSON, Protobuf, or CBOR are too expensive, and where both ends of the link already share a fixed schema. The motivating use case is low-bandwidth embedded communication such as **LoRa / LPWAN**, sensor telemetry, and custom radio protocols.

At a high level, Sub8 provides two things:

1. A **C++ bit-packing runtime** for compact field-level encoding.
2. A **YAML-driven code generator** that turns message/type definitions into C++ types and serialization code.


## Why this project exists

In highly constrained environments, every bit matters.

A message may need to fit into a small RF payload, be transmitted infrequently, and remain deterministic across firmware versions. In those situations, the goal is not general-purpose interoperability. The goal is:

- **predictable binary layout**
- **precise control over memory stability and stack unwinding for embedded hardware**
    - **no malloc by default, with option to fully disable**
    - **no stack unwinding by default, with option to fully exceptions**
- **minimal payload size**
- **low runtime overhead**
- **strong control over field encoding**
- **generated code rather than hand-written serialization**

Sub8 enables this design space.

## What Sub8 does

Sub8 lets you define message schemas in YAML and generate C++ code for compact binary encoding.

The current model supports concepts such as:

- fixed-width integers
- custom floating-point encodings
- enums
- optional values
- arrays
- object/message types
- tagged variants
- constrained string encodings (including B5-style packing)

## Example: Define a Small Message Protocol and Generate C++ Types

A device message might need to transmit:

- a small discriminator
- a few bounded integers
- an optional field
- a short encoded string

Instead of manually packing bits and maintaining that logic by hand, Sub8 aims to let you describe the schema once and generate the C++ representation from it. 

```yaml
import:
  - core.yaml

substitute:
  no_malloc_exceptions: false

codegen:
  sub8_file_path: "."
  default_namespace: "users::name::space"

  cpp:
    header: "generated/example.h"
    source: "generated/example.cpp"
    inline_headers: true

code_config:
  never_throw_exceptions: false
  no_malloc_exceptions: "${no_malloc_exceptions}"

types:
  HelloRequestMessage:
    kind: object
    namespace: "users::name::space"
    fields:
      say_it_n_time: u16
      hello_phrase:
        kind: b5_string
        terminated_sequence: false
        starting_multi_code_state: false
        bounded_buffer_size: 64

  HelloMessage:
    kind: variant
    namespace: "users::name::space"
    options:
      hello_request:
        id: 1
        type: HelloRequestMessage
      hello_response:
        id: 2
        type:
          name: HelloResponseMessage
          type: object
          fields:
            response_phrase:
              kind: b5_string
              max_length: 64
              terminated_sequence: false
              bounded_buffer_size: 64
            list:
              kind: array
              min_elements: 0
              max_elements: 5
              encoding: delimited
              type: MessageItem

  MessageItem:
    kind: object
    namespace: "users::name::space"
    fields:
      feild_1: u16
      feild_2: u32
      feild_3: bfloat16
```

### 2. Run the pipeline

From the repository root:

```bash
python3 src/pipeline.py all example.yaml \
  --repo-root . \
  -o /code_gen
```

This generates:

```text
example.h
example.cpp
```

### 3. Use the generated types

```cpp
#include "generated/example.h"

using namespace sub8;

int main() {
  UnboundedBitWriter bw;

  users::name::space::HelloRequestMessage tx{};
  tx.say_it_n_time = 3;
  tx.hello_phrase = "ping";

  write_or_throw(bw, tx);

  UnboundedBitReader br(bw.storage(), bw.size());
  auto rx = read_or_throw<users::name::space::HelloRequestMessage>(br);

  return (rx.say_it_n_time == tx.say_it_n_time &&
          rx.hello_phrase.value() == tx.hello_phrase.value()) ? 0 : 1;
}
```

### 4. Variant example

The same example schema also generates a tagged union-like type:

```cpp
#include "generated/example.h"

using namespace sub8;

int main() {
  auto bw = make_bounded_writer_for<users::name::space::HelloMessage>();

  users::name::space::HelloRequestMessage req{};
  req.say_it_n_time = 3;
  req.hello_phrase = "ping";

  users::name::space::HelloMessage msg{};
  if (msg.set_hello_request(req) != BitFieldResult::Ok) {
    return 1;
  }

  write_or_throw(bw, msg);

  BoundedBitReader br(bw.storage(), bw.size());
  auto rx = read_or_throw<users::name::space::HelloMessage>(br);

  if (!rx.is_hello_request() || rx.get_hello_request() == nullptr) {
    return 1;
  }

  return rx.get_hello_request()->hello_phrase.value() == req.hello_phrase.value() ? 0 : 1;
}
```