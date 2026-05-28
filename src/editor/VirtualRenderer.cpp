#include "editor/VirtualRenderer.h"
#include <SDL2/SDL_ttf.h>
#include <chrono>
#include <algorithm>

namespace liteedit {

FontCache& FontCache::instance() {
    static FontCache instance;
    return instance;
}

FontCache::~FontCache() {
    clear();
}

void* FontCache::getFont(const std::string& name, int size) {
    FontKey key{name, size};
    
    auto it = m_fonts.find(key);
    if (it != m_fonts.end()) {
        return it->second;
    }
    
    // In real implementation, load font here
    // For now, return nullptr as placeholder
    void* font = nullptr;  // TTF_OpenFont would be called here
    m_fonts[key] = font;
    return font;
}

void FontCache::clear() {
    for (auto& pair : m_fonts) {
        if (pair.second) {
            // TTF_CloseFont(pair.second);
        }
    }
    m_fonts.clear();
}

VirtualRenderer::VirtualRenderer()
    : m_colorScheme(ColorScheme::light())
    , m_initialized(false) {
    m_stats = {0, 0, 0.0};
}

VirtualRenderer::~VirtualRenderer() {
    shutdown();
}

bool VirtualRenderer::initialize(int windowWidth, int windowHeight) {
    // Initialize SDL_ttf in real implementation
    // TTF_Init();
    m_initialized = true;
    return true;
}

void VirtualRenderer::shutdown() {
    if (m_initialized) {
        FontCache::instance().clear();
        // TTF_Quit();
        m_initialized = false;
    }
}

void VirtualRenderer::setVisibleRange(int scrollY, int viewportHeight, int lineHeight,
                                     size_t& firstVisibleLine, size_t& lastVisibleLine) {
    if (lineHeight <= 0) {
        firstVisibleLine = 0;
        lastVisibleLine = 0;
        return;
    }
    
    firstVisibleLine = static_cast<size_t>(scrollY / lineHeight);
    lastVisibleLine = firstVisibleLine + static_cast<size_t>((viewportHeight / lineHeight) + 1);
}

void VirtualRenderer::render(Document& doc, SDL_Renderer* renderer,
                            int scrollX, int scrollY,
                            int viewportWidth, int viewportHeight) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    m_stats.linesRendered = 0;
    m_stats.charsRendered = 0;
    
    // Clear background
    SDL_SetRenderDrawColor(renderer, m_colorScheme.background.r,
                          m_colorScheme.background.g,
                          m_colorScheme.background.b,
                          m_colorScheme.background.a);
    SDL_RenderClear(renderer);
    
    int lineHeight = doc.getLineHeight();
    size_t firstVisible, lastVisible;
    setVisibleRange(scrollY, viewportHeight, lineHeight, firstVisible, lastVisible);
    
    // Render only visible lines (virtualization)
    int yOffset = -scrollY;
    for (size_t lineIdx = firstVisible; lineIdx < lastVisible && lineIdx < doc.getLineCount(); ++lineIdx) {
        const TextLine* line = doc.getLine(lineIdx);
        if (!line) continue;
        
        int xPos = 10 - scrollX;  // Left margin
        int yPos = yOffset + static_cast<int>(lineIdx) * lineHeight;
        
        // Check if line is within viewport
        if (yPos + lineHeight < 0 || yPos > viewportHeight) {
            yOffset += lineHeight;
            continue;
        }
        
        renderLine(renderer, line, xPos, yPos, scrollX, scrollX + viewportWidth);
        m_stats.linesRendered++;
        
        yOffset += lineHeight;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.renderTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
}

void VirtualRenderer::renderLine(SDL_Renderer* renderer, const TextLine* line, int x, int y,
                                int clipLeft, int clipRight) {
    if (!line || line->length() == 0) {
        return;
    }
    
    const auto& chars = line->getChars();
    if (chars.empty()) {
        return;
    }
    
    // Simple text rendering - in production, use SDL_ttf
    int currentX = x;
    int charWidth = 8;  // Fixed width for simplicity
    int lineHeight = 16;  // Fixed height for simplicity
    
    for (const auto& ch : chars) {
        // Clip horizontally
        if (currentX + charWidth < clipLeft) {
            currentX += charWidth;
            continue;
        }
        if (currentX > clipRight) {
            break;
        }
        
        // Set color based on style
        SDL_Color color = m_colorScheme.text;
        
        // Draw character (placeholder - use SDL_ttf in production)
        if (ch.character >= 32 && ch.character < 127) {
            // Simple ASCII rendering using SDL primitives
            // In production, use TTF_RenderText_Blended
            
            // For demonstration, draw a rectangle for each character
            SDL_Rect rect = {currentX, y, charWidth - 1, lineHeight - 2};
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &rect);
            
            m_stats.charsRendered++;
        }
        
        currentX += charWidth;
    }
}

int VirtualRenderer::measureTextWidth(const std::vector<StyledChar>& chars, int start, int count) {
    if (chars.empty()) {
        return 0;
    }
    
    int actualCount = (count < 0) ? static_cast<int>(chars.size()) : count;
    actualCount = std::min(actualCount, static_cast<int>(chars.size()) - start);
    
    return actualCount * 8;  // Fixed width approximation
}

int VirtualRenderer::measureTextHeight(const std::vector<StyledChar>& chars) {
    return 16;  // Fixed height approximation
}

void VirtualRenderer::renderCursor(SDL_Renderer* renderer, int x, int y, int height, bool isVisible) {
    if (!isVisible) {
        return;
    }
    
    SDL_SetRenderDrawColor(renderer, m_colorScheme.cursor.r,
                          m_colorScheme.cursor.g,
                          m_colorScheme.cursor.b,
                          m_colorScheme.cursor.a);
    
    SDL_Rect cursorRect = {x, y, 2, height};
    SDL_RenderFillRect(renderer, &cursorRect);
}

void VirtualRenderer::renderSelection(SDL_Renderer* renderer, int x, int y, int width, int height) {
    SDL_SetRenderDrawColor(renderer, m_colorScheme.selection.r,
                          m_colorScheme.selection.g,
                          m_colorScheme.selection.b,
                          m_colorScheme.selection.a);
    
    SDL_Rect selRect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &selRect);
}

void VirtualRenderer::drawText(SDL_Renderer* renderer, const std::vector<StyledChar>& chars,
                              int x, int y, int maxWidth) {
    // Placeholder for text drawing
    // In production, use SDL_ttf with proper font caching
}

} // namespace liteedit
