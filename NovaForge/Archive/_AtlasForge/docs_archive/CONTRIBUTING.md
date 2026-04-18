# Contributing to Atlas Game Engine

Thank you for your interest in contributing to the Atlas Game Engine!

## Before You Start

Please read these documents **before** writing any code:

- [`docs/ATLAS_CORE_CONTRACT.md`](docs/ATLAS_CORE_CONTRACT.md) — Non-negotiable engine rules
- [`docs/ATLAS_CONTRIBUTOR_RULES.md`](docs/ATLAS_CONTRIBUTOR_RULES.md) — What you can and cannot do
- [`docs/ATLAS_DETERMINISM_ENFORCEMENT.md`](docs/ATLAS_DETERMINISM_ENFORCEMENT.md) — How rules are enforced

### Key Rules

1. **Determinism first** — All simulation code must be bit-exact reproducible
2. **No third-party UI libraries** — Atlas uses a custom UI stack. ImGui, Nuklear, and all external UI frameworks are permanently banned. Use `UILayoutSolver`, `GUIDSLParser`, `UISceneGraph`, `WidgetDSL`, and `FontBootstrap` instead.
3. **Run the contract scanner** before submitting: `python3 tools/contract_scan.py`

## How to Contribute

### Reporting Issues

- Use the GitHub issue tracker to report bugs or suggest features
- Provide as much detail as possible about the issue
- Include steps to reproduce for bugs

### Pull Requests

1. Fork the repository
2. Create a new branch for your feature or bugfix
3. Make your changes
4. Run `python3 tools/contract_scan.py` to check for violations
5. Run `./build.sh --test` to verify all tests pass
6. Submit a pull request with a clear description of your changes

## Code of Conduct

Please be respectful and constructive in all interactions with the project and community.

## Questions?

Feel free to open an issue for any questions about contributing.
