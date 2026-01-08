#include "../include/gui/MainWindow.hpp"
#include <iostream>

/**
 * @brief Main entry point for GUI version of Library Management System
 * 
 * This file contains the main function that creates and runs the
 * graphical user interface using Dear ImGui and SFML.
 */

int main() {
    try {
        // Create and run the main window
        MainWindow mainWindow;
        mainWindow.run();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred!" << std::endl;
        return 1;
    }
}
