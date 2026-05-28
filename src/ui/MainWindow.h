#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "editor/TextEditor.h"
#include <SDL2/SDL.h>
#include <string>

namespace liteedit {

// Simple toolbar for basic operations
class Toolbar {
public:
    struct Button {
        std::string label;
        SDL_Rect rect;
        bool isActive;
        int keyCode;  // Keyboard shortcut
        
        Button() : isActive(false), keyCode(0) {}
        Button(const std::string& l, int key = 0) 
            : label(l), isActive(false), keyCode(key) {}
    };
    
    Toolbar();
    
    void initialize(int width, int height);
    void render(SDL_Renderer* renderer);
    bool handleEvent(const SDL_Event& event, TextEditor& editor);
    bool handleClick(int x, int y, TextEditor& editor);
    
    void updateButtonState(const std::string& name, bool active);
    
private:
    std::vector<Button> m_buttons;
    int m_buttonWidth;
    int m_buttonHeight;
    int m_spacing;
    
    void layoutButtons(int width);
};

// Main application window
class MainWindow {
public:
    MainWindow();
    ~MainWindow();
    
    bool initialize(int width, int height, const std::string& title);
    void shutdown();
    
    void run();
    void quit();
    
private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    TextEditor m_editor;
    Toolbar m_toolbar;
    
    int m_width;
    int m_height;
    bool m_running;
    
    void processEvents();
    void update(float deltaTime);
    void render();
    
    void showFileDialog(bool isSave);
    void showSearchDialog();
    void showPrintDialog();
    void showAboutDialog();
};

} // namespace liteedit

#endif // MAINWINDOW_H
