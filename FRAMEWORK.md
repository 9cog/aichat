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
framework AI operations:

```
9P write → aifs.Session → aichat client → LLM API → response → 9P read
```

## See Also

- [9cog Framework Architecture](https://github.com/9cog/9fs9rc/blob/main/framework/ARCHITECTURE.md)
- [aichat Architecture Overview](architecture_overview.md)
