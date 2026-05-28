#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace liteedit {

// Text formatting flags
enum class TextStyle : uint8_t {
    None = 0,
    Bold = 1 << 0,
    Italic = 1 << 1,
    Underline = 1 << 2
};

inline TextStyle operator|(TextStyle a, TextStyle b) {
    return static_cast<TextStyle>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline TextStyle operator&(TextStyle a, TextStyle b) {
    return static_cast<TextStyle>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

// Single character with formatting
struct StyledChar {
    char32_t character;
    TextStyle style;
    std::string fontName;
    int fontSize;
    
    StyledChar() : character(0), style(TextStyle::None), fontName("Arial"), fontSize(12) {}
    StyledChar(char32_t c, TextStyle s = TextStyle::None, const std::string& font = "Arial", int size = 12)
        : character(c), style(s), fontName(font), fontSize(size) {}
};

// Line of text (optimized for lazy loading)
class TextLine {
public:
    TextLine() : m_isDirty(true), m_cachedWidth(0), m_cachedHeight(0) {}
    
    void appendChar(const StyledChar& ch);
    void insertChar(size_t pos, const StyledChar& ch);
    void removeChar(size_t pos);
    
    const std::vector<StyledChar>& getChars() const { return m_chars; }
    size_t length() const { return m_chars.size(); }
    
    bool isDirty() const { return m_isDirty; }
    void markClean() { m_isDirty = false; }
    void markDirty() { m_isDirty = true; }
    
    int getCachedWidth() const { return m_cachedWidth; }
    int getCachedHeight() const { return m_cachedHeight; }
    void setCachedMetrics(int width, int height) { 
        m_cachedWidth = width; 
        m_cachedHeight = height; 
    }
    
private:
    std::vector<StyledChar> m_chars;
    bool m_isDirty;
    mutable int m_cachedWidth;
    mutable int m_cachedHeight;
};

// Main document class with virtualized storage
class Document {
public:
    Document();
    ~Document();
    
    // Copy constructor and assignment (needed for vector operations)
    Document(const Document& other);
    Document& operator=(const Document& other);
    
    // Move constructor and assignment
    Document(Document&& other) noexcept;
    Document& operator=(Document&& other) noexcept;
    
    // File operations
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path);
    bool loadPlainText(const std::string& text);
    
    // Content access with lazy loading
    TextLine* getLine(size_t index);
    const TextLine* getLine(size_t index) const;
    size_t getLineCount() const { return m_lines.size(); }
    
    // Editing operations
    void insertText(size_t line, size_t col, const std::u32string& text, TextStyle style);
    void deleteText(size_t line, size_t colStart, size_t colEnd);
    void applyStyle(size_t lineStart, size_t colStart, size_t lineEnd, size_t colEnd, TextStyle style);
    
    // Search
    struct SearchResult {
        size_t line;
        size_t colStart;
        size_t colEnd;
    };
    std::vector<SearchResult> search(const std::u32string& pattern, bool caseSensitive = true) const;
    
    // Document state
    bool isModified() const { return m_isModified; }
    void setModified(bool modified) { m_isModified = modified; }
    const std::string& getFilePath() const { return m_filePath; }
    
    // Metrics (for virtualization)
    int getLineHeight() const { return m_defaultLineHeight; }
    int getCharWidth() const { return m_defaultCharWidth; }
    
private:
    std::vector<std::unique_ptr<TextLine>> m_lines;
    std::string m_filePath;
    bool m_isModified;
    
    // Default metrics for virtualization
    int m_defaultLineHeight;
    int m_defaultCharWidth;
    
    void ensureMinimumLines();
    void recalculateMetrics();
};

} // namespace liteedit

#endif // DOCUMENT_H
