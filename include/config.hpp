#ifndef LIBRARY_CONFIG_HPP
#define LIBRARY_CONFIG_HPP

#include <string>

namespace LibraryConfig {
    // Version
    constexpr int MAJOR_VERSION = 1;
    constexpr int MINOR_VERSION = 0;
    constexpr int PATCH_VERSION = 0;

    // Data file paths
    const std::string DATA_DIRECTORY = "data/";
    const std::string BOOKS_FILE = DATA_DIRECTORY + "books.csv";
    const std::string STUDENTS_FILE = DATA_DIRECTORY + "students.csv";
    const std::string TRANSACTIONS_FILE = DATA_DIRECTORY + "transactions.csv";
    const std::string REQUESTS_FILE = DATA_DIRECTORY + "requests.csv";
    const std::string CONFIG_FILE = DATA_DIRECTORY + "settings.ini";

    // Policies
    constexpr int MAX_BOOKS_PER_STUDENT = 3;
    constexpr int LOAN_PERIOD_DAYS = 14;
    constexpr double FINE_PER_DAY = 0.50;
    constexpr double MAX_FINE_AMOUNT = 10.00;
    constexpr int REQUEST_EXPIRY_DAYS = 30;
    constexpr int HISTORY_SIZE = 1000;
    constexpr int BACKUP_COUNT = 5;

    // GUI Configuration
    namespace GUI {
        constexpr int DEFAULT_WINDOW_WIDTH = 1280;
        constexpr int DEFAULT_WINDOW_HEIGHT = 720;
        constexpr int MIN_WINDOW_WIDTH = 800;
        constexpr int MIN_WINDOW_HEIGHT = 600;
        constexpr const char* WINDOW_TITLE = "Library Management System";
        constexpr int FPS_LIMIT = 60;
        constexpr bool VSYNC_ENABLED = true;
    }

    // GUI Theme/colors (RGBA)
    // Light theme accent (mint/green) - intentionally NOT blue.
    constexpr float PRIMARY_COLOR[4]      = {0.16f, 0.73f, 0.55f, 1.00f};
    constexpr float SUCCESS_COLOR[4]      = {0.00f, 0.78f, 0.33f, 1.00f};
    constexpr float ERROR_COLOR[4]        = {0.96f, 0.26f, 0.21f, 1.00f};
    constexpr float WARNING_COLOR[4]      = {1.00f, 0.76f, 0.03f, 1.00f};

    // Font settings
    constexpr int FONT_SIZE = 16;
    const std::string FONT_PATH = "third_party/fonts/Roboto-Regular.ttf";
}

#endif // LIBRARY_CONFIG_HPP
