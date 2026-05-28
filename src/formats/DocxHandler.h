#ifndef DOCXHANDLER_H
#define DOCXHANDLER_H

#include <string>
#include <vector>
#include <memory>

namespace liteedit {

// Structure to hold document content with formatting
struct DocxParagraph {
    std::string text;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    std::string fontName;
    int fontSize = 12;
    
    DocxParagraph() : fontName("Arial") {}
    DocxParagraph(const std::string& t, bool b = false, bool i = false, bool u = false)
        : text(t), bold(b), italic(i), underline(u) {}
};

struct DocxDocument {
    std::vector<DocxParagraph> paragraphs;
    std::string title;
    std::string creator;
    std::string lastModified;
};

// DOCX file handler using pugixml and minizip
class DocxHandler {
public:
    DocxHandler();
    ~DocxHandler();
    
    // Load DOCX file
    bool loadFromFile(const std::string& path, DocxDocument& doc);
    
    // Save to DOCX file
    bool saveToFile(const std::string& path, const DocxDocument& doc);
    
    // Extract plain text from DOCX (fallback mode)
    static bool extractPlainText(const std::string& path, std::string& text);
    
    // Get error message if operation failed
    std::string getLastError() const { return m_lastError; }
    
private:
    std::string m_lastError;
    
    // Internal helpers
    bool extractZipFile(const std::string& zipPath, const std::string& fileName, std::string& content);
    bool parseDocumentXml(const std::string& xmlContent, DocxDocument& doc);
    bool parseCoreProperties(const std::string& xmlContent, DocxDocument& doc);
    std::string unescapeXml(const std::string& text);
    void processTextElement(const std::string& text, bool bold, bool italic, bool underline, 
                           const std::string& font, int size, std::vector<DocxParagraph>& paragraphs);
};

} // namespace liteedit

#endif // DOCXHANDLER_H
