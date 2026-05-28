#include "core/UndoManager.h"

namespace liteedit {

void InsertTextOperation::undo(Document& doc) {
    doc.deleteText(m_line, m_col, m_col + m_text.length());
}

void InsertTextOperation::redo(Document& doc) {
    doc.insertText(m_line, m_col, m_text, m_style);
}

void DeleteTextOperation::undo(Document& doc) {
    doc.insertText(m_line, m_colStart, m_text, TextStyle::None);
}

void DeleteTextOperation::redo(Document& doc) {
    doc.deleteText(m_line, m_colStart, m_colEnd);
}

void StyleChangeOperation::undo(Document& doc) {
    doc.applyStyle(m_lineStart, m_colStart, m_lineEnd, m_colEnd, m_oldStyle);
}

void StyleChangeOperation::redo(Document& doc) {
    doc.applyStyle(m_lineStart, m_colStart, m_lineEnd, m_colEnd, m_newStyle);
}

UndoManager::UndoManager(size_t maxHistorySize)
    : m_maxHistorySize(maxHistorySize) {
}

void UndoManager::addOperation(std::unique_ptr<UndoOperation> op) {
    if (!op) {
        return;
    }
    
    // Clear redo stack on new operation
    m_redoStack.clear();
    
    // Add to undo stack
    m_undoStack.push_back(std::move(op));
    
    // Trim history to prevent memory bloat
    trimHistory();
}

void UndoManager::undo(Document& doc) {
    if (m_undoStack.empty()) {
        return;
    }
    
    auto& op = m_undoStack.back();
    op->undo(doc);
    
    // Move to redo stack
    m_redoStack.push_back(std::move(op));
    m_undoStack.pop_back();
}

void UndoManager::redo(Document& doc) {
    if (m_redoStack.empty()) {
        return;
    }
    
    auto& op = m_redoStack.back();
    op->redo(doc);
    
    // Move back to undo stack
    m_undoStack.push_back(std::move(op));
    m_redoStack.pop_back();
}

void UndoManager::clear() {
    m_undoStack.clear();
    m_redoStack.clear();
}

void UndoManager::trimHistory() {
    while (m_undoStack.size() > m_maxHistorySize) {
        m_undoStack.erase(m_undoStack.begin());
    }
}

} // namespace liteedit
