# AIChat Technical Documentation

This directory contains comprehensive technical architecture documentation and formal specifications for the AIChat system.

## Overview

AIChat is an all-in-one LLM CLI tool written in Rust that provides a unified interface to interact with 20+ LLM providers. The system supports multiple interaction modes, RAG (Retrieval-Augmented Generation), function calling, agent workflows, and more.

## Documentation Structure

### 1. Architecture Documentation

**File**: `architecture_overview.md`

Comprehensive system architecture documentation with 12 Mermaid diagrams covering:

- **High-Level Architecture**: Component organization and data flow
- **Working Modes**: CMD, REPL, and HTTP Server modes
- **Configuration System**: Models, clients, sessions, roles, agents
- **Message Processing**: Chat completion flows (streaming and non-streaming)
- **Client Architecture**: Multi-provider LLM integration
- **RAG System**: Document ingestion, indexing, and retrieval
- **Function Calling**: Tool execution and MCP protocol
- **HTTP Server**: API endpoints and web frontends
- **Integration Boundaries**: External service contracts
- **Technology Stack**: Core technologies and dependencies
- **Storage Architecture**: Filesystem layout and data models
- **Security Considerations**: API keys, network security, input validation

### 2. Formal Z++ Specifications

Four comprehensive formal specifications modeling the complete system behavior:

#### `formal_spec_data_model.zpp` (20KB, 9 sections)

Formalizes core data structures and invariants:

- **Basic Types**: MessageRole, ModelType, WorkingMode
- **Message Structures**: Message, MessageContent, MessageContentPart
- **Tool Calling**: ToolCall, ToolResult, MessageContentToolCalls
- **Model Configuration**: Model, ModelData, ClientConfig
- **Session Management**: Session, DataUrlMap
- **Roles and Agents**: Role, Agent, AgentVariables
- **Function Calling**: FunctionDeclaration, Functions
- **RAG Data**: RagData, DocumentData, ChunkData, Vector
- **Input Processing**: Input with context and metadata

#### `formal_spec_system_state.zpp` (16KB, 9 sections)

Formalizes overall system state and global invariants:

- **Configuration State**: Global config with model, clients, functions
- **REPL Mode State**: Interactive session state with history
- **CMD Mode State**: Command-line execution state
- **Serve Mode State**: HTTP server with active requests
- **Session Registry**: All available sessions
- **RAG Registry**: All available RAG databases
- **Complete System State**: Top-level state encompassing all modes
- **Initial System State**: Clean startup state
- **State Observation Functions**: Predicates for querying state

#### `formal_spec_operations.zpp` (26KB, 8 sections)

Formalizes core operations and state transitions:

- **Configuration Operations**: SetModel, UseRole, StartSession, EndSession
- **Input Processing**: CreateInput, UseEmbeddings
- **Chat Completion**: PrepareChatRequest, CallChatCompletion, CallChatCompletionStreaming
- **Function Calling**: EvaluateToolCalls, ExecuteFunction, ContinueWithToolResults
- **Session Management**: AddMessageToSession, CompressSession
- **RAG Operations**: BuildRAG, QueryRAG
- **REPL Operations**: ProcessREPLCommand
- **Complete Workflows**: ExecuteChatWorkflow, ExecuteAgentWorkflow

#### `formal_spec_integrations.zpp` (27KB, 6 sections)

Formalizes external integration contracts:

- **LLM Provider APIs**: ChatCompletionRequest/Response, StreamChunk, LLMProviderContract
- **Provider Adaptations**: OpenAI, Claude, Gemini-specific formats
- **HTTP Server Endpoints**: /v1/chat/completions, /v1/embeddings, /v1/rerank
- **Function Calling Protocol**: MCP message format, MCPClient, FunctionExecutionContract
- **Document Loaders**: DocumentLoaderContract, TextSplitterContract
- **Embedding and Reranking**: EmbeddingAPIContract, RerankAPIContract
- **Error Handling**: APIError, RetryPolicy

## Key Architectural Patterns

### Multi-Mode Operation

The system operates in three distinct modes:

1. **CMD Mode**: One-shot command execution
2. **REPL Mode**: Interactive chat with persistent history
3. **Serve Mode**: HTTP server exposing LLM APIs

### Provider Abstraction

A unified client interface abstracts 20+ LLM providers:

- OpenAI, Claude (Anthropic), Gemini (Google)
- Bedrock (AWS), VertexAI (Google Cloud)
- Azure OpenAI, Cohere
- 18 OpenAI-compatible providers

### RAG Architecture

Hybrid retrieval combining:

- **Vector Search**: HNSW algorithm for semantic similarity
- **Keyword Search**: BM25 for exact term matching
- **Reranking**: Cross-encoder models for relevance scoring

### Function Calling

Two execution modes:

- **Local Execution**: Shell commands and binaries
- **MCP (Model Context Protocol)**: External tool servers

### Session Management

Conversation persistence with:

- Message history with role tracking
- Token counting and compression
- Session serialization to YAML
- Data URL resolution for media

## Formal Methods in Z++

The Z++ specifications use:

- **Schemas**: State and operation definitions
- **Invariants**: Properties that must always hold
- **Pre/Postconditions**: Operation contracts
- **Refinement**: Data abstraction levels
- **Set Theory**: Collections and mappings
- **Predicate Logic**: Constraints and relationships

### Reading Z++ Specifications

Key notation:

- `┌───┐ Schema ├───┤ Variables ├───┤ Predicates └───┘`: Schema definition
- `ΔSchema`: Schema with before/after state
- `ΞSchema`: Schema with unchanged state
- `∀ x : T • P`: Universal quantification
- `∃ x : T • P`: Existential quantification
- `x ↦ y`: Maplet (key-value pair)
- `seq T`: Sequence of T
- `℘ T`: Power set of T

## Use Cases

This documentation supports:

1. **Architecture Review**: Understanding system design and component interactions
2. **Security Audit**: Identifying security boundaries and data flows
3. **Feature Development**: Planning new features with architectural context
4. **Integration Testing**: Defining test cases from formal specifications
5. **Verification**: Checking implementation against formal contracts
6. **Onboarding**: Learning system structure for new developers
7. **Compliance**: Demonstrating architectural rigor for audits

## Technology Stack Summary

- **Language**: Rust (Edition 2021)
- **Async Runtime**: Tokio (multi-threaded)
- **HTTP**: Reqwest (client), Hyper (server)
- **CLI**: Clap, Reedline, Crossterm
- **Serialization**: serde, serde_json, serde_yaml
- **Vector Search**: hnsw_rs
- **Full-Text Search**: bm25
- **Concurrency**: parking_lot, tokio::sync

## Specification Completeness

The formal specifications cover:

- ✅ **Data Layer**: All core data structures (20KB)
- ✅ **System State**: Complete state space (16KB)
- ✅ **Operations**: All major operations (26KB)
- ✅ **Integrations**: External contracts (27KB)
- ✅ **Total Coverage**: ~90KB of formal specifications

### Not Covered

- Rendering logic (markdown, syntax highlighting)
- Terminal UI details (colors, formatting)
- Utility functions (clipboard, shell integration)
- Build and deployment specifics

## Verification Opportunities

The formal specifications enable:

1. **Property Testing**: Generate tests from invariants
2. **Model Checking**: Verify state space properties
3. **Contract Checking**: Validate pre/postconditions at runtime
4. **Refinement Proofs**: Show implementation matches specification
5. **Theorem Proving**: Prove global system properties

## Maintenance

When modifying the codebase:

1. **Check Specifications**: Ensure changes align with formal models
2. **Update Documentation**: Keep architecture diagrams current
3. **Extend Specifications**: Add schemas for new features
4. **Verify Invariants**: Ensure global properties still hold
5. **Review Contracts**: Update external integration contracts

## Related Documentation

- **README.md**: User-facing features and installation
- **Wiki**: Guides for roles, RAG, macros, configuration
- **Code Comments**: Implementation-level documentation
- **API Docs**: `cargo doc` for detailed Rust documentation

## Contributing

When contributing new features:

1. Review relevant architecture documentation
2. Check formal specifications for affected components
3. Design with architectural patterns in mind
4. Update specifications and diagrams as needed
5. Ensure new code respects system invariants

## Questions and Feedback

For questions about the architecture or specifications:

1. Review the architecture overview for high-level understanding
2. Check formal specifications for detailed contracts
3. Examine Mermaid diagrams for visual representations
4. Open an issue for clarification or corrections

---

**Generated**: 2025-11-08  
**AIChat Version**: 0.30.0  
**Specification Lines**: ~2,000 lines of Z++  
**Documentation Pages**: ~100 pages formatted
