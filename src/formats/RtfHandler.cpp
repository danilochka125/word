#include "formats/RtfHandler.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <tuple>

namespace liteedit {

RtfHandler::RtfHandler() {}

RtfHandler::~RtfHandler() {}

bool RtfHandler::loadFromFile(const std::string& path, RtfDocument& doc) {
    m_lastError.clear();
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        m_lastError = "Failed to open RTF file: " + path;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    return parseRtf(content, doc);
}

bool RtfHandler::saveToFile(const std::string& path, const RtfDocument& doc) {
    m_lastError.clear();
    
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        m_lastError = "Failed to create RTF file: " + path;
        return false;
    }
    
    // Write RTF header
    file << "{\\rtf1\\ansi\\deff0\n";
    
    // Font table
    file << "{\\fonttbl\n";
    file << "{\\f0\\fswiss\\fcharset0 " << escapeRtf(doc.defaultFont) << ";}\n";
    
    // Add fonts from paragraphs
    std::vector<std::string> addedFonts = {doc.defaultFont};
    int fontIndex = 1;
    for (const auto& para : doc.paragraphs) {
        if (std::find(addedFonts.begin(), addedFonts.end(), para.fontName) == addedFonts.end()) {
            file << "{\\f" << fontIndex << "\\fnil\\fcharset0 " << escapeRtf(para.fontName) << ";}\n";
            addedFonts.push_back(para.fontName);
            fontIndex++;
        }
    }
    file << "}\n";
    
    // Color table (optional)
    file << "{\\colortbl ;\\red0\\green0\\blue0;}\n";
    
    // Document info
    if (!doc.title.empty()) {
        file << "{\\title " << escapeRtf(doc.title) << "}\n";
    }
    if (!doc.author.empty()) {
        file << "{\\author " << escapeRtf(doc.author) << "}\n";
    }
    if (!doc.subject.empty()) {
        file << "{\\subject " << escapeRtf(doc.subject) << "}\n";
    }
    
    // Default font size
    file << "\\fs" << doc.defaultFontSize << "\n";
    
    // Write paragraphs
    for (const auto& para : doc.paragraphs) {
        // Apply formatting
        if (para.bold) file << "\\b";
        if (para.italic) file << "\\i";
        if (para.underline) file << "\\ul";
        if (para.strikeout) file << "\\strike";
        
        // Font
        auto it = std::find(addedFonts.begin(), addedFonts.end(), para.fontName);
        if (it != addedFonts.end()) {
            int idx = std::distance(addedFonts.begin(), it);
            file << "\\f" << idx;
        }
        
        // Font size
        if (para.fontSize != doc.defaultFontSize) {
            file << "\\fs" << para.fontSize;
        }
        
        // Text content
        file << escapeRtf(para.text);
        
        // Reset formatting
        if (para.bold) file << "\\b0";
        if (para.italic) file << "\\i0";
        if (para.underline) file << "\\ulnone";
        if (para.strikeout) file << "\\strike0";
        
        // Paragraph break
        file << "\\par\n";
    }
    
    file << "}";
    
    bool success = file.good();
    file.close();
    
    return success;
}

bool RtfHandler::extractPlainText(const std::string& path, std::string& text) {
    RtfDocument doc;
    RtfHandler handler;
    
    if (!handler.loadFromFile(path, doc)) {
        return false;
    }
    
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

bool RtfHandler::parseRtf(const std::string& rtfContent, RtfDocument& doc) {
    if (rtfContent.empty() || rtfContent[0] != '{') {
        m_lastError = "Invalid RTF format - must start with {";
        return false;
    }
    
    ParserState state;
    state.reset();
    
    std::vector<RtfParagraph> paragraphs;
    std::string currentText;
    
    size_t i = 0;
    while (i < rtfContent.size()) {
        char c = rtfContent[i];
        
        if (state.inControlWord) {
            if (std::isalpha(static_cast<unsigned char>(c))) {
                state.currentControlWord += c;
            } else if (c >= '0' && c <= '9') {
                if (!state.hasParameter) {
                    // End of control word, start of parameter
                    processControlWord(state, doc, paragraphs, currentText);
                    state.currentControlWord.clear();
                }
                state.parameter = state.parameter * 10 + (c - '0');
                state.hasParameter = true;
            } else if (c == '-') {
                // Negative parameter
                state.parameter = 0;
                state.hasParameter = true;
                // Will be handled as negative in processing
            } else {
                // End of control word
                processControlWord(state, doc, paragraphs, currentText);
                state.currentControlWord.clear();
                state.parameter = 0;
                state.hasParameter = false;
                
                if (c == ' ') {
                    state.inControlWord = false;
                } else if (c == '\n' || c == '\r' || c == '\t') {
                    state.inControlWord = false;
                } else {
                    // Could be start of control symbol or other character
                    i--; // Re-process this character
                    state.inControlWord = false;
                }
            }
        } else if (state.inControlSymbol) {
            // Control symbols are single character
            state.currentControlWord = c;
            processControlSymbol(state, doc, paragraphs, currentText);
            state.currentControlWord.clear();
            state.inControlSymbol = false;
        } else {
            if (c == '\\') {
                state.inControlWord = true;
                state.currentControlWord.clear();
                state.parameter = 0;
                state.hasParameter = false;
            } else if (c == '{') {
                state.braceDepth++;
                // Push current state (for nested formatting)
            } else if (c == '}') {
                state.braceDepth--;
                if (state.braceDepth < 0) {
                    m_lastError = "Unbalanced braces in RTF";
                    return false;
                }
                // Pop state (restore previous formatting)
                // For simplicity, we just reset some state
                if (!currentText.empty()) {
                    RtfParagraph para;
                    para.text = unescapeRtf(currentText);
                    para.bold = state.bold;
                    para.italic = state.italic;
                    para.underline = state.underline;
                    para.strikeout = state.strikeout;
                    para.fontName = state.fontName;
                    para.fontSize = state.fontSize;
                    paragraphs.push_back(para);
                    currentText.clear();
                }
            } else if (c == '\n' || c == '\r') {
                // Ignore newlines in RTF source
            } else {
                currentText += c;
            }
        }
        
        i++;
    }
    
    // Add final paragraph if there's remaining text
    if (!currentText.empty()) {
        RtfParagraph para;
        para.text = unescapeRtf(currentText);
        para.bold = state.bold;
        para.italic = state.italic;
        para.underline = state.underline;
        para.strikeout = state.strikeout;
        para.fontName = state.fontName;
        para.fontSize = state.fontSize;
        paragraphs.push_back(para);
    }
    
    doc.paragraphs = std::move(paragraphs);
    doc.defaultFont = state.fontName;
    doc.defaultFontSize = state.fontSize;
    
    return true;
}

void RtfHandler::processControlWord(ParserState& state, RtfDocument& doc,
                                    std::vector<RtfParagraph>& paragraphs, 
                                    std::string& currentText) {
    const std::string& cw = state.currentControlWord;
    int param = state.parameter;
    if (state.hasParameter && cw != "par" && cw != "line") {
        // Check for negative sign in the original RTF would require more complex parsing
        // For now, assume positive parameters
    }
    
    // Handle destination controls (skip their content)
    if (cw == "author") {
        state.destDestination = 1;
        return;
    } else if (cw == "title") {
        state.destDestination = 2;
        return;
    } else if (cw == "subject") {
        state.destDestination = 3;
        return;
    } else if (cw == "info" || cw == "fonttbl" || cw == "colortbl" || 
               cw == "stylesheet" || cw == "pict" || cw == "obj") {
        // Skip these destinations
        state.destDestination = 100;
        return;
    }
    
    // Formatting controls
    if (cw == "b" || cw == "bold") {
        state.bold = (param == 0) ? false : true;
    } else if (cw == "b0") {
        state.bold = false;
    } else if (cw == "i" || cw == "italic") {
        state.italic = (param == 0) ? false : true;
    } else if (cw == "i0") {
        state.italic = false;
    } else if (cw == "ul" || cw == "underline") {
        state.underline = (param == 0) ? false : true;
    } else if (cw == "ulnone" || cw == "ul0") {
        state.underline = false;
    } else if (cw == "strike" || cw == "striked") {
        state.strikeout = (param == 0) ? false : true;
    } else if (cw == "strike0") {
        state.strikeout = false;
    } else if (cw == "fs") {
        // Font size in half-points
        state.fontSize = param;
    } else if (cw == "f") {
        // Font index (would need font table lookup)
        // For now, keep current font
    } else if (cw == "par") {
        // Paragraph break
        if (!currentText.empty()) {
            RtfParagraph para;
            para.text = unescapeRtf(currentText);
            para.bold = state.bold;
            para.italic = state.italic;
            para.underline = state.underline;
            para.strikeout = state.strikeout;
            para.fontName = state.fontName;
            para.fontSize = state.fontSize;
            paragraphs.push_back(para);
            currentText.clear();
        }
    } else if (cw == "line") {
        // Line break within paragraph
        currentText += "\n";
    } else if (cw == "tab") {
        currentText += "\t";
    } else if (cw == "emdash") {
        currentText += "\u2014";
    } else if (cw == "endash") {
        currentText += "\u2013";
    } else if (cw == "lquote") {
        currentText += "\u2018";
    } else if (cw == "rquote") {
        currentText += "\u2019";
    } else if (cw == "ldblquote") {
        currentText += "\u201C";
    } else if (cw == "rdblquote") {
        currentText += "\u201D";
    } else if (cw == "bullet") {
        currentText += "\u2022";
    } else if (cw == "u") {
        // Unicode character - simplified handling
        // In full implementation, would convert UTF-16 to UTF-8
        currentText += "?";
    } else if (cw == "uc") {
        // Unicode skip count - ignore
    } else if (cw == "ansicpg") {
        // Code page - ignore for now
    } else if (cw == "ansi" || cw == "mac" || cw == "pc" || cw == "pca") {
        // Character set - ignore for now
    } else if (cw == "deff") {
        // Default font - ignore for now
    } else if (cw == "deflang") {
        // Default language - ignore
    } else if (cw == "plain") {
        // Reset all formatting
        state.bold = false;
        state.italic = false;
        state.underline = false;
        state.strikeout = false;
        state.fontSize = doc.defaultFontSize;
    } else if (cw == "chpad") {
        // Character padding - ignore
    } else if (cw == "li" || cw == "lin" || cw == "ri" || cw == "rin" ||
               cw == "fi" || cw == "fi0" || cw == "qc" || cw == "qr" || cw == "ql") {
        // Paragraph formatting - ignore for now
    } else if (cw == "sa" || cw == "sb" || cw == "sl" || cw == "slmult") {
        // Spacing - ignore
    } else if (cw == "keep" || cw == "keepn" || cw == "pagebb") {
        // Page breaks - ignore
    } else if (cw == "widctlpar" || cw == "aspalpha" || cw == "aspnum" || cw == "faauto") {
        // Advanced formatting - ignore
    } else if (cw == "sectd" || cw == "pgnx" || cw == "pgny" || cw == "cols" || cw == "colno") {
        // Section formatting - ignore
    } else if (cw == "header" || cw == "footer" || cw == "headerl" || cw == "headerr" || 
               cw == "headerf" || cw == "footerl" || cw == "footerr" || cw == "footerf") {
        // Headers/footers - skip
        state.destDestination = 100;
    } else if (cw == "footnote" || cw == "ftnalt" || cw == "annotation") {
        // Footnotes/annotations - skip
        state.destDestination = 100;
    } else if (cw == "pn" || cw == "pnlvl" || cw == "pndec" || cw == "pntxtb" || cw == "pntxta") {
        // List formatting - ignore
    } else if (cw == "hyphauto" || cw == "hyphhotz" || cw == "hyphconsec") {
        // Hyphenation - ignore
    } else if (cw == "formshade" || cw == "formprot" || cw == "formdisp") {
        // Form fields - ignore
    } else if (cw == "validatexml" || cw == "showxmlerrors") {
        // XML validation - ignore
    } else if (cw == "rsidroot" || cw == "rsidrev" || cw == "rsidtr") {
        // Revision tracking - ignore
    } else if (cw == "margl" || cw == "margr" || cw == "margt" || cw == "margb" ||
               cw == "margmirror" || cw == "margcolsx" || cw == "guttersx" || cw == "gutterprsc") {
        // Margins - ignore
    } else if (cw == "paperw" || cw == "paperh" || cw == "papersize" || cw == "papernox" || cw == "papernoy") {
        // Paper size - ignore
    } else if (cw == "landscape" || cw == "portrait") {
        // Orientation - ignore
    } else if (cw == "nofpages" || cw == "noftnnl" || cw == "noftnnr" || cw == "noftnsep" || cw == "noftnsepc") {
        // Page numbering - ignore
    } else if (cw == "startncl" || cw == "restartncl" || cw == "nclncol" || cw == "nclnfc" || cw == "nclnid") {
        // Chapter numbering - ignore
    } else if (cw == "listtable" || cw == "listoverride" || cw == "listoverridetable") {
        // Lists - skip
        state.destDestination = 100;
    } else if (cw == "latentstyles" || cw == "lsdlocked" || cw == "lsdpriority" || 
               cw == "lsqsemihidden" || cw == "lsqunhideused" || cw == "lsqformatlock" ||
               cw == "lsqnamefromstyleui" || cw == "lsqsatisfied" || cw == "lsqhidden" ||
               cw == "lsqdefpri" || cw == "lsqcustomstylemark") {
        // Styles - skip
        state.destDestination = 100;
    } else if (cw == "datastore" || cw == "doctemp" || cw == "doccomm" || cw == "rxe") {
        // Data/templates - skip
        state.destDestination = 100;
    } else if (cw == "shpgrp" || cw == "shp" || cw == "sp" || cw == "sn" || cw == "sv") {
        // Shapes - skip
        state.destDestination = 100;
    } else if (cw == "field" || cw == "fldinst" || cw == "flddirty" || cw == "fldedit" || 
               cw == "fldpriv" || cw == "fldlock" || cw == "fldrslt") {
        // Fields - skip content
        state.destDestination = 100;
    } else if (cw == "revtbl" || cw == "revprop" || cw == "revisions" || cw == "reviser") {
        // Revisions - skip
        state.destDestination = 100;
    } else if (cw == "generator" || cw == "version" || cw == "operatingsystem" || 
               cw == "company" || cw == "manager" || cw == "presentationtarget" ||
               cw == "template" || cw == "themedata" || cw == "themekey") {
        // Metadata - skip
        state.destDestination = 100;
    } else if (cw == "xmlnstbl" || cw == "rsids" || cw == "rsidroot" || cw == "rsidreg" ||
               cw == "rsidrevlog" || cw == "rsidtrperiod" || cw == "rsiddelim" || cw == "rsidfont") {
        // XML/RSID - skip
        state.destDestination = 100;
    } else if (cw == "omml" || cw == "math" || cw == "mr" || cw == "acc" || cw == "bar" ||
               cw == "box" || cw == "borderBox" || cw == "d" || cw == "eqArr" || cw == "f" ||
               cw == "func" || cw == "groupChr" || cw == "lim" || cw == "limLow" || cw == "limUpp" ||
               cw == "m" || cw == "nary" || cw == "rad" || cw == "rPh" || cw == "sSub" || cw == "sSup" ||
               cw == "sSubPre" || cw == "sSupPre" || cw == "phant" || cw == "e" || cw == "sub" ||
               cw == "sup" || cw == "beg" || cw == "end" || cw == "prop" || cw == "vertJc" ||
               cw == "argSz" || cw == "wrap") {
        // Math/OMML - skip
        state.destDestination = 100;
    } else if (cw == "nonvisualproperties" || cw == "applicationproperties" || 
               cw == "shapeproperties" || cw == "formdata") {
        // Properties - skip
        state.destDestination = 100;
    }
    // Unknown control words are ignored per RTF spec
}

void RtfHandler::processControlSymbol(ParserState& state, RtfDocument& doc,
                                      std::vector<RtfParagraph>& paragraphs,
                                      std::string& currentText) {
    char sym = state.currentControlWord[0];
    
    if (sym == '\'') {
        // Hex-encoded character (e.g., \'e9 for é)
        // Next two characters should be hex digits
        // This is simplified - full implementation would decode properly
        currentText += "?";
    } else if (sym == '~') {
        // Non-breaking space
        currentText += " ";
    } else if (sym == '-') {
        // Optional hyphen
        currentText += "-";
    } else if (sym == '_') {
        // Non-breaking hyphen
        currentText += "-";
    } else if (sym == '*') {
        // Symbol with parameter (ignored)
    } else if (sym == ':' || sym == '|' || sym == '.' || sym == ',') {
        // Special symbols
        currentText += sym;
    }
    // Other control symbols are ignored
}

std::string RtfHandler::unescapeRtf(const std::string& text) {
    std::string result = text;
    
    // Replace common RTF escape sequences
    size_t pos = 0;
    while ((pos = result.find("\\'", pos)) != std::string::npos) {
        // Hex-encoded character - simplified handling
        if (pos + 4 < result.size()) {
            int hexValue;
            std::istringstream iss(result.substr(pos + 2, 2));
            if (iss >> std::hex >> hexValue) {
                result.replace(pos, 4, 1, static_cast<char>(hexValue));
            } else {
                result.erase(pos, 4);
            }
        } else {
            result.erase(pos, 2);
        }
    }
    
    pos = 0;
    while ((pos = result.find("\\\\", pos)) != std::string::npos) {
        result.replace(pos, 2, "\\");
        pos++;
    }
    
    pos = 0;
    while ((pos = result.find("\\{", pos)) != std::string::npos) {
        result.replace(pos, 2, "{");
        pos++;
    }
    
    pos = 0;
    while ((pos = result.find("\\}", pos)) != std::string::npos) {
        result.replace(pos, 2, "}");
        pos++;
    }
    
    return result;
}

std::string RtfHandler::escapeRtf(const std::string& text) {
    std::string result;
    result.reserve(text.size() * 1.2);
    
    for (char c : text) {
        if (c == '\\') {
            result += "\\\\";
        } else if (c == '{') {
            result += "\\{";
        } else if (c == '}') {
            result += "\\}";
        } else if (c == '\n') {
            result += "\\par\n";
        } else if (c == '\t') {
            result += "\\tab ";
        } else if (static_cast<unsigned char>(c) > 127) {
            // Non-ASCII character - encode as hex
            std::ostringstream oss;
            oss << "\\'" << std::hex << std::uppercase << static_cast<int>(static_cast<unsigned char>(c));
            result += oss.str();
        } else {
            result += c;
        }
    }
    
    return result;
}

} // namespace liteedit
