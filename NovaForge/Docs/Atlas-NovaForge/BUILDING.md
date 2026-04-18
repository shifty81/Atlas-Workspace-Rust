# Building Atlas

This guide covers how to build the Atlas engine from source, including prerequisites,
build script usage, log output, manual CMake builds, and troubleshooting.

---

## Prerequisites

| Requirement | Minimum Version | Notes |
|-------------|-----------------|-------|
| **CMake** | 3.22+ | Build system generator |
| **C++ Compiler** | C++20 support | GCC 13+, Clang 15+, or MSVC 2022+ |
| **Bash** | 4.0+ | Required for `build.sh` (Linux/macOS/WSL) |

### Platform Notes

- **Linux (Ubuntu/Debian):**
  ```bash
  sudo apt update
  sudo apt install build-essential cmake
  ```

- **macOS:**
  ```bash
  xcode-select --install
  brew install cmake
  ```

- **Windows:**
  Use [Visual Studio 2022](https://visualstudio.microsoft.com/) with the
  "Desktop development with C++" workload, or build under WSL/MSYS2 using the
  Linux instructions.

---

## Quick Start

```bash
# Clone the repository
git clone https://github.com/shifty81/Atlas.git
cd Atlas

# Build all targets in Release mode
./build.sh

# Executables are placed in dist/
ls dist/
```

---

## Build Script (`build.sh`)

The build script is the recommended way to build Atlas. It wraps CMake with
sensible defaults and captures all output to a log file.

### Usage

```
./build.sh [options] [targets...]
```

### Targets

| Target | CMake Target | Description |
|--------|-------------|-------------|
| `server` | `AtlasServer` | Headless authoritative server |
| `client` | `AtlasClient` | Player runtime client |
| `editor` | `AtlasEditor` | Developer client (editor + runtime superset) |
| `runtime` | `AtlasRuntime` | Standalone runtime with CLI project loading |
| `tests` | `AtlasTests` | Build and run unit tests |
| `all` | All of the above (excluding tests) | **Default** when no target is specified |

### Options

| Option | Description | Default |
|--------|-------------|---------|
| `-b, --build-type TYPE` | Build type: `Debug`, `Release`, `Development` | `Release` |
| `-j, --jobs N` | Number of parallel build jobs | Auto-detected |
| `-o, --output DIR` | Output directory for executables | `dist/` |
| `-c, --clean` | Clean the build directory before building | Off |
| `-t, --test` | Run tests after building | Off |
| `-h, --help` | Show help message | — |

### Build Types

| Type | CMake Equivalent | Use Case |
|------|-----------------|----------|
| `Release` | `Release` | Production builds, fully optimized |
| `Debug` | `Debug` | Development, full debug symbols, no optimization |
| `Development` | `RelWithDebInfo` | Optimized with debug symbols for profiling |

### Examples

```bash
# Build all targets in Release mode (default)
./build.sh

# Build only the server and client
./build.sh server client

# Debug build of the editor with tests
./build.sh -b Debug editor --test

# Clean rebuild of everything
./build.sh --clean all

# Build with a custom output directory
./build.sh -o ./my-output server

# Build and run all tests
./build.sh tests

# Build all + run tests after
./build.sh --test
```

---

## Build Logs

All build output is automatically captured to `logs/build.log`. This includes:

- CMake configuration output
- Compiler output (warnings, errors)
- Test results (when `--test` is used)
- Timestamps for build start and finish

### Log Location

```
Atlas/
└── logs/
    ├── build.log      ← Build script output (auto-generated)
    └── atlas.log      ← Engine runtime log (generated when running Atlas)
```

### Viewing Build Logs

```bash
# View the full build log
cat logs/build.log

# View only errors
grep -i "error" logs/build.log

# View only warnings
grep -i "warning" logs/build.log

# Follow the log during a build (in another terminal)
tail -f logs/build.log
```

Build logs are appended across runs. To start fresh, delete the log file before
building or use `--clean` which removes the build directory (but not the log).

> **Note:** The `logs/` directory and its contents (except `.gitkeep`) are
> excluded from version control via `.gitignore`.

---

## Runtime Logs

When Atlas executables run, the engine logger writes to `logs/atlas.log`. This
includes timestamped `[INFO]`, `[WARN]`, and `[ERROR]` messages from all engine
subsystems.

```bash
# View runtime log
cat logs/atlas.log

# Watch runtime log live
tail -f logs/atlas.log
```

---

## Manual CMake Build

If you prefer not to use the build script, you can build directly with CMake:

```bash
# Configure
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build all targets
cmake --build . -j$(nproc)

# Or build specific targets
cmake --build . --target AtlasServer -j$(nproc)
cmake --build . --target AtlasEditor -j$(nproc)
```

### Available CMake Targets

- `AtlasServer` — Headless server
- `AtlasClient` — Player client
- `AtlasEditor` — Editor (developer client)
- `AtlasRuntime` — Standalone runtime
- `AtlasTests` — Unit tests

---

## Running Tests

```bash
# Via build script (builds and runs tests)
./build.sh tests

# Or build everything then run tests
./build.sh --test

# Via CMake/CTest
cd build
ctest

# Or run the test binary directly
./build/tests/AtlasTests
```

Test results are printed to the console and captured in `logs/build.log` when
using the build script.

---

## Running Executables

After building, executables are in `dist/` (or your custom output directory):

```bash
# Editor / developer client (authoring + testing)
./dist/AtlasEditor

# Runtime (standalone with project loading)
./dist/AtlasRuntime --project projects/atlas-sample/sample.atlas

# Client (player runtime)
./dist/AtlasClient

# Server (headless)
./dist/AtlasServer
```

---

## Directory Structure (Build-Related)

```
Atlas/
├── build.sh             # Build script (entry point)
├── CMakeLists.txt       # Root CMake configuration
├── build/               # CMake build directory (generated, gitignored)
├── dist/                # Output executables (generated, gitignored)
├── logs/                # Build and runtime logs (generated, gitignored)
│   ├── .gitkeep         # Keeps the directory in version control
│   ├── build.log        # Build script output
│   └── atlas.log        # Engine runtime log
├── engine/              # Engine source (static library)
├── editor/              # Editor application source
├── runtime/             # Runtime application source
├── client/              # Client application source
├── server/              # Server application source
└── tests/               # Unit tests
```

---

## Troubleshooting

### CMake version too old

```
CMake Error at CMakeLists.txt:1:
  CMake 3.22 or higher is required. You are running version X.XX
```

**Fix:** Update CMake. On Ubuntu: `sudo apt install cmake` or install from
[cmake.org](https://cmake.org/download/).

### Compiler does not support C++20

```
error: 'std::filesystem' has not been declared
```

**Fix:** Ensure you have GCC 13+, Clang 15+, or MSVC 2022+. On Ubuntu:
```bash
sudo apt install g++-13
export CXX=g++-13
```

### Permission denied on `build.sh`

```bash
chmod +x build.sh
```

### Build fails with missing dependencies

Atlas has no external library dependencies beyond the C++ standard library and
CMake. If you encounter missing header errors, ensure your compiler and standard
library are up to date.

### Log file not created

The `logs/` directory must exist. The build script creates it automatically, but
if building manually with CMake, create it before running an Atlas executable:

```bash
mkdir -p logs
```

### Platform-Specific Notes

#### Linux (Ubuntu / Debian)

- **Minimum GCC version:** GCC 13+ is recommended for full C++20 support.
  ```bash
  sudo apt update
  sudo apt install g++-13 cmake
  export CXX=g++-13
  ```
- **Ninja (optional):** The build script auto-detects Ninja if installed. Ninja
  provides faster incremental builds than Make.
  ```bash
  sudo apt install ninja-build
  ```

#### macOS

- **Xcode Command Line Tools** are required:
  ```bash
  xcode-select --install
  ```
- **Homebrew CMake** is recommended over the Xcode-bundled version:
  ```bash
  brew install cmake
  ```
- **Apple Silicon (M-series):** Builds natively as ARM64. No Rosetta required.
  If you see `arch` warnings, ensure you are using native Homebrew
  (`/opt/homebrew/bin/cmake`), not the x86 variant.
- **`std::filesystem` errors on older macOS:** Ensure deployment target is
  macOS 10.15+ (`-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15`).

#### Windows

- **Visual Studio 2022** with the "Desktop development with C++" workload is
  the easiest path. Open a Developer Command Prompt and run:
  ```cmd
  cmake -G "Visual Studio 17 2022" -S . -B build
  cmake --build build --config Release
  ```
- **WSL / MSYS2:** Follow the Linux instructions inside the WSL or MSYS2
  environment. The build scripts are Bash-based and are not tested with
  PowerShell.
- **`build.sh` not recognized:** On native Windows without WSL, use CMake
  directly instead of the Bash build script.

### Incremental Rebuilds

After the initial build, only changed source files are recompiled:

```bash
# Rebuild only modified files (fast)
./build.sh

# Force clean rebuild (slow, use when switching branches or build types)
./build.sh --clean
```

If you see stale build artifacts after switching between `Debug` and `Release`,
run a clean build to ensure the CMake cache is regenerated.
