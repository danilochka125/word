#ifndef RTFHANDLER_H
#define RTFHANDLER_H

#include <string>
#include <vector>
#include <memory>

namespace liteedit {

// RTF document structure with formatting
struct RtfParagraph {
    std::string text;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool strikeout = false;
    std::string fontName;
    int fontSize = 24; // RTF uses half-points, so 24 = 12pt
    int red = 0, green = 0, blue = 0;
    
    RtfParagraph() : fontName("Arial") {}
};

struct RtfDocument {
    std::vector<RtfParagraph> paragraphs;
    std::string title;
    std::string author;
    std::string subject;
    int defaultFontSize = 24;
    std::string defaultFont = "Arial";
};

// RTF file handler with full parsing support
class RtfHandler {
public:
    RtfHandler();
    ~RtfHandler();
    
    // Load RTF file
    bool loadFromFile(const std::string& path, RtfDocument& doc);
    
    // Save to RTF file
    bool saveToFile(const std::string& path, const RtfDocument& doc);
    
    // Extract plain text from RTF
    static bool extractPlainText(const std::string& path, std::string& text);
    
    // Get error message if operation failed
    std::string getLastError() const { return m_lastError; }
    
private:
    std::string m_lastError;
    
    // RTF parser state
    struct ParserState {
        bool inControlWord = false;
        bool inControlSymbol = false;
        int braceDepth = 0;
        std::string currentControlWord;
        int parameter = 0;
        bool hasParameter = false;
        
        // Current formatting state
        bool bold = false;
        bool italic = false;
        bool underline = false;
        bool strikeout = false;
        std::string fontName;
        int fontSize = 24;
        int destDestination = 0; // 0=none, 1=author, 2=title, 3=subject
        
        // Color state
        int colorTableIndex = -1;
        std::vector<std::tuple<int,int,int>> colorTable;
        
        void reset() {
            inControlWord = false;
            inControlSymbol = false;
            braceDepth = 0;
            currentControlWord.clear();
            parameter = 0;
            hasParameter = false;
            bold = false;
            italic = false;
            underline = false;
            strikeout = false;
            fontName = "Arial";
            fontSize = 24;
            destDestination = 0;
            colorTableIndex = -1;
            colorTable.clear();
        }
    };
    
    // Parser methods
    bool parseRtf(const std::string& rtfContent, RtfDocument& doc);
    void processControlWord(ParserState& state, RtfDocument& doc, 
                           std::vector<RtfParagraph>& paragraphs, std::string& currentText);
    void processControlSymbol(ParserState& state, RtfDocument& doc,
                             std::vector<RtfParagraph>& paragraphs, std::string& currentText);
    std::string unescapeRtf(const std::string& text);
    std::string escapeRtf(const std::string& text);
};

} // namespace liteedit

#endif // RTFHANDLER_H
