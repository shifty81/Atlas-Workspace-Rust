# Tasks

## Completed
- [x] confirm repo naming and default branch
- [x] add project manifest (v2 with Atlas Suite bridge, capabilities, safety)
- [x] integrate extracted game source into `NovaForge/` (C++ CMake project)
- [x] set up `Integrations/AtlasSuite/` (Runtime bridge, Adapter, Plugins)
- [x] set up `AtlasAI/ProjectAdapters/NovaForge/` (AI adapter)
- [x] merge extracted phase documentation into `Docs/`
- [x] move alignment payload to `Docs/`
- [x] remove `_EXTRACT_TO_NOVAFORGE_REPO/` staging folder
- [x] update governance docs for new structure

## Remaining
- [x] add packaging entry point
- [x] add starter test lane
- [x] verify CMake build end-to-end
- [x] wire up bridge roundtrip test (Suite → manifest → bridge endpoint → NovaForge)
- [x] validate content schemas against CONTENT_RULES.md
- [x] wire up CI validation pipeline

## Buildout
- implement world and voxel primitives
- implement player and R.I.G. baseline
- implement building and salvage baseline
- implement season authority
- implement client and server startup
- implement validation suite
- implement packaging and release pipeline
