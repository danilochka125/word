#include "fonts/FontManager.h"
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#else
// For Unix-like systems, font loading would use FreeType or similar
#endif

namespace liteedit {

FontManager& FontManager::getInstance() {
    static FontManager instance;
    return instance;
}

bool FontManager::initialize() {
    if (m_initialized) {
        return true;
    }
    
    populateSystemFonts();
    m_currentFont = "Arial";
    m_currentFontSize = 12;
    m_initialized = true;
    
    return true;
}

void FontManager::shutdown() {
    clearCache();
    m_initialized = false;
}

bool FontManager::loadFont(const std::string& path, const std::string& name) {
    // In production, this would load font file using FreeType or platform API
    // For now, we register the name as available
    if (std::find(m_availableFonts.begin(), m_availableFonts.end(), name) == m_availableFonts.end()) {
        m_availableFonts.push_back(name);
    }
    return true;
}

bool FontManager::registerSystemFont(const std::string& name) {
    if (std::find(m_availableFonts.begin(), m_availableFonts.end(), name) == m_availableFonts.end()) {
        m_availableFonts.push_back(name);
    }
    return true;
}

bool FontManager::setFont(const std::string& name, int size) {
    if (size <= 0) {
        return false;
    }
    
    // Auto-register font if not exists
    if (std::find(m_availableFonts.begin(), m_availableFonts.end(), name) == m_availableFonts.end()) {
        m_availableFonts.push_back(name);
    }
    
    m_currentFont = name;
    m_currentFontSize = size;
    return true;
}

std::vector<std::string> FontManager::getAvailableFonts() const {
    // Extended list of common fonts for cross-platform compatibility
    std::vector<std::string> fonts = m_availableFonts;
    
    // Add standard fonts if not already present
    auto addIfMissing = [&fonts](const std::string& font) {
        if (std::find(fonts.begin(), fonts.end(), font) == fonts.end()) {
            fonts.push_back(font);
        }
    };
    
    // Serif fonts
    addIfMissing("Times New Roman");
    addIfMissing("Georgia");
    addIfMissing("Palatino Linotype");
    addIfMissing("Book Antiqua");
    addIfMissing("Garamond");
    addIfMissing("Baskerville");
    addIfMissing("Cambria");
    
    // Sans-serif fonts
    addIfMissing("Arial");
    addIfMissing("Helvetica");
    addIfMissing("Verdana");
    addIfMissing("Tahoma");
    addIfMissing("Trebuchet MS");
    addIfMissing("Impact");
    addIfMissing("Comic Sans MS");
    addIfMissing("Calibri");
    addIfMissing("Segoe UI");
    addIfMissing("Open Sans");
    addIfMissing("Roboto");
    addIfMissing("Lato");
    addIfMissing("Noto Sans");
    
    // Monospace fonts
    addIfMissing("Courier New");
    addIfMissing("Consolas");
    addIfMissing("Lucida Console");
    addIfMissing("Monaco");
    addIfMissing("DejaVu Sans Mono");
    addIfMissing("Source Code Pro");
    
    // Decorative/Display fonts
    addIfMissing("Brush Script MT");
    addIfMissing("Lucida Handwriting");
    addIfMissing("Papyrus");
    
    return fonts;
}

int FontManager::measureTextWidth(const std::string& text) const {
    if (text.empty()) {
        return 0;
    }
    
    int totalWidth = 0;
    for (char c : text) {
        totalWidth += estimateCharWidth(c, m_currentFont, m_currentFontSize);
    }
    
    return totalWidth;
}

int FontManager::measureTextHeight() const {
    auto metrics = getCurrentMetrics();
    return metrics.getLineHeight();
}

FontMetrics FontManager::getFontMetrics(const std::string& name, int size) const {
    // Check cache first
    auto fontIt = m_fontCache.find(name);
    if (fontIt != m_fontCache.end()) {
        auto sizeIt = fontIt->second.find(size);
        if (sizeIt != fontIt->second.end()) {
            return sizeIt->second;
        }
    }
    
    // Calculate metrics based on font characteristics
    FontMetrics metrics;
    metrics.name = name;
    metrics.size = size;
    
    // Estimate metrics based on font type and size
    // In production, this would query the actual font file
    bool isMonospace = (name.find("Mono") != std::string::npos || 
                        name.find("Courier") != std::string::npos ||
                        name.find("Consolas") != std::string::npos);
    
    bool isSerif = (name.find("Times") != std::string::npos ||
                    name.find("Georgia") != std::string::npos ||
                    name.find("Garamond") != std::string::npos ||
                    name.find("Palatino") != std::string::npos);
    
    // Base metrics scaled by font size
    double scaleFactor = size / 12.0;
    
    if (isMonospace) {
        metrics.averageCharWidth = static_cast<int>(8 * scaleFactor);
        metrics.ascent = static_cast<int>(10 * scaleFactor);
        metrics.descent = static_cast<int>(3 * scaleFactor);
        metrics.lineGap = static_cast<int>(2 * scaleFactor);
    } else if (isSerif) {
        metrics.averageCharWidth = static_cast<int>(7 * scaleFactor);
        metrics.ascent = static_cast<int>(11 * scaleFactor);
        metrics.descent = static_cast<int>(4 * scaleFactor);
        metrics.lineGap = static_cast<int>(2 * scaleFactor);
    } else {
        // Sans-serif (default)
        metrics.averageCharWidth = static_cast<int>(7 * scaleFactor);
        metrics.ascent = static_cast<int>(10 * scaleFactor);
        metrics.descent = static_cast<int>(3 * scaleFactor);
        metrics.lineGap = static_cast<int>(2 * scaleFactor);
    }
    
    metrics.maxHeight = metrics.ascent + metrics.descent;
    
    return metrics;
}

FontMetrics FontManager::getCurrentMetrics() const {
    return getFontMetrics(m_currentFont, m_currentFontSize);
}

void FontManager::clearCache() {
    m_fontCache.clear();
}

void FontManager::populateSystemFonts() {
    // Add basic set of fonts that are likely available on most systems
    m_availableFonts = {
        "Arial",
        "Times New Roman",
        "Courier New",
        "Verdana",
        "Georgia",
        "Tahoma",
        "Trebuchet MS",
        "Impact",
        "Comic Sans MS"
    };
    
    // Platform-specific fonts could be detected here
#ifdef _WIN32
    // Windows-specific fonts would be enumerated using GDI
#elif defined(__APPLE__)
    // macOS fonts via Core Text
#else
    // Linux fonts via fontconfig
#endif
}

int FontManager::estimateCharWidth(char c, const std::string& fontName, int size) const {
    // Simple estimation based on character type
    // In production, this would use actual font metrics
    
    double baseWidth;
    
    if (c >= 'A' && c <= 'Z') {
        baseWidth = 1.0;
    } else if (c >= 'a' && c <= 'z') {
        baseWidth = 0.85;
    } else if (c >= '0' && c <= '9') {
        baseWidth = 0.9;
    } else if (c == ' ' || c == '\t') {
        baseWidth = 0.5;
    } else if (c == 'i' || c == 'l' || c == '1') {
        baseWidth = 0.4;
    } else if (c == 'm' || c == 'w') {
        baseWidth = 1.3;
    } else {
        baseWidth = 0.7;
    }
    
    // Adjust for monospace fonts
    if (fontName.find("Mono") != std::string::npos ||
        fontName.find("Courier") != std::string::npos ||
        fontName.find("Consolas") != std::string::npos) {
        baseWidth = 1.0;
    }
    
    auto metrics = getFontMetrics(fontName, size);
    return static_cast<int>(baseWidth * metrics.averageCharWidth);
}

FontManager::~FontManager() {
    shutdown();
}

} // namespace liteedit
