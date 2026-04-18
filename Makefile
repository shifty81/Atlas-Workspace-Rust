# AtlasWorkspace — Development task shortcuts
# Usage: make <target>

LOG_DIR  := Logs
LOG_FILE := $(LOG_DIR)/build.log
TEST_LOG := $(LOG_DIR)/test.log

.PHONY: help configure build build-debug build-release test clean

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'

# ── Configure ─────────────────────────────────────────────────────

configure: ## Configure debug build with tests
	@mkdir -p $(LOG_DIR)
	cmake --preset debug 2>&1 | tee -a $(LOG_FILE)

configure-release: ## Configure release build
	@mkdir -p $(LOG_DIR)
	cmake --preset release 2>&1 | tee -a $(LOG_FILE)

# ── Build ─────────────────────────────────────────────────────────

build: configure ## Build debug (all targets)
	cmake --build --preset debug --parallel 2>&1 | tee -a $(LOG_FILE)

build-release: configure-release ## Build release (all targets)
	cmake --build --preset release --parallel 2>&1 | tee -a $(LOG_FILE)

# ── Test ──────────────────────────────────────────────────────────

test: build ## Run all tests
	ctest --preset debug --output-on-failure 2>&1 | tee $(TEST_LOG)

# ── Clean ─────────────────────────────────────────────────────────

clean: ## Remove all build artifacts
	rm -rf Builds/ build/ out/ dist/
