# Credits & Attribution

This document lists all upstream projects, contributors, and third-party resources used by the Atlas Workspace and the NovaForge game project.

---

## Veloren

**Nova-Forge** is a fork of [Veloren](https://veloren.net), an open-world voxel RPG written in Rust and developed by a large open-source community.

- **Project home**: <https://veloren.net>
- **Source repository**: <https://gitlab.com/veloren/veloren>
- **License**: GNU General Public License v3.0

Nova-Forge, and by extension any code or assets ported from it into this workspace (`novaforge-game/`, `novaforge-assets/`), inherits the GPLv3 license. Full license text: [`LICENSES/GPL-3.0`](LICENSES/GPL-3.0).

### Veloren contributors

The `novaforge-game` crate exists because of the extraordinary work of the Veloren community:

- **Core developers** — the engineers who built the voxel engine, ECS (specs), networking, world simulation, and tooling
- **Artists** — voxel modellers, texture artists, UI designers, and environmental artists
- **Composers & sound designers** — the musicians and audio engineers behind Veloren's soundtrack and SFX
- **Translators** — community members who localised the game into dozens of languages
- **Contributors worldwide** — every person who filed a bug report, submitted a patch, or wrote documentation

Full contributor list: <https://gitlab.com/veloren/veloren/-/graphs/master>

---

## Nova-Forge

**Nova-Forge** is a fork of Veloren maintained by [@shifty81](https://github.com/shifty81). It removes mandatory online authentication and adds first-class LAN co-op support.

- **Repository**: <https://github.com/shifty81/Nova-Forge>
- **Fork of**: Veloren
- **License**: GNU General Public License v3.0 (inherited from Veloren)

Nova-Forge is **not affiliated with or endorsed by the Veloren project or its maintainers.**

### Nova-Forge contributors

<!-- TODO: add Nova-Forge-specific contributors here as the project grows -->

---

## Atlas Workspace

Atlas Workspace is the **Rust + Vulkan** game development workspace that hosts NovaForge (and other game projects). It is independent of Veloren and Nova-Forge.

- **Repository**: <https://github.com/shifty81/Atlas-Workspace-Rust>
- **License**: MIT OR Apache-2.0
- **Authors**: Atlas Workspace Contributors

### Atlas Workspace contributors

<!-- TODO: list Atlas Workspace contributors here -->

---

## Third-party libraries

The following open-source libraries are used by Atlas Workspace core crates (`atlas-*`).
All are available under permissive licenses compatible with MIT/Apache-2.0.

| Library | Version | License | Purpose |
|---------|---------|---------|---------|
| [glam](https://github.com/bitshifter/glam-rs) | 0.27 | MIT OR Apache-2.0 | Math primitives |
| [egui](https://github.com/emilk/egui) | 0.27 | MIT OR Apache-2.0 | Immediate-mode GUI |
| [winit](https://github.com/rust-windowing/winit) | 0.29 | Apache-2.0 | Window/event loop |
| [ash](https://github.com/ash-rs/ash) | 0.37 | MIT OR Apache-2.0 | Vulkan bindings |
| [ash-window](https://github.com/ash-rs/ash) | 0.12 | MIT OR Apache-2.0 | Vulkan surface creation |
| [gpu-allocator](https://github.com/Traverse-Research/gpu-allocator) | 0.25 | MIT OR Apache-2.0 | GPU memory allocation |
| [serde](https://github.com/serde-rs/serde) | 1 | MIT OR Apache-2.0 | Serialization |
| [serde_json](https://github.com/serde-rs/json) | 1 | MIT OR Apache-2.0 | JSON serialization |
| [log](https://github.com/rust-lang/log) | 0.4 | MIT OR Apache-2.0 | Logging facade |
| [env_logger](https://github.com/rust-cli/env_logger) | 0.11 | MIT OR Apache-2.0 | Logger implementation |
| [rayon](https://github.com/rayon-rs/rayon) | 1 | MIT OR Apache-2.0 | Data parallelism |
| [noise](https://github.com/razaekel/noise-rs) | 0.9 | MIT | Procedural noise |
| [uuid](https://github.com/uuid-rs/uuid) | 1 | MIT OR Apache-2.0 | UUID generation |
| [thiserror](https://github.com/dtolnay/thiserror) | 1 | MIT OR Apache-2.0 | Error types |
| [anyhow](https://github.com/dtolnay/anyhow) | 1 | MIT OR Apache-2.0 | Error handling |
| [bitflags](https://github.com/bitflags/bitflags) | 2 | MIT OR Apache-2.0 | Bit flag enums |
| [parking_lot](https://github.com/Amanieu/parking_lot) | 0.12 | MIT OR Apache-2.0 | Synchronisation |

### Nova-Forge / Veloren third-party assets

<!-- TODO: list any assets used specifically by novaforge-game beyond those already covered by Veloren's own acknowledgements -->

---

## License texts

Full license texts are provided in [`LICENSES/`](LICENSES/):

- [`LICENSES/MIT`](LICENSES/MIT) — applies to Atlas Workspace core crates
- [`LICENSES/Apache-2.0`](LICENSES/Apache-2.0) — applies to Atlas Workspace core crates
- [`LICENSES/GPL-3.0`](LICENSES/GPL-3.0) — applies to `novaforge-game` and all Nova-Forge / Veloren derived code and assets
