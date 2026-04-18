# Contributing to Atlas

Thank you for your interest in contributing to Atlas! This document provides guidelines for contributing to the project.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Getting Started](#getting-started)
- [Development Process](#development-process)
- [Style Guidelines](#style-guidelines)
- [Commit Messages](#commit-messages)
- [Pull Request Process](#pull-request-process)

---

## Code of Conduct

This project aims to be welcoming and inclusive. Please be respectful and constructive in all interactions.

---

## How Can I Contribute?

### 🐛 Reporting Bugs

Before creating bug reports, please check the existing issues to avoid duplicates. When creating a bug report, include:

- **Clear title and description**
- **Steps to reproduce** the issue
- **Expected behavior** vs actual behavior
- **Environment details** (OS, compiler version, etc.)
- **Screenshots** if applicable

### 💡 Suggesting Features

Feature suggestions are welcome! Please:

- Check the [roadmap](docs/ROADMAP.md) first
- Search existing issues for similar suggestions
- Provide clear use cases and benefits
- Consider implementation complexity

### 📝 Improving Documentation

Documentation improvements are always appreciated:

- Fix typos or clarify confusing sections
- Add examples or tutorials
- Update outdated information
- Improve organization

### 🎨 Adding Content

The easiest way to contribute is by adding game content:

#### Ships
Add new ship definitions in `data/ships/`:
```json
{
  "ship_id": {
    "name": "Ship Name",
    "class": "Frigate",
    "hull_hp": 500,
    "armor_hp": 400,
    "shield_hp": 600,
    ...
  }
}
```

#### Modules
Add new modules in `data/modules/`:
```json
{
  "module_id": {
    "name": "Module Name",
    "type": "weapon",
    "slot": "high",
    ...
  }
}
```

#### Missions
Add new missions in `data/missions/`:
```json
{
  "mission_id": {
    "name": "Mission Name",
    "level": 1,
    "type": "combat",
    ...
  }
}
```

### 🔧 Code Contributions

See [Priority Areas](#priority-areas) for where help is most needed.

---

## Getting Started

### 1. Fork and Clone

```bash
# Fork the repository on GitHub, then:
git clone https://github.com/YOUR_USERNAME/Atlas-EveOffline.git
cd Atlas-EveOffline
```

### 2. Set Up Development Environment

```bash
# Linux/macOS — install deps and build
sudo apt-get install build-essential cmake libgl1-mesa-dev libglew-dev \
  libglfw3-dev libglm-dev nlohmann-json3-dev libopenal-dev libfreetype-dev
./scripts/build_project.sh

# Windows — see docs/guides/VS2022_SETUP_GUIDE.md
scripts\build_project.bat Release all

# Run tests to verify setup
./scripts/build_project.sh Release test
```

### 3. Create a Branch

```bash
git checkout -b feature/your-feature-name
# or
git checkout -b fix/bug-description
```

---

## Development Process

### 1. Make Your Changes

- Follow the [style guidelines](#style-guidelines)
- Add tests for new functionality
- Update documentation as needed
- Keep changes focused and minimal

### 2. Test Your Changes

```bash
# Build and run all tests
./scripts/build_project.sh Release test

# Build specific targets
./scripts/build_project.sh Release engine     # Engine only
./scripts/build_project.sh Release client     # Client only
./scripts/build_project.sh Release server     # Server only

# Validate project structure
./scripts/build_project.sh Release validate

# Or use Make shortcuts
make build
make test
```

### 3. Update Documentation

- Update relevant docs in `docs/` folder
- Add docstrings to new functions/classes
- Update README.md if needed

---

## Style Guidelines

### C++ Code

Follow the [Atlas Naming Conventions](https://github.com/shifty81/Atlas/blob/main/docs/ATLAS_NAMING_CONVENTIONS.md):

```cpp
// Use PascalCase for classes, types, and components
// Use PascalCase + "System" suffix for ECS systems
// Use snake_case for file names
// Use camelCase for local variables and function parameters
// Use UPPER_SNAKE_CASE for constants and enums

class HealthComponent {
public:
    float currentHP = 100.0f;
    float maxHP = 100.0f;
};

class DamageSystem {
public:
    /// Apply damage to an entity with the given damage type.
    void applyDamage(Entity target, float amount, DamageType type);
};
```

### JSON Data Files

Use consistent formatting:

```json
{
  "entity_id": {
    "name": "Entity Name",
    "property": value,
    "nested": {
      "key": "value"
    }
  }
}
```

- Use 2-space indentation
- Use double quotes
- Keep IDs lowercase with underscores
- Group related properties together

### Documentation

- Use Markdown for all documentation
- Include table of contents for long documents
- Use clear, descriptive headings
- Add code examples where helpful
- Keep line length reasonable (80-100 chars)

---

## Commit Messages

Write clear, descriptive commit messages:

### Format

```
Short summary (50 chars or less)

More detailed explanation if needed. Wrap at 72 characters.
Explain what changed and why, not how.

- Bullet points are okay
- Use present tense: "Add feature" not "Added feature"
```

### Examples

Good:
```
Add manufacturing system with blueprint research

Implements ME/TE research, manufacturing queue, and blueprint
copying mechanics. Includes tests and documentation.
```

Bad:
```
Fixed stuff
```

---

## Pull Request Process

### 1. Before Submitting

- [ ] Tests pass: `./scripts/build_project.sh Release test`
- [ ] Code follows style guidelines
- [ ] Documentation updated
- [ ] Commit messages are clear
- [ ] Branch is up to date with main

### 2. Submit PR

1. Push your branch to GitHub
2. Create Pull Request from your branch to `main`
3. Fill out the PR template:
   - **Title**: Clear, descriptive summary
   - **Description**: What changes and why
   - **Testing**: How you tested the changes
   - **Related Issues**: Link any related issues

### 3. Review Process

- Maintainers will review your PR
- Address any feedback or requested changes
- Keep discussion focused and constructive
- Be patient - reviews take time

### 4. After Merge

- Delete your feature branch
- Pull latest changes from main
- Celebrate! 🎉

---

## Priority Areas

Help is especially welcome in these areas:

### 🔥 High Priority

1. **Engine Migration**
   - StrategyGraph and ConversationGraph module completion
   - AI Signal Registry integration
   - Server Rules system

2. **Vertical Slice Polish**
   - One full star system, playable loop, AI-driven
   - Combat, mining, and economy in a single system
   - Performance profiling and optimization

3. **Testing & Bug Reports**
   - Test existing features
   - Report bugs with reproduction steps
   - Add test coverage for untested areas

### 📊 Medium Priority

1. **UI/UX Improvements**
   - Atlas UI widget enhancements
   - Better visual feedback
   - Improved HUD elements

2. **Additional Content**
   - More ship definitions and variations
   - More modules and fittings
   - New missions and storylines

3. **Performance Optimization**
   - Profiling and optimization
   - Spatial partitioning
   - Interest management

### 📝 Low Priority

1. **Future Phases** (Phase 8+)
   - Cinematic warp effects
   - Fleet AI behaviors
   - Tactical overlay

2. **Additional Content**
   - More NPC varieties
   - More mission types
   - Exploration sites

3. **Community Tools**
   - Mission editor
   - Ship designer
   - Mod manager

---

## Questions?

- **Documentation**: Check [docs/](docs/) folder
- **Issues**: Search or create an issue on GitHub
- **Discussions**: Use GitHub Discussions for questions
- **Roadmap**: See [docs/ROADMAP.md](docs/ROADMAP.md)

---

## Thank You!

Your contributions make EVE OFFLINE better for everyone. Thank you for taking the time to contribute! 🚀

---

*This contributing guide is based on best practices from open source projects.*
