#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "ui/MainWindow.h"

namespace liteedit {

inline Toolbar::Toolbar()
    : m_buttonWidth(80)
    , m_buttonHeight(30)
    , m_spacing(5) {
}

inline void Toolbar::initialize(int width, int height) {
    m_buttons.clear();
    
    // Add default buttons
    m_buttons.emplace_back("New", SDLK_n);
    m_buttons.emplace_back("Open", SDLK_o);
    m_buttons.emplace_back("Save", SDLK_s);
    m_buttons.emplace_back("---");  // Separator
    m_buttons.emplace_back("Undo", SDLK_z);
    m_buttons.emplace_back("Redo", SDLK_y);
    m_buttons.emplace_back("---");
    m_buttons.emplace_back("Bold", SDLK_b);
    m_buttons.emplace_back("Italic", SDLK_i);
    m_buttons.emplace_back("Underline", SDLK_u);
    m_buttons.emplace_back("---");
    m_buttons.emplace_back("Find", SDLK_f);
    m_buttons.emplace_back("Print", SDLK_p);
    
    layoutButtons(width);
}

inline void Toolbar::layoutButtons(int width) {
    int x = m_spacing;
    int y = m_spacing;
    
    for (auto& button : m_buttons) {
        if (button.label == "---") {
            button.rect = {x, y + 10, 2, m_buttonHeight - 20};
            x += 10;
        } else {
            button.rect = {x, y, m_buttonWidth, m_buttonHeight};
            x += m_buttonWidth + m_spacing;
            
            if (x + m_buttonWidth > width) {
                x = m_spacing;
                y += m_buttonHeight + m_spacing;
            }
        }
    }
}

inline void Toolbar::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    
    for (const auto& button : m_buttons) {
        if (button.label == "---") {
            // Render separator
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderFillRect(renderer, &button.rect);
        } else {
            // Render button background
            SDL_Color bgColor = button.isActive ? 
                SDL_Color{200, 220, 255, 255} : 
                SDL_Color{255, 255, 255, 255};
            
            SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
            SDL_RenderFillRect(renderer, &button.rect);
            
            // Render button border
            SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
            SDL_RenderDrawRect(renderer, &button.rect);
            
            // In production, render text label here using SDL_ttf
        }
    }
}

inline bool Toolbar::handleEvent(const SDL_Event& event, TextEditor& editor) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.mod & KMOD_CTRL) {
        for (const auto& button : m_buttons) {
            if (button.keyCode == event.key.keysym.sym) {
                return handleClick(button.rect.x, button.rect.y, editor);
            }
        }
    }
    return false;
}

inline bool Toolbar::handleClick(int x, int y, TextEditor& editor) {
    for (auto& button : m_buttons) {
        if (button.label == "---") {
            continue;
        }
        
        if (x >= button.rect.x && x < button.rect.x + button.rect.w &&
            y >= button.rect.y && y < button.rect.y + button.rect.h) {
            
            // Handle button click
            if (button.label == "New") {
                editor.newFile();
            } else if (button.label == "Bold") {
                editor.toggleBold();
            } else if (button.label == "Italic") {
                editor.toggleItalic();
            } else if (button.label == "Underline") {
                editor.toggleUnderline();
            } else if (button.label == "Find") {
                // Open search dialog
            } else if (button.label == "Print") {
                // Open print dialog
            }
            
            return true;
        }
    }
    
    return false;
}

inline void Toolbar::updateButtonState(const std::string& name, bool active) {
    for (auto& button : m_buttons) {
        if (button.label == name) {
            button.isActive = active;
            break;
        }
    }
}

} // namespace liteedit

#endif // TOOLBAR_H
