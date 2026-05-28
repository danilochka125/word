#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <vector>

namespace liteedit {

// File handler with support for multiple formats
class FileHandler {
public:
    enum class Format {
        Unknown,
        PlainText,    // .txt
        RichText,     // .rtf
        WordDocument  // .docx
    };
    
    static Format detectFormat(const std::string& path);
    static std::string getExtension(Format format);
    
    // Text file operations (fully implemented)
    static bool loadPlainText(const std::string& path, std::string& content);
    static bool savePlainText(const std::string& path, const std::string& content);
    
    // RTF operations (basic support)
    static bool loadRTF(const std::string& path, std::string& content);
    static bool saveRTF(const std::string& path, const std::string& content);
    
    // DOCX operations (placeholder - requires external library)
    static bool loadDOCX(const std::string& path, std::string& content);
    static bool saveDOCX(const std::string& path, const std::string& content);
    
    // Utility functions
    static std::string getFileName(const std::string& path);
    static std::string getDirectory(const std::string& path);
    static bool fileExists(const std::string& path);
    static std::vector<std::string> listFiles(const std::string& directory, 
                                              const std::string& extension = "");
};

} // namespace liteedit

#endif // FILEHANDLER_H
