#include "core/Document.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

namespace liteedit {

void TextLine::appendChar(const StyledChar& ch) {
    m_chars.push_back(ch);
    markDirty();
}

void TextLine::insertChar(size_t pos, const StyledChar& ch) {
    if (pos > m_chars.size()) {
        pos = m_chars.size();
    }
    m_chars.insert(m_chars.begin() + pos, ch);
    markDirty();
}

void TextLine::removeChar(size_t pos) {
    if (pos < m_chars.size()) {
        m_chars.erase(m_chars.begin() + pos);
        markDirty();
    }
}

Document::Document() 
    : m_isModified(false)
    , m_defaultLineHeight(16)
    , m_defaultCharWidth(8) {
    ensureMinimumLines();
}

Document::~Document() = default;

Document::Document(const Document& other)
    : m_filePath(other.m_filePath)
    , m_isModified(other.m_isModified)
    , m_defaultLineHeight(other.m_defaultLineHeight)
    , m_defaultCharWidth(other.m_defaultCharWidth) {
    // Deep copy lines
    for (const auto& line : other.m_lines) {
        auto copy = std::make_unique<TextLine>();
        *copy = *line;
        m_lines.push_back(std::move(copy));
    }
}

Document& Document::operator=(const Document& other) {
    if (this != &other) {
        m_filePath = other.m_filePath;
        m_isModified = other.m_isModified;
        m_defaultLineHeight = other.m_defaultLineHeight;
        m_defaultCharWidth = other.m_defaultCharWidth;
        
        m_lines.clear();
        for (const auto& line : other.m_lines) {
            auto copy = std::make_unique<TextLine>();
            *copy = *line;
            m_lines.push_back(std::move(copy));
        }
    }
    return *this;
}

Document::Document(Document&& other) noexcept
    : m_lines(std::move(other.m_lines))
    , m_filePath(std::move(other.m_filePath))
    , m_isModified(other.m_isModified)
    , m_defaultLineHeight(other.m_defaultLineHeight)
    , m_defaultCharWidth(other.m_defaultCharWidth) {
}

Document& Document::operator=(Document&& other) noexcept {
    if (this != &other) {
        m_lines = std::move(other.m_lines);
        m_filePath = std::move(other.m_filePath);
        m_isModified = other.m_isModified;
        m_defaultLineHeight = other.m_defaultLineHeight;
        m_defaultCharWidth = other.m_defaultCharWidth;
    }
    return *this;
}

void Document::ensureMinimumLines() {
    if (m_lines.empty()) {
        m_lines.push_back(std::make_unique<TextLine>());
    }
}

bool Document::loadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    m_filePath = path;
    m_lines.clear();
    
    std::string line;
    while (std::getline(file, line)) {
        auto textLine = std::make_unique<TextLine>();
        
        // Convert to UTF-32 and create styled characters
        std::u32string utf32;
        for (size_t i = 0; i < line.size(); ) {
            char32_t ch;
            unsigned char c = line[i];
            
            if ((c & 0x80) == 0) {
                ch = c;
                i++;
            } else if ((c & 0xE0) == 0xC0) {
                ch = ((c & 0x1F) << 6) | (line[i+1] & 0x3F);
                i += 2;
            } else if ((c & 0xF0) == 0xE0) {
                ch = ((c & 0x0F) << 12) | ((line[i+1] & 0x3F) << 6) | (line[i+2] & 0x3F);
                i += 3;
            } else {
                ch = ((c & 0x07) << 18) | ((line[i+1] & 0x3F) << 12) | 
                     ((line[i+2] & 0x3F) << 6) | (line[i+3] & 0x3F);
                i += 4;
            }
            
            textLine->appendChar(StyledChar(ch));
        }
        
        m_lines.push_back(std::move(textLine));
    }
    
    ensureMinimumLines();
    m_isModified = false;
    recalculateMetrics();
    
    return true;
}

bool Document::saveToFile(const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    for (size_t i = 0; i < m_lines.size(); ++i) {
        const auto& line = m_lines[i];
        
        // Convert UTF-32 back to UTF-8
        for (const auto& ch : line->getChars()) {
            if (ch.character < 0x80) {
                file.put(static_cast<char>(ch.character));
            } else if (ch.character < 0x800) {
                file.put(static_cast<char>(0xC0 | (ch.character >> 6)));
                file.put(static_cast<char>(0x80 | (ch.character & 0x3F)));
            } else if (ch.character < 0x10000) {
                file.put(static_cast<char>(0xE0 | (ch.character >> 12)));
                file.put(static_cast<char>(0x80 | ((ch.character >> 6) & 0x3F)));
                file.put(static_cast<char>(0x80 | (ch.character & 0x3F)));
            } else {
                file.put(static_cast<char>(0xF0 | (ch.character >> 18)));
                file.put(static_cast<char>(0x80 | ((ch.character >> 12) & 0x3F)));
                file.put(static_cast<char>(0x80 | ((ch.character >> 6) & 0x3F)));
                file.put(static_cast<char>(0x80 | (ch.character & 0x3F)));
            }
        }
        
        if (i < m_lines.size() - 1) {
            file.put('\n');
        }
    }
    
    m_filePath = path;
    m_isModified = false;
    return true;
}

bool Document::loadPlainText(const std::string& text) {
    m_lines.clear();
    
    std::istringstream stream(text);
    std::string line;
    
    while (std::getline(stream, line)) {
        auto textLine = std::make_unique<TextLine>();
        for (unsigned char c : line) {
            textLine->appendChar(StyledChar(c));
        }
        m_lines.push_back(std::move(textLine));
    }
    
    ensureMinimumLines();
    m_isModified = false;
    recalculateMetrics();
    
    return true;
}

TextLine* Document::getLine(size_t index) {
    if (index >= m_lines.size()) {
        return nullptr;
    }
    return m_lines[index].get();
}

const TextLine* Document::getLine(size_t index) const {
    if (index >= m_lines.size()) {
        return nullptr;
    }
    return m_lines[index].get();
}

void Document::insertText(size_t line, size_t col, const std::u32string& text, TextStyle style) {
    if (line >= m_lines.size()) {
        return;
    }
    
    auto& currentLine = m_lines[line];
    
    for (char32_t ch : text) {
        if (ch == U'\n') {
            // Split line
            auto newLine = std::make_unique<TextLine>();
            
            // Move characters after cursor to new line
            while (currentLine->length() > col) {
                auto chars = currentLine->getChars();
                StyledChar ch = chars.back();
                newLine->insertChar(0, ch);
                currentLine->removeChar(currentLine->length() - 1);
            }
            
            m_lines.insert(m_lines.begin() + line + 1, std::move(newLine));
            col = 0;
        } else {
            currentLine->insertChar(col++, StyledChar(ch, style));
        }
    }
    
    m_isModified = true;
    recalculateMetrics();
}

void Document::deleteText(size_t line, size_t colStart, size_t colEnd) {
    if (line >= m_lines.size()) {
        return;
    }
    
    auto& currentLine = m_lines[line];
    
    // Delete characters in reverse order to maintain indices
    for (size_t i = colEnd; i > colStart && i <= currentLine->length(); --i) {
        if (i > 0) {
            currentLine->removeChar(i - 1);
        }
    }
    
    m_isModified = true;
    recalculateMetrics();
}

void Document::applyStyle(size_t lineStart, size_t colStart, size_t lineEnd, size_t colEnd, TextStyle style) {
    for (size_t line = lineStart; line <= lineEnd && line < m_lines.size(); ++line) {
        auto& textLine = m_lines[line];
        size_t startCol = (line == lineStart) ? colStart : 0;
        size_t endCol = (line == lineEnd) ? colEnd : textLine->length();
        
        for (size_t col = startCol; col < endCol && col < textLine->length(); ++col) {
            auto chars = const_cast<std::vector<StyledChar>&>(textLine->getChars());
            if (col < chars.size()) {
                chars[col].style = style;
                textLine->markDirty();
            }
        }
    }
    
    m_isModified = true;
}

std::vector<Document::SearchResult> Document::search(const std::u32string& pattern, bool caseSensitive) const {
    std::vector<SearchResult> results;
    
    if (pattern.empty()) {
        return results;
    }
    
    std::u32string searchPattern = pattern;
    if (!caseSensitive) {
        std::transform(searchPattern.begin(), searchPattern.end(), searchPattern.begin(),
            [](char32_t ch) { return static_cast<char32_t>(std::tolower(static_cast<int>(ch))); });
    }
    
    for (size_t lineIdx = 0; lineIdx < m_lines.size(); ++lineIdx) {
        const auto& line = m_lines[lineIdx];
        const auto& chars = line->getChars();
        
        std::u32string lineText;
        for (const auto& ch : chars) {
            lineText += ch.character;
        }
        
        std::u32string searchText = lineText;
        if (!caseSensitive) {
            std::transform(searchText.begin(), searchText.end(), searchText.begin(),
                [](char32_t ch) { return static_cast<char32_t>(std::tolower(static_cast<int>(ch))); });
        }
        
        size_t pos = 0;
        while ((pos = searchText.find(searchPattern, pos)) != std::u32string::npos) {
            SearchResult result;
            result.line = lineIdx;
            result.colStart = pos;
            result.colEnd = pos + pattern.length();
            results.push_back(result);
            pos++;
        }
    }
    
    return results;
}

void Document::recalculateMetrics() {
    // Simple metric calculation - can be enhanced with font metrics
    m_defaultLineHeight = 16;
    m_defaultCharWidth = 8;
}

} // namespace liteedit
