#include "formats/DocxHandler.h"
#include <sstream>
#include <algorithm>
#include <cstring>

// Include pugixml for XML parsing
#include "pugixml.hpp"

// Include minizip for ZIP extraction
extern "C" {
#include "mz.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"
}

namespace liteedit {

DocxHandler::DocxHandler() {}

DocxHandler::~DocxHandler() {}

bool DocxHandler::loadFromFile(const std::string& path, DocxDocument& doc) {
    m_lastError.clear();
    
    // DOCX is a ZIP archive containing XML files
    // Main document content is in word/document.xml
    
    std::string documentXml;
    if (!extractZipFile(path, "word/document.xml", documentXml)) {
        m_lastError = "Failed to extract word/document.xml from DOCX";
        return false;
    }
    
    if (!parseDocumentXml(documentXml, doc)) {
        m_lastError = "Failed to parse document.xml";
        return false;
    }
    
    // Try to extract core properties (optional)
    std::string corePropsXml;
    if (extractZipFile(path, "docProps/core.xml", corePropsXml)) {
        parseCoreProperties(corePropsXml, doc);
    }
    
    return true;
}

bool DocxHandler::saveToFile(const std::string& path, const DocxDocument& doc) {
    m_lastError.clear();
    
    // Creating a valid DOCX file requires:
    // 1. Multiple XML files with specific structure
    // 2. [Content_Types].xml
    // 3. _rels/.rels
    // 4. word/document.xml
    // 5. word/styles.xml
    // 6. docProps/core.xml
    // 7. ZIP packaging with specific order
    
    // This is a complex operation. For MVP, we provide basic implementation
    // In production, consider using a dedicated library like docx4j or OpenXML SDK
    
    // For now, return false to indicate this feature needs full implementation
    m_lastError = "DOCX saving requires full implementation with ZIP creation";
    return false;
}

bool DocxHandler::extractPlainText(const std::string& path, std::string& text) {
    DocxDocument doc;
    DocxHandler handler;
    
    if (!handler.loadFromFile(path, doc)) {
        return false;
    }
    
    // Concatenate all paragraphs
    std::stringstream ss;
    for (size_t i = 0; i < doc.paragraphs.size(); ++i) {
        ss << doc.paragraphs[i].text;
        if (i < doc.paragraphs.size() - 1) {
            ss << "\n";
        }
    }
    
    text = ss.str();
    return true;
}

bool DocxHandler::extractZipFile(const std::string& zipPath, const std::string& fileName, 
                                  std::string& content) {
    // Use minizip to extract file from ZIP archive
    void* zipHandle = mz_zip_reader_create();
    if (!zipHandle) {
        return false;
    }
    
    int32_t err = mz_zip_reader_open_file(zipHandle, zipPath.c_str());
    if (err != MZ_OK) {
        mz_zip_reader_delete(&zipHandle);
        return false;
    }
    
    // Find the file in the archive
    err = mz_zip_reader_locate_entry(zipHandle, fileName.c_str(), 0);
    if (err != MZ_OK) {
        mz_zip_reader_delete(&zipHandle);
        return false;
    }
    
    // Get entry info
    mz_zip_file* fileInfo = nullptr;
    err = mz_zip_reader_entry_get_info(zipHandle, &fileInfo);
    if (err != MZ_OK || !fileInfo) {
        mz_zip_reader_delete(&zipHandle);
        return false;
    }
    
    // Open entry for reading
    err = mz_zip_reader_entry_open(zipHandle);
    if (err != MZ_OK) {
        mz_zip_reader_delete(&zipHandle);
        return false;
    }
    
    // Read content
    std::vector<char> buffer(fileInfo->uncompressed_size + 1);
    int32_t bytesRead = mz_zip_reader_entry_read(zipHandle, buffer.data(), buffer.size());
    
    mz_zip_reader_entry_close(zipHandle);
    mz_zip_reader_delete(&zipHandle);
    
    if (bytesRead < 0) {
        return false;
    }
    
    buffer[bytesRead] = '\0';
    content.assign(buffer.begin(), buffer.begin() + bytesRead);
    
    return true;
}

bool DocxHandler::parseDocumentXml(const std::string& xmlContent, DocxDocument& doc) {
    pugi::xml_document xmlDoc;
    pugi::xml_parse_result result = xmlDoc.load_string(xmlContent.c_str());
    
    if (!result) {
        return false;
    }
    
    // DOCX uses Office Open XML namespace
    const char* wNamespace = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    
    // Find document root
    pugi::xml_node document = xmlDoc.child("w:document");
    if (!document) {
        // Try without namespace prefix
        document = xmlDoc.first_child();
    }
    
    if (!document) {
        return false;
    }
    
    // Iterate through paragraphs
    for (pugi::xml_node para : document.children()) {
        std::string nodeName = para.name();
        
        // Handle namespaced and non-namespaced nodes
        if (nodeName != "w:p" && nodeName != "p") {
            continue;
        }
        
        // Extract text from runs within paragraph
        std::string paragraphText;
        bool isBold = false;
        bool isItalic = false;
        bool isUnderline = false;
        std::string fontName = "Arial";
        int fontSize = 12;
        
        for (pugi::xml_node run : para.children()) {
            std::string runName = run.name();
            if (runName != "w:r" && runName != "r") {
                continue;
            }
            
            // Check formatting in run properties
            pugi::xml_node rPr = run.child("w:rPr");
            if (!rPr) {
                rPr = run.child("rPr");
            }
            
            if (rPr) {
                // Check bold
                if (rPr.child("w:b") || rPr.child("b")) {
                    isBold = true;
                }
                
                // Check italic
                if (rPr.child("w:i") || rPr.child("i")) {
                    isItalic = true;
                }
                
                // Check underline
                if (rPr.child("w:u") || rPr.child("u")) {
                    isUnderline = true;
                }
                
                // Check font
                pugi::xml_node fontNode = rPr.child("w:rFonts");
                if (!fontNode) {
                    fontNode = rPr.child("rFonts");
                }
                if (fontNode) {
                    const char* asciiFont = fontNode.attribute("w:ascii").value();
                    if (!asciiFont) {
                        asciiFont = fontNode.attribute("ascii").value();
                    }
                    if (asciiFont && strlen(asciiFont) > 0) {
                        fontName = asciiFont;
                    }
                }
                
                // Check font size
                pugi::xml_node szNode = rPr.child("w:sz");
                if (!szNode) {
                    szNode = rPr.child("sz");
                }
                if (szNode) {
                    const char* val = szNode.attribute("w:val").value();
                    if (!val) {
                        val = szNode.attribute("val").value();
                    }
                    if (val) {
                        fontSize = atoi(val) / 2; // Font size is stored as half-points
                    }
                }
            }
            
            // Extract text from text element
            pugi::xml_node textNode = run.child("w:t");
            if (!textNode) {
                textNode = run.child("t");
            }
            
            if (textNode && textNode.value()) {
                paragraphText += unescapeXml(textNode.value());
            }
        }
        
        if (!paragraphText.empty()) {
            DocxParagraph paraStruct(paragraphText, isBold, isItalic, isUnderline);
            paraStruct.fontName = fontName;
            paraStruct.fontSize = fontSize;
            doc.paragraphs.push_back(paraStruct);
        }
    }
    
    return true;
}

bool DocxHandler::parseCoreProperties(const std::string& xmlContent, DocxDocument& doc) {
    pugi::xml_document xmlDoc;
    pugi::xml_parse_result result = xmlDoc.load_string(xmlContent.c_str());
    
    if (!result) {
        return false;
    }
    
    // Core properties namespace
    pugi::xml_node root = xmlDoc.first_child();
    if (!root) {
        return false;
    }
    
    // Extract title
    pugi::xml_node titleNode = root.child("dc:title");
    if (titleNode && titleNode.value()) {
        doc.title = titleNode.value();
    }
    
    // Extract creator
    pugi::xml_node creatorNode = root.child("dc:creator");
    if (creatorNode && creatorNode.value()) {
        doc.creator = creatorNode.value();
    }
    
    // Extract last modified date
    pugi::xml_node modifiedNode = root.child("cp:lastModifiedBy");
    if (modifiedNode && modifiedNode.value()) {
        doc.lastModified = modifiedNode.value();
    }
    
    return true;
}

std::string DocxHandler::unescapeXml(const std::string& text) {
    std::string result = text;
    
    // Replace XML entities
    size_t pos = 0;
    while ((pos = result.find("&lt;", pos)) != std::string::npos) {
        result.replace(pos, 4, "<");
        pos++;
    }
    
    pos = 0;
    while ((pos = result.find("&gt;", pos)) != std::string::npos) {
        result.replace(pos, 4, ">");
        pos++;
    }
    
    pos = 0;
    while ((pos = result.find("&amp;", pos)) != std::string::npos) {
        result.replace(pos, 5, "&");
        pos++;
    }
    
    pos = 0;
    while ((pos = result.find("&quot;", pos)) != std::string::npos) {
        result.replace(pos, 6, "\"");
        pos++;
    }
    
    pos = 0;
    while ((pos = result.find("&apos;", pos)) != std::string::npos) {
        result.replace(pos, 6, "'");
        pos++;
    }
    
    // Handle special characters (tab, newline, carriage return)
    pos = 0;
    while ((pos = result.find("&#9;", pos)) != std::string::npos) {
        result.replace(pos, 5, "\t");
        pos++;
    }
    
    pos = 0;
    while ((pos = result.find("&#10;", pos)) != std::string::npos) {
        result.replace(pos, 5, "\n");
        pos++;
    }
    
    pos = 0;
    while ((pos = result.find("&#13;", pos)) != std::string::npos) {
        result.replace(pos, 5, "\r");
        pos++;
    }
    
    return result;
}

void DocxHandler::processTextElement(const std::string& text, bool bold, bool italic, 
                                      bool underline, const std::string& font, int size,
                                      std::vector<DocxParagraph>& paragraphs) {
    if (text.empty()) {
        return;
    }
    
    DocxParagraph para(text, bold, italic, underline);
    para.fontName = font;
    para.fontSize = size;
    paragraphs.push_back(para);
}

} // namespace liteedit
