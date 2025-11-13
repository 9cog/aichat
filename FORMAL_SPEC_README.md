# AIChat Formal Specification and Architecture Documentation

## Overview

This directory contains comprehensive technical architecture documentation and formal Z++ specifications for the AIChat system. These documents provide a rigorous, evidence-based model of the system's behavior, architecture, and contracts.

## Document Structure

### 1. Architecture Documentation

**File:** `architecture_overview.md`

A comprehensive technical architecture document featuring:
- Executive summary and technology stack analysis
- High-level component diagrams using Mermaid
- Data flow architecture diagrams
- Component interaction sequence diagrams
- State management architecture
- Core architectural components detailed breakdown
- Integration boundaries and external service contracts
- Security, performance, and operational characteristics

**Purpose:** Provides a human-readable architectural overview with visual diagrams to understand the system structure and design decisions.

### 2. Z++ Formal Specifications

The formal specifications are organized into four modular files:

#### a) `data_model.zpp` - Data Layer Formalization

**Contains:**
- Basic types (String, Float64, Option, etc.)
- Enumeration types (WorkingMode, MessageRole, ModelType, etc.)
- Entity schemas for all data models:
  - Model and ModelData
  - Message and MessageContent
  - Config and ClientConfig
  - Role
  - Session
  - Agent and AgentDefinition
  - Function declarations and tool calls
  - RAG (Retrieval-Augmented Generation) components
  - Macro definitions
  - Input types
- Data integrity invariants
- Relationships between entities

**Key Invariants:**
- Unique naming constraints
- Token limit validations
- Configuration consistency
- Referential integrity between models

#### b) `system_state.zpp` - System State Schemas

**Contains:**
- ApplicationState (top-level state schema)
- Mode-specific states:
  - ReplState (REPL mode)
  - CmdState (command-line mode)
  - ServerState (HTTP server mode)
- Session lifecycle states
- Message processing pipeline states
- State flags and assertions
- Global invariants

**Key Properties:**
- Single active context (session XOR agent)
- State consistency validation
- Token tracking and compression triggers
- Server connection management

#### c) `operations.zpp` - Operation Specifications

**Contains:**
- Configuration operations (init, update, sync)
- Model operations (select, list)
- Session operations (create, load, save, switch, delete, compress)
- Role operations (set, clear, list)
- Agent operations (activate, deactivate, update variables)
- Message processing operations (process input, stream response, execute tool calls)
- RAG operations (init, query, build context)
- Function calling operations (register, execute)
- REPL operations (execute command, handle multiline)
- Server operations (handle chat completion, embeddings, rerank)
- Abort operations

**Pre/Post-conditions:**
- Each operation specifies preconditions that must hold before execution
- Post-conditions describe the resulting state changes
- State transitions preserve global invariants

#### d) `integrations.zpp` - External Integration Contracts

**Contains:**
- HTTP protocol specifications
- LLM provider API contracts:
  - OpenAI API
  - Claude (Anthropic) API
  - Gemini (Google AI) API
  - Azure OpenAI API
  - AWS Bedrock API
- Streaming protocol (Server-Sent Events)
- File system operations
- Document loading interfaces
- Web scraping operations
- Function execution integrations (shell, MCP)
- Rate limiting specifications
- Error handling and retry logic
- Authentication and security contracts

**Security Properties:**
- Authentication requirement for all API calls
- Path traversal prevention
- Command injection prevention
- Rate limit enforcement
- Bounded retry attempts

## Z++ Notation Guide

### Basic Constructs

- `[SchemaName]` - Defines a schema (structured type)
- `Type ::= Constructor1 | Constructor2` - Defines an algebraic data type
- `where` - Introduces constraints/invariants
- `let ... in` - Local variable binding

### Logical Operators

- `∧` - Logical AND
- `∨` - Logical OR
- `⇒` - Logical implication
- `¬` - Logical negation
- `∀` - Universal quantifier (for all)
- `∃` - Existential quantifier (there exists)

### Set/Sequence Operators

- `seq T` - Sequence of type T
- `T ⇸ U` - Partial function from T to U (map)
- `#s` - Size/length of sequence or set s
- `∈` - Element membership
- `⊕` - Map override
- `++` - Sequence concatenation
- `ran` - Range of a function/map
- `dom` - Domain of a function/map

### Mathematical Operators

- `ℤ` - Integer type
- `ℝ` - Real number type
- `𝔹` - Boolean type
- `⊥` - Undefined/bottom value
- `Σ` - Summation
- `⌈x⌉` - Ceiling function
- `⌊x⌋` - Floor function

### State Change Notation

- `Ξ` prefix - Read-only operation (no state change)
- `Δ` prefix - State-modifying operation
- `'` suffix - After-state (e.g., `x'` is the value of x after the operation)

### Special Constructs

- `Option[T]` - Optional value (Some or None)
- `the x : S | P` - The unique x in set S satisfying predicate P

## How to Use These Specifications

### For System Understanding

1. **Start with `architecture_overview.md`** to get a high-level understanding of the system
2. **Review the Mermaid diagrams** to visualize component interactions
3. **Read `data_model.zpp`** to understand the core data structures
4. **Study `system_state.zpp`** to see how state is managed
5. **Examine `operations.zpp`** to understand system behaviors
6. **Review `integrations.zpp`** to see external API contracts

### For Implementation

1. **Data models** in `data_model.zpp` map to Rust structs in `src/config/` and `src/client/`
2. **Operations** in `operations.zpp` map to functions in various modules
3. **Integration contracts** in `integrations.zpp` guide API client implementations

### For Verification

1. **Invariants** can be used to write property-based tests
2. **Pre/post-conditions** can be checked with assertions
3. **State transitions** can be validated in integration tests
4. **Security properties** should be enforced through code reviews

### For Documentation

1. Reference specific schemas when documenting code
2. Use invariants to explain validation logic
3. Cite operation specifications when describing behavior
4. Link to diagrams in design documents

## Mapping to Rust Implementation

### Data Models

| Z++ Schema | Rust Implementation |
|------------|-------------------|
| `Config` | `src/config/mod.rs::Config` |
| `Model` | `src/client/model.rs::Model` |
| `Message` | `src/client/message.rs::Message` |
| `Session` | `src/config/session.rs::Session` |
| `Role` | `src/config/role.rs::Role` |
| `Agent` | `src/config/agent.rs::Agent` |
| `Rag` | `src/rag/mod.rs::Rag` |
| `Functions` | `src/function.rs::Functions` |

### Operations

| Z++ Operation | Rust Implementation |
|---------------|-------------------|
| `InitConfig` | `src/config/mod.rs::Config::init()` |
| `CreateSession` | `src/config/session.rs::Session::new()` |
| `ProcessInput` | `src/main.rs::run()` |
| `ExecuteToolCalls` | `src/function.rs::eval_tool_calls()` |
| `QueryRag` | `src/rag/mod.rs::Rag::search()` |

### Integration Contracts

| Z++ Contract | Rust Implementation |
|--------------|-------------------|
| `OpenAIChatCompletion` | `src/client/openai.rs` |
| `ClaudeMessages` | `src/client/claude.rs` |
| `GeminiGenerateContent` | `src/client/gemini.rs` |
| `StreamChatCompletion` | `src/client/stream.rs` |

## Validation and Consistency

### Invariant Checking

The specifications define numerous invariants that should hold at all times:

1. **Data Integrity**: All references (model IDs, session names, etc.) must be valid
2. **Token Limits**: Session tokens should not exceed 2x the model's maximum
3. **Unique Names**: No duplicate names within each collection
4. **Type Safety**: All optional values are properly handled
5. **State Consistency**: Active objects must exist in their respective collections

### Safety Properties

Key safety properties defined in the specifications:

1. **State Safety**: System never enters invalid state after any operation
2. **Authentication Safety**: All external API calls are authenticated
3. **Path Safety**: No path traversal attacks possible
4. **Command Safety**: Shell commands are sanitized
5. **Rate Limit Safety**: Rate limits are always respected

## Contributing

When modifying the AIChat codebase:

1. **Review relevant specifications** before making changes
2. **Update specifications** if behavior changes
3. **Add new schemas** for new data types
4. **Document new operations** with pre/post-conditions
5. **Maintain invariants** - don't break existing constraints
6. **Update diagrams** if architecture changes

## Tools and Validation

While Z++ is a formal notation, these specifications are designed to be:

- **Human-readable**: Clear natural language descriptions accompany all schemas
- **Implementation-guiding**: Direct mapping to Rust code
- **Verification-ready**: Can be converted to property-based tests
- **Documentation-grade**: Suitable for system documentation

## References

### Formal Methods

- Z Notation: ISO/IEC 13568:2002
- Z++: Extension of Z for object-oriented systems
- Formal specification best practices

### Architecture Documentation

- C4 Model for software architecture
- Mermaid diagram syntax
- Technical architecture documentation patterns

### AIChat Codebase

- Main repository: https://github.com/sigoden/aichat
- Documentation: https://github.com/sigoden/aichat/wiki
- Configuration guide: `config.example.yaml`

## License

These specifications are provided as documentation for the AIChat project and are subject to the same license as the main codebase (MIT OR Apache-2.0).

---

**Note**: While these formal specifications provide a rigorous model of the system, they are documentation and design artifacts. The actual Rust implementation is the authoritative source of truth for system behavior. These specifications should be kept synchronized with the code through regular reviews and updates.
