#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include "core/Document.h"
#include "core/UndoManager.h"
#include "editor/VirtualRenderer.h"
#include <SDL2/SDL.h>
#include <string>
#include <chrono>

namespace liteedit {

// Main text editor controller
class TextEditor {
public:
    TextEditor();
    ~TextEditor();
    
    bool initialize(int windowWidth, int windowHeight);
    void shutdown();
    
    // File operations
    bool newFile();
    bool openFile(const std::string& path);
    bool saveFile(const std::string& path);
    bool saveFileAs(const std::string& path);
    
    // Event handling
    void handleEvent(const SDL_Event& event);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer, int viewportWidth, int viewportHeight);
    
private:
    // Event handlers
    void handleKeyDown(const SDL_KeyboardEvent& event);
    void handleTextInput(const SDL_TextInputEvent& event);
    void handleMouseButtonDown(const SDL_MouseButtonEvent& event);
    void handleMouseMotion(const SDL_MouseMotionEvent& event);
    
    // Cursor and selection
    void setCursorPosition(size_t line, size_t col);
    void moveCursor(int deltaX, int deltaY);
    void selectText(size_t startLine, size_t startCol, size_t endLine, size_t endCol);
    
    // Editing operations
    void insertText(const std::u32string& text);
    void deleteSelection();
    void deleteChar(bool forward);
    
    // Undo/Redo
    void undo();
    void redo();
    
    // Search and Replace
    struct SearchOptions {
        std::u32string pattern;
        std::u32string replacement;
        bool caseSensitive;
        bool wrapAround;
    };
    
    bool search(const SearchOptions& options);
    bool replaceNext(const SearchOptions& options);
    int replaceAll(const SearchOptions& options);
    
    // View control
    void scrollToLine(size_t line);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    
    // State access
    Document& getDocument() { return m_document; }
    const Document& getDocument() const { return m_document; }
    UndoManager& getUndoManager() { return m_undoManager; }
    VirtualRenderer& getRenderer() { return m_renderer; }
    
    bool isModified() const { return m_document.isModified(); }
    size_t getCurrentLine() const { return m_cursorLine; }
    size_t getCurrentColumn() const { return m_cursorCol; }
    
    // Formatting - public for toolbar access
    void toggleBold();
    void toggleItalic();
    void toggleUnderline();
    void setFontSize(int size);
    void setFontName(const std::string& name);

private:
    Document m_document;
    UndoManager m_undoManager;
    VirtualRenderer m_renderer;
    
    // Cursor state
    size_t m_cursorLine;
    size_t m_cursorCol;
    bool m_cursorVisible;
    double m_cursorBlinkTimer;
    double m_cursorBlinkInterval;
    
    // Selection state
    bool m_isSelecting;
    size_t m_selectionStartLine;
    size_t m_selectionStartCol;
    size_t m_selectionEndLine;
    size_t m_selectionEndCol;
    
    // Scroll state
    int m_scrollX;
    int m_scrollY;
    
    // Zoom level
    float m_zoomLevel;
    
    // Search state
    std::vector<Document::SearchResult> m_searchResults;
    size_t m_currentSearchResult;
    
    // Helper methods
    void ensureCursorVisible();
    void updateScrollPosition();
    void recalculateLayout();
    bool isInSelection(size_t line, size_t col) const;
    void copyToClipboard();
    void pasteFromClipboard();
};

} // namespace liteedit

#endif // TEXTEDITOR_H
