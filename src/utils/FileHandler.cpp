#include "utils/FileHandler.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

namespace liteedit {

FileHandler::Format FileHandler::detectFormat(const std::string& path) {
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) {
        return Format::PlainText;
    }
    
    std::string ext = path.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".txt") {
        return Format::PlainText;
    } else if (ext == ".rtf") {
        return Format::RichText;
    } else if (ext == ".docx") {
        return Format::WordDocument;
    }
    
    return Format::PlainText;  // Default to plain text
}

std::string FileHandler::getExtension(Format format) {
    switch (format) {
        case Format::PlainText:
            return ".txt";
        case Format::RichText:
            return ".rtf";
        case Format::WordDocument:
            return ".docx";
        default:
            return ".txt";
    }
}

bool FileHandler::loadPlainText(const std::string& path, std::string& content) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    
    return true;
}

bool FileHandler::savePlainText(const std::string& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(content.c_str(), content.size());
    return file.good();
}

bool FileHandler::loadRTF(const std::string& path, std::string& content) {
    // Basic RTF loading - strips RTF formatting
    // In production, use proper RTF parser
    
    std::string rtfContent;
    if (!loadPlainText(path, rtfContent)) {
        return false;
    }
    
    // Simple RTF stripping (remove control words and braces)
    std::string plainText;
    bool inControlWord = false;
    int braceDepth = 0;
    
    for (size_t i = 0; i < rtfContent.size(); ++i) {
        char c = rtfContent[i];
        
        if (c == '{') {
            braceDepth++;
            inControlWord = false;
        } else if (c == '}') {
            braceDepth--;
            inControlWord = false;
        } else if (c == '\\' && !inControlWord) {
            inControlWord = true;
        } else if (inControlWord) {
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                inControlWord = false;
            }
        } else if (braceDepth > 1) {
            // Skip content inside outer braces
            continue;
        } else {
            plainText += c;
        }
    }
    
    content = plainText;
    return true;
}

bool FileHandler::saveRTF(const std::string& path, const std::string& content) {
    // Basic RTF saving - wraps plain text in minimal RTF structure
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Minimal RTF header
    file << "{\\rtf1\\ansi\\deff0\n";
    file << "{\\fonttbl\n";
    file << "{\\f0\\fswiss\\fcharset0 Arial;}\n";
    file << "}\n";
    
    // Escape special characters
    for (char c : content) {
        if (c == '\\') {
            file << "\\\\";
        } else if (c == '{') {
            file << "\\{";
        } else if (c == '}') {
            file << "\\}";
        } else if (c == '\n') {
            file << "\\par\n";
        } else if (static_cast<unsigned char>(c) > 127) {
            file << "\\'" << std::hex << static_cast<int>(static_cast<unsigned char>(c));
        } else {
            file << c;
        }
    }
    
    file << "\n}";
    return file.good();
}

bool FileHandler::loadDOCX(const std::string& path, std::string& content) {
    // DOCX is a ZIP archive containing XML files
    // Proper implementation requires:
    // 1. ZIP extraction library (e.g., minizip, libzip)
    // 2. XML parser (e.g., pugixml, tinyxml2)
    // 3. Understanding of Office Open XML structure
    
    // Placeholder - returns false to indicate unsupported
    // For MVP, recommend using plain text or RTF instead
    
    return false;
}

bool FileHandler::saveDOCX(const std::string& path, const std::string& content) {
    // DOCX creation is complex and requires:
    // 1. Multiple XML files with specific structure
    // 2. ZIP packaging
    // 3. Proper relationships and content types
    
    // Placeholder - returns false to indicate unsupported
    return false;
}

std::string FileHandler::getFileName(const std::string& path) {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        return path;
    }
    return path.substr(lastSlash + 1);
}

std::string FileHandler::getDirectory(const std::string& path) {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        return "";
    }
    return path.substr(0, lastSlash);
}

bool FileHandler::fileExists(const std::string& path) {
#ifdef _WIN32
    return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
#else
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
#endif
}

std::vector<std::string> FileHandler::listFiles(const std::string& directory, 
                                                const std::string& extension) {
    std::vector<std::string> files;
    
#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    HANDLE hFind;
    
    std::string searchPath = directory + "\\*" + extension;
    hFind = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(directory + "\\" + findData.cFileName);
            }
        } while (FindNextFileA(hFind, &findData) != 0);
        
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(directory.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            
            if (name == "." || name == "..") {
                continue;
            }
            
            if (extension.empty() || 
                name.size() >= extension.size() &&
                name.substr(name.size() - extension.size()) == extension) {
                files.push_back(directory + "/" + name);
            }
        }
        closedir(dir);
    }
#endif
    
    return files;
}

} // namespace liteedit
