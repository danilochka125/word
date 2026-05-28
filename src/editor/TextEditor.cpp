#include "editor/TextEditor.h"
#include <algorithm>
#include <cmath>

namespace liteedit {

TextEditor::TextEditor()
    : m_undoManager(50)
    , m_cursorLine(0)
    , m_cursorCol(0)
    , m_cursorVisible(true)
    , m_cursorBlinkTimer(0.0)
    , m_cursorBlinkInterval(0.5)
    , m_isSelecting(false)
    , m_selectionStartLine(0)
    , m_selectionStartCol(0)
    , m_selectionEndLine(0)
    , m_selectionEndCol(0)
    , m_scrollX(0)
    , m_scrollY(0)
    , m_zoomLevel(1.0f)
    , m_currentSearchResult(0) {
}

TextEditor::~TextEditor() {
    shutdown();
}

bool TextEditor::initialize(int windowWidth, int windowHeight) {
    return m_renderer.initialize(windowWidth, windowHeight);
}

void TextEditor::shutdown() {
    m_renderer.shutdown();
}

bool TextEditor::newFile() {
    m_document = Document();
    m_undoManager.clear();
    m_cursorLine = 0;
    m_cursorCol = 0;
    m_scrollX = 0;
    m_scrollY = 0;
    return true;
}

bool TextEditor::openFile(const std::string& path) {
    if (m_document.loadFromFile(path)) {
        m_undoManager.clear();
        m_cursorLine = 0;
        m_cursorCol = 0;
        m_scrollX = 0;
        m_scrollY = 0;
        recalculateLayout();
        return true;
    }
    return false;
}

bool TextEditor::saveFile(const std::string& path) {
    return m_document.saveToFile(path);
}

bool TextEditor::saveFileAs(const std::string& path) {
    return saveFile(path);
}

void TextEditor::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN:
            handleKeyDown(event.key);
            break;
            
        case SDL_TEXTINPUT:
            handleTextInput(event.text);
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            handleMouseButtonDown(event.button);
            break;
            
        case SDL_MOUSEMOTION:
            handleMouseMotion(event.motion);
            break;
            
        default:
            break;
    }
}

void TextEditor::handleKeyDown(const SDL_KeyboardEvent& event) {
    // Handle modifier keys for formatting
    if (event.keysym.mod & KMOD_CTRL) {
        switch (event.keysym.sym) {
            case SDLK_z:
                if (event.keysym.mod & KMOD_SHIFT) {
                    redo();
                } else {
                    undo();
                }
                return;
                
            case SDLK_s:
                saveFile(m_document.getFilePath());
                return;
                
            case SDLK_o:
                // Open file dialog would be called here
                return;
                
            case SDLK_f:
                // Open search dialog
                return;
                
            case SDLK_b:
                toggleBold();
                return;
                
            case SDLK_i:
                toggleItalic();
                return;
                
            case SDLK_u:
                toggleUnderline();
                return;
                
            case SDLK_EQUALS:
            case SDLK_PLUS:
                zoomIn();
                return;
                
            case SDLK_MINUS:
                zoomOut();
                return;
                
            default:
                break;
        }
    }
    
    // Navigation and editing
    switch (event.keysym.sym) {
        case SDLK_LEFT:
            moveCursor(-1, 0);
            break;
            
        case SDLK_RIGHT:
            moveCursor(1, 0);
            break;
            
        case SDLK_UP:
            moveCursor(0, -1);
            break;
            
        case SDLK_DOWN:
            moveCursor(0, 1);
            break;
            
        case SDLK_HOME:
            m_cursorCol = 0;
            ensureCursorVisible();
            break;
            
        case SDLK_END:
            {
                const TextLine* line = m_document.getLine(m_cursorLine);
                m_cursorCol = line ? line->length() : 0;
                ensureCursorVisible();
            }
            break;
            
        case SDLK_PAGEUP:
            moveCursor(0, -20);
            break;
            
        case SDLK_PAGEDOWN:
            moveCursor(0, 20);
            break;
            
        case SDLK_BACKSPACE:
            deleteChar(false);
            break;
            
        case SDLK_DELETE:
            deleteChar(true);
            break;
            
        case SDLK_RETURN:
            insertText(U"\n");
            break;
            
        case SDLK_TAB:
            insertText(U"    ");
            break;
            
        default:
            break;
    }
}

void TextEditor::handleTextInput(const SDL_TextInputEvent& event) {
    // Convert UTF-8 to UTF-32
    std::u32string text;
    const char* ptr = event.text;
    
    while (*ptr) {
        char32_t ch;
        unsigned char c = *ptr;
        
        if ((c & 0x80) == 0) {
            ch = c;
            ptr++;
        } else if ((c & 0xE0) == 0xC0) {
            ch = ((c & 0x1F) << 6) | (ptr[1] & 0x3F);
            ptr += 2;
        } else if ((c & 0xF0) == 0xE0) {
            ch = ((c & 0x0F) << 12) | ((ptr[1] & 0x3F) << 6) | (ptr[2] & 0x3F);
            ptr += 3;
        } else {
            ch = ((c & 0x07) << 18) | ((ptr[1] & 0x3F) << 12) | 
                 ((ptr[2] & 0x3F) << 6) | (ptr[3] & 0x3F);
            ptr += 4;
        }
        
        text += ch;
    }
    
    insertText(text);
}

void TextEditor::handleMouseButtonDown(const SDL_MouseButtonEvent& event) {
    if (event.button != SDL_BUTTON_LEFT) {
        return;
    }
    
    // Calculate line and column from mouse position
    int lineHeight = m_document.getLineHeight();
    size_t line = static_cast<size_t>((event.y + m_scrollY) / lineHeight);
    
    // Simple column calculation (fixed width)
    size_t col = static_cast<size_t>((event.x + m_scrollX - 10) / 8);
    
    setCursorPosition(line, col);
    m_isSelecting = true;
    m_selectionStartLine = line;
    m_selectionStartCol = col;
}

void TextEditor::handleMouseMotion(const SDL_MouseMotionEvent& event) {
    if (!m_isSelecting) {
        return;
    }
    
    int lineHeight = m_document.getLineHeight();
    size_t line = static_cast<size_t>((event.y + m_scrollY) / lineHeight);
    size_t col = static_cast<size_t>((event.x + m_scrollX - 10) / 8);
    
    selectText(m_selectionStartLine, m_selectionStartCol, line, col);
}

void TextEditor::update(float deltaTime) {
    // Cursor blinking
    m_cursorBlinkTimer += deltaTime;
    if (m_cursorBlinkTimer >= m_cursorBlinkInterval) {
        m_cursorBlinkTimer = 0.0;
        m_cursorVisible = !m_cursorVisible;
    }
}

void TextEditor::render(SDL_Renderer* renderer, int viewportWidth, int viewportHeight) {
    m_renderer.render(m_document, renderer, m_scrollX, m_scrollY, viewportWidth, viewportHeight);
    
    // Render cursor
    int lineHeight = m_document.getLineHeight();
    int cursorX = 10 + static_cast<int>(m_cursorCol) * 8 - m_scrollX;
    int cursorY = static_cast<int>(m_cursorLine) * lineHeight - m_scrollY;
    
    m_renderer.renderCursor(renderer, cursorX, cursorY, lineHeight, m_cursorVisible);
}

void TextEditor::setCursorPosition(size_t line, size_t col) {
    m_cursorLine = std::min(line, m_document.getLineCount() - 1);
    
    const TextLine* currentLine = m_document.getLine(m_cursorLine);
    if (currentLine) {
        m_cursorCol = std::min(col, currentLine->length());
    } else {
        m_cursorCol = 0;
    }
    
    ensureCursorVisible();
}

void TextEditor::moveCursor(int deltaX, int deltaY) {
    if (deltaY != 0) {
        int newLine = static_cast<int>(m_cursorLine) + deltaY;
        if (newLine >= 0 && newLine < static_cast<int>(m_document.getLineCount())) {
            m_cursorLine = static_cast<size_t>(newLine);
            
            // Adjust column to fit new line length
            const TextLine* line = m_document.getLine(m_cursorLine);
            if (line && m_cursorCol > line->length()) {
                m_cursorCol = line->length();
            }
        }
    }
    
    if (deltaX != 0) {
        const TextLine* line = m_document.getLine(m_cursorLine);
        if (line) {
            int newCol = static_cast<int>(m_cursorCol) + deltaX;
            if (newCol >= 0) {
                m_cursorCol = std::min(static_cast<size_t>(newCol), line->length());
            }
        }
    }
    
    ensureCursorVisible();
}

void TextEditor::selectText(size_t startLine, size_t startCol, size_t endLine, size_t endCol) {
    m_selectionStartLine = startLine;
    m_selectionStartCol = startCol;
    m_selectionEndLine = endLine;
    m_selectionEndCol = endCol;
    m_isSelecting = true;
}

void TextEditor::insertText(const std::u32string& text) {
    if (text.empty()) {
        return;
    }
    
    // Save state for undo
    auto op = std::make_unique<InsertTextOperation>(m_cursorLine, m_cursorCol, text, TextStyle::None);
    
    m_document.insertText(m_cursorLine, m_cursorCol, text, TextStyle::None);
    
    // Update cursor position
    for (char32_t ch : text) {
        if (ch == U'\n') {
            m_cursorLine++;
            m_cursorCol = 0;
        } else {
            m_cursorCol++;
        }
    }
    
    m_undoManager.addOperation(std::move(op));
    ensureCursorVisible();
}

void TextEditor::deleteSelection() {
    if (!m_isSelecting) {
        return;
    }
    
    // Implement selection deletion
    m_isSelecting = false;
}

void TextEditor::deleteChar(bool forward) {
    const TextLine* line = m_document.getLine(m_cursorLine);
    if (!line) {
        return;
    }
    
    if (forward) {
        if (m_cursorCol < line->length()) {
            auto op = std::make_unique<DeleteTextOperation>(m_cursorLine, m_cursorCol, m_cursorCol + 1, U"");
            m_document.deleteText(m_cursorLine, m_cursorCol, m_cursorCol + 1);
            m_undoManager.addOperation(std::move(op));
        }
    } else {
        if (m_cursorCol > 0) {
            auto op = std::make_unique<DeleteTextOperation>(m_cursorLine, m_cursorCol - 1, m_cursorCol, U"");
            m_document.deleteText(m_cursorLine, m_cursorCol - 1, m_cursorCol);
            m_cursorCol--;
            m_undoManager.addOperation(std::move(op));
        }
    }
}

void TextEditor::undo() {
    if (m_undoManager.canUndo()) {
        m_undoManager.undo(m_document);
    }
}

void TextEditor::redo() {
    if (m_undoManager.canRedo()) {
        m_undoManager.redo(m_document);
    }
}

bool TextEditor::search(const SearchOptions& options) {
    if (options.pattern.empty()) {
        return false;
    }
    
    m_searchResults = m_document.search(options.pattern, options.caseSensitive);
    m_currentSearchResult = 0;
    
    if (!m_searchResults.empty()) {
        auto& result = m_searchResults[0];
        setCursorPosition(result.line, result.colStart);
        return true;
    }
    
    return false;
}

bool TextEditor::replaceNext(const SearchOptions& options) {
    if (m_searchResults.empty() || m_currentSearchResult >= m_searchResults.size()) {
        return false;
    }
    
    auto& result = m_searchResults[m_currentSearchResult];
    m_document.deleteText(result.line, result.colStart, result.colEnd);
    m_document.insertText(result.line, result.colStart, options.replacement, TextStyle::None);
    
    m_currentSearchResult++;
    return true;
}

int TextEditor::replaceAll(const SearchOptions& options) {
    auto results = m_document.search(options.pattern, options.caseSensitive);
    int count = 0;
    
    // Replace in reverse order to maintain indices
    for (auto it = results.rbegin(); it != results.rend(); ++it) {
        m_document.deleteText(it->line, it->colStart, it->colEnd);
        m_document.insertText(it->line, it->colStart, options.replacement, TextStyle::None);
        count++;
    }
    
    return count;
}

void TextEditor::toggleBold() {
    // Apply bold style to selection or current word
}

void TextEditor::toggleItalic() {
    // Apply italic style to selection or current word
}

void TextEditor::toggleUnderline() {
    // Apply underline style to selection or current word
}

void TextEditor::setFontSize(int size) {
    // Change font size for selected text
}

void TextEditor::setFontName(const std::string& name) {
    // Change font for selected text
}

void TextEditor::scrollToLine(size_t line) {
    m_scrollY = static_cast<int>(line) * m_document.getLineHeight();
    updateScrollPosition();
}

void TextEditor::zoomIn() {
    m_zoomLevel = std::min(m_zoomLevel + 0.1f, 3.0f);
    recalculateLayout();
}

void TextEditor::zoomOut() {
    m_zoomLevel = std::max(m_zoomLevel - 0.1f, 0.5f);
    recalculateLayout();
}

void TextEditor::resetZoom() {
    m_zoomLevel = 1.0f;
    recalculateLayout();
}

void TextEditor::ensureCursorVisible() {
    updateScrollPosition();
}

void TextEditor::updateScrollPosition() {
    int lineHeight = m_document.getLineHeight();
    int cursorY = static_cast<int>(m_cursorLine) * lineHeight;
    
    // Vertical scrolling
    if (cursorY < m_scrollY) {
        m_scrollY = cursorY;
    } else if (cursorY + lineHeight > m_scrollY + 600) {  // Assume 600px viewport
        m_scrollY = cursorY + lineHeight - 600;
    }
    
    // Horizontal scrolling
    int cursorX = 10 + static_cast<int>(m_cursorCol) * 8;
    if (cursorX < m_scrollX) {
        m_scrollX = cursorX;
    } else if (cursorX > m_scrollX + 800) {  // Assume 800px viewport
        m_scrollX = cursorX - 800;
    }
    
    m_scrollX = std::max(0, m_scrollX);
    m_scrollY = std::max(0, m_scrollY);
}

void TextEditor::recalculateLayout() {
    updateScrollPosition();
}

bool TextEditor::isInSelection(size_t line, size_t col) const {
    if (!m_isSelecting) {
        return false;
    }
    
    if (line < m_selectionStartLine || line > m_selectionEndLine) {
        return false;
    }
    
    if (line == m_selectionStartLine && col < m_selectionStartCol) {
        return false;
    }
    
    if (line == m_selectionEndLine && col >= m_selectionEndCol) {
        return false;
    }
    
    return true;
}

void TextEditor::copyToClipboard() {
    // Copy selected text to clipboard
}

void TextEditor::pasteFromClipboard() {
    // Paste text from clipboard
}

} // namespace liteedit
