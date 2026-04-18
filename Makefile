# Atlas Workspace Makefile
# Primary: Rust/Cargo | Legacy: C++/CMake
.PHONY: all build release editor game workspace test check clippy fmt fmt-check doc shaders fetch-assets package dist clean cpp-build cpp-test cpp-clean help

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

fetch-assets:
	bash Scripts/fetch_novaforge_assets.sh

package: release
	@mkdir -p dist
	@cp target/release/atlas-workspace dist/ 2>/dev/null || true
	@cp target/release/atlas-game      dist/ 2>/dev/null || true
	@if [ -d novaforge-assets ]; then cp -r novaforge-assets dist/; fi
	@echo "Package contents in dist/"

dist: package
	@cd dist && tar czf atlas-workspace-$$(uname -s | tr '[:upper:]' '[:lower:]').tar.gz \
		atlas-workspace atlas-game novaforge-assets 2>/dev/null || true
	@echo "Distribution archive created in dist/"

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
	@echo "Asset targets:  fetch-assets"
	@echo "Release:        package dist"
	@echo "Legacy C++:     cpp-build cpp-test cpp-clean"
