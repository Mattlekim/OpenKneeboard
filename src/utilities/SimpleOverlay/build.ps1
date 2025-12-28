#!/usr/bin/env pwsh
#
# SimpleOverlay Build Script
# Automates the complete build process for native and C# components
#

param(
    [switch]$SkipCMake = $false,
    [switch]$SkipCSharp = $false,
    [switch]$SkipExample = $false,
    [ValidateSet('Debug', 'Release', 'Both')]
    [string]$Configuration = 'Both'
)

$ErrorActionPreference = 'Stop'

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "SimpleOverlay Build Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $ScriptDir))
$BuildDir = Join-Path $RootDir "build"
$BinDir = Join-Path $BuildDir "bin"

# Ensure build directory exists
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Track overall success
$BuildSuccess = $true

# Build native library with CMake
if (-not $SkipCMake) {
    Write-Host "[1/4] Configuring CMake for SimpleOverlay..." -ForegroundColor Yellow
    
    try {
        # Configure CMake
        Push-Location $ScriptDir
        
        $CMakeBuildDir = Join-Path $ScriptDir "build"
        if (-not (Test-Path $CMakeBuildDir)) {
            New-Item -ItemType Directory -Path $CMakeBuildDir | Out-Null
        }
        
        Write-Host "Running: cmake -B $CMakeBuildDir -G `"Visual Studio 17 2022`" -A x64" -ForegroundColor Gray
        & cmake -B $CMakeBuildDir -G "Visual Studio 17 2022" -A x64
        
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed with exit code $LASTEXITCODE"
        }
        
        Write-Host "✓ CMake configuration completed successfully" -ForegroundColor Green
        Write-Host ""
        
        # Build configurations
        $Configs = @()
        if ($Configuration -eq 'Both') {
            $Configs = @('Debug', 'Release')
        } else {
            $Configs = @($Configuration)
        }
        
        foreach ($Config in $Configs) {
            Write-Host "[2/4] Building SimpleOverlay ($Config)..." -ForegroundColor Yellow
            
            Write-Host "Running: cmake --build $CMakeBuildDir --config $Config" -ForegroundColor Gray
            & cmake --build $CMakeBuildDir --config $Config
            
            if ($LASTEXITCODE -ne 0) {
                throw "CMake build failed for $Config configuration with exit code $LASTEXITCODE"
            }
            
            Write-Host "✓ Native library built successfully ($Config)" -ForegroundColor Green
            Write-Host ""
        }
        
        Pop-Location
    }
    catch {
        Write-Host "✗ Native library build failed: $_" -ForegroundColor Red
        $BuildSuccess = $false
        Pop-Location
        
        if (-not $SkipCSharp -and -not $SkipExample) {
            Write-Host ""
            Write-Host "Skipping C# builds due to native build failure." -ForegroundColor Yellow
            exit 1
        }
    }
}
else {
    Write-Host "[1/4] Skipping CMake build (--SkipCMake specified)" -ForegroundColor Gray
    Write-Host ""
}

# Build C# wrapper library
if (-not $SkipCSharp -and $BuildSuccess) {
    Write-Host "[3/4] Building C# wrapper library..." -ForegroundColor Yellow
    
    try {
        $CSharpProject = Join-Path $ScriptDir "SimpleOverlay.CSharp\SimpleOverlay.csproj"
        
        if (-not (Test-Path $CSharpProject)) {
            throw "C# project not found at: $CSharpProject"
        }
        
        Write-Host "Running: dotnet build $CSharpProject --configuration Release" -ForegroundColor Gray
        & dotnet build $CSharpProject --configuration Release
        
        if ($LASTEXITCODE -ne 0) {
            throw "C# wrapper build failed with exit code $LASTEXITCODE"
        }
        
        Write-Host "✓ C# wrapper library built successfully" -ForegroundColor Green
        Write-Host ""
    }
    catch {
        Write-Host "✗ C# wrapper library build failed: $_" -ForegroundColor Red
        $BuildSuccess = $false
    }
}
else {
    if (-not $BuildSuccess) {
        # Already reported error above
    }
    elseif ($SkipCSharp) {
        Write-Host "[3/4] Skipping C# wrapper build (--SkipCSharp specified)" -ForegroundColor Gray
        Write-Host ""
    }
}

# Build C# example application
if (-not $SkipExample -and $BuildSuccess) {
    Write-Host "[4/4] Building C# example application..." -ForegroundColor Yellow
    
    try {
        $ExampleProject = Join-Path $ScriptDir "Examples\CSharp\SimpleOverlayExample.csproj"
        
        if (-not (Test-Path $ExampleProject)) {
            throw "Example project not found at: $ExampleProject"
        }
        
        Write-Host "Running: dotnet build $ExampleProject --configuration Release" -ForegroundColor Gray
        & dotnet build $ExampleProject --configuration Release
        
        if ($LASTEXITCODE -ne 0) {
            throw "Example application build failed with exit code $LASTEXITCODE"
        }
        
        # Copy DLL to example output directory
        $SourceDLL = Join-Path $ScriptDir "build\bin\Release\SimpleOverlay.dll"
        $ExampleBinDir = Join-Path $ScriptDir "Examples\CSharp\bin\Release\net6.0"
        
        if (Test-Path $SourceDLL) {
            if (-not (Test-Path $ExampleBinDir)) {
                New-Item -ItemType Directory -Path $ExampleBinDir -Force | Out-Null
            }
            
            Copy-Item $SourceDLL -Destination $ExampleBinDir -Force
            Write-Host "✓ Copied SimpleOverlay.dll to example output directory" -ForegroundColor Green
        }
        else {
            Write-Host "⚠ Warning: SimpleOverlay.dll not found at $SourceDLL" -ForegroundColor Yellow
            Write-Host "  The example may not run without the native DLL" -ForegroundColor Yellow
        }
        
        Write-Host "✓ C# example application built successfully" -ForegroundColor Green
        Write-Host ""
    }
    catch {
        Write-Host "✗ C# example application build failed: $_" -ForegroundColor Red
        $BuildSuccess = $false
    }
}
else {
    if (-not $BuildSuccess) {
        # Already reported error above
    }
    elseif ($SkipExample) {
        Write-Host "[4/4] Skipping example build (--SkipExample specified)" -ForegroundColor Gray
        Write-Host ""
    }
}

# Print summary
Write-Host "========================================" -ForegroundColor Cyan
if ($BuildSuccess) {
    Write-Host "BUILD SUCCESSFUL" -ForegroundColor Green
    Write-Host ""
    Write-Host "Outputs:" -ForegroundColor Cyan
    
    $OutputDLL = Join-Path $ScriptDir "build\bin\Release\SimpleOverlay.dll"
    if (Test-Path $OutputDLL) {
        Write-Host "  • Native DLL: $OutputDLL" -ForegroundColor White
    }
    
    $CSharpDLL = Join-Path $ScriptDir "SimpleOverlay.CSharp\bin\Release\net6.0\OpenKneeboard.SimpleOverlay.dll"
    if (Test-Path $CSharpDLL) {
        Write-Host "  • C# Library: $CSharpDLL" -ForegroundColor White
    }
    
    $ExampleExe = Join-Path $ScriptDir "Examples\CSharp\bin\Release\net6.0\SimpleOverlayExample.exe"
    if (Test-Path $ExampleExe) {
        Write-Host "  • Example App: $ExampleExe" -ForegroundColor White
    }
    
    Write-Host ""
    Write-Host "To run the example:" -ForegroundColor Cyan
    Write-Host "  cd Examples\CSharp" -ForegroundColor White
    Write-Host "  dotnet run" -ForegroundColor White
}
else {
    Write-Host "BUILD FAILED" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please review the errors above and try again." -ForegroundColor Yellow
    exit 1
}

Write-Host "========================================" -ForegroundColor Cyan
exit 0
