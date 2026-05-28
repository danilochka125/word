#include "ui/MainWindow.h"
#include <iostream>
#include <chrono>

namespace liteedit {

MainWindow::MainWindow()
    : m_window(nullptr)
    , m_renderer(nullptr)
    , m_width(1024)
    , m_height(768)
    , m_running(false) {
}

MainWindow::~MainWindow() {
    shutdown();
}

bool MainWindow::initialize(int width, int height, const std::string& title) {
    m_width = width;
    m_height = height;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create window
    m_window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    
    if (!m_window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    
    // Create renderer
    m_renderer = SDL_CreateRenderer(m_window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (!m_renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return false;
    }
    
    // Initialize editor
    if (!m_editor.initialize(width, height - 40)) {  // Reserve space for toolbar
        std::cerr << "Editor initialization failed" << std::endl;
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return false;
    }
    
    // Initialize toolbar
    m_toolbar.initialize(width - 10, 40);
    
    m_running = true;
    return true;
}

void MainWindow::shutdown() {
    m_editor.shutdown();
    
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    
    SDL_Quit();
}

void MainWindow::run() {
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (m_running) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        processEvents();
        update(deltaTime);
        render();
        
        // Cap frame rate
        SDL_Delay(16);  // ~60 FPS
    }
}

void MainWindow::quit() {
    m_running = false;
}

void MainWindow::processEvents() {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        // Let toolbar handle keyboard shortcuts first
        if (m_toolbar.handleEvent(event, m_editor)) {
            continue;
        }
        
        switch (event.type) {
            case SDL_QUIT:
                quit();
                break;
                
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    m_width = event.window.data1;
                    m_height = event.window.data2;
                    m_editor.initialize(m_width, m_height - 40);
                    m_toolbar.initialize(m_width - 10, 40);
                }
                break;
                
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit();
                }
                break;
                
            default:
                m_editor.handleEvent(event);
                break;
        }
    }
}

void MainWindow::update(float deltaTime) {
    m_editor.update(deltaTime);
}

void MainWindow::render() {
    // Clear screen
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    SDL_RenderClear(m_renderer);
    
    // Render toolbar
    m_toolbar.render(m_renderer);
    
    // Render editor
    m_editor.render(m_renderer, m_width, m_height - 40);
    
    // Present
    SDL_RenderPresent(m_renderer);
}

void MainWindow::showFileDialog(bool isSave) {
    // In production, use native file dialog or SDL_dialog
    // For now, this is a placeholder
    std::cout << (isSave ? "Save" : "Open") << " file dialog" << std::endl;
}

void MainWindow::showSearchDialog() {
    // In production, implement search dialog UI
    std::cout << "Search dialog" << std::endl;
}

void MainWindow::showPrintDialog() {
    // In production, implement print functionality
    std::cout << "Print dialog" << std::endl;
}

void MainWindow::showAboutDialog() {
    std::cout << "LiteEdit - Lightweight Text Editor v1.0" << std::endl;
    std::cout << "Optimized for low-end devices" << std::endl;
}

} // namespace liteedit

// Entry point
#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmdLine, int showCmd) {
#else
int main(int argc, char* argv[]) {
#endif
    liteedit::MainWindow window;
    
    if (!window.initialize(1024, 768, "LiteEdit - Lightweight Text Editor")) {
        return 1;
    }
    
    window.run();
    return 0;
}
