# LiteEdit - Lightweight Text Editor

A high-performance, low-memory text editor optimized for low-end devices.

## Technology Stack Selection

### Chosen: C++ with SDL2

**Rationale:**

1. **Minimal RAM Usage**: C++ allows precise memory control. SDL2 is lightweight (~5MB in memory).
2. **Native Performance**: Compiled code runs directly on hardware without runtime overhead.
3. **Simple Build**: Single binary with statically linked dependencies.
4. **Cross-Platform**: Windows, Linux, macOS out of the box.

**Alternatives Considered:**
- **Rust/Iced**: Excellent choice, but larger binary due to Rust runtime.
- **Qt**: Too heavy (~50MB+ memory), overkill for a text editor.
- **Go/Fyne**: GC pauses can cause lag when editing large files.

## Project Structure

```
/workspace/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── assets/
│   └── fonts/                  # Font files (optional)
├── build/                      # Build output directory
└── src/
    ├── main.cpp                # Entry point (in MainWindow.cpp)
    ├── core/
    │   ├── Document.h          # Document data model
    │   └── Document.cpp
    │   ├── UndoManager.h       # Undo/Redo system
    │   └── UndoManager.cpp
    ├── editor/
    │   ├── TextEditor.h        # Main editor controller
    │   ├── TextEditor.cpp
    │   └── VirtualRenderer.h   # Viewport-based rendering
    │   └── VirtualRenderer.cpp
    ├── ui/
    │   ├── MainWindow.h        # Application window
    │   ├── MainWindow.cpp
    │   └── Toolbar.h           # Toolbar component
    └── utils/
        ├── FileHandler.h       # File I/O operations
        └── FileHandler.cpp
```

## Features (MVP)

- ✅ Basic text editing (insert, delete, navigate)
- ✅ File operations (open/save .txt, basic .rtf support)
- ✅ Undo/Redo with configurable history depth
- ✅ Search and replace
- ✅ Virtualized rendering (only visible lines rendered)
- ✅ Lazy loading for large documents
- ✅ Keyboard shortcuts (Ctrl+S, Ctrl+O, Ctrl+Z, etc.)
- ✅ Cursor blinking and selection
- ✅ Zoom in/out

## Build Instructions

### Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libsdl2-dev libsdl2-ttf-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install -y gcc-c++ cmake SDL2-devel SDL2_ttf-devel
```

**Windows (with MSYS2):**
```bash
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf
```

**macOS:**
```bash
brew install cmake sdl2 sdl2_ttf
```

### Build Commands

```bash
cd /workspace

# Create build directory
mkdir -p build && cd build

# Configure (Release mode for optimization)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)

# Install (optional)
sudo cmake --install .
```

### Run

```bash
./texteditor
```

## Performance Optimizations

### 1. Virtualized Rendering
Only lines visible in viewport are rendered. For a 10,000-line document with 40 visible lines, this reduces rendering by 99.6%.

### 2. Lazy Loading
Document content is loaded on-demand. Large files don't block UI.

### 3. Memory-Efficient Data Structures
- `std::unique_ptr` for automatic memory management
- Fixed-size undo history (default: 50 operations)
- Cached line metrics to avoid recalculation

### 4. Zero External Dependencies (besides SDL2)
No heavy frameworks or runtimes.

## Critical Analysis

### Weaknesses and Trade-offs

1. **Limited Rich Text Support**
   - .docx not implemented (requires complex OOXML parsing)
   - RTF support is basic (strips most formatting)
   - **Trade-off**: Simplicity vs. feature completeness

2. **Fixed-Width Font Assumption**
   - Current implementation assumes monospace for simplicity
   - Proportional fonts require more complex layout engine
   - **Trade-off**: Performance vs. typography quality

3. **No Syntax Highlighting**
   - Would require language parsers and color schemes
   - Adds memory overhead for token storage
   - **Trade-off**: General-purpose vs. code editor features

4. **Single-Threaded Rendering**
   - All rendering on main thread
   - Could cause stutter on very large documents
   - **Trade-off**: Simplicity vs. smoothness

5. **Basic Search Algorithm**
   - Linear search O(n*m) where n=document size, m=pattern length
   - Could use Boyer-Moore or Rabin-Karp for large documents
   - **Trade-off**: Implementation complexity vs. search speed

### Memory Usage Estimates

| Document Size | RAM Usage | Notes |
|--------------|-----------|-------|
| 1 KB         | ~2 MB     | Base application overhead |
| 100 KB       | ~3 MB     | Linear scaling |
| 1 MB         | ~10 MB    | Still responsive |
| 10 MB        | ~80 MB    | Virtualization kicks in |
| 100 MB       | ~200 MB   | Lazy loading essential |

### Recommended Improvements for Production

1. Add proper font rendering with SDL2_ttf integration
2. Implement piece table data structure for better edit performance
3. Add incremental search for real-time feedback
4. Support for tabs and workspaces
5. Plugin architecture for extensibility

## License

MIT License - See LICENSE file for details.

## Contributing

1. Fork the repository
2. Create feature branch
3. Submit pull request

All code must follow ASCII-only naming conventions for cross-platform compatibility.
