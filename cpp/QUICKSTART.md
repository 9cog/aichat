# Quick Start Guide - AIChat C/C++

## 5-Minute Quick Start

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install -y cmake build-essential libreadline-dev git

# macOS
brew install cmake readline

# Fedora/RHEL
sudo dnf install -y cmake gcc-c++ readline-devel git
```

### Build

```bash
# Clone repository (if not already cloned)
cd /path/to/aichat

# Build C++ implementation
cd cpp
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

This takes ~5-10 minutes on first build (downloads GGML and llama.cpp).

### Get a Model

Download a GGUF model (example: Phi-2):

```bash
# Using curl
curl -L -o phi-2.Q4_K_M.gguf \
  https://huggingface.co/TheBloke/phi-2-GGUF/resolve/main/phi-2.Q4_K_M.gguf

# Or using wget
wget https://huggingface.co/TheBloke/phi-2-GGUF/resolve/main/phi-2.Q4_K_M.gguf
```

**Alternative models:**
- [TinyLlama](https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF)
- [Mistral-7B](https://huggingface.co/TheBloke/Mistral-7B-Instruct-v0.2-GGUF)
- [Llama-2-7B](https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF)

### Run

#### Command Mode (Single Query)

```bash
./aichat -m phi-2.Q4_K_M.gguf "What is the capital of France?"
```

#### REPL Mode (Interactive)

```bash
./aichat -r -m phi-2.Q4_K_M.gguf
```

Then type queries:
```
> Hello, how are you?
> What is 2+2?
> quit
```

#### With Streaming

```bash
./aichat -s -m phi-2.Q4_K_M.gguf "Tell me a story"
```

### Common Options

```bash
# Lower temperature (more deterministic)
./aichat -m model.gguf -t 0.3 "query"

# More tokens
./aichat -m model.gguf -n 1024 "query"

# Streaming REPL
./aichat -r -s -m model.gguf
```

### Run Tests

```bash
cd build
ctest

# Or run individual tests
./tests/test_kernel bootstrap
./tests/test_cognitive atomspace
```

### Expected Output

#### Bootstrap
```
AIChat C++ v0.1.0 - Cognitive Kernel Edition
Built with GGML and llama.cpp

[STAGE0] GGML context initialized (128 MB)
[STAGE1] Hypergraph filesystem initialized
[STAGE2] Scheduler initialized (target: 5 µs/tick)
[STAGE3] Cognitive components initialized
[BOOTSTRAP] Completed up to stage 3
```

#### Chat
```
Loading model: phi-2.Q4_K_M.gguf
Model loaded successfully
AIChat REPL (type 'quit' to exit)

> What is AI?
AI stands for Artificial Intelligence. It refers to computer systems
designed to perform tasks that typically require human intelligence...
```

## Troubleshooting

### Build Issues

**Error: "Could not find ggml"**
```bash
# Make sure you have internet connection
# CMake will auto-download via FetchContent
cmake .. -DFETCHCONTENT_QUIET=OFF
```

**Error: "readline not found"**
```bash
# Install readline development headers
sudo apt-get install libreadline-dev  # Ubuntu
brew install readline                  # macOS
```

### Runtime Issues

**Error: "Failed to load model"**
- Check model path is correct
- Ensure GGUF format (not older GGML format)
- Verify file is not corrupted

**Error: "Out of memory"**
- Try smaller model (e.g., TinyLlama instead of Llama-70B)
- Reduce context size in code
- Increase system RAM or swap

### Performance Issues

**Slow generation**
- Use quantized models (Q4_K_M, Q5_K_M)
- Enable CPU acceleration flags in CMake
- Try smaller models for testing

## Next Steps

### Explore Features

1. **Test kernel primitives**
   ```bash
   ./tests/test_kernel scheduler
   ./tests/test_kernel memory
   ```

2. **Test cognitive components**
   ```bash
   ./tests/test_cognitive atomspace
   ./tests/test_cognitive pln
   ```

3. **Experiment with different models**
   - Compare responses across models
   - Test different temperature settings
   - Try streaming vs. non-streaming

### Extend the Implementation

1. **Add new cognitive functions**
   - See `cpp/src/cognitive/` for examples
   - Follow OpenCog patterns

2. **Implement missing kernel primitives**
   - See `cpp/KERNEL_FUNCTION_MANIFEST.md`
   - Interrupts, syscalls, I/O, etc.

3. **Add RAG support**
   - Integrate document loading
   - Build vector search
   - Implement retrieval pipeline

### Read Documentation

- **README**: `cpp/README.md`
- **Implementation Notes**: `cpp/IMPLEMENTATION_NOTES.md`
- **Function Manifest**: `cpp/KERNEL_FUNCTION_MANIFEST.md`
- **Validation Report**: `cpp/VALIDATION_REPORT.md`

## Getting Help

### Check Logs

```bash
# Verbose build
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
make VERBOSE=1

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
gdb ./aichat
```

### Common Questions

**Q: Why is first run slow?**
A: Model loading and initialization takes time. Subsequent runs are faster.

**Q: Can I use OpenAI models?**
A: Not directly. This implementation uses local GGUF models via llama.cpp.

**Q: How do I add GPU support?**
A: llama.cpp supports CUDA/Metal. Enable in CMake:
```bash
cmake .. -DLLAMA_CUBLAS=ON  # NVIDIA
cmake .. -DLLAMA_METAL=ON   # Apple Silicon
```

**Q: Is this compatible with the Rust version?**
A: It's a complete reimplementation with different architecture. APIs differ.

## Example Session

```bash
$ cd cpp/build
$ ./aichat -r -s -m ~/models/phi-2.Q4_K_M.gguf

AIChat C++ v0.1.0 - Cognitive Kernel Edition
Built with GGML and llama.cpp

[STAGE0] GGML context initialized (128 MB)
[STAGE1] Hypergraph filesystem initialized
[STAGE2] Scheduler initialized (target: 5 µs/tick)
[STAGE3] Cognitive components initialized
[BOOTSTRAP] Completed up to stage 3
Loading model: /home/user/models/phi-2.Q4_K_M.gguf
Model loaded successfully
AIChat REPL (type 'quit' to exit)

> What is the meaning of life?
The meaning of life is a philosophical question that has been debated
for centuries. Different people and cultures have different answers...

> Tell me a joke
Why don't scientists trust atoms? Because they make up everything!

> quit

Goodbye!
```

## Success!

You now have a working C/C++ cognitive AI system with:
- ✅ GGML tensor operations
- ✅ llama.cpp LLM inference
- ✅ OpenCog-inspired cognitive architecture
- ✅ Interactive REPL
- ✅ Command mode
- ✅ Streaming support

Happy hacking! 🚀
