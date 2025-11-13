# AIChat C/C++ Implementation

## Overview

This is a complete C/C++ implementation of AIChat as a cognitive kernel system, built on **GGML** and **llama.cpp** backends. It implements OpenCog cognitive primitives as high-performance tensor operations following the Echo.Kern architecture.

## Architecture

### Kernel Layer (C)
- **Bootstrap**: Multi-stage initialization (Stage 0-3)
- **Scheduler**: Membrane-aware task scheduling (≤5µs/tick target)
- **Memory**: Tensor-backed memory management (≤100ns/op target)
- **HGFS**: Hypergraph filesystem using GGML tensors

### Cognitive Layer (C++)
- **AtomSpace**: Hypergraph knowledge representation
- **ECAN**: Economic attention allocation networks
- **PLN**: Probabilistic logic networks for reasoning
- **ESN**: Echo state network reservoir computing

### LLM Layer (C++)
- **Inference**: llama.cpp integration
- **Chat**: Chat completion API
- Streaming support

### CLI Layer (C++)
- Command-line argument parsing
- REPL mode with readline
- Single-query command mode

## Building

### Prerequisites

```bash
# Install dependencies
sudo apt-get install cmake build-essential libreadline-dev

# GGML and llama.cpp are fetched automatically by CMake
```

### Build Steps

```bash
cd cpp
mkdir build && cd build
cmake ..
make -j$(nproc)
```

This will build:
- `aichat` - Main executable
- `libaichat-core.a` - Core library
- Test executables (if BUILD_TESTS=ON)

### Build Options

```bash
cmake -DBUILD_TESTS=OFF ..           # Disable tests
cmake -DCMAKE_BUILD_TYPE=Release ..  # Release build with optimizations
```

## Usage

### Command Mode

```bash
./aichat -m model.gguf "What is the capital of France?"
```

### REPL Mode

```bash
./aichat -r -m model.gguf
```

### Options

```
-m, --model PATH      Path to GGUF model file (required)
-c, --config PATH     Path to configuration file
-r, --repl            Start in REPL mode
-s, --stream          Enable streaming output
-t, --temperature T   Sampling temperature (default: 0.7)
-n, --max-tokens N    Maximum tokens to generate (default: 512)
-h, --help            Show help message
```

## Testing

```bash
# Run all tests
cd build
ctest

# Run specific test
./tests/test_kernel bootstrap
./tests/test_cognitive atomspace
```

## API Documentation

Generate Doxygen documentation:

```bash
cd build
make doc
# Open docs/html/index.html
```

## Performance Targets

- **Scheduler tick**: ≤5µs per tick
- **Memory operations**: ≤100ns per operation
- **Context switch**: Real-time capable
- **Tensor operations**: Optimized via GGML SIMD kernels

## Implementation Status

### Kernel Primitives (10 Core Functions)
- [x] Bootstrap (Stage 0-3)
- [x] Scheduler (dtesn_sched_*)
- [x] Memory Management (dtesn_mem_*)
- [x] Hypergraph FS (hgfs_*)
- [ ] Interrupts
- [ ] Syscalls
- [ ] I/O
- [ ] Sync primitives
- [ ] Timers
- [ ] Protection/ABI

### Cognitive Components
- [x] AtomSpace (hypergraph tensor)
- [x] ECAN (attention allocation)
- [x] PLN (probabilistic reasoning)
- [x] ESN (reservoir computing)
- [ ] CognitiveLoop orchestration

### LLM Integration
- [x] Model loading (llama.cpp)
- [x] Chat completion
- [x] Streaming
- [ ] Multi-model support
- [ ] Function calling
- [ ] RAG integration

### CLI/REPL
- [x] Argument parsing
- [x] REPL with readline
- [x] Command mode
- [ ] Session management
- [ ] Role/agent support
- [ ] History persistence

## Technical Details

### Memory Layout

All memory is allocated through the tensor-backed allocator (`dtesn_mem_alloc`), which uses GGML tensors as backing store. This enables:
- Unified memory for CPU and GPU operations
- Automatic quantization support
- Hardware acceleration via GGML kernels

### Hypergraph Representation

The AtomSpace is implemented as a hypergraph where:
- Nodes are GGML tensors (512-dim embeddings)
- Edges are weighted connections
- Queries traverse tensor graph using GGML operations

### Attention Allocation

ECAN implements economic attention using:
- STI (short-term importance) spreading
- LTI (long-term importance) accumulation
- Decay dynamics for forgetting

### Probabilistic Reasoning

PLN implements basic inference rules:
- Deduction: (A→B, B→C) ⇒ (A→C)
- Induction: (A→B, C→B) ⇒ (A→C)
- Abduction: (A→B, A→C) ⇒ (C→B)

Truth values are computed using tensor operations.

## Development

### Code Structure

```
cpp/
├── CMakeLists.txt          # Build configuration
├── include/
│   ├── aichat.h            # Main header
│   └── aichat/
│       ├── kernel.h        # Kernel API
│       ├── cognitive.h     # Cognitive API
│       ├── llm.h           # LLM API
│       └── cli.h           # CLI API
├── src/
│   ├── kernel/             # Kernel implementation (C)
│   ├── cognitive/          # Cognitive components (C++)
│   ├── llm/                # LLM integration (C++)
│   ├── cli/                # CLI/REPL (C++)
│   └── main.cpp            # Entry point
├── tests/                  # Test suite
└── docs/                   # Documentation

```

### Coding Standards

- **C**: C99, K&R braces, 4-space indent
- **C++**: C++17, same style as C
- **Comments**: Doxygen `/** ... */` for all public functions
- **Alignment**: 64-byte for SIMD optimization
- **Error handling**: Return codes (0 = success, negative = error)

## License

Dual-licensed under MIT or Apache 2.0, same as the Rust version.

## References

- [OpenCog Cognitive Architecture](https://wiki.opencog.org/)
- [GGML](https://github.com/ggerganov/ggml)
- [llama.cpp](https://github.com/ggerganov/llama.cpp)
- [Echo.Kern Architecture](../IMPLEMENTATION_SUMMARY.md)
