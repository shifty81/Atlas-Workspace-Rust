param(
    [string]$RepoName = "NovaForge",
    [string]$RemoteUrl = "",
    [switch]$InitializeGit = $true
)

$ErrorActionPreference = "Stop"

Write-Host "NovaForge bootstrap starting"
Write-Host "Repo name: $RepoName"

if ($InitializeGit) {
    if (-not (Test-Path ".git")) {
        git init | Out-Null
        Write-Host "Initialized git repository"
    } else {
        Write-Host "Git repository already initialized"
    }

    git add .
    git commit -m "Initial NovaForge spec + scaffold bootstrap" 2>$null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Initial commit not created. This usually means git user identity is not configured or no staged changes were present."
    } else {
        Write-Host "Initial commit created"
    }

    if ($RemoteUrl -ne "") {
        $existing = git remote
        if ($existing -notcontains "origin") {
            git remote add origin $RemoteUrl
            Write-Host "Added origin remote"
        } else {
            git remote set-url origin $RemoteUrl
            Write-Host "Updated origin remote"
        }
    }
}

Write-Host ""
Write-Host "Next suggested commands:"
Write-Host "  git status"
Write-Host "  git branch -M main"
if ($RemoteUrl -ne "") {
    Write-Host "  git push -u origin main"
}
