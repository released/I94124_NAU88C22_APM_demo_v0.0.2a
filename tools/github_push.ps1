param(
    [string]$Message = 'Update NAU88C22 audio project',
    [switch]$CheckOnly
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
[Console]::OutputEncoding = New-Object System.Text.UTF8Encoding($false)

function Invoke-Git {
    param([string[]]$GitArguments, [int[]]$AllowedCodes = @(0))
    $savedPreference = $ErrorActionPreference
    try {
        # Git writes normal progress to stderr; evaluate its exit code explicitly.
        $ErrorActionPreference = 'Continue'
        $result = @(& git -C $projectRoot @GitArguments 2>&1)
        $code = $LASTEXITCODE
    }
    finally { $ErrorActionPreference = $savedPreference }
    $lines = @($result | ForEach-Object { $_.ToString() })
    if ($code -notin $AllowedCodes) {
        throw "git $($GitArguments[0]) failed (exit $code): $($lines -join [Environment]::NewLine)"
    }
    return $lines
}

function Assert-PublishablePaths {
    param([string[]]$Paths)
    foreach ($raw in $Paths) {
        $path = $raw.Replace('\', '/').Trim("`r", "`n")
        if (-not $path) { continue }
        if ($path -match '^(\.local/|docs/verification/|MANUAL_CLEANUP\.md$|CODEX_HANDOFF\.md$)' -or
            $path -match '(^|/)(__LA/|Colsonic/backup/|KEIL/(obj/|lst/|Nu_Link_Driver\.ini$))' -or
            $path -match '\.uvguix\.' -or
            $path -match '(^|/)\.env(?:\.|$)' -or $path -match '\.(?:pem|pfx|p12|key)$') {
            throw "Local/private content is included in the upload scope: $path"
        }
    }
}

function Assert-RemoteDestination {
    param([string]$Repository, [switch]$AllowMissing)
    $remoteNames = @(Invoke-Git -GitArguments @('remote'))
    if ('origin' -notin $remoteNames) {
        if ($AllowMissing) { return }
        throw 'Origin is missing. Run the confirmed one-time gh repo create command first.'
    }
    $fetchUrls = @(Invoke-Git -GitArguments @('remote', 'get-url', '--all', 'origin'))
    $pushUrls = @(Invoke-Git -GitArguments @('remote', 'get-url', '--push', '--all', 'origin'))
    if ($fetchUrls.Count -ne 1 -or $pushUrls.Count -ne 1) {
        throw 'Origin must have exactly one fetch URL and one push URL.'
    }
    foreach ($url in @($fetchUrls[0], $pushUrls[0])) {
        $match = [regex]::Match($url, '^(?:https://github\.com/|git@github\.com:|ssh://git@github\.com/)([^/]+/[^/]+?)(?:\.git)?/?$')
        if (-not $match.Success -or $match.Groups[1].Value -ne $Repository) {
            throw 'Origin differs from github_repository.json. No upload was performed.'
        }
    }
}

try {
    Get-Command git -ErrorAction Stop | Out-Null
    $configurationPath = Join-Path $projectRoot 'github_repository.json'
    if (-not (Test-Path -LiteralPath $configurationPath)) {
        throw 'github_repository.json is missing; confirm the repository destination first.'
    }
    $configuration = Get-Content -LiteralPath $configurationPath -Raw | ConvertFrom-Json
    $repository = [string]$configuration.repository
    $branch = [string]$configuration.branch
    $visibility = [string]$configuration.visibility
    if ($repository -notmatch '^[A-Za-z0-9-]+/[A-Za-z0-9_.-]+$' -or
        $branch -ne 'main' -or $visibility -notin @('public', 'private')) {
        throw 'Invalid repository, branch or visibility configuration.'
    }
    $top = @(Invoke-Git -GitArguments @('rev-parse', '--show-toplevel'))
    if ($top.Count -ne 1 -or [IO.Path]::GetFullPath($top[0]) -ne [IO.Path]::GetFullPath($projectRoot)) {
        throw 'The script must operate on this project repository only.'
    }
    $currentBranch = @(Invoke-Git -GitArguments @('symbolic-ref', '--quiet', '--short', 'HEAD'))
    if ($currentBranch.Count -ne 1 -or $currentBranch[0] -ne $branch) {
        throw 'Switch to the configured main branch before using this upload script.'
    }
    foreach ($state in @('MERGE_HEAD', 'CHERRY_PICK_HEAD', 'REVERT_HEAD', 'rebase-merge', 'rebase-apply')) {
        $statePath = @(Invoke-Git -GitArguments @('rev-parse', '--git-path', $state))[0]
        if (-not [IO.Path]::IsPathRooted($statePath)) { $statePath = Join-Path $projectRoot $statePath }
        if (Test-Path -LiteralPath $statePath) { throw 'Finish the active Git operation before uploading.' }
    }
    Assert-RemoteDestination -Repository $repository -AllowMissing:$CheckOnly
    $candidates = @(Invoke-Git -GitArguments @('-c', 'core.quotepath=false', 'ls-files', '--cached', '--others', '--exclude-standard'))
    Assert-PublishablePaths -Paths $candidates
    foreach ($path in $candidates) {
        $fullPath = Join-Path $projectRoot $path
        if (Test-Path -LiteralPath $fullPath -PathType Leaf) {
            if ((Get-Item -LiteralPath $fullPath).Length -ge 100MB) { throw "File exceeds upload size policy: $path" }
        }
    }
    $head = @(Invoke-Git -GitArguments @('rev-parse', '--verify', '--quiet', 'HEAD') -AllowedCodes @(0, 1))
    if ($head.Count -gt 0) {
        $history = @(Invoke-Git -GitArguments @('-c', 'core.quotepath=false', 'log', '--format=', '--name-only', '--no-renames', 'HEAD'))
        Assert-PublishablePaths -Paths $history
    }
    & (Join-Path $PSScriptRoot 'check_apm_consistency.ps1') | Out-Null
    Write-Output "Local upload checks passed: $($candidates.Count) candidate files, destination $repository, branch $branch."
    if ($CheckOnly) {
        Write-Output 'CheckOnly: no files staged, no commit created and no remote changed.'
        exit 0
    }

    Get-Command gh -ErrorAction Stop | Out-Null
    $remoteJson = @(& gh repo view $repository --json nameWithOwner,isPrivate,viewerPermission)
    if ($LASTEXITCODE -ne 0) { throw 'Cannot inspect the confirmed GitHub repository.' }
    $remote = ($remoteJson -join "`n") | ConvertFrom-Json
    if ($remote.nameWithOwner -ne $repository -or
        [bool]$remote.isPrivate -ne ($visibility -eq 'private') -or
        $remote.viewerPermission -notin @('ADMIN', 'MAINTAIN', 'WRITE')) {
        throw 'GitHub destination, visibility or write permission does not match the configuration.'
    }
    $remoteBranches = @(Invoke-Git -GitArguments @('ls-remote', '--heads', 'origin', ('refs/heads/' + $branch)))
    if ($remoteBranches.Count -gt 0) {
        Invoke-Git -GitArguments @('fetch', '--quiet', 'origin', ('refs/heads/' + $branch + ':refs/remotes/origin/' + $branch)) | Out-Null
        Invoke-Git -GitArguments @('merge-base', '--is-ancestor', ('refs/remotes/origin/' + $branch), 'HEAD') | Out-Null
    }
    Invoke-Git -GitArguments @('add', '--all', '--', '.') | Out-Null
    $staged = @(Invoke-Git -GitArguments @('-c', 'core.quotepath=false', 'diff', '--cached', '--name-only'))
    Assert-PublishablePaths -Paths $staged
    if ($staged.Count -gt 0) {
        Invoke-Git -GitArguments @('commit', '-m', $Message)
    }
    Invoke-Git -GitArguments @('push', '--set-upstream', 'origin', ($branch + ':refs/heads/' + $branch))
    Write-Output "Upload completed: https://github.com/$repository"
    exit 0
}
catch {
    Write-Error $_.Exception.Message -ErrorAction Continue
    exit 1
}
