#ifndef VIRTUALRENDERER_H
#define VIRTUALRENDERER_H

#include "core/Document.h"
#include <SDL2/SDL.h>
#include <vector>
#include <string>

namespace liteedit {

// Font cache for efficient font management
class FontCache {
public:
    static FontCache& instance();
    
    void* getFont(const std::string& name, int size);
    void clear();
    
private:
    FontCache() = default;
    ~FontCache();
    
    struct FontKey {
        std::string name;
        int size;
        
        bool operator==(const FontKey& other) const {
            return name == other.name && size == other.size;
        }
    };
    
    struct FontKeyHash {
        size_t operator()(const FontKey& key) const {
            return std::hash<std::string>()(key.name) ^ (size_t)key.size;
        }
    };
    
    std::unordered_map<FontKey, void*, FontKeyHash> m_fonts;
};

// Virtual renderer with viewport-based rendering
class VirtualRenderer {
public:
    VirtualRenderer();
    ~VirtualRenderer();
    
    bool initialize(int windowWidth, int windowHeight);
    void shutdown();
    
    // Render only visible portion
    void render(Document& doc, SDL_Renderer* renderer, 
                int scrollX, int scrollY, 
                int viewportWidth, int viewportHeight);
    
    // Text measurement
    int measureTextWidth(const std::vector<StyledChar>& chars, int start, int count);
    int measureTextHeight(const std::vector<StyledChar>& chars);
    
    // Cursor rendering
    void renderCursor(SDL_Renderer* renderer, int x, int y, int height, bool isVisible);
    
    // Selection rendering
    void renderSelection(SDL_Renderer* renderer, int x, int y, int width, int height);
    
    // Viewport calculations
    void setVisibleRange(int scrollY, int viewportHeight, int lineHeight,
                        size_t& firstVisibleLine, size_t& lastVisibleLine);
    
    // Color scheme
    struct ColorScheme {
        SDL_Color background;
        SDL_Color text;
        SDL_Color selection;
        SDL_Color cursor;
        SDL_Color lineNumber;
        
        static ColorScheme light() {
            return {
                {255, 255, 255, 255},
                {0, 0, 0, 255},
                {200, 220, 255, 255},
                {0, 0, 0, 255},
                {128, 128, 128, 255}
            };
        }
        
        static ColorScheme dark() {
            return {
                {30, 30, 30, 255},
                {220, 220, 220, 255},
                {60, 80, 120, 255},
                {255, 255, 255, 255},
                {150, 150, 150, 255}
            };
        }
    };
    
    void setColorScheme(const ColorScheme& scheme) { m_colorScheme = scheme; }
    const ColorScheme& getColorScheme() const { return m_colorScheme; }
    
    // Performance metrics
    struct RenderStats {
        size_t linesRendered;
        size_t charsRendered;
        double renderTimeMs;
    };
    
    const RenderStats& getStats() const { return m_stats; }
    
private:
    ColorScheme m_colorScheme;
    RenderStats m_stats;
    bool m_initialized;
    
    void renderLine(SDL_Renderer* renderer, const TextLine* line, int x, int y,
                   int clipLeft, int clipRight);
    void drawText(SDL_Renderer* renderer, const std::vector<StyledChar>& chars,
                 int x, int y, int maxWidth);
};

} // namespace liteedit

#endif // VIRTUALRENDERER_H
