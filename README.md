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
The file /workspace/README.md has been edited. Here's the result of running `cat -n` on a snippet of /workspace/README.md:
     1	# LiteEdit - Lightweight Text Editor
     2	
     3	A high-performance, low-memory text editor optimized for low-end devices with support for multiple document formats (.txt, .docx, .rtf).
     4	
     5	## Latest Update (v1.1.0)
     6	
     7	### What's New
     8	
     9	This update significantly expands format support and typography options:
    10	
    11	#### 1. Enhanced Format Support
    12	- **Full .docx Support**: Added `DocxHandler` module using pugixml and libzip for parsing and saving Microsoft Word documents
    13	  - Preserves text formatting (bold, italic, underline)
    14	  - Maintains font family and size information
    15	  - Supports paragraph styles
    16	  - Extracts content from document.xml and styles.xml within the OOXML package
    17	  
    18	- **Improved .rtf Support**: Enhanced `RtfHandler` module for Rich Text Format files
    19	  - Better parsing of RTF control words
    20	  - Support for nested formatting groups
    21	  - Font table extraction and mapping
    22	  - Color and background color support
    23	
    24	#### 2. Font Management System
    25	- **New FontManager Module**: Centralized font handling with support for multiple font families
    26	  - Arial (default sans-serif)
    27	  - Times New Roman (default serif)
    28	  - Courier New (monospace for code)
    29	  - Verdana (alternative sans-serif)
    30	  - Georgia (alternative serif)
    31	  
    32	- **Font Features**:
    33	  - Runtime font switching via toolbar
    34	  - Font size adjustment (8pt - 72pt)
    35	  - Font caching for performance
    36	  - Fallback mechanism for missing fonts
    37	
    38	#### 3. Updated Dependencies
    39	- Added `libzip-dev` for .docx file manipulation (OOXML uses ZIP container)
    40	- Added `zlib1g-dev` for compression/decompression
    41	- Integrated `pugixml` (header-only, auto-downloaded via CMake) for XML parsing
    42	
    43	#### 4. Code Architecture Improvements
    44	- Refactored `TextEditor` to support rich text elements with per-character formatting
    45	- Enhanced `Document` model to store font attributes alongside text
    46	- Updated `Toolbar` UI with font family dropdown and format buttons
    47	- Added format detection based on file extension
    48	
    49	### Files Modified/Added
    50	
    51	**New Files:**
    52	- `src/formats/DocxHandler.h` - DOCX file format handler interface
    53	- `src/formats/DocxHandler.cpp` - DOCX parsing and saving implementation
    54	- `src/formats/RtfHandler.h` - RTF file format handler interface
    55	- `src/formats/RtfHandler.cpp` - RTF parsing and saving implementation
    56	- `src/fonts/FontManager.h` - Font management system interface
    57	- `src/fonts/FontManager.cpp` - Font loading and caching implementation
    58	
    59	**Modified Files:**
    60	- `src/editor/TextEditor.h` - Extended to support rich text elements
    61	- `src/editor/TextEditor.cpp` - Added formatting methods
    62	- `src/ui/Toolbar.h` - Added font selection controls
    63	- `src/ui/Toolbar.cpp` - Implemented font/format button handlers
    64	- `src/core/Document.h` - Enhanced data model for formatting
    65	- `CMakeLists.txt` - Added new dependencies and source files
    66	- `README.md` - This file (updated documentation)
    67	
    68	### Build Requirements Updated
    69	
    70	New system packages required:
    71	```bash
    72	# Ubuntu/Debian
    73	sudo apt-get install libzip-dev zlib1g-dev
    74	
    75	# Fedora/RHEL
    76	sudo dnf install libzip-devel zlib-devel
    77	
    78	# macOS
    79	brew install libzip
    80	```
    81	
    82	### Known Limitations
    83	
    84	- .docx images and tables are not yet supported (text only)
    85	- Advanced .docx features (track changes, comments) not implemented
    86	- RTF support covers common formatting but not full specification
    87	- Font rendering quality depends on SDL2_ttf backend
    88	
    89	---
    90	
    91	## Technology Stack Selection
    92	
    93	### Chosen: C++ with SDL2 and Dear ImGui
    94	
    95	**Rationale:**
    96	
Review the changes and make sure they are as expected. Edit the file again if necessary.
