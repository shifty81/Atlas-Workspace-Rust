# Snippet: Atlas Directory Layout

**Source:** MasterRepoRefactor
**Use Case:** Reference for AI tool directory organization

## Layout Convention

All AI tools live under `AtlasAI/` with the `Atlas_` prefix:

```
AtlasAI/
├── README.md                    # Broker overview
├── Atlas_Arbiter/
│   └── README.md                # Rule-based decision engine
└── Atlas_SwissAgent/
    └── README.md                # Conversational AI query tool
```

## Naming Rule

- Legacy: `ArbiterAI`, `SwissAgent`, `Arbiter`
- Canonical: `Atlas_Arbiter`, `Atlas_SwissAgent`
- All route through the single `AtlasAI` broker
- See `Docs/Architecture/NAMING_CANON.md` for full naming convention
