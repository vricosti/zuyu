# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

yuzu is a Nintendo Switch emulator written in C++20. It implements high-level emulation (HLE) of the Switch's Horizon OS, including CPU (ARM64 via Dynarmic), GPU (Vulkan/OpenGL), audio, input, and 50+ system services. The project supports Windows, Linux, macOS, and Android.

## Build Commands

```bash
# Configure (from repo root, using Ninja)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Build with tests enabled
cmake -B build -G Ninja -DYUZU_TESTS=ON
cmake --build build

# Run tests (Catch2)
cd build && ctest

# Format check (CI uses clang-format-15)
clang-format-15 -i src/path/to/file.cpp

# Android build
cd src/android && ./gradlew assembleDebug
# Android lint
cd src/android && ./gradlew ktlintCheck
```

Key CMake options: `ENABLE_SDL2`, `ENABLE_QT`, `ENABLE_OPENGL`, `ENABLE_CUBEB`, `YUZU_TESTS`, `YUZU_USE_PRECOMPILED_HEADERS` (ON by default), `YUZU_USE_BUNDLED_VCPKG` (ON for MSVC).

## Code Style

- **Formatter**: clang-format-15 with LLVM-based config in `src/.clang-format`
- **Column limit**: 100
- **Indentation**: 4 spaces, no tabs anywhere (enforced by pre-commit hook)
- **Braces**: Attach (K&R), no namespace indentation
- **Pointers**: Left-aligned (`int* ptr`)
- **Templates**: Always break after declaration
- **Trailing whitespace**: Prohibited in `src/`, `*.yml`, `*.txt`, `*.md`
- **License headers**: All files require SPDX `FileCopyrightText` and `License-Identifier` headers
- **Compiler warnings**: Treated as errors (`/WX` on MSVC, `-Werror` on GCC/Clang)

## Architecture

### Core Emulation (`src/core/`)

The central `Core::System` class (`core/core.h`) owns and orchestrates all emulation subsystems. Key subsystems:

- **CPU**: ARM64 emulation via Dynarmic (`core/arm/`). `CpuManager` handles multi-core thread scheduling.
- **Kernel**: HLE kernel (`core/hle/kernel/`) — processes, threads, scheduling, SVCs, memory management.
- **HLE Services**: 50+ Nintendo Switch OS services in `core/hle/service/<service_name>/`. Each service registers IPC command handlers. Services use CMIF serialization (`cmif_serialization.h`, `cmif_types.h`).
- **Filesystem**: Virtual filesystem abstraction (`core/file_sys/`) with content providers for game loading.
- **Loader**: Game format loaders in `core/loader/` (NSP, XCI, NRO, etc.).
- **Timing**: Cycle-accurate timing via `CoreTiming`.
- **Memory**: Guest-to-host virtual memory mapping (`core/memory/`, `device_memory.h`).

### GPU Emulation (`src/video_core/`)

- **GPU core**: `gpu.h/cpp` manages command processing via DMA pushers.
- **Rendering backends**: `renderer_vulkan/`, `renderer_opengl/`, `renderer_null/`.
- **Shader recompilation**: `src/shader_recompiler/` — parses GPU shaders, optimizes IR, generates GLSL/SPIR-V via sirit.
- **Caching layers**: `buffer_cache/`, `texture_cache/`, `query_cache/` for GPU resource management.
- **Video decoding**: `host1x/codecs/` with FFmpeg integration.

### Audio (`src/audio_core/`)

Audio renderer with ADSP emulation, Opus codec support. Sink backends: cubeb, SDL2.

### Input (`src/input_common/`, `src/hid_core/`)

Input abstraction layer with drivers for SDL2, hidapi, XInput. `hid_core` handles HID device resources and IR sensor emulation.

### Frontends

- **Qt GUI**: `src/yuzu/` — main desktop frontend with game list, configuration UI, debugger.
- **SDL/CLI**: `src/yuzu_cmd/` — minimal command-line frontend.
- **Android**: `src/android/` — Gradle/Java/Kotlin project with JNI bridge.
- **Shared utilities**: `src/frontend_common/` — code shared across frontends.

### Common Utilities (`src/common/`)

Platform abstractions, logging, filesystem helpers, threading primitives, cryptography. Platform-specific code in `common/windows/`, `common/linux/`, `common/arm64/`, `common/x64/`.

### Networking (`src/network/`, `src/dedicated_room/`)

Network socket implementation and LDN multiplayer room server.

## External Dependencies

29 git submodules in `externals/`. Key ones: **Dynarmic** (ARM64 JIT), **SDL** (windowing/input), **sirit** (SPIR-V generation), **Vulkan-Headers**, **VulkanMemoryAllocator**, **mbedtls** (crypto), **FFmpeg** (video), **cubeb** (audio), **xbyak/oaknut** (x86-64/ARM64 JIT helpers). Additional dependencies via vcpkg: boost, fmt, lz4, nlohmann-json, zlib, zstd.

## Namespace Conventions

Top-level namespaces mirror directory structure: `Core::`, `VideoCore::`, `AudioCore::`, `FileSys::`, `Service::`, `Shader::`. HLE services use `Service::<ServiceName>::` (e.g., `Service::AM::`, `Service::HID::`).
