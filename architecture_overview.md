# AIChat: Technical Architecture Documentation

## Executive Summary

AIChat is a comprehensive LLM (Large Language Model) CLI tool written in Rust that provides a unified interface to over 20 LLM providers. The system supports multiple interaction modes (CMD, REPL, Server), RAG (Retrieval-Augmented Generation), function calling, agents, and sessions with context management.

## Technology Stack

### Core Technologies
- **Language**: Rust (Edition 2021)
- **Runtime**: Tokio (async multi-threaded)
- **HTTP Client**: Reqwest with rustls-tls
- **HTTP Server**: Hyper 1.0 with hyper-util
- **Terminal UI**: Reedline REPL with Crossterm
- **Serialization**: Serde (JSON/YAML)
- **Configuration**: YAML-based configuration files

### Key Dependencies
- **AI/ML**: Vector similarity search (HNSW), BM25 ranking, embeddings
- **Processing**: Async/await with futures-util, tokio-stream
- **Data**: IndexMap for ordered maps, parking_lot for RwLock
- **Parsing**: Fancy-regex for pattern matching, scraper for HTML
- **Security**: SHA2 for hashing, HMAC for authentication

## System Architecture

### High-Level Component Diagram

```mermaid
graph TB
    subgraph "User Interface Layer"
        CLI[CLI Parser]
        REPL[REPL Mode]
        CMD[CMD Mode]
    end
    
    subgraph "Core Application Layer"
        CONFIG[Configuration Manager]
        SESSION[Session Manager]
        ROLE[Role Manager]
        AGENT[Agent Manager]
        MACRO[Macro Manager]
    end
    
    subgraph "Processing Layer"
        FUNCTION[Function Calling]
        RAG[RAG System]
        RENDER[Markdown Renderer]
        INPUT[Input Processor]
    end
    
    subgraph "Client Layer"
        MODEL[Model Manager]
        OPENAI[OpenAI Client]
        CLAUDE[Claude Client]
        GEMINI[Gemini Client]
        AZURE[Azure OpenAI Client]
        VERTEXAI[VertexAI Client]
        BEDROCK[Bedrock Client]
        COMPATIBLE[OpenAI Compatible Clients]
    end
    
    subgraph "Server Layer"
        SERVER[HTTP Server]
        PLAYGROUND[LLM Playground]
        ARENA[LLM Arena]
    end
    
    subgraph "External Services"
        LLM1[OpenAI API]
        LLM2[Anthropic API]
        LLM3[Google AI API]
        LLM4[Other LLM APIs]
        FS[File System]
        WEB[Web URLs]
    end
    
    CLI --> CONFIG
    REPL --> CONFIG
    CMD --> CONFIG
    
    CONFIG --> SESSION
    CONFIG --> ROLE
    CONFIG --> AGENT
    CONFIG --> MACRO
    
    SESSION --> INPUT
    ROLE --> INPUT
    AGENT --> INPUT
    
    INPUT --> FUNCTION
    INPUT --> RAG
    INPUT --> RENDER
    
    FUNCTION --> MODEL
    RAG --> MODEL
    RENDER --> MODEL
    
    MODEL --> OPENAI
    MODEL --> CLAUDE
    MODEL --> GEMINI
    MODEL --> AZURE
    MODEL --> VERTEXAI
    MODEL --> BEDROCK
    MODEL --> COMPATIBLE
    
    OPENAI --> LLM1
    CLAUDE --> LLM2
    GEMINI --> LLM3
    COMPATIBLE --> LLM4
    AZURE --> LLM1
    VERTEXAI --> LLM3
    BEDROCK --> LLM2
    
    SERVER --> MODEL
    PLAYGROUND --> SERVER
    ARENA --> SERVER
    
    RAG --> FS
    INPUT --> FS
    INPUT --> WEB
    CONFIG --> FS
```

### Data Flow Architecture

```mermaid
flowchart LR
    subgraph "Input Sources"
        STDIN[Standard Input]
        FILES[Local Files]
        URLS[Remote URLs]
        CMD_ARGS[CLI Arguments]
        REPL_IN[REPL Input]
    end
    
    subgraph "Input Processing"
        PARSER[Input Parser]
        LOADER[Content Loader]
        VALIDATOR[Input Validator]
    end
    
    subgraph "Context Building"
        ROLE_CTX[Role Context]
        SESSION_CTX[Session Context]
        RAG_CTX[RAG Context]
        AGENT_CTX[Agent Context]
    end
    
    subgraph "Message Construction"
        MSG_BUILD[Message Builder]
        TOKEN_COUNT[Token Counter]
        COMPRESS[Message Compressor]
    end
    
    subgraph "LLM Interaction"
        CLIENT_SEL[Client Selector]
        REQ_BUILD[Request Builder]
        API_CALL[API Call]
        STREAM_PROC[Stream Processor]
    end
    
    subgraph "Response Processing"
        RESP_PARSE[Response Parser]
        TOOL_EXEC[Tool Execution]
        RAG_PROC[RAG Processing]
        RENDER_OUT[Output Renderer]
    end
    
    subgraph "Output Destinations"
        STDOUT[Standard Output]
        SESSION_SAVE[Session Storage]
        HISTORY[History File]
    end
    
    STDIN --> PARSER
    FILES --> LOADER
    URLS --> LOADER
    CMD_ARGS --> PARSER
    REPL_IN --> PARSER
    
    PARSER --> VALIDATOR
    LOADER --> VALIDATOR
    
    VALIDATOR --> ROLE_CTX
    VALIDATOR --> SESSION_CTX
    VALIDATOR --> RAG_CTX
    VALIDATOR --> AGENT_CTX
    
    ROLE_CTX --> MSG_BUILD
    SESSION_CTX --> MSG_BUILD
    RAG_CTX --> MSG_BUILD
    AGENT_CTX --> MSG_BUILD
    
    MSG_BUILD --> TOKEN_COUNT
    TOKEN_COUNT --> COMPRESS
    COMPRESS --> CLIENT_SEL
    
    CLIENT_SEL --> REQ_BUILD
    REQ_BUILD --> API_CALL
    API_CALL --> STREAM_PROC
    
    STREAM_PROC --> RESP_PARSE
    RESP_PARSE --> TOOL_EXEC
    RESP_PARSE --> RAG_PROC
    TOOL_EXEC --> RENDER_OUT
    RAG_PROC --> RENDER_OUT
    
    RENDER_OUT --> STDOUT
    RENDER_OUT --> SESSION_SAVE
    RENDER_OUT --> HISTORY
```

### Component Interaction Diagram

```mermaid
sequenceDiagram
    participant User
    participant CLI/REPL
    participant Config
    participant Session
    participant RAG
    participant Function
    participant Client
    participant LLM_API
    
    User->>CLI/REPL: Input Query
    CLI/REPL->>Config: Load Configuration
    Config-->>CLI/REPL: Configuration Loaded
    
    CLI/REPL->>Session: Get/Create Session
    Session-->>CLI/REPL: Session Context
    
    opt RAG Enabled
        CLI/REPL->>RAG: Query Documents
        RAG-->>CLI/REPL: Retrieved Context
    end
    
    CLI/REPL->>Client: Build Request
    Client->>Client: Prepare Messages
    Client->>Client: Apply Role/Agent
    Client->>LLM_API: Send Request
    
    alt Streaming Response
        loop Stream Chunks
            LLM_API-->>Client: Response Chunk
            Client-->>CLI/REPL: Display Chunk
        end
    else Non-Streaming
        LLM_API-->>Client: Complete Response
        Client-->>CLI/REPL: Display Response
    end
    
    opt Function Calling
        Client->>Function: Execute Tool Calls
        Function-->>Client: Tool Results
        Client->>LLM_API: Send Tool Results
        LLM_API-->>Client: Final Response
    end
    
    CLI/REPL->>Session: Save Message History
    Session-->>User: Display Final Output
```

### State Management Architecture

```mermaid
stateDiagram-v2
    [*] --> Initialization
    
    Initialization --> ConfigLoading
    ConfigLoading --> ModeSelection
    
    ModeSelection --> CMD_Mode: CLI Arguments
    ModeSelection --> REPL_Mode: Interactive
    ModeSelection --> Server_Mode: --serve flag
    
    state CMD_Mode {
        [*] --> ParseInput
        ParseInput --> LoadContext
        LoadContext --> ExecuteQuery
        ExecuteQuery --> DisplayResult
        DisplayResult --> [*]
    }
    
    state REPL_Mode {
        [*] --> ShowPrompt
        ShowPrompt --> ReadInput
        ReadInput --> ProcessCommand
        ProcessCommand --> ExecuteQuery: Chat Input
        ProcessCommand --> HandleCommand: REPL Command
        HandleCommand --> ShowPrompt
        ExecuteQuery --> DisplayResult
        DisplayResult --> UpdateSession
        UpdateSession --> ShowPrompt
    }
    
    state Server_Mode {
        [*] --> StartServer
        StartServer --> ListenRequests
        ListenRequests --> HandleRequest
        HandleRequest --> RouteEndpoint
        RouteEndpoint --> ChatCompletions: /v1/chat/completions
        RouteEndpoint --> Embeddings: /v1/embeddings
        RouteEndpoint --> Rerank: /v1/rerank
        RouteEndpoint --> Playground: /playground
        RouteEndpoint --> Arena: /arena
        ChatCompletions --> SendResponse
        Embeddings --> SendResponse
        Rerank --> SendResponse
        Playground --> SendResponse
        Arena --> SendResponse
        SendResponse --> ListenRequests
    }
    
    CMD_Mode --> [*]
    REPL_Mode --> [*]: .exit
    Server_Mode --> [*]: Shutdown Signal
```

## Core Architectural Components

### 1. Configuration System

The configuration system manages application-wide settings, client configurations, and user preferences.

**Key Responsibilities:**
- Load and parse YAML configuration files
- Manage multiple LLM client configurations
- Handle environment variables and .env files
- Support dynamic configuration updates
- Manage roles, agents, macros, sessions, and RAGs

**Configuration Hierarchy:**
```
~/.config/aichat/
├── config.yaml          # Main configuration
├── .env                 # Environment variables
├── roles/               # Custom roles
├── agents/              # Agent definitions
│   └── {agent_name}/
│       ├── index.yaml   # Agent definition
│       ├── functions.json
│       └── documents/
├── sessions/            # Saved sessions
├── rags/                # RAG databases
├── macros/              # Custom macros
└── functions/           # Global functions
```

### 2. Session Management

Sessions provide context-aware conversations with message history and compression.

**Key Features:**
- Persistent conversation history
- Automatic message compression when token threshold exceeded
- Session naming and retrieval
- Token counting and management
- Message export/import

**Session Lifecycle:**
```mermaid
stateDiagram-v2
    [*] --> Created
    Created --> Active: First Message
    Active --> Active: Add Messages
    Active --> Compressing: Token Threshold Exceeded
    Compressing --> Active: Compression Complete
    Active --> Saved: User Save
    Saved --> Loaded: User Load
    Loaded --> Active
    Active --> [*]: Session End
```

### 3. Role System

Roles define LLM behavior through prompts and model configurations.

**Built-in Roles:**
- `%shell%` - Shell command generation
- `%explain-shell%` - Shell command explanation
- `%code%` - Code generation
- `%create-title%` - Title generation

**Custom Roles:**
- User-defined prompts with metadata
- Model override per role
- Temperature and top_p settings
- Tool usage configuration

### 4. Agent System

Agents combine instructions, tools, and documents (RAG) for autonomous task execution.

**Agent Components:**
```mermaid
graph LR
    AGENT[Agent] --> INSTRUCTIONS[Instructions/Prompt]
    AGENT --> TOOLS[Function Calling Tools]
    AGENT --> DOCUMENTS[Documents/RAG]
    AGENT --> VARIABLES[Agent Variables]
    AGENT --> MODEL[Model Configuration]
    
    TOOLS --> FUNC_DECL[Function Declarations]
    TOOLS --> FUNC_IMPL[Function Implementations]
    
    DOCUMENTS --> RAG_DB[Vector Database]
    DOCUMENTS --> RAG_RETRIEVAL[Retrieval Logic]
    
    VARIABLES --> SHARED[Shared Variables]
    VARIABLES --> SESSION[Session Variables]
```

### 5. RAG (Retrieval-Augmented Generation)

RAG enables querying external documents to augment LLM responses.

**RAG Pipeline:**
```mermaid
flowchart TB
    subgraph "Document Ingestion"
        DOC[Documents] --> LOAD[Document Loader]
        LOAD --> SPLIT[Text Splitter]
        SPLIT --> CHUNK[Chunks]
    end
    
    subgraph "Indexing"
        CHUNK --> EMBED[Embedding Model]
        EMBED --> VECTOR[Vector Database HNSW]
        CHUNK --> BM25_IDX[BM25 Index]
    end
    
    subgraph "Query Processing"
        QUERY[User Query] --> Q_EMBED[Query Embedding]
        Q_EMBED --> VECTOR_SEARCH[Vector Search]
        QUERY --> BM25_SEARCH[BM25 Search]
    end
    
    subgraph "Retrieval & Ranking"
        VECTOR_SEARCH --> CANDIDATES[Candidate Chunks]
        BM25_SEARCH --> CANDIDATES
        CANDIDATES --> RERANK[Reranker Model]
        RERANK --> TOP_K[Top-K Results]
    end
    
    subgraph "Context Building"
        TOP_K --> TEMPLATE[RAG Template]
        QUERY --> TEMPLATE
        TEMPLATE --> CONTEXT[Augmented Prompt]
    end
    
    CONTEXT --> LLM[LLM Query]
```

**RAG Configuration:**
- Embedding model selection
- Reranker model (optional)
- Top-K retrieval count
- Chunk size and overlap
- Custom RAG template

### 6. Function Calling

Function calling extends LLM capabilities with external tools and integrations.

**Function Call Flow:**
```mermaid
sequenceDiagram
    participant LLM
    participant System
    participant Function
    participant External
    
    LLM->>System: Response with Tool Calls
    System->>System: Parse Tool Calls
    
    loop For Each Tool Call
        System->>Function: Execute Function
        Function->>External: Call External Tool/API
        External-->>Function: Result
        Function-->>System: Tool Result
    end
    
    System->>System: Build Tool Results Message
    System->>LLM: Send Tool Results
    LLM-->>System: Final Response
```

**Function Types:**
- Built-in system functions
- User-defined functions
- MCP (Model Context Protocol) integrations
- Agent-specific functions

### 7. Client Abstraction Layer

Unified interface for multiple LLM providers with provider-specific implementations.

**Supported Providers:**
```mermaid
graph TB
    CLIENT[Client Interface] --> OPENAI[OpenAI]
    CLIENT --> COMPATIBLE[OpenAI Compatible]
    CLIENT --> CLAUDE[Claude/Anthropic]
    CLIENT --> GEMINI[Gemini/Google AI]
    CLIENT --> AZURE[Azure OpenAI]
    CLIENT --> VERTEXAI[VertexAI]
    CLIENT --> BEDROCK[AWS Bedrock]
    
    COMPATIBLE --> AI21[AI21]
    COMPATIBLE --> CLOUDFLARE[Cloudflare]
    COMPATIBLE --> DEEPINFRA[DeepInfra]
    COMPATIBLE --> DEEPSEEK[DeepSeek]
    COMPATIBLE --> GROQ[Groq]
    COMPATIBLE --> MISTRAL[Mistral]
    COMPATIBLE --> OPENROUTER[OpenRouter]
    COMPATIBLE --> XAI[xAI Grok]
    COMPATIBLE --> OTHERS[18+ more providers]
```

**Client Capabilities:**
- Chat completions (streaming/non-streaming)
- Embeddings
- Reranking
- Vision support
- Function calling support
- Token counting
- Rate limiting

### 8. HTTP Server

Built-in HTTP server for API proxy and web interfaces.

**Server Endpoints:**
```
GET/POST /v1/chat/completions  - OpenAI-compatible chat API
POST     /v1/embeddings        - Embeddings API
POST     /v1/rerank            - Rerank API
GET      /playground           - Interactive LLM playground
GET      /arena               - LLM comparison arena
```

**Server Features:**
- OpenAI API compatibility
- Model selection and proxying
- Streaming support (SSE)
- Web-based UIs (Playground, Arena)
- CORS support
- Graceful shutdown

## Data Model Overview

### Core Entities

1. **Config**: Global application configuration
2. **Model**: LLM model metadata and capabilities
3. **Message**: Chat message with role and content
4. **Session**: Conversation context with message history
5. **Role**: LLM behavior template with prompt
6. **Agent**: Autonomous assistant with tools and documents
7. **RAG**: Document retrieval system
8. **Function**: Callable tool definition

### Entity Relationships

```mermaid
erDiagram
    CONFIG ||--o{ CLIENT_CONFIG : contains
    CONFIG ||--o{ ROLE : manages
    CONFIG ||--o{ AGENT : manages
    CONFIG ||--o{ SESSION : manages
    CONFIG ||--o{ RAG : manages
    CONFIG ||--o{ MACRO : manages
    
    CLIENT_CONFIG ||--o{ MODEL : provides
    
    SESSION ||--o{ MESSAGE : contains
    SESSION }o--|| ROLE : uses
    SESSION }o--o| AGENT : uses
    SESSION }o--o| RAG : uses
    
    AGENT ||--o{ FUNCTION : has
    AGENT }o--o| RAG : has
    AGENT ||--o{ AGENT_VARIABLE : has
    
    MESSAGE ||--o{ MESSAGE_CONTENT : contains
    MESSAGE_CONTENT ||--o{ TOOL_CALL : may_contain
    
    RAG ||--o{ DOCUMENT : indexes
    DOCUMENT ||--o{ CHUNK : split_into
    CHUNK ||--|| VECTOR : has
    
    FUNCTION ||--o{ TOOL_CALL : invoked_as
    TOOL_CALL ||--|| TOOL_RESULT : produces
```

## Integration Boundaries

### External Service Integrations

```mermaid
graph TB
    subgraph "AIChat System"
        CORE[Core System]
    end
    
    subgraph "LLM Providers"
        CORE -->|HTTPS/JSON| OPENAI_API[OpenAI API]
        CORE -->|HTTPS/JSON| CLAUDE_API[Claude API]
        CORE -->|HTTPS/JSON| GEMINI_API[Gemini API]
        CORE -->|HTTPS/JSON| AZURE_API[Azure OpenAI]
        CORE -->|HTTPS/AWS SDK| BEDROCK_API[AWS Bedrock]
        CORE -->|HTTPS/GCP SDK| VERTEX_API[VertexAI]
    end
    
    subgraph "Document Sources"
        CORE -->|File I/O| FS[File System]
        CORE -->|HTTP GET| WEB[Web URLs]
        CORE -->|Scraping| HTML[HTML Content]
    end
    
    subgraph "Function Integrations"
        CORE -->|Exec| SCRIPTS[Shell Scripts]
        CORE -->|IPC| MCP[MCP Servers]
        CORE -->|HTTP| EXTERNAL_API[External APIs]
    end
    
    subgraph "User Interfaces"
        TERMINAL[Terminal] -->|Stdin/Stdout| CORE
        BROWSER[Web Browser] -->|HTTP| CORE
    end
```

### API Contract Patterns

**Chat Completion Request:**
- Standard OpenAI format for most providers
- Provider-specific transformations
- Streaming via Server-Sent Events (SSE)
- Function calling support varies by provider

**Embedding Request:**
- Input text or array of texts
- Model selection
- Dimensions configuration (provider-specific)

**Rerank Request:**
- Query and document list
- Top-N selection
- Relevance scoring

## Security Considerations

### Authentication & Authorization
- API keys stored in configuration or environment variables
- Per-client authentication configuration
- No built-in multi-user authentication (single-user CLI)

### Data Privacy
- Local session storage
- No telemetry or external tracking
- User controls data retention

### Input Validation
- Shell command sanitization
- Path traversal protection
- URL validation
- Function call parameter validation

## Performance Characteristics

### Concurrency Model
- Tokio async runtime with multi-threaded executor
- Non-blocking I/O for HTTP requests
- Streaming responses for low latency
- Parallel document processing in RAG

### Resource Management
- Token counting for cost control
- Message compression for context window management
- Lazy loading of configurations
- Caching of embeddings and RAG indices

### Scalability
- Single-user CLI design
- Server mode supports concurrent requests
- RAG scales with document count
- Memory usage proportional to session size

## Operational Characteristics

### Configuration Management
- YAML-based configuration
- Environment variable overrides
- Hot-reload not supported (restart required)
- Migration support for config updates

### Logging & Monitoring
- Simple logger with configurable levels
- Request/response logging (debug mode)
- Error reporting to stderr
- Dry-run mode for testing

### Error Handling
- Graceful degradation
- User-friendly error messages
- Abort signal for long-running operations
- Retry logic for transient failures

## Extension Points

### Adding New LLM Providers
1. Implement client trait
2. Register in client module
3. Add configuration schema
4. Implement message transformation

### Custom Functions
1. Define function declaration (JSON)
2. Implement function handler (shell script, binary, MCP)
3. Place in functions directory
4. Configure in agent or global settings

### Document Loaders
1. Add loader for new file format
2. Implement content extraction
3. Register in document_loaders config
4. Support in RAG pipeline

### Custom Roles/Agents
1. Create role/agent definition file
2. Place in roles/agents directory
3. Configure prompts and variables
4. Optional: Add functions and documents
