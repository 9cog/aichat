# AIChat System Architecture Overview

## Executive Summary

AIChat is an all-in-one LLM (Large Language Model) CLI tool written in Rust that provides a unified interface to interact with 20+ LLM providers. The system supports multiple interaction modes (CMD, REPL, HTTP Server), RAG (Retrieval-Augmented Generation), function calling, agent workflows, and session management.

## System Architecture

### High-Level Architecture

```mermaid
graph TB
    subgraph "User Interface Layer"
        CLI[CLI Arguments]
        REPL[REPL Interactive]
        HTTP[HTTP Client]
    end
    
    subgraph "Core Application Layer"
        Main[Main Controller]
        Config[Configuration Manager]
        SessionMgr[Session Manager]
        RoleMgr[Role Manager]
        AgentMgr[Agent Manager]
    end
    
    subgraph "Business Logic Layer"
        InputProc[Input Processor]
        FuncCall[Function Calling Engine]
        RAGEngine[RAG Engine]
        Renderer[Output Renderer]
    end
    
    subgraph "Client Abstraction Layer"
        ClientFactory[Client Factory]
        ModelRegistry[Model Registry]
        StreamHandler[Stream Handler]
    end
    
    subgraph "Provider Integration Layer"
        OpenAI[OpenAI Client]
        Claude[Claude Client]
        Gemini[Gemini Client]
        Bedrock[Bedrock Client]
        VertexAI[VertexAI Client]
        Compatible[OpenAI Compatible Clients]
        Others[Other Providers...]
    end
    
    subgraph "External Services"
        LLMProviders[20+ LLM Providers]
        FuncTools[External Function Tools]
        MCPServers[MCP Servers]
    end
    
    subgraph "Storage Layer"
        FSConfig[Filesystem Config]
        Sessions[Session Storage]
        RAGData[RAG Databases]
        Functions[Function Definitions]
    end
    
    CLI --> Main
    REPL --> Main
    HTTP --> Main
    
    Main --> Config
    Main --> SessionMgr
    Main --> RoleMgr
    Main --> AgentMgr
    
    Config --> FSConfig
    SessionMgr --> Sessions
    
    Main --> InputProc
    InputProc --> FuncCall
    InputProc --> RAGEngine
    InputProc --> Renderer
    
    FuncCall --> Functions
    RAGEngine --> RAGData
    
    InputProc --> ClientFactory
    ClientFactory --> ModelRegistry
    ClientFactory --> StreamHandler
    
    ClientFactory --> OpenAI
    ClientFactory --> Claude
    ClientFactory --> Gemini
    ClientFactory --> Bedrock
    ClientFactory --> VertexAI
    ClientFactory --> Compatible
    ClientFactory --> Others
    
    OpenAI --> LLMProviders
    Claude --> LLMProviders
    Gemini --> LLMProviders
    Bedrock --> LLMProviders
    VertexAI --> LLMProviders
    Compatible --> LLMProviders
    Others --> LLMProviders
    
    FuncCall --> FuncTools
    FuncCall --> MCPServers
```

## Component Architecture

### 1. Entry Points and Working Modes

```mermaid
stateDiagram-v2
    [*] --> ParseCLI
    ParseCLI --> DetermineMode
    
    DetermineMode --> ServeMode: --serve flag
    DetermineMode --> ReplMode: No text/files
    DetermineMode --> CmdMode: Has text or files
    
    ServeMode --> HTTPServer
    HTTPServer --> HandleRequest
    HandleRequest --> ProcessCompletion
    HandleRequest --> ServeFrontend
    
    ReplMode --> InitREPL
    InitREPL --> ReadCommand
    ReadCommand --> ParseREPLCmd
    ParseREPLCmd --> ExecuteREPL
    ExecuteREPL --> ReadCommand
    ExecuteREPL --> [*]
    
    CmdMode --> ParseInput
    ParseInput --> ExecuteMode: --execute flag
    ParseInput --> ProcessChat: Normal mode
    
    ExecuteMode --> GenerateShell
    GenerateShell --> PromptUser
    PromptUser --> ExecuteShell
    PromptUser --> [*]
    
    ProcessChat --> SendToLLM
    SendToLLM --> [*]
```

### 2. Configuration System

```mermaid
classDiagram
    class Config {
        +WorkingMode working_mode
        +Model model
        +Session session
        +Role role
        +Agent agent
        +Rag rag
        +Vec~ClientConfig~ clients
        +HashMap functions
        +init() Config
        +use_model(id)
        +use_session(name)
        +use_role(name)
    }
    
    class ClientConfig {
        +String name
        +String api_key
        +String api_base
        +Vec~Model~ models
    }
    
    class Model {
        +String client_name
        +String name
        +ModelType model_type
        +Option~usize~ max_input_tokens
        +Option~usize~ max_output_tokens
        +bool supports_vision
        +bool supports_function_calling
    }
    
    class Session {
        +String name
        +String model_id
        +Vec~Message~ messages
        +Vec~Message~ compressed_messages
        +HashMap data_urls
        +bool dirty
    }
    
    class Role {
        +String name
        +String prompt
        +Option~f64~ temperature
        +Option~f64~ top_p
    }
    
    class Agent {
        +String name
        +String instructions
        +Vec~String~ functions
        +Vec~String~ documents
        +AgentVariables variables
    }
    
    class Rag {
        +String name
        +Model embedding_model
        +Hnsw hnsw_index
        +SearchEngine bm25
        +Vec~Document~ documents
    }
    
    Config --> ClientConfig
    Config --> Model
    Config --> Session
    Config --> Role
    Config --> Agent
    Config --> Rag
```

### 3. Message Flow and Processing

```mermaid
sequenceDiagram
    participant User
    participant Main
    participant Config
    participant Input
    participant Client
    participant LLMProvider
    participant Functions
    participant RAG
    
    User->>Main: Submit Input
    Main->>Config: Load Configuration
    Config-->>Main: Config Ready
    
    Main->>Input: Create Input Object
    Input->>RAG: Query RAG (if enabled)
    RAG-->>Input: Contextual Documents
    
    Input->>Input: Build Message Chain
    Input-->>Main: Input Ready
    
    Main->>Client: Create Client for Model
    Client-->>Main: Client Instance
    
    Main->>Client: call_chat_completions()
    Client->>LLMProvider: HTTP Request
    LLMProvider-->>Client: Response/Stream
    
    alt Has Function Calls
        Client-->>Main: Tool Calls
        Main->>Functions: eval_tool_calls()
        Functions->>Functions: Execute External Tools
        Functions-->>Main: Tool Results
        Main->>Input: Merge Tool Results
        Main->>Client: Continue with Results
    end
    
    Client-->>Main: Final Output
    Main->>Config: Update Session
    Config->>Config: Save if needed
    Main->>User: Display Result
```

### 4. Client Architecture

```mermaid
graph LR
    subgraph "Client Interface"
        ClientTrait[Client Trait]
    end
    
    subgraph "Common Components"
        RequestBuilder[Request Builder]
        ResponseParser[Response Parser]
        StreamDecoder[Stream Decoder]
        TokenEstimator[Token Estimator]
    end
    
    subgraph "Provider Implementations"
        OpenAIClient[OpenAI Client]
        ClaudeClient[Claude Client]
        GeminiClient[Gemini Client]
        BedrockClient[Bedrock Client]
        VertexAIClient[VertexAI Client]
        CohereClient[Cohere Client]
        AzureClient[Azure OpenAI Client]
        CompatClient[Compatible Client]
    end
    
    ClientTrait --> OpenAIClient
    ClientTrait --> ClaudeClient
    ClientTrait --> GeminiClient
    ClientTrait --> BedrockClient
    ClientTrait --> VertexAIClient
    ClientTrait --> CohereClient
    ClientTrait --> AzureClient
    ClientTrait --> CompatClient
    
    OpenAIClient --> RequestBuilder
    OpenAIClient --> ResponseParser
    OpenAIClient --> StreamDecoder
    
    ClaudeClient --> RequestBuilder
    ClaudeClient --> ResponseParser
    ClaudeClient --> StreamDecoder
    
    RequestBuilder --> TokenEstimator
```

### 5. RAG System Architecture

```mermaid
graph TB
    subgraph "Document Ingestion"
        DocLoader[Document Loaders]
        Splitter[Text Splitter]
        Chunker[Chunk Generator]
    end
    
    subgraph "Indexing"
        Embedder[Embedding Model]
        HNSWIndex[HNSW Vector Index]
        BM25Index[BM25 Full-Text Index]
    end
    
    subgraph "Retrieval"
        QueryProc[Query Processor]
        VectorSearch[Vector Search]
        KeywordSearch[Keyword Search]
        Reranker[Reranker Model]
        Merger[Result Merger]
    end
    
    subgraph "Storage"
        RagData[RAG Data YAML]
        Vectors[Vector Database]
        Metadata[Document Metadata]
    end
    
    DocLoader --> Splitter
    Splitter --> Chunker
    Chunker --> Embedder
    
    Embedder --> HNSWIndex
    Embedder --> Vectors
    Chunker --> BM25Index
    Chunker --> Metadata
    
    QueryProc --> VectorSearch
    QueryProc --> KeywordSearch
    
    VectorSearch --> HNSWIndex
    KeywordSearch --> BM25Index
    
    VectorSearch --> Merger
    KeywordSearch --> Merger
    Merger --> Reranker
    
    HNSWIndex --> RagData
    BM25Index --> RagData
    Vectors --> RagData
    Metadata --> RagData
```

### 6. Function Calling System

```mermaid
graph TB
    subgraph "Function Definition"
        FuncJSON[functions.json]
        FuncDecl[Function Declaration]
        Schema[Parameter Schema]
    end
    
    subgraph "Execution Engine"
        Parser[Call Parser]
        Validator[Input Validator]
        Executor[Executor]
        BinRunner[Binary Runner]
        ScriptRunner[Script Runner]
        MCPClient[MCP Client]
    end
    
    subgraph "External Integration"
        LocalBin[Local Binaries]
        Scripts[Shell Scripts]
        MCPServers[MCP Servers]
        WebAPIs[Web APIs]
    end
    
    FuncJSON --> FuncDecl
    FuncDecl --> Schema
    
    FuncDecl --> Parser
    Parser --> Validator
    Validator --> Executor
    
    Executor --> BinRunner
    Executor --> ScriptRunner
    Executor --> MCPClient
    
    BinRunner --> LocalBin
    ScriptRunner --> Scripts
    MCPClient --> MCPServers
    ScriptRunner --> WebAPIs
```

### 7. HTTP Server Architecture (Serve Mode)

```mermaid
graph TB
    subgraph "HTTP Server"
        Listener[TCP Listener]
        Router[Route Handler]
        Middleware[Middleware]
    end
    
    subgraph "API Endpoints"
        ChatAPI[/v1/chat/completions]
        EmbedAPI[/v1/embeddings]
        RerankAPI[/v1/rerank]
    end
    
    subgraph "Web Frontends"
        Playground[/playground]
        Arena[/arena]
        Static[Static Assets]
    end
    
    subgraph "Processing"
        RequestValidator[Request Validator]
        ModelSelector[Model Selector]
        StreamProcessor[Stream Processor]
        ResponseBuilder[Response Builder]
    end
    
    Listener --> Router
    Router --> Middleware
    
    Middleware --> ChatAPI
    Middleware --> EmbedAPI
    Middleware --> RerankAPI
    Middleware --> Playground
    Middleware --> Arena
    Middleware --> Static
    
    ChatAPI --> RequestValidator
    EmbedAPI --> RequestValidator
    RerankAPI --> RequestValidator
    
    RequestValidator --> ModelSelector
    ModelSelector --> StreamProcessor
    StreamProcessor --> ResponseBuilder
```

## Data Flow Diagrams

### 1. Chat Completion Flow (Non-Streaming)

```mermaid
sequenceDiagram
    participant User
    participant CLI
    participant Config
    participant Input
    participant Client
    participant Provider
    
    User->>CLI: aichat "message"
    CLI->>Config: Load config
    Config->>Config: Select model
    CLI->>Input: Create input
    
    alt Has RAG
        Input->>Input: Query RAG context
    end
    
    alt Has Session
        Input->>Input: Load message history
    end
    
    Input->>Client: Prepare request
    Client->>Provider: POST /chat/completions
    Provider-->>Client: Response JSON
    Client->>Client: Parse response
    
    alt Has Tool Calls
        Client-->>CLI: Tool calls detected
        CLI->>CLI: Execute tools
        CLI->>Input: Create follow-up input
        Input->>Client: Retry with results
        Client->>Provider: POST with tool results
        Provider-->>Client: Final response
    end
    
    Client-->>CLI: Final output
    
    alt Has Session
        CLI->>Config: Save to session
    end
    
    CLI-->>User: Display result
```

### 2. Chat Completion Flow (Streaming)

```mermaid
sequenceDiagram
    participant User
    participant CLI
    participant Client
    participant Provider
    participant Renderer
    
    User->>CLI: aichat "message" (stream)
    CLI->>Client: call_chat_completions_streaming()
    Client->>Provider: POST /chat/completions (stream: true)
    Provider-->>Client: Open SSE connection
    
    loop Stream chunks
        Provider-->>Client: data: {delta}
        Client->>Client: Parse SSE event
        Client->>Renderer: Render chunk
        Renderer-->>User: Display incremental output
    end
    
    Provider-->>Client: data: [DONE]
    Client->>Client: Close stream
    
    alt Has Tool Calls
        Client-->>CLI: Tool calls in stream
        Note over CLI: Handle recursively
    end
    
    Client-->>CLI: Complete output
    CLI-->>User: Final newline
```

### 3. REPL Command Processing

```mermaid
flowchart TD
    Start([User Input]) --> Parse[Parse Command]
    
    Parse --> IsDot{Starts with '.'}
    IsDot -->|Yes| ParseREPLCmd[Parse REPL Command]
    IsDot -->|No| ChatInput[Treat as Chat Input]
    
    ParseREPLCmd --> CheckCmd{Command Type}
    
    CheckCmd -->|.model| SetModel[Set Model]
    CheckCmd -->|.role| SetRole[Set Role]
    CheckCmd -->|.session| ManageSession[Session Management]
    CheckCmd -->|.agent| StartAgent[Start Agent]
    CheckCmd -->|.rag| UseRAG[Use RAG]
    CheckCmd -->|.file| AddFiles[Add Files]
    CheckCmd -->|.info| ShowInfo[Show Info]
    CheckCmd -->|.help| ShowHelp[Show Help]
    CheckCmd -->|.exit| Exit([Exit REPL])
    
    SetModel --> UpdateConfig[Update Config]
    SetRole --> UpdateConfig
    ManageSession --> UpdateConfig
    StartAgent --> UpdateConfig
    UseRAG --> UpdateConfig
    
    UpdateConfig --> Prompt[Show Prompt]
    AddFiles --> Prompt
    ShowInfo --> Prompt
    ShowHelp --> Prompt
    
    ChatInput --> ProcessInput[Process Chat Input]
    ProcessInput --> SendToLLM[Send to LLM]
    SendToLLM --> DisplayResult[Display Result]
    DisplayResult --> Prompt
    
    Prompt --> Start
```

### 4. Agent Workflow Execution

```mermaid
sequenceDiagram
    participant User
    participant Main
    participant Agent
    participant Functions
    participant Documents
    participant LLM
    
    User->>Main: aichat --agent myagent
    Main->>Agent: Load agent config
    Agent->>Agent: Parse instructions
    Agent->>Agent: Load variables
    
    alt Has Documents
        Agent->>Documents: Load RAG documents
        Documents-->>Agent: Document context
    end
    
    Agent->>Agent: Build system prompt
    Main->>Agent: Get initial input
    
    loop Agent Conversation
        Agent->>LLM: Send with instructions + tools
        LLM-->>Agent: Response + tool calls
        
        alt Has Tool Calls
            Agent->>Functions: Execute functions
            Functions->>Functions: Run external tools
            Functions-->>Agent: Tool results
            Agent->>LLM: Continue with results
        end
    end
    
    Agent-->>Main: Final output
    Main-->>User: Display result
```

## Integration Boundaries

### External LLM Provider Integration

```mermaid
graph LR
    subgraph "AIChat"
        ClientImpl[Client Implementation]
        Auth[Authentication]
        RateLimit[Rate Limiter]
        Retry[Retry Logic]
    end
    
    subgraph "HTTP Layer"
        TLS[TLS Connection]
        Headers[HTTP Headers]
        Body[Request Body]
    end
    
    subgraph "Provider API"
        Endpoint[API Endpoint]
        Validation[Request Validation]
        Processing[LLM Processing]
        Response[Response Generation]
    end
    
    ClientImpl --> Auth
    Auth --> RateLimit
    RateLimit --> Retry
    
    Retry --> TLS
    TLS --> Headers
    Headers --> Body
    
    Body --> Endpoint
    Endpoint --> Validation
    Validation --> Processing
    Processing --> Response
    
    Response --> Body
```

### Function Calling Integration (MCP)

```mermaid
graph TB
    subgraph "AIChat Function Engine"
        FuncDef[Function Definition]
        MCPClient[MCP Client]
        StdIO[STDIO Transport]
    end
    
    subgraph "MCP Server"
        MCPServer[MCP Server Process]
        ToolRegistry[Tool Registry]
        Handler[Request Handler]
    end
    
    subgraph "External Tools"
        FileSystem[File System]
        Database[Database]
        WebService[Web Service]
        SystemCmd[System Commands]
    end
    
    FuncDef --> MCPClient
    MCPClient --> StdIO
    StdIO <--> MCPServer
    
    MCPServer --> ToolRegistry
    ToolRegistry --> Handler
    
    Handler --> FileSystem
    Handler --> Database
    Handler --> WebService
    Handler --> SystemCmd
```

## Technology Stack Summary

### Core Technologies
- **Language**: Rust 2021 Edition
- **Async Runtime**: Tokio (multi-threaded)
- **HTTP Client**: Reqwest (with TLS)
- **HTTP Server**: Hyper + Hyper-util
- **CLI Parsing**: Clap (derive API)
- **REPL**: Reedline + Crossterm

### Data & Serialization
- **Configuration**: YAML (serde_yaml)
- **JSON**: serde_json (with preserve_order)
- **Binary**: bincode
- **Serialization**: serde

### AI/ML Components
- **Vector Search**: hnsw_rs (HNSW algorithm)
- **Full-Text Search**: bm25
- **Embeddings**: Provider APIs
- **Reranking**: Provider APIs

### Concurrency & Synchronization
- **Locks**: parking_lot (RwLock, Mutex)
- **Async Streams**: tokio-stream
- **Graceful Shutdown**: tokio-graceful
- **Channels**: tokio::sync::mpsc

### Utilities
- **Markdown Rendering**: Custom renderer with syntect
- **Syntax Highlighting**: syntect
- **Shell Integration**: shell-words, duct
- **Clipboard**: arboard (platform-specific)
- **HTML Parsing**: scraper, html_to_markdown

## Storage Architecture

### Filesystem Layout

```
~/.config/aichat/
├── config.yaml              # Main configuration
├── .env                     # Environment variables
├── roles/                   # Role definitions
│   ├── role1.md
│   └── role2.yaml
├── sessions/                # Session history
│   ├── session1.yaml
│   └── session2.yaml
├── rags/                    # RAG databases
│   ├── rag1.yaml
│   └── rag2.yaml
├── functions/               # Function definitions
│   ├── functions.json
│   └── bin/                 # Function binaries
├── agents/                  # Agent configurations
│   ├── agent1.yaml
│   └── agent2.yaml
└── macros/                  # Macro definitions
    ├── macro1.sh
    └── macro2.sh
```

### Configuration Data Model

```yaml
# config.yaml
model: openai:gpt-4
temperature: 0.7
top_p: 0.95
save_session: false
stream: true

clients:
  - type: openai
    api_key: sk-...
    models:
      - name: gpt-4
        max_input_tokens: 128000
        max_output_tokens: 4096
        supports_vision: true
        supports_function_calling: true

  - type: claude
    api_key: sk-ant-...
    models:
      - name: claude-3-opus-20240229
        max_input_tokens: 200000
        max_output_tokens: 4096

rag_embedding_model: openai:text-embedding-3-small
rag_reranker_model: cohere:rerank-english-v3.0
rag_top_k: 5
```

### Session Data Model

```yaml
# sessions/example.yaml
model: openai:gpt-4
temperature: 0.7
role_name: assistant
messages:
  - role: system
    content: "You are a helpful assistant."
  - role: user
    content: "Hello, how are you?"
  - role: assistant
    content: "I'm doing well, thank you!"
compressed_messages: []
data_urls: {}
```

### RAG Data Model

```yaml
# rags/codebase.yaml
embedding_model: openai:text-embedding-3-small
chunk_size: 1000
chunk_overlap: 200
reranker_model: cohere:rerank-english-v3.0
top_k: 5
document_paths:
  - "/path/to/docs"
  - "/path/to/code"
documents:
  - id: 0
    path: "/path/to/file.md"
    hash: "abc123..."
    chunks:
      - id: 0
        text: "Document content..."
        start_char: 0
        end_char: 1000
vectors:
  - [0.1, 0.2, 0.3, ...]  # Embeddings
```

## Security Considerations

1. **API Key Management**: 
   - Keys stored in config files with restricted permissions
   - Support for environment variables
   - Never logged or displayed

2. **Network Security**:
   - TLS for all external communications
   - Certificate validation enabled
   - Support for SOCKS proxies

3. **Input Validation**:
   - File path validation and sanitization
   - Command injection prevention in shell execution
   - JSON schema validation for API requests

4. **Function Execution**:
   - Sandboxed execution of external tools
   - Whitelisted function definitions
   - User confirmation for dangerous operations

## Performance Characteristics

1. **Streaming**: Real-time output for better UX
2. **Async I/O**: Non-blocking operations for file and network I/O
3. **Parallel Processing**: RAG document processing uses rayon
4. **Caching**: Model metadata cached in memory
5. **Efficient Search**: HNSW for vector search, BM25 for full-text

## Scalability & Limitations

1. **Single Process**: Designed as single-user CLI tool
2. **Local Storage**: All data stored on local filesystem
3. **Memory**: Session history and RAG indices loaded in memory
4. **Concurrency**: HTTP server handles multiple concurrent requests
5. **Token Limits**: Respects provider token limits, with compression support
