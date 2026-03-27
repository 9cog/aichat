# aichat — 9cog Framework Role

## Role: AI Services Engine

aichat is the AI processing engine for the 9cog framework, providing:

- **20+ LLM provider support** (Anthropic, OpenAI, Google, AWS, Azure, Cohere, etc.)
- **RAG pipeline** (embeddings + BM25 + reranking)
- **Function calling** and tool use
- **AI agents** with autonomous task execution
- **Session management** with context compression

## Framework Integration

### 9P Service Mode

When integrated with the 9cog framework, aichat's capabilities are
exposed as 9P files through the AI filesystem:

```
aichat feature          → 9P filesystem path
─────────────────────── → ────────────────────────
Chat (multi-provider)   → /ai/sessions/{id}/ctl
Model selection         → /ai/models/active
RAG queries             → /ai/features/rag
Provider list           → /ai/models/list
Session history         → /ai/sessions/{id}/history
```

### Provider Abstraction

aichat's client abstraction (`src/client/`) is the backend for all
framework AI operations. When a user writes to `/ai/sessions/{id}/ctl`,
the request flows through:

```
9P write → aifs.Session → aichat client → LLM API → response → 9P read
```

### Supported Providers via Framework

| Provider | Models | Via aichat |
|----------|--------|------------|
| Anthropic | Claude Opus/Sonnet/Haiku | `src/client/claude.rs` |
| OpenAI | GPT-4o, GPT-4o-mini | `src/client/openai.rs` |
| Google | Gemini 2.5 Pro/Flash | `src/client/gemini.rs` |
| AWS | Bedrock models | `src/client/bedrock.rs` |
| Azure | Azure OpenAI | `src/client/azure_openai.rs` |
| Cohere | Command R+ | `src/client/cohere.rs` |
| OpenAI-compatible | Ollama, Groq, Mistral, etc. | `src/client/openai_compatible.rs` |

### RAG Integration

aichat's RAG system feeds the knowledge graph:

```
1. Document ingestion → embeddings → vector index
2. Query → BM25 + vector search → reranking → results
3. Results → knowledge graph nodes → /ai/knowledge/concepts/
```

### Key Files for Integration

| File | Purpose |
|------|--------|
| `src/client/mod.rs` | Provider abstraction interface |
| `src/config/mod.rs` | Configuration engine |
| `src/rag/mod.rs` | RAG pipeline |
| `src/serve.rs` | HTTP server (reference for 9P server) |
| `src/config/session.rs` | Session management |
| `models.yaml` | 1000+ model definitions |

## Architecture

```
┌──────────────────────────────────┐
│  /ai/sessions/  /ai/models/     │  9P filesystem
├──────────────────────────────────┤
│  aifs (Go, in 9fs9rc)           │  Framework core
├──────────────────────────────────┤
│  aichat (Rust, this repo)       │  AI engine
│  ├── client/ (20+ providers)    │
│  ├── rag/ (embeddings + BM25)   │
│  ├── config/ (sessions, roles)  │
│  └── serve.rs (HTTP reference)  │
└──────────────────────────────────┘
```

## See Also

- [9cog Framework Architecture](https://github.com/9cog/9fs9rc/blob/main/framework/ARCHITECTURE.md)
- [aichat Architecture Overview](architecture_overview.md)
