#include "utils/FileHandler.h"
#include "formats/DocxHandler.h"
#include "formats/RtfHandler.h"
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
    } else if (ext == ".doc") {
        return Format::WordDocument;  // Legacy .doc treated as Word format
    } else if (ext == ".odt") {
        return Format::WordDocument;  // OpenDocument Text
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
    // Use RtfHandler for full RTF support with formatting
    RtfHandler handler;
    RtfDocument doc;
    
    if (handler.loadFromFile(path, doc)) {
        // Extract plain text from all paragraphs
        std::stringstream ss;
        for (size_t i = 0; i < doc.paragraphs.size(); ++i) {
            ss << doc.paragraphs[i].text;
            if (i < doc.paragraphs.size() - 1) {
                ss << "\n";
            }
        }
        content = ss.str();
        return true;
    }
    
    // Fallback to simple extraction
    return RtfHandler::extractPlainText(path, content);
}

bool FileHandler::saveRTF(const std::string& path, const std::string& content) {
    // Create RTF document with basic formatting
    RtfDocument doc;
    
    // Split content into paragraphs
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        RtfParagraph para;
        para.text = line;
        doc.paragraphs.push_back(para);
    }
    
    RtfHandler handler;
    return handler.saveToFile(path, doc);
}

bool FileHandler::loadDOCX(const std::string& path, std::string& content) {
    // Use DocxHandler for full DOCX support
    DocxHandler handler;
    DocxDocument doc;
    
    if (handler.loadFromFile(path, doc)) {
        // Extract plain text from all paragraphs
        std::stringstream ss;
        for (size_t i = 0; i < doc.paragraphs.size(); ++i) {
            ss << doc.paragraphs[i].text;
            if (i < doc.paragraphs.size() - 1) {
                ss << "\n";
            }
        }
        content = ss.str();
        return true;
    }
    
    // Fallback to simple extraction
    return DocxHandler::extractPlainText(path, content);
}

bool FileHandler::saveDOCX(const std::string& path, const std::string& content) {
    // Create DOCX document with basic formatting
    DocxDocument doc;
    
    // Split content into paragraphs
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        DocxParagraph para;
        para.text = line;
        doc.paragraphs.push_back(para);
    }
    
    DocxHandler handler;
    return handler.saveToFile(path, doc);
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
