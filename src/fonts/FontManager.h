#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace liteedit {

// Font metrics structure
struct FontMetrics {
    std::string name;
    int size;
    int ascent;
    int descent;
    int lineGap;
    int averageCharWidth;
    int maxHeight;
    
    int getLineHeight() const { return ascent + descent + lineGap; }
};

// Font manager with extended font support
class FontManager {
public:
    static FontManager& getInstance();
    
    // Initialize font system
    bool initialize();
    void shutdown();
    
    // Font registration
    bool loadFont(const std::string& path, const std::string& name);
    bool registerSystemFont(const std::string& name);
    
    // Font selection
    bool setFont(const std::string& name, int size);
    std::string getCurrentFont() const { return m_currentFont; }
    int getCurrentFontSize() const { return m_currentFontSize; }
    
    // Extended font list (predefined common fonts)
    std::vector<std::string> getAvailableFonts() const;
    std::vector<int> getAvailableSizes() const { 
        return {8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72}; 
    }
    
    // Text measurement
    int measureTextWidth(const std::string& text) const;
    int measureTextHeight() const;
    
    // Font metrics
    FontMetrics getFontMetrics(const std::string& name, int size) const;
    FontMetrics getCurrentMetrics() const;
    
    // Cache management
    void clearCache();
    
private:
    FontManager() = default;
    ~FontManager();
    
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    
    std::unordered_map<std::string, std::unordered_map<int, FontMetrics>> m_fontCache;
    std::vector<std::string> m_availableFonts;
    std::string m_currentFont;
    int m_currentFontSize;
    bool m_initialized;
    
    void populateSystemFonts();
    int estimateCharWidth(char c, const std::string& fontName, int size) const;
};

} // namespace liteedit

#endif // FONTMANAGER_H
