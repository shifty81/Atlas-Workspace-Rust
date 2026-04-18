# Atlas Workspace Makefile
# Primary: Rust/Cargo | Legacy: C++/CMake
.PHONY: all build release editor game workspace test check clippy fmt fmt-check doc shaders clean cpp-build cpp-test cpp-clean help

all: build

build:
cargo build --workspace

release:
cargo build --workspace --release

editor:
cargo build -p atlas-editor

game:
cargo build -p atlas-game

workspace:
cargo build -p atlas-workspace

test:
cargo test --workspace

check:
cargo check --workspace

clippy:
cargo clippy --workspace -- -D warnings

fmt:
cargo fmt --all

fmt-check:
cargo fmt --all -- --check

doc:
cargo doc --workspace --no-deps --open

shaders:
bash Scripts/build_shaders.sh

clean:
cargo clean

cpp-build:
@echo "WARNING: Building C++ legacy reference only"
bash legacy-cpp/Scripts/build_cpp_legacy.sh Debug

cpp-test:
@echo "WARNING: Running C++ legacy tests"
bash legacy-cpp/Scripts/build_cpp_legacy.sh Debug --test

cpp-clean:
rm -rf Builds/

help:
@echo "Rust targets:   build release editor game workspace test check clippy fmt fmt-check doc shaders clean"
@echo "Legacy C++:     cpp-build cpp-test cpp-clean"
