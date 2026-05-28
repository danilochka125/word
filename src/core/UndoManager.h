#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

#include "core/Document.h"
#include <vector>
#include <memory>
#include <string>

namespace liteedit {

// Base undo operation
class UndoOperation {
public:
    virtual ~UndoOperation() = default;
    virtual void undo(Document& doc) = 0;
    virtual void redo(Document& doc) = 0;
    virtual std::string getDescription() const = 0;
};

// Text insertion operation
class InsertTextOperation : public UndoOperation {
public:
    InsertTextOperation(size_t line, size_t col, const std::u32string& text, TextStyle style)
        : m_line(line), m_col(col), m_text(text), m_style(style) {}
    
    void undo(Document& doc) override;
    void redo(Document& doc) override;
    std::string getDescription() const override { return "Insert text"; }
    
private:
    size_t m_line;
    size_t m_col;
    std::u32string m_text;
    TextStyle m_style;
};

// Text deletion operation
class DeleteTextOperation : public UndoOperation {
public:
    DeleteTextOperation(size_t line, size_t colStart, size_t colEnd, const std::u32string& text)
        : m_line(line), m_colStart(colStart), m_colEnd(colEnd), m_text(text) {}
    
    void undo(Document& doc) override;
    void redo(Document& doc) override;
    std::string getDescription() const override { return "Delete text"; }
    
private:
    size_t m_line;
    size_t m_colStart;
    size_t m_colEnd;
    std::u32string m_text;
};

// Style change operation
class StyleChangeOperation : public UndoOperation {
public:
    StyleChangeOperation(size_t lineStart, size_t colStart, size_t lineEnd, size_t colEnd, 
                        TextStyle oldStyle, TextStyle newStyle)
        : m_lineStart(lineStart), m_colStart(colStart), m_lineEnd(lineEnd), m_colEnd(colEnd)
        , m_oldStyle(oldStyle), m_newStyle(newStyle) {}
    
    void undo(Document& doc) override;
    void redo(Document& doc) override;
    std::string getDescription() const override { return "Change style"; }
    
private:
    size_t m_lineStart, m_colStart, m_lineEnd, m_colEnd;
    TextStyle m_oldStyle, m_newStyle;
};

// Undo manager with limited history for low memory usage
class UndoManager {
public:
    explicit UndoManager(size_t maxHistorySize = 50);
    
    void addOperation(std::unique_ptr<UndoOperation> op);
    void undo(Document& doc);
    void redo(Document& doc);
    
    bool canUndo() const { return !m_undoStack.empty(); }
    bool canRedo() const { return !m_redoStack.empty(); }
    
    void clear();
    size_t getHistorySize() const { return m_undoStack.size(); }
    
private:
    std::vector<std::unique_ptr<UndoOperation>> m_undoStack;
    std::vector<std::unique_ptr<UndoOperation>> m_redoStack;
    size_t m_maxHistorySize;
    
    void trimHistory();
};

} // namespace liteedit

#endif // UNDOMANAGER_H
