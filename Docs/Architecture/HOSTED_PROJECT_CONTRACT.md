# Hosted Project Contract

## Overview

Atlas Workspace hosts projects through the `IGameProjectAdapter` interface.
Projects are logically detachable — they can be loaded and unloaded at runtime
without affecting the workspace core.

## IGameProjectAdapter Interface

```cpp
class IGameProjectAdapter {
    virtual std::string projectId() const = 0;
    virtual std::string projectDisplayName() const = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual std::vector<GameplaySystemPanelDescriptor> panelDescriptors() const = 0;
    virtual std::vector<std::string> contentRoots() const = 0;
    virtual std::vector<std::string> customCommands() const = 0;
};
```

## Project Loading Flow

1. User selects project root (File → Open Project)
2. Workspace detects project type (NovaForge marker file or generic)
3. Appropriate adapter is instantiated
4. `adapter->initialize()` is called
5. `WorkspaceShell::loadProject()` takes ownership
6. `ProjectSystemsTool::loadFromAdapter()` ingests panel descriptors
7. Tools are notified via `notifyProjectLoaded()`

## Panel Hosting

Project panels implement `IEditorPanel` and are created by factory lambdas
stored in `GameplaySystemPanelDescriptor::createPanel`.

## Content Roots

Adapters declare content roots for asset scanning. The workspace asset catalog
populates from these roots during project load.

## Boundary Rules

- No game-specific logic in `Source/Workspace/` or `Source/Editor/`
- Project adapters live in their own module (e.g., `NovaForge/Source/EditorAdapter/`)
- Workspace core treats all adapters identically
