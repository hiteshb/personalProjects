function Set-OptimizedDNS {
    # Store original DNS settings
    $global:OriginalDnsSettings = @{}
    $dnsServers = @("1.1.1.1", "8.8.8.8")
    $adapters = Get-DnsClient | Where-Object {
        $_.InterfaceAlias -ne "Loopback Pseudo-Interface 1" -and
        $_.InterfaceAlias -notmatch "Virtual" -and
        $_.InterfaceAlias -notmatch "VPN"
    }

    foreach ($adapter in $adapters) {
        try {
            $global:OriginalDnsSettings[$adapter.InterfaceAlias] = (Get-DnsClientServerAddress -InterfaceAlias $adapter.InterfaceAlias).ServerAddresses
            Write-Host "[info] Setting DNS for: $($adapter.InterfaceAlias)" -ForegroundColor Cyan
            Set-DnsClientServerAddress -InterfaceAlias $adapter.InterfaceAlias -ServerAddresses $dnsServers -ErrorAction Stop
        } catch {
            Write-Host "[error] Failed to set DNS for: $($adapter.InterfaceAlias)" -ForegroundColor Red
        }
    }

    Write-Host "`n🧹 Flushing DNS cache..." -ForegroundColor Yellow
    ipconfig /flushdns
    Write-Host "[info] DNS updated to Cloudflare + Google." -ForegroundColor Green
}

function Revert-DNS {
    if ($null -eq $global:OriginalDnsSettings) {
        Write-Host "[warn] No original DNS settings were stored. Skipping revert." -ForegroundColor Yellow
        return
    }

    foreach ($entry in $global:OriginalDnsSettings.GetEnumerator()) {
        try {
            $interface = $entry.Key
            $servers = $entry.Value
            if ($servers.Count -eq 0) {
                # If original was auto (DHCP), reset
                Set-DnsClientServerAddress -InterfaceAlias $interface -ResetServerAddresses
                Write-Host "[info] Reset DNS to automatic for: $interface" -ForegroundColor Green
            } else {
                Set-DnsClientServerAddress -InterfaceAlias $interface -ServerAddresses $servers
                Write-Host "[info] Restored DNS for: $interface" -ForegroundColor Green
            }
        } catch {
            Write-Host "[error] Failed to revert DNS for: $interface" -ForegroundColor Red
        }
    }

    Write-Host "`n🧼 Flushing DNS cache again..." -ForegroundColor Yellow
    ipconfig /flushdns
    Write-Host "[info] DNS reverted to original settings." -ForegroundColor Cyan
}

# Ask to enable DNS boost
$dnsChoice = Read-Host "[optional] Boost speed by applying fast DNS (1.1.1.1 + 8.8.8.8)? (y/n)"
if ($dnsChoice -match '^(y|Y)') {
    Set-OptimizedDNS
} else {
    Write-Host "[info] Skipping DNS changes." -ForegroundColor Yellow
}

# Ask if batch mode
$useBatch = Read-Host "[optional] Use batch download file (URL|filename.txt)? (y/n)"

if ($useBatch -match '^(y|Y)') {
    $batchFile = Read-Host "[info] Enter full path to batch file"
    if (-not (Test-Path $batchFile)) {
        Write-Host "[error] File not found: $batchFile" -ForegroundColor Red
        exit
    }

    # Check for yt-dlp and aria2c
    if (-not (Get-Command yt-dlp -ErrorAction SilentlyContinue)) {
        Write-Host "[error] yt-dlp not found in PATH. Please install yt-dlp." -ForegroundColor Red
        exit
    }

    if (-not (Get-Command aria2c -ErrorAction SilentlyContinue)) {
        Write-Host "[error] aria2c not found in PATH. Please install aria2." -ForegroundColor Red
        exit
    }

    $lines = Get-Content $batchFile | Where-Object { $_.Trim() -ne "" }

    foreach ($line in $lines) {
        $parts = $line -split '\|'
        if ($parts.Count -lt 1) {
            Write-Host "[warn] Skipping malformed line: $line" -ForegroundColor Yellow
            continue
        }

        $downloadUrl = $parts[0].Trim('" ')
        $filename = if ($parts.Count -ge 2) { $parts[1].Trim() } else { "download_$(Get-Random).mp4" }

        if (-not ($filename -match '\.\w{2,5}$')) {
            $filename += ".mp4"
        }

        Write-Host "`n🚀 Downloading: $filename" -ForegroundColor Cyan

        $ytArgs = @(
            "-N", "4",
            "-R", "infinite",
            "--fragment-retries", "20",
            "--retries", "infinite",
            "--user-agent", "Mozilla/5.0 (Linux; Android 14; Pixel 8 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.6478.55 Mobile Safari/537.36",
            "--no-part",
            "--no-cache-dir",
            "--concurrent-fragments", "4",
            "--downloader", "aria2c",
            "--downloader-args", 'aria2c:-c -x 16 -s 16 -k 2M --timeout=60 --connect-timeout=30 --max-tries=15 --retry-wait=5 --file-allocation=none --summary-interval=0 --enable-http-keep-alive=true --disable-ipv6=true',
            "-o", $filename,
            $downloadUrl
        )

        & yt-dlp @ytArgs
    }
} else {
    # Ask for inputs
    $downloadUrl = Read-Host "[info] Enter PikPak download URL"
    $downloadUrl = $downloadUrl.Trim('"')  # <-- clean quotes
    $filename = Read-Host "[info] Enter output filename (e.g., movie or movie.mkv)"

    # Auto-add .mp4 if no extension
    if (-not ($filename -match '\.\w{2,5}$')) {
        $filename += ".mp4"
        Write-Host "[info] No extension provided. Defaulting to: $filename"
    }

    # Check for yt-dlp
    if (-not (Get-Command yt-dlp -ErrorAction SilentlyContinue)) {
        Write-Host "[error] yt-dlp not found in PATH. Please install yt-dlp." -ForegroundColor Red
        exit
    }

    # Check for aria2c
    if (-not (Get-Command aria2c -ErrorAction SilentlyContinue)) {
        Write-Host "[error] aria2c not found in PATH. Please install aria2." -ForegroundColor Red
        exit
    }

    # Build argument array safely
    $ytArgs = @(
        "-N", "4",
        "-R", "infinite",
        "--fragment-retries", "20",
        "--retries", "infinite",
        "--user-agent", "Mozilla/5.0 (Linux; Android 14; Pixel 8 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.6478.55 Mobile Safari/537.36",
        "--no-part",
        "--no-cache-dir",
        "--concurrent-fragments", "4",
        "--downloader", "aria2c",
        "--downloader-args", 'aria2c:-c -x 16 -s 16 -k 2M --timeout=60 --connect-timeout=30 --max-tries=15 --retry-wait=5 --file-allocation=none --summary-interval=0 --enable-http-keep-alive=true --disable-ipv6=true',
        "-o", $filename,
        $downloadUrl
    )

    # Run the command
    Write-Host "`n🚀 Downloading..." -ForegroundColor Cyan
    & yt-dlp @ytArgs
}

# Ask to revert DNS after download
if ($dnsChoice -match '^(y|Y)') {
    $revertChoice = Read-Host "`n[optional] Revert DNS to original settings now? (y/n)"
    if ($revertChoice -match '^(y|Y)') {
        Revert-DNS
    } else {
        Write-Host "[info] Keeping optimized DNS." -ForegroundColor Yellow
    }
}
