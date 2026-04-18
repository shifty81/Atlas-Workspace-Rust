# Local Commands

## PowerShell
```powershell
pwsh -ExecutionPolicy Bypass -File .\Scripts\bootstrap_repo.ps1 -RemoteUrl "<your repo url>"
pwsh -ExecutionPolicy Bypass -File .\Scripts\validate_project.ps1
git branch -M main
git push -u origin main
```

## Bash
```bash
./Scripts/bootstrap_repo.sh "NovaForge" "<your repo url>"
./Scripts/validate_project.sh
./Scripts/validate_content.sh
./Scripts/verify_build.sh [Debug|Development|Release]
./Scripts/package.sh <ProfileId>   # ClientDev, ClientRelease, ServerRelease
```

## Test Lane
```bash
./Tests/NovaForge.Tests/run_tests.sh           # full test lane
./Tests/NovaForge.Tests/test_manifest.sh        # manifest validation only
./Tests/NovaForge.Tests/test_bridge_roundtrip.sh # bridge config test only
```
