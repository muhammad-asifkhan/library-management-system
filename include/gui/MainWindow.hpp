#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include "../core/LibrarySystem.hpp"
#include "../config.hpp"
#include <string>
#include <vector>

/**
 * @class MainWindow
 * @brief Main GUI window for Library Management System
 * 
 * Provides a professional user interface using Dear ImGui with an SFML backend.
 * Features tabbed interface for all library operations.
 */
class MainWindow {
private:
    sf::RenderWindow window;
    sf::Clock deltaClock;
    LibrarySystem& library;
    
    // UI State
    bool showDemoWindow;
    bool showShortcutsWindow = false;
    int currentTab;
    bool firstFrame = true;
    int themeIndex = 3; // 0=Mint, 1=Lavender, 2=Peach, 3=Light Blue (default)

    // Simple typography
    ImFont* fontBody = nullptr;
    ImFont* fontHeading = nullptr;

    // ======================
    // Authentication / Roles
    // ======================
    enum class UserRole { None = 0, Student = 1, Librarian = 2 };
    bool isAuthenticated = false;
    UserRole role = UserRole::None;
    int sessionStudentId = 0;
    std::string sessionDisplayName;

    // Login UI state
    int loginMode = 0; // 0=Student, 1=Librarian
    int loginStudentId = 100001;
    char loginStudentPin[16]{};
    char loginUsername[64]{};
    char loginPassword[64]{};
    bool showLoginError = false;
    std::string loginErrorText;

    // Student Assistant (simple offline bot)
    std::vector<std::string> assistantChat;
    char assistantInput[256]{};

    // Data caches (refreshed on demand)
    bool booksDirty = true;
    bool studentsDirty = true;
    bool transactionsDirty = true;
    std::vector<Book> allBooks;
    std::vector<Student> allStudents;
    
    // Book Management State
    int bookId;
    char bookTitle[256];
    char bookAuthor[256];
    char bookISBN[14];
    int bookQuantity;
    int searchBookId;
    char searchBookTitle[256];
    char searchBookAuthor[256];
    std::vector<Book> bookSearchResults;
    int selectedBookIndex;
    
    // Enhanced search filters
    bool filterAvailableOnly = false;
    int bookSortMode = 0; // 0=ID, 1=Title, 2=Author, 3=Most Borrowed

    // Book modals
    bool showEditBookModal = false;
    bool showDeleteBookModal = false;
    int modalBookId = 0;
    char editBookTitle[256]{};
    char editBookAuthor[256]{};
    char editBookISBN[14]{};
    int editBookQuantity = 1;
    
    // Student Management State
    int studentId;
    char studentName[256];
    char studentDepartment[256];
    char studentEmail[256];
    char studentPhone[256];
    int searchStudentId;
    char searchStudentName[256];
    std::vector<Student> studentSearchResults;
    int selectedStudentIndex;

    // Student modals
    bool showDeleteStudentModal = false;
    int modalStudentId = 0;
    
    // Issue/Return State
    int issueStudentId;
    int issueBookId;
    int returnStudentId;
    int returnBookId;
    
    // Request Queue State
    std::vector<BookRequest> requestQueue;
    int selectedRequestIndex;
    int requestStudentId;
    int requestBookId;
    int requestPriority;
    
    // Transaction History State
    std::vector<Transaction> transactions;
    int transactionHistoryLimit;
    
    // Statistics
    SystemStatistics currentStats;
    std::vector<std::pair<Book, int>> popularBooks;
    
    // Messages
    std::string lastMessage;
    sf::Clock messageTimer;
    bool showMessageFlag;
    bool messageSuccess;

public:
    /**
     * @brief Constructor
     */
    MainWindow();
    
    /**
     * @brief Destructor
     */
    ~MainWindow();
    
    /**
     * @brief Run the main application loop
     */
    void run();
    
private:
    /**
     * @brief Initialize the window and ImGui
     */
    void initialize();
    
    /**
     * @brief Process events
     */
    void processEvents();

    void update();
    void render();

    // UI sections
    void showMenuBar();
    void showBookManagementTab();
    void showStudentManagementTab();
    void showIssueReturnTab();
    void showRequestQueueTab();
    void showTransactionHistoryTab();
    void showStatisticsTab();
    void showDashboardTab();
    void showAssistantTab();
    void showSidebar();
    void showStatusBar();
    void showLoginScreen();

    // Data refresh helpers
    void refreshBooks();
    void refreshStudents();
    void refreshTransactions();

    // Demo seeding helpers
    void seedDemoDataIfNeeded();
    void addMoreDemoBooksSafe();
    void ensureDemoStudentsSafe();

    // Auth helpers
    bool loginAsStudent(int studentId, const std::string& pin);
    bool loginAsLibrarian(const std::string& user, const std::string& pass);
    void logout();
    static std::string expectedStudentPin(int studentId);
    
    /**
     * @brief Show a temporary message
     */
    void showMessage(const std::string& message, bool success = true);

    // UI helpers
    void setupStyle();
    void setupFonts();
    void applyTheme(int idx);
    bool inputText(const char* label, char* buffer, size_t bufferSize);
    bool centerButton(const char* label, float width = 0.0f);
    void showMessageBox();
    
    /**
     * @brief Format book information for display
     */
    std::string formatBookInfo(const Book& book);
    
    /**
     * @brief Format student information for display
     */
    std::string formatStudentInfo(const Student& student);
    
    /**
     * @brief Format transaction information for display
     */
    std::string formatTransactionInfo(const Transaction& trans);
    
    /**
     * @brief Show keyboard shortcuts help window
     */
    void showShortcutsDialog();
};

#endif // MAINWINDOW_HPP
