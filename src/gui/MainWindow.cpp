#include "../../include/gui/MainWindow.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

namespace {
ImVec4 rgba(const float c[4], float aOverride = -1.0f) {
    return ImVec4(c[0], c[1], c[2], aOverride >= 0.0f ? aOverride : c[3]);
}
} // namespace

MainWindow::MainWindow()
    : window(sf::VideoMode(LibraryConfig::GUI::DEFAULT_WINDOW_WIDTH,
                           LibraryConfig::GUI::DEFAULT_WINDOW_HEIGHT),
             LibraryConfig::GUI::WINDOW_TITLE, sf::Style::Close),
      library(LibrarySystem::getInstance()),
      showDemoWindow(false),
      currentTab(0),
      firstFrame(true),
      themeIndex(3),
      bookId(1001),
      bookQuantity(1),
      searchBookId(0),
      selectedBookIndex(-1),
      studentId(100001),
      searchStudentId(0),
      selectedStudentIndex(-1),
      issueStudentId(0),
      issueBookId(0),
      returnStudentId(0),
      returnBookId(0),
      selectedRequestIndex(-1),
      requestStudentId(0),
      requestBookId(0),
      requestPriority(3),
      transactionHistoryLimit(10),
      showMessageFlag(false),
      messageSuccess(true) {
    std::memset(bookTitle, 0, sizeof(bookTitle));
    std::memset(bookAuthor, 0, sizeof(bookAuthor));
    std::memset(bookISBN, 0, sizeof(bookISBN));
    std::memset(studentName, 0, sizeof(studentName));
    std::memset(studentDepartment, 0, sizeof(studentDepartment));
    std::memset(studentEmail, 0, sizeof(studentEmail));
    std::memset(studentPhone, 0, sizeof(studentPhone));
    std::memset(searchBookTitle, 0, sizeof(searchBookTitle));
    std::memset(searchBookAuthor, 0, sizeof(searchBookAuthor));
    std::memset(searchStudentName, 0, sizeof(searchStudentName));
    std::memset(loginStudentPin, 0, sizeof(loginStudentPin));
    std::memset(loginUsername, 0, sizeof(loginUsername));
    std::memset(loginPassword, 0, sizeof(loginPassword));
    std::memset(assistantInput, 0, sizeof(assistantInput));

    initialize();
}

MainWindow::~MainWindow() {
    ImGui::SFML::Shutdown();
}

void MainWindow::initialize() {
    window.setFramerateLimit(LibraryConfig::GUI::FPS_LIMIT);
    window.setVerticalSyncEnabled(LibraryConfig::GUI::VSYNC_ENABLED);

    if (!ImGui::SFML::Init(window)) {
        std::cerr << "Failed to initialize ImGui-SFML" << std::endl;
    }

    setupStyle();
    setupFonts();

    library.loadAllData();
    seedDemoDataIfNeeded();
    booksDirty = true;
    studentsDirty = true;
    transactionsDirty = true;
}

void MainWindow::run() {
    while (window.isOpen()) {
        processEvents();
        update();
        render();
    }
}

void MainWindow::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(window, event);
        if (event.type == sf::Event::Closed) {
            library.saveAllData();
            window.close();
        }
    }
}

void MainWindow::update() {
    ImGui::SFML::Update(window, deltaClock.restart());

    currentStats = library.getStatistics();
    popularBooks = library.getMostPopularBooks(5);

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Library Management System", nullptr, flags);

    showMenuBar();

    // Login gate: don't show the app until authenticated.
    if (!isAuthenticated) {
        showLoginScreen();
        if (showMessageFlag) showMessageBox();
        ImGui::End();
        return;
    }

    // Keyboard shortcuts
    ImGuiIO& io = ImGui::GetIO();
    
    // Ctrl+S: Save
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        if (role == UserRole::Librarian) {
            bool ok = library.saveAllData();
            showMessage(ok ? "Saved successfully (Ctrl+S)" : "Save failed", ok);
        } else {
            showMessage("Save is available for Librarian only.", false);
        }
    }
    
    // Ctrl+R: Reload
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R, false)) {
        if (role == UserRole::Librarian) {
            library.clearAllData();
            bool ok = library.loadAllData();
            booksDirty = studentsDirty = transactionsDirty = true;
            showMessage(ok ? "Reloaded successfully (Ctrl+R)" : "Reload failed", ok);
        }
    }
    
    // Ctrl+F: Focus on search (Books tab)
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F, false)) {
        currentTab = 1; // Switch to Books tab
        showMessage("Search mode (Ctrl+F) - Books tab activated", true);
    }
    
    // Ctrl+Q: Quit (with confirmation)
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Q, false)) {
        window.close();
    }
    
    // F1: Show keyboard shortcuts help
    if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
        showShortcutsWindow = true;
    }
    
    // Ctrl+1 through Ctrl+7: Quick tab switching (Librarian)
    if (io.KeyCtrl && role == UserRole::Librarian) {
        for (int i = 0; i < 7; i++) {
            ImGuiKey key = static_cast<ImGuiKey>(ImGuiKey_1 + i);
            if (ImGui::IsKeyPressed(key, false)) {
                currentTab = i;
                break;
            }
        }
    }

    // Layout: sidebar + content + status bar
    const float statusBarH = 34.0f;
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("Body", ImVec2(avail.x, avail.y - statusBarH), false);

    ImGui::BeginChild("Sidebar", ImVec2(220.0f, 0.0f), true);
    showSidebar();
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("Content", ImVec2(0.0f, 0.0f), false);

    // App header (screenshot-ready)
    if (fontHeading) ImGui::PushFont(fontHeading);
    ImGui::TextUnformatted("Library Management System");
    if (fontHeading) ImGui::PopFont();
    ImGui::TextDisabled("C++17 • Dear ImGui + SFML • CSV persistence");
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(20, 0));
    ImGui::SameLine();
    if (role == UserRole::Librarian) {
        if (ImGui::Button("Save (Ctrl+S)", ImVec2(140, 0))) {
            bool ok = library.saveAllData();
            showMessage(ok ? "Saved successfully" : "Save failed", ok);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload", ImVec2(90, 0))) {
            library.clearAllData();
            bool ok = library.loadAllData();
            booksDirty = studentsDirty = transactionsDirty = true;
            showMessage(ok ? "Reloaded successfully" : "Reload failed", ok);
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Demo Books", ImVec2(140, 0))) {
            addMoreDemoBooksSafe();
            booksDirty = true;
            showMessage("Demo books added (skipped duplicates).", true);
        }
    } else {
        ImGui::TextDisabled("Logged in as Student (limited permissions)");
    }

    ImGui::Separator();

    switch (currentTab) {
        case 0: showDashboardTab(); break;
        case 1: showBookManagementTab(); break;
        case 2:
            if (role == UserRole::Librarian) showStudentManagementTab();
            else showAssistantTab();
            break;
        case 3: showIssueReturnTab(); break;
        case 4: showRequestQueueTab(); break;
        case 5:
            if (role == UserRole::Librarian) showTransactionHistoryTab();
            else showAssistantTab();
            break;
        case 6:
            if (role == UserRole::Librarian) showStatisticsTab();
            else showAssistantTab();
            break;
        default: showDashboardTab(); break;
    }

    ImGui::EndChild(); // Content
    ImGui::EndChild(); // Body

    showStatusBar();

    if (showMessageFlag) showMessageBox();

    ImGui::End();
}

void MainWindow::render() {
    // Light background (neutral). ImGui draws the UI; SFML clear just avoids a dark edge.
    window.clear(sf::Color(248, 249, 251, 255));
    ImGui::SFML::Render(window);
    window.display();
}

void MainWindow::showMenuBar() {
    if (!ImGui::BeginMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save", nullptr, false, isAuthenticated && role == UserRole::Librarian)) {
            bool ok = library.saveAllData();
            showMessage(ok ? "Saved successfully" : "Save failed", ok);
        }
        if (ImGui::MenuItem("Logout", nullptr, false, isAuthenticated)) {
            logout();
        }
        if (ImGui::MenuItem("Exit")) {
            library.saveAllData();
            window.close();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Tools")) {
        if (ImGui::BeginMenu("Theme")) {
            const char* themes[] = {"Light Mint", "Light Lavender", "Light Peach", "Light Blue & White"};
            for (int i = 0; i < 4; i++) {
                bool selected = (themeIndex == i);
                if (ImGui::MenuItem(themes[i], nullptr, selected)) {
                    themeIndex = i;
                    applyTheme(themeIndex);
                    showMessage(std::string("Theme: ") + themes[i], true);
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Reload Data", nullptr, false, isAuthenticated && role == UserRole::Librarian)) {
            library.clearAllData();
            bool ok = library.loadAllData();
            showMessage(ok ? "Reloaded successfully" : "Reload failed", ok);
        }
        if (ImGui::MenuItem("Clear All Data", nullptr, false, isAuthenticated && role == UserRole::Librarian)) {
            library.clearAllData();
            showMessage("All data cleared", true);
        }
        if (ImGui::MenuItem("Add Demo Books", nullptr, false, isAuthenticated && role == UserRole::Librarian)) {
            addMoreDemoBooksSafe();
            booksDirty = true;
            showMessage("Demo books added (skipped duplicates).", true);
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("Keyboard Shortcuts", "F1")) {
            showShortcutsWindow = true;
        }
        ImGui::Separator();
        ImGui::MenuItem("Dear ImGui Demo", nullptr, &showDemoWindow);
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();

    if (showDemoWindow) ImGui::ShowDemoWindow(&showDemoWindow);
    if (showShortcutsWindow) showShortcutsDialog();
}

void MainWindow::showSidebar() {
    if (fontHeading) ImGui::PushFont(fontHeading);
    ImGui::TextUnformatted("Navigation");
    if (fontHeading) ImGui::PopFont();
    ImGui::Separator();

    if (role == UserRole::Librarian) {
        const char* items[] = {"Dashboard", "Books", "Students", "Issue / Return", "Requests", "History", "Statistics"};
        for (int i = 0; i < 7; i++) {
            bool selected = (currentTab == i);
            if (ImGui::Selectable(items[i], selected, 0, ImVec2(0, 34))) currentTab = i;
        }
    } else {
        const char* items[] = {"Dashboard", "Books", "Assistant", "Return Book", "Requests"};
        // Map to internal tab indices:
        // 0=Dashboard, 1=Books, 2=Assistant, 3=Issue/Return (student shows return only), 4=Requests
        const int tabMap[] = {0, 1, 2, 3, 4};
        for (int i = 0; i < 5; i++) {
            bool selected = (currentTab == tabMap[i]);
            if (ImGui::Selectable(items[i], selected, 0, ImVec2(0, 34))) currentTab = tabMap[i];
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Keyboard Shortcuts");
    ImGui::Spacing();
    
    if (role == UserRole::Librarian) {
        ImGui::BulletText("Ctrl+S: Save");
        ImGui::BulletText("Ctrl+R: Reload");
        ImGui::BulletText("Ctrl+F: Search");
        ImGui::BulletText("Ctrl+Q: Quit");
        ImGui::BulletText("Ctrl+1-7: Tabs");
    } else {
        ImGui::BulletText("Ctrl+F: Search");
        ImGui::BulletText("Ctrl+Q: Quit");
    }
}

void MainWindow::showStatusBar() {
    ImGui::Separator();
    ImGui::BeginChild("StatusBar", ImVec2(0, 30), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextDisabled("User: %s", sessionDisplayName.empty() ? "-" : sessionDisplayName.c_str());
    ImGui::SameLine();
    ImGui::TextDisabled(" | Role: %s", role == UserRole::Librarian ? "Librarian" : "Student");
    ImGui::SameLine();
    ImGui::TextDisabled(" | Theme: %s",
                        themeIndex == 3 ? "Light Blue & White"
                        : themeIndex == 2 ? "Light Peach"
                        : themeIndex == 1 ? "Light Lavender"
                        : "Light Mint");
    ImGui::SameLine();
    ImGui::TextDisabled(" | Books: %d  Students: %d  Requests: %d",
                        currentStats.totalBooks, currentStats.totalStudents, currentStats.pendingRequests);
    ImGui::EndChild();
}

void MainWindow::refreshBooks() {
    allBooks = library.getAllBooks();
    booksDirty = false;
}

void MainWindow::refreshStudents() {
    allStudents = library.getAllStudents();
    studentsDirty = false;
}

void MainWindow::refreshTransactions() {
    transactions = library.getRecentTransactions(transactionHistoryLimit);
    transactionsDirty = false;
}

void MainWindow::showBookManagementTab() {
    if (booksDirty) refreshBooks();

    // SEARCH BAR
    ImGui::Text("Search & Filter");
    ImGui::Separator();
    ImGui::Columns(4, "bookSearchCols", false);
    ImGui::SetColumnWidth(0, 110);
    ImGui::SetColumnWidth(1, 220);
    ImGui::SetColumnWidth(2, 220);
    ImGui::SetColumnWidth(3, 120);

    ImGui::Text("ID:");
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    ImGui::InputInt("##BookSearchId", &searchBookId);
    ImGui::PopItemWidth();
    ImGui::NextColumn();

    ImGui::Text("Title:");
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##BookSearchTitle", searchBookTitle, sizeof(searchBookTitle));
    ImGui::PopItemWidth();
    ImGui::NextColumn();

    ImGui::Text("Author:");
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##BookSearchAuthor", searchBookAuthor, sizeof(searchBookAuthor));
    ImGui::PopItemWidth();
    ImGui::NextColumn();

    ImGui::Text(" ");
    ImGui::NextColumn();
    if (ImGui::Button("Search", ImVec2(-1, 0))) {
        bookSearchResults.clear();
        if (searchBookId > 0) {
            if (auto* b = library.searchBookById(searchBookId)) bookSearchResults.push_back(*b);
        } else if (std::strlen(searchBookTitle) > 0) {
            bookSearchResults = library.searchBookByTitle(searchBookTitle);
        } else if (std::strlen(searchBookAuthor) > 0) {
            bookSearchResults = library.searchBookByAuthor(searchBookAuthor);
        } else {
            bookSearchResults = allBooks;
        }
        
        // Apply filters
        if (filterAvailableOnly) {
            auto it = std::remove_if(bookSearchResults.begin(), bookSearchResults.end(),
                [](const Book& b) { return !b.isAvailable(); });
            bookSearchResults.erase(it, bookSearchResults.end());
        }
        
        // Apply sorting
        if (bookSortMode == 1) { // Sort by Title
            std::sort(bookSearchResults.begin(), bookSearchResults.end(),
                [](const Book& a, const Book& b) { return a.getTitle() < b.getTitle(); });
        } else if (bookSortMode == 2) { // Sort by Author
            std::sort(bookSearchResults.begin(), bookSearchResults.end(),
                [](const Book& a, const Book& b) { return a.getAuthor() < b.getAuthor(); });
        } else if (bookSortMode == 3) { // Sort by Most Borrowed
            std::sort(bookSearchResults.begin(), bookSearchResults.end(),
                [](const Book& a, const Book& b) { return a.getTimesBorrowed() > b.getTimesBorrowed(); });
        }
        // Default (0) is by ID, which is already sorted by BST
    }
    ImGui::Columns(1);
    
    // Filters and sort options
    ImGui::Checkbox("Available Only", &filterAvailableOnly);
    ImGui::SameLine();
    ImGui::Text(" | Sort by:");
    ImGui::SameLine();
    ImGui::RadioButton("ID", &bookSortMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Title", &bookSortMode, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Author", &bookSortMode, 2);
    ImGui::SameLine();
    ImGui::RadioButton("Popular", &bookSortMode, 3);

    ImGui::SameLine();
    if (ImGui::Button("Clear Search")) {
        searchBookId = 0;
        std::memset(searchBookTitle, 0, sizeof(searchBookTitle));
        std::memset(searchBookAuthor, 0, sizeof(searchBookAuthor));
        filterAvailableOnly = false;
        bookSortMode = 0;
        bookSearchResults = allBooks;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        booksDirty = true;
        refreshBooks();
        bookSearchResults = allBooks;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Export Books")) {
        std::string filename = "data/export_books_" + std::to_string(time(nullptr)) + ".csv";
        bool ok = library.exportBooksToCSV(filename);
        showMessage(ok ? ("Exported to " + filename) : "Export failed", ok);
    }

    ImGui::Spacing();
    ImGui::Separator();

    // TABLE
    ImGui::Text("Book Catalog");
    ImGui::BeginChild("BooksTableChild", ImVec2(0, 320), true, ImGuiWindowFlags_HorizontalScrollbar);

    const std::vector<Book>& rows = bookSearchResults.empty() ? allBooks : bookSearchResults;
    if (ImGui::BeginTable("BooksTable", 7,
                          ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                              ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable |
                              ImGuiTableFlags_Reorderable | ImGuiTableFlags_Sortable,
                          ImVec2(0, 300))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Author", ImGuiTableColumnFlags_WidthFixed, 180);
        ImGui::TableSetupColumn("ISBN", ImGuiTableColumnFlags_WidthFixed, 140);
        ImGui::TableSetupColumn("Total", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableSetupColumn("Avail", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 160);
        ImGui::TableHeadersRow();

        for (const auto& b : rows) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", b.getId());
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(b.getTitle().c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(b.getAuthor().c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(b.getIsbn().c_str());
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%d", b.getTotalQuantity());
            ImGui::TableSetColumnIndex(5);
            int avail = b.getAvailableCount();
            if (avail == 0) ImGui::TextColored(rgba(LibraryConfig::ERROR_COLOR), "%d", avail);
            else if (avail < 3) ImGui::TextColored(rgba(LibraryConfig::WARNING_COLOR), "%d", avail);
            else ImGui::TextColored(rgba(LibraryConfig::SUCCESS_COLOR), "%d", avail);

            ImGui::TableSetColumnIndex(6);
            if (role == UserRole::Librarian) {
                std::string editLabel = "Edit##" + std::to_string(b.getId());
                if (ImGui::SmallButton(editLabel.c_str())) {
                    modalBookId = b.getId();
                    std::snprintf(editBookTitle, sizeof(editBookTitle), "%s", b.getTitle().c_str());
                    std::snprintf(editBookAuthor, sizeof(editBookAuthor), "%s", b.getAuthor().c_str());
                    std::snprintf(editBookISBN, sizeof(editBookISBN), "%s", b.getIsbn().c_str());
                    editBookQuantity = b.getTotalQuantity();
                    showEditBookModal = true;
                    ImGui::OpenPopup("Edit Book");
                }
                ImGui::SameLine();
                std::string delLabel = "Delete##" + std::to_string(b.getId());
                if (ImGui::SmallButton(delLabel.c_str())) {
                    modalBookId = b.getId();
                    showDeleteBookModal = true;
                    ImGui::OpenPopup("Delete Book");
                }
            } else {
                ImGui::TextDisabled("View");
            }
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();

    // ADD FORM (Librarian only)
    if (role == UserRole::Librarian) {
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Add New Book", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Columns(2, "addBookCols", false);
        ImGui::SetColumnWidth(0, 150);

        ImGui::Text("Book ID:");
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
        ImGui::InputInt("##AddBookId", &bookId);
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        ImGui::Text("Title:*");
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##AddBookTitle", bookTitle, sizeof(bookTitle));
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        ImGui::Text("Author:*");
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##AddBookAuthor", bookAuthor, sizeof(bookAuthor));
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        ImGui::Text("ISBN:*");
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##AddBookISBN", bookISBN, sizeof(bookISBN));
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        ImGui::Text("Quantity:*");
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
        ImGui::InputInt("##AddBookQty", &bookQuantity);
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        ImGui::Columns(1);

        if (ImGui::Button("Add Book", ImVec2(120, 34))) {
            try {
                Book nb(bookId, bookTitle, bookAuthor, bookISBN, bookQuantity);
                bool ok = library.addBook(nb);
                showMessage(ok ? "Book added successfully" : "Failed to add book", ok);
                if (ok) {
                    booksDirty = true;
                    refreshBooks();
                    bookSearchResults = allBooks;
                    std::memset(bookTitle, 0, sizeof(bookTitle));
                    std::memset(bookAuthor, 0, sizeof(bookAuthor));
                    std::memset(bookISBN, 0, sizeof(bookISBN));
                    bookQuantity = 1;
                }
            } catch (const std::exception& e) {
                showMessage(std::string("Error: ") + e.what(), false);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear", ImVec2(120, 34))) {
            std::memset(bookTitle, 0, sizeof(bookTitle));
            std::memset(bookAuthor, 0, sizeof(bookAuthor));
            std::memset(bookISBN, 0, sizeof(bookISBN));
            bookQuantity = 1;
        }
        }

        // EDIT MODAL
        if (ImGui::BeginPopupModal("Edit Book", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Editing Book ID: %d", modalBookId);
        ImGui::Separator();
        ImGui::InputText("Title", editBookTitle, sizeof(editBookTitle));
        ImGui::InputText("Author", editBookAuthor, sizeof(editBookAuthor));
        ImGui::InputText("ISBN", editBookISBN, sizeof(editBookISBN));
        ImGui::InputInt("Total Quantity", &editBookQuantity);

        if (ImGui::Button("Save", ImVec2(120, 34))) {
            try {
                Book updated(modalBookId, editBookTitle, editBookAuthor, editBookISBN, editBookQuantity);
                bool ok = library.updateBook(modalBookId, updated);
                showMessage(ok ? "Book updated" : "Update failed", ok);
                if (ok) {
                    booksDirty = true;
                    refreshBooks();
                    bookSearchResults = allBooks;
                }
                ImGui::CloseCurrentPopup();
            } catch (const std::exception& e) {
                showMessage(std::string("Error: ") + e.what(), false);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 34))) {
            ImGui::CloseCurrentPopup();
        }
            ImGui::EndPopup();
        }

        // DELETE MODAL
        if (ImGui::BeginPopupModal("Delete Book", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Delete Book ID %d?", modalBookId);
        ImGui::TextDisabled("This cannot be undone.");
        ImGui::Separator();
        if (ImGui::Button("Delete", ImVec2(120, 34))) {
            bool ok = library.removeBook(modalBookId);
            showMessage(ok ? "Book deleted" : "Delete failed (maybe borrowed?)", ok);
            if (ok) {
                booksDirty = true;
                refreshBooks();
                bookSearchResults = allBooks;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 34))) {
            ImGui::CloseCurrentPopup();
        }
            ImGui::EndPopup();
        }
    }
}

void MainWindow::showStudentManagementTab() {
    if (role != UserRole::Librarian) {
        ImGui::TextDisabled("Student Management is available for Librarian only.");
        return;
    }
    if (studentsDirty) refreshStudents();

    // Search & filter
    ImGui::Text("Search Students");
    ImGui::Separator();
    ImGui::Columns(3, "studentSearchCols", false);
    ImGui::SetColumnWidth(0, 110);
    ImGui::SetColumnWidth(1, 260);
    ImGui::SetColumnWidth(2, 120);

    ImGui::Text("ID:");
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    ImGui::InputInt("##StudentSearchId", &searchStudentId);
    ImGui::PopItemWidth();
    ImGui::NextColumn();
    if (ImGui::Button("Search ID", ImVec2(-1, 0))) {
        studentSearchResults.clear();
        if (searchStudentId > 0) {
            if (auto* s = library.searchStudentById(searchStudentId)) studentSearchResults.push_back(*s);
        }
    }
    ImGui::NextColumn();

    ImGui::Text("Name:");
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##StudentSearchName", searchStudentName, sizeof(searchStudentName));
    ImGui::PopItemWidth();
    ImGui::NextColumn();
    if (ImGui::Button("Search Name", ImVec2(-1, 0))) {
        studentSearchResults = library.searchStudentByName(searchStudentName);
    }
    ImGui::NextColumn();

    ImGui::Columns(1);
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        studentsDirty = true;
        refreshStudents();
        studentSearchResults = allStudents;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Export Students")) {
        std::string filename = "data/export_students_" + std::to_string(time(nullptr)) + ".csv";
        bool ok = library.exportStudentsToCSV(filename);
        showMessage(ok ? ("Exported to " + filename) : "Export failed", ok);
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Table
    ImGui::Text("Student Directory");
    ImGui::BeginChild("StudentsTableChild", ImVec2(0, 320), true, ImGuiWindowFlags_HorizontalScrollbar);

    const std::vector<Student>& rows = studentSearchResults.empty() ? allStudents : studentSearchResults;
    if (ImGui::BeginTable("StudentsTable", 6,
                          ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable,
                          ImVec2(0, 300))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 90);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Department", ImGuiTableColumnFlags_WidthFixed, 180);
        ImGui::TableSetupColumn("Email", ImGuiTableColumnFlags_WidthFixed, 220);
        ImGui::TableSetupColumn("Borrowed", ImGuiTableColumnFlags_WidthFixed, 90);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 120);
        ImGui::TableHeadersRow();

        for (const auto& s : rows) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", s.getId());
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(s.getName().c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(s.getDepartment().c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(s.getEmail().c_str());
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%d/%d", s.getBorrowedCount(), Student::getMaxBorrowLimit());
            ImGui::TableSetColumnIndex(5);

            std::string del = "Delete##" + std::to_string(s.getId());
            if (ImGui::SmallButton(del.c_str())) {
                modalStudentId = s.getId();
                showDeleteStudentModal = true;
                ImGui::OpenPopup("Delete Student");
            }
        }

        ImGui::EndTable();
    }
    ImGui::EndChild();

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Register New Student", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Columns(2, "addStudentCols", false);
        ImGui::SetColumnWidth(0, 160);

        ImGui::Text("Student ID:");
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
        ImGui::InputInt("##AddStudentId", &studentId);
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        ImGui::Text("Name:*");
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##AddStudentName", studentName, sizeof(studentName));
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        ImGui::Text("Department:*");
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##AddStudentDept", studentDepartment, sizeof(studentDepartment));
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        ImGui::Text("Email:");
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##AddStudentEmail", studentEmail, sizeof(studentEmail));
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        ImGui::Text("Phone:");
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##AddStudentPhone", studentPhone, sizeof(studentPhone));
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        ImGui::Columns(1);

        if (ImGui::Button("Register", ImVec2(120, 34))) {
            try {
                Student ns(studentId, studentName, studentDepartment, studentEmail, studentPhone);
                bool ok = library.registerStudent(ns);
                showMessage(ok ? "Student registered" : "Register failed", ok);
                if (ok) {
                    studentsDirty = true;
                    refreshStudents();
                    studentSearchResults = allStudents;
                    std::memset(studentName, 0, sizeof(studentName));
                    std::memset(studentDepartment, 0, sizeof(studentDepartment));
                    std::memset(studentEmail, 0, sizeof(studentEmail));
                    std::memset(studentPhone, 0, sizeof(studentPhone));
                }
            } catch (const std::exception& e) {
                showMessage(std::string("Error: ") + e.what(), false);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear", ImVec2(120, 34))) {
            std::memset(studentName, 0, sizeof(studentName));
            std::memset(studentDepartment, 0, sizeof(studentDepartment));
            std::memset(studentEmail, 0, sizeof(studentEmail));
            std::memset(studentPhone, 0, sizeof(studentPhone));
        }
    }

    if (ImGui::BeginPopupModal("Delete Student", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Delete Student ID %d?", modalStudentId);
        ImGui::TextDisabled("Will fail if student has borrowed books.");
        ImGui::Separator();
        if (ImGui::Button("Delete", ImVec2(120, 34))) {
            bool ok = library.removeStudent(modalStudentId);
            showMessage(ok ? "Student deleted" : "Delete failed (borrowed books?)", ok);
            if (ok) {
                studentsDirty = true;
                refreshStudents();
                studentSearchResults = allStudents;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 34))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void MainWindow::showIssueReturnTab() {
    ImGui::Text("Issue / Return");
    ImGui::Separator();

    // Robust layout: table columns (avoids old Columns() sizing issues)
    if (ImGui::BeginTable("IssueReturnLayout", 2, ImGuiTableFlags_SizingStretchSame)) {
        // --------------------
        // Issue (Librarian)
        // --------------------
        ImGui::TableNextColumn();
        ImGui::BeginChild("IssuePanel", ImVec2(0, 260), true);
        ImGui::Text("Issue Book");
        ImGui::Separator();

        if (role != UserRole::Librarian) {
            ImGui::TextDisabled("Issue is available for Librarian only.");
            if (ImGui::Button("Switch to Librarian Login", ImVec2(220, 34))) {
                logout();
                loginMode = 1;
                std::memset(loginUsername, 0, sizeof(loginUsername));
                std::memset(loginPassword, 0, sizeof(loginPassword));
            }
        } else {
            ImGui::InputInt("Student ID##issue", &issueStudentId);
            ImGui::InputInt("Book ID##issue", &issueBookId);
            if (ImGui::Button("Issue Book", ImVec2(160, 34))) {
                BorrowResult r = library.issueBook(issueStudentId, issueBookId);
                showMessage(r.success ? ("Issued. Due: " + r.dueDate) : r.message, r.success);
                booksDirty = true;
                studentsDirty = true;
            }
        }
        ImGui::EndChild();

        // --------------------
        // Return (Student + Librarian)
        // --------------------
        ImGui::TableNextColumn();
        ImGui::BeginChild("ReturnPanel", ImVec2(0, 260), true);
        ImGui::Text("Return Book");
        ImGui::Separator();

        if (role == UserRole::Student) {
            returnStudentId = sessionStudentId;
            ImGui::BeginDisabled(true);
            ImGui::InputInt("Student ID##return", &returnStudentId);
            ImGui::EndDisabled();
        } else {
            ImGui::InputInt("Student ID##return", &returnStudentId);
        }

        ImGui::InputInt("Book ID##return", &returnBookId);
        if (ImGui::Button("Return Book", ImVec2(160, 34))) {
            ReturnResult r = library.returnBook(returnStudentId, returnBookId);
            showMessage(r.message, r.success);
            booksDirty = true;
            studentsDirty = true;
        }
        ImGui::EndChild();

        ImGui::EndTable();
    }
}

void MainWindow::showRequestQueueTab() {
    ImGui::Text("Request Queue");
    ImGui::Separator();

    ImGui::BeginChild("RequestControls", ImVec2(0, 120), true);
    ImGui::Columns(2, "rqCols", false);
    ImGui::SetColumnWidth(0, 260);
    if (role == UserRole::Student) {
        requestStudentId = sessionStudentId;
        ImGui::BeginDisabled(true);
        ImGui::InputInt("Student ID", &requestStudentId);
        ImGui::EndDisabled();
    } else {
        ImGui::InputInt("Student ID", &requestStudentId);
    }
    ImGui::InputInt("Book ID", &requestBookId);
    ImGui::NextColumn();
    ImGui::SliderInt("Priority", &requestPriority, 1, 5);
    if (ImGui::Button("Request", ImVec2(120, 34))) {
        RequestResult rr = library.requestBook(requestStudentId, requestBookId, requestPriority);
        showMessage(rr.message, rr.success);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 34))) {
        bool ok = library.cancelRequest(requestStudentId, requestBookId);
        showMessage(ok ? "Request cancelled" : "Request not found", ok);
    }
    ImGui::Columns(1);
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Text("Queues by Book");
    ImGui::BeginChild("QueuesTable", ImVec2(0, 0), true);

    auto books = library.getAllBooks();
    for (const auto& b : books) {
        auto q = library.getQueueForBook(b.getId());
        if (q.empty()) continue;

        std::string header = std::to_string(b.getId()) + " • " + b.getTitle() + " (" + std::to_string(q.size()) + ")";
        if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginTable(("rq_tbl_" + std::to_string(b.getId())).c_str(), 4,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("Pos", ImGuiTableColumnFlags_WidthFixed, 60);
                ImGui::TableSetupColumn("Request ID", ImGuiTableColumnFlags_WidthFixed, 110);
                ImGui::TableSetupColumn("Student ID", ImGuiTableColumnFlags_WidthFixed, 120);
                ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_WidthFixed, 90);
                ImGui::TableHeadersRow();
                for (size_t i = 0; i < q.size(); i++) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%zu", i + 1);
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", q[i].getRequestId());
                    ImGui::TableSetColumnIndex(2); ImGui::Text("%d", q[i].getStudentId());
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%d", q[i].getPriority());
                }
                ImGui::EndTable();
            }
        }
    }

    ImGui::EndChild();
}

void MainWindow::showAssistantTab() {
    if (role != UserRole::Student) {
        ImGui::TextDisabled("Assistant is available for Students.");
        return;
    }

    ImGui::TextColored(rgba(LibraryConfig::PRIMARY_COLOR), "🤖 Student Assistant (Offline AI)");
    ImGui::Separator();
    ImGui::TextDisabled("Ask me about: recommend | status | popular | available | search <author> | help");
    ImGui::Spacing();

    ImGui::BeginChild("ChatLog", ImVec2(0, -60), true);
    for (const auto& m : assistantChat) {
        ImGui::TextWrapped("%s", m.c_str());
        ImGui::Spacing();
    }
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::InputText("##assistant_input", assistantInput, sizeof(assistantInput));
    ImGui::SameLine();
    if (ImGui::Button("Send", ImVec2(90, 0))) {
        std::string userMsg = assistantInput;
        std::memset(assistantInput, 0, sizeof(assistantInput));
        if (!userMsg.empty()) {
            assistantChat.push_back("You: " + userMsg);

            // Very simple "AI" logic (no internet / no external API).
            std::string msgLower = userMsg;
            std::transform(msgLower.begin(), msgLower.end(), msgLower.begin(), ::tolower);

            auto books = library.getAllBooks();
            auto reply = std::string("Assistant: ");

            // Enhanced AI responses
            if (msgLower.find("help") != std::string::npos || msgLower == "?") {
                reply += "📚 I can help you with:\n\n"
                         "• \"recommend\" - Get personalized book suggestions\n"
                         "• \"popular\" - See the most borrowed books\n"
                         "• \"status\" - Check your borrowed books\n"
                         "• \"available\" - Count available books\n"
                         "• \"search <author>\" - Find books by author\n"
                         "• \"due\" - Check your due dates\n\n"
                         "💡 Tip: You can request books in the Requests tab!";
            } else if (msgLower.find("status") != std::string::npos || msgLower.find("my books") != std::string::npos) {
                Student* s = library.searchStudentById(sessionStudentId);
                if (s && s->getBorrowedBooks().size() > 0) {
                    reply += "📖 You currently have " + std::to_string(s->getBorrowedBooks().size()) + " book(s):\n";
                    for (int bookId : s->getBorrowedBooks()) {
                        Book* b = library.searchBookById(bookId);
                        if (b) {
                            reply += "• [" + std::to_string(b->getId()) + "] " + b->getTitle() + "\n";
                        }
                    }
                    reply += "\n💡 You can borrow up to " + std::to_string(3 - s->getBorrowedBooks().size()) + " more book(s).";
                } else {
                    reply += "📭 You haven't borrowed any books yet.\n\n"
                             "💡 Browse the Books tab and use the Requests tab to borrow!";
                }
            } else if (msgLower.find("due") != std::string::npos) {
                auto transactions = library.getRecentTransactions(100);
                bool found = false;
                reply += "📅 Your due dates:\n";
                for (const auto& t : transactions) {
                    if (t.getStudentId() == sessionStudentId && t.getType() == BORROW && t.getReturnDate().empty()) {
                        Book* b = library.searchBookById(t.getBookId());
                        std::string status = t.isOverdue() ? " ⚠️ OVERDUE!" : "";
                        reply += "• " + (b ? b->getTitle() : "Book #" + std::to_string(t.getBookId())) 
                                + " - Due: " + t.getDueDate() + status + "\n";
                        found = true;
                    }
                }
                if (!found) reply += "No active borrowings. You're all clear! ✅";
            } else if (msgLower.find("popular") != std::string::npos || msgLower.find("trending") != std::string::npos) {
                auto popular = library.getMostPopularBooks(5);
                if (!popular.empty()) {
                    reply += "🔥 Most Popular Books:\n";
                    int rank = 1;
                    for (const auto& p : popular) {
                        std::string medal = (rank == 1) ? "🥇" : (rank == 2) ? "🥈" : (rank == 3) ? "🥉" : "📖";
                        reply += medal + " [" + std::to_string(p.first.getId()) + "] " 
                                + p.first.getTitle() + " (" + std::to_string(p.second) + " borrows)\n";
                        rank++;
                    }
                } else {
                    reply += "No popularity data available yet.";
                }
            } else if (msgLower.find("recommend") != std::string::npos) {
                // Smart recommendations based on popularity, excluding already borrowed
                Student* s = library.searchStudentById(sessionStudentId);
                std::vector<int> borrowed = s ? s->getBorrowedBooks() : std::vector<int>{};
                std::sort(books.begin(), books.end(),
                          [](const Book& a, const Book& b) { return a.getTimesBorrowed() > b.getTimesBorrowed(); });
                reply += "✨ Personalized Recommendations:\n\n";
                int shown = 0;
                for (const auto& b : books) {
                    if (!b.isAvailable()) continue;
                    if (std::find(borrowed.begin(), borrowed.end(), b.getId()) != borrowed.end()) continue;
                    reply += "📚 [" + std::to_string(b.getId()) + "] " + b.getTitle() + "\n"
                            + "   by " + b.getAuthor() + " • " + std::to_string(b.getTimesBorrowed()) + " borrows\n\n";
                    if (++shown >= 5) break;
                }
                if (shown == 0) {
                    reply += "😔 No available books match your profile right now.\n\n"
                             "💡 Try requesting a popular book in the Requests tab!";
                } else {
                    reply += "💡 Go to Books tab → Search by ID to view details!";
                }
            } else if (msgLower.find("available") != std::string::npos || msgLower.find("count") != std::string::npos) {
                int availableCount = 0;
                int totalBooks = 0;
                for (const auto& b : books) {
                    totalBooks++;
                    if (b.isAvailable()) availableCount++;
                }
                float availRate = totalBooks > 0 ? (float)availableCount / totalBooks * 100 : 0;
                reply += "📊 Library Status:\n\n"
                         "• Available books: " + std::to_string(availableCount) + " / " + std::to_string(totalBooks) + "\n"
                         "• Availability rate: " + std::to_string((int)availRate) + "%\n\n"
                         "💡 Use the Books tab to search by title/author!";
            } else {
                // Smart search: Try author name or title keywords
                std::vector<Book> authorMatches, titleMatches;
                for (const auto& b : books) {
                    std::string a = b.getAuthor();
                    std::string t = b.getTitle();
                    std::transform(a.begin(), a.end(), a.begin(), ::tolower);
                    std::transform(t.begin(), t.end(), t.begin(), ::tolower);
                    if (a.find(msgLower) != std::string::npos) authorMatches.push_back(b);
                    else if (t.find(msgLower) != std::string::npos) titleMatches.push_back(b);
                }
                
                if (!authorMatches.empty()) {
                    reply += "🔍 Books by that author:\n\n";
                    int shown = 0;
                    for (const auto& b : authorMatches) {
                        std::string avail = b.isAvailable() ? "✅ Available" : "❌ Borrowed";
                        reply += "📖 [" + std::to_string(b.getId()) + "] " + b.getTitle() + " - " + avail + "\n";
                        if (++shown >= 6) break;
                    }
                    if (authorMatches.size() > 6) {
                        reply += "\n...and " + std::to_string(authorMatches.size() - 6) + " more. Search in Books tab!";
                    }
                } else if (!titleMatches.empty()) {
                    reply += "🔍 Books matching your search:\n\n";
                    int shown = 0;
                    for (const auto& b : titleMatches) {
                        std::string avail = b.isAvailable() ? "✅" : "❌";
                        reply += avail + " [" + std::to_string(b.getId()) + "] " + b.getTitle() + " by " + b.getAuthor() + "\n";
                        if (++shown >= 5) break;
                    }
                } else {
                    reply += "🤔 Hmm, I didn't quite catch that.\n\n"
                             "Try asking:\n"
                             "• \"recommend\" for book suggestions\n"
                             "• \"popular\" to see trending books\n"
                             "• \"status\" to check your borrowings\n"
                             "• \"help\" for more commands\n\n"
                             "Or search by author/title in the Books tab!";
                }
            }
            assistantChat.push_back(reply);
        }
    }
}

void MainWindow::showLoginScreen() {
    // Centered login panel
    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 size(520, 360);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - size.x) * 0.5f, (io.DisplaySize.y - size.y) * 0.5f),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    ImGui::BeginChild("LoginPanel", size, true);

    if (fontHeading) ImGui::PushFont(fontHeading);
    ImGui::TextUnformatted("Welcome");
    if (fontHeading) ImGui::PopFont();
    ImGui::TextDisabled("Choose mode and login");
    ImGui::Separator();

    ImGui::RadioButton("Student", &loginMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Librarian", &loginMode, 1);
    ImGui::Spacing();

    if (loginMode == 0) {
        ImGui::Text("Student Login");
        ImGui::InputInt("Student ID", &loginStudentId);
        ImGui::InputText("PIN (last 4 digits)", loginStudentPin, sizeof(loginStudentPin));
        ImGui::TextDisabled("Demo: Student 100001 PIN 0001, Student 100002 PIN 0002");

        if (ImGui::Button("Login", ImVec2(120, 34))) {
            if (loginAsStudent(loginStudentId, loginStudentPin)) {
                showLoginError = false;
                currentTab = 0;
            } else {
                showLoginError = true;
            }
        }
    } else {
        ImGui::Text("Librarian Login");
        ImGui::InputText("Username", loginUsername, sizeof(loginUsername));
        ImGui::InputText("Password", loginPassword, sizeof(loginPassword), ImGuiInputTextFlags_Password);
        ImGui::TextDisabled("Demo: admin / admin123");

        if (ImGui::Button("Login", ImVec2(120, 34))) {
            if (loginAsLibrarian(loginUsername, loginPassword)) {
                showLoginError = false;
                currentTab = 0;
            } else {
                showLoginError = true;
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Create Demo Users/Books", ImVec2(200, 34))) {
        ensureDemoStudentsSafe();
        addMoreDemoBooksSafe();
        booksDirty = studentsDirty = true;
        showMessage("Demo data ensured (skipped duplicates).", true);
    }

    if (showLoginError) {
        ImGui::Spacing();
        ImGui::TextColored(rgba(LibraryConfig::ERROR_COLOR), "%s",
                           loginErrorText.empty() ? "Login failed." : loginErrorText.c_str());
    }

    ImGui::EndChild();
}

std::string MainWindow::expectedStudentPin(int studentId) {
    // Default policy: last 4 digits of the student ID, left padded with 0s.
    int last4 = std::abs(studentId) % 10000;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << last4;
    return oss.str();
}

bool MainWindow::loginAsStudent(int studentId, const std::string& pin) {
    Student* s = library.searchStudentById(studentId);
    if (!s) {
        loginErrorText = "Student not found. Ask Librarian to register you.";
        return false;
    }
    if (pin != expectedStudentPin(studentId)) {
        loginErrorText = "Invalid PIN.";
        return false;
    }
    isAuthenticated = true;
    role = UserRole::Student;
    sessionStudentId = studentId;
    sessionDisplayName = s->getName();
    assistantChat.clear();
    assistantChat.push_back("Assistant: Hi " + sessionDisplayName + "! Type \"recommend\" for book suggestions.");
    return true;
}

bool MainWindow::loginAsLibrarian(const std::string& user, const std::string& pass) {
    // Demo credentials. (You can later move this to settings.ini.)
    if (user == "admin" && pass == "admin123") {
        isAuthenticated = true;
        role = UserRole::Librarian;
        sessionStudentId = 0;
        sessionDisplayName = "Admin Librarian";
        return true;
    }
    loginErrorText = "Invalid username/password.";
    return false;
}

void MainWindow::logout() {
    isAuthenticated = false;
    role = UserRole::None;
    sessionStudentId = 0;
    sessionDisplayName.clear();
    std::memset(loginPassword, 0, sizeof(loginPassword));
    currentTab = 0;
}

void MainWindow::seedDemoDataIfNeeded() {
    // Ensure 2 demo students and a set of demo books only if storage is empty.
    auto s = library.getAllStudents();
    auto b = library.getAllBooks();
    if (s.empty()) ensureDemoStudentsSafe();
    if (b.empty()) addMoreDemoBooksSafe();
}

void MainWindow::ensureDemoStudentsSafe() {
    if (!library.searchStudentById(100001)) {
        library.registerStudent(Student(100001, "Ali Raza", "Computer Science", "ali@example.com", "0300-0000001"));
    }
    if (!library.searchStudentById(100002)) {
        library.registerStudent(Student(100002, "Sara Khan", "Software Engineering", "sara@example.com", "0300-0000002"));
    }
}

void MainWindow::addMoreDemoBooksSafe() {
    struct DemoBookRow {
        int id;
        const char* title;
        const char* author;
        const char* isbn13;
        int qty;
    };
    const DemoBookRow demo[] = {
        {1001, "Clean Code", "Robert C. Martin", "9780132350884", 5},
        {1002, "The Pragmatic Programmer", "Andrew Hunt", "9780201616224", 4},
        {1003, "Design Patterns", "Erich Gamma", "9780201633610", 3},
        {1004, "Introduction to Algorithms", "Thomas H. Cormen", "9780262033848", 2},
        {1005, "Effective Modern C++", "Scott Meyers", "9781491903995", 4},
        {1006, "Operating System Concepts", "Abraham Silberschatz", "9781119800361", 2},
        {1007, "Computer Networks", "Andrew S. Tanenbaum", "9780132126953", 3},
        {1008, "Artificial Intelligence", "Stuart Russell", "9780136042594", 2},
        {1009, "Database System Concepts", "Abraham Silberschatz", "9780073523323", 3},
        {1010, "C++ Primer", "Stanley B. Lippman", "9780321714114", 4},
        {1011, "Refactoring", "Martin Fowler", "9780201485677", 3},
        {1012, "Head First Design Patterns", "Eric Freeman", "9780596007126", 3},
    };

    for (const auto& r : demo) {
        if (library.searchBookById(r.id)) continue;
        (void)library.addBook(Book(r.id, r.title, r.author, r.isbn13, r.qty));
    }
}

void MainWindow::showTransactionHistoryTab() {
    ImGui::Text("Transaction History");
    ImGui::Separator();

    ImGui::InputInt("Limit", &transactionHistoryLimit);
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        transactionsDirty = true;
        refreshTransactions();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Export Transactions")) {
        std::string filename = "data/export_transactions_" + std::to_string(time(nullptr)) + ".csv";
        bool ok = library.exportTransactionsToCSV(filename, transactionHistoryLimit);
        showMessage(ok ? ("Exported to " + filename) : "Export failed", ok);
    }

    if (transactionsDirty) refreshTransactions();

    if (transactions.empty()) {
        ImGui::TextDisabled("No transactions.");
        return;
    }

    ImGui::BeginChild("TxTableChild", ImVec2(0, 0), true);
    if (ImGui::BeginTable("TxTable", 6,
                          ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_Resizable,
                          ImVec2(0, 0))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 220);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 90);
        ImGui::TableSetupColumn("Student", ImGuiTableColumnFlags_WidthFixed, 110);
        ImGui::TableSetupColumn("Book", ImGuiTableColumnFlags_WidthFixed, 90);
        ImGui::TableSetupColumn("Issue", ImGuiTableColumnFlags_WidthFixed, 110);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 120);
        ImGui::TableHeadersRow();

        for (const auto& t : transactions) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(t.getTransactionId().c_str());
            ImGui::TableSetColumnIndex(1);
            if (t.getType() == BORROW) ImGui::TextColored(rgba(LibraryConfig::SUCCESS_COLOR), "BORROW");
            else ImGui::TextColored(rgba(LibraryConfig::PRIMARY_COLOR), "RETURN");
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", t.getStudentId());
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", t.getBookId());
            ImGui::TableSetColumnIndex(4);
            ImGui::TextUnformatted(t.getIssueDate().c_str());
            ImGui::TableSetColumnIndex(5);
            if (t.isOverdue()) ImGui::TextColored(rgba(LibraryConfig::ERROR_COLOR), "OVERDUE");
            else if (t.getType() == BORROW) ImGui::TextColored(rgba(LibraryConfig::WARNING_COLOR), "ACTIVE");
            else ImGui::TextColored(rgba(LibraryConfig::SUCCESS_COLOR), "DONE");
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

void MainWindow::showStatisticsTab() {
    ImGui::Text("Dashboard");
    ImGui::Separator();
    
    if (ImGui::Button("Export Statistics Report")) {
        std::string filename = "data/statistics_report_" + std::to_string(time(nullptr)) + ".txt";
        bool ok = library.exportStatisticsReport(filename);
        showMessage(ok ? ("Report exported to " + filename) : "Export failed", ok);
    }
    
    ImGui::Spacing();

    // Enhanced statistics cards with progress bars
    ImGui::Columns(3, "dashCols", false);
    
    // BOOKS CARD
    ImGui::BeginChild("card_books", ImVec2(0, 140), true);
    ImGui::TextColored(rgba(LibraryConfig::PRIMARY_COLOR), "📚 BOOKS");
    ImGui::Separator();
    ImGui::Text("Total Books: %d", currentStats.totalBooks);
    ImGui::Text("Available: %d", currentStats.availableBooks);
    ImGui::Text("Borrowed: %d", currentStats.borrowedBooks);
    
    // Availability progress bar
    float availabilityRate = currentStats.totalBooks > 0 
        ? (float)currentStats.availableBooks / currentStats.totalBooks : 0.0f;
    ImGui::Text("Availability:");
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.3f, 1.0f));
    ImGui::ProgressBar(availabilityRate, ImVec2(-1, 0), 
        (std::to_string((int)(availabilityRate * 100)) + "%").c_str());
    ImGui::PopStyleColor();
    ImGui::EndChild();
    ImGui::NextColumn();

    // STUDENTS CARD
    ImGui::BeginChild("card_students", ImVec2(0, 140), true);
    ImGui::TextColored(rgba(LibraryConfig::PRIMARY_COLOR), "👥 STUDENTS");
    ImGui::Separator();
    ImGui::Text("Total: %d", currentStats.totalStudents);
    ImGui::Text("Active: %d", currentStats.activeStudents);
    ImGui::Text("Inactive: %d", currentStats.totalStudents - currentStats.activeStudents);
    
    // Active rate progress bar
    float activeRate = currentStats.totalStudents > 0 
        ? (float)currentStats.activeStudents / currentStats.totalStudents : 0.0f;
    ImGui::Text("Active Rate:");
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
    ImGui::ProgressBar(activeRate, ImVec2(-1, 0), 
        (std::to_string((int)(activeRate * 100)) + "%").c_str());
    ImGui::PopStyleColor();
    ImGui::EndChild();
    ImGui::NextColumn();

    // TRANSACTIONS CARD
    ImGui::BeginChild("card_tx", ImVec2(0, 140), true);
    ImGui::TextColored(rgba(LibraryConfig::PRIMARY_COLOR), "📊 ACTIVITY");
    ImGui::Separator();
    ImGui::Text("Transactions: %d", currentStats.totalTransactions);
    ImGui::Text("Pending Requests: %d", currentStats.pendingRequests);
    ImGui::Text("Total Fines: $%.2f", currentStats.totalFinesCollected);
    
    // Average fines per transaction
    float avgFine = currentStats.totalTransactions > 0 
        ? currentStats.totalFinesCollected / currentStats.totalTransactions : 0.0f;
    ImGui::Text("Avg Fine: $%.2f", avgFine);
    ImGui::EndChild();
    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::Separator();

    // Popular Books Section with Visual Bars
    ImGui::TextColored(rgba(LibraryConfig::PRIMARY_COLOR), "🏆 Most Popular Books (Top 5)");
    if (popularBooks.empty()) {
        ImGui::TextDisabled("No borrowing data yet.");
        return;
    }
    
    ImGui::Spacing();
    int maxBorrows = popularBooks.empty() ? 1 : popularBooks[0].second;
    
    if (ImGui::BeginTable("PopularBooks", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Rank", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableSetupColumn("Book Details", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Popularity", ImGuiTableColumnFlags_WidthFixed, 200);
        ImGui::TableHeadersRow();
        
        int rank = 1;
        for (const auto& p : popularBooks) {
            ImGui::TableNextRow();
            
            // Rank with medal colors
            ImGui::TableSetColumnIndex(0);
            if (rank == 1) ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "#%d", rank);
            else if (rank == 2) ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "#%d", rank);
            else if (rank == 3) ImGui::TextColored(ImVec4(0.8f, 0.5f, 0.2f, 1.0f), "#%d", rank);
            else ImGui::Text("#%d", rank);
            
            // Book details
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", p.first.getTitle().c_str());
            ImGui::TextDisabled("by %s", p.first.getAuthor().c_str());
            
            // Popularity bar
            ImGui::TableSetColumnIndex(2);
            float ratio = maxBorrows > 0 ? (float)p.second / maxBorrows : 0.0f;
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, 
                ImVec4(0.2f + ratio * 0.3f, 0.5f + ratio * 0.3f, 1.0f - ratio * 0.2f, 1.0f));
            char label[32];
            snprintf(label, sizeof(label), "%d borrows", p.second);
            ImGui::ProgressBar(ratio, ImVec2(-1, 0), label);
            ImGui::PopStyleColor();
            
            rank++;
        }
        ImGui::EndTable();
    }
    
    // Additional Insights
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(rgba(LibraryConfig::PRIMARY_COLOR), "📈 Quick Insights");
    ImGui::Spacing();
    
    ImGui::Columns(2, "insightsCols", false);
    
    // Book utilization
    float utilizationRate = currentStats.totalBooks > 0 
        ? (float)currentStats.borrowedBooks / currentStats.totalBooks : 0.0f;
    ImGui::BulletText("Book Utilization: %.1f%%", utilizationRate * 100);
    
    // Student engagement
    float engagementRate = currentStats.totalStudents > 0 && currentStats.borrowedBooks > 0
        ? (float)currentStats.borrowedBooks / currentStats.totalStudents : 0.0f;
    ImGui::BulletText("Avg Books/Student: %.2f", engagementRate);
    ImGui::NextColumn();
    
    // Request fulfillment indicator
    if (currentStats.pendingRequests > 5) {
        ImGui::TextColored(rgba(LibraryConfig::WARNING_COLOR), 
            "⚠️ High demand (%d pending)", currentStats.pendingRequests);
    } else if (currentStats.pendingRequests > 0) {
        ImGui::BulletText("Pending Requests: %d", currentStats.pendingRequests);
    } else {
        ImGui::TextColored(rgba(LibraryConfig::SUCCESS_COLOR), "✓ All requests fulfilled");
    }
    
    ImGui::Columns(1);
}

void MainWindow::showMessage(const std::string& message, bool success) {
    lastMessage = message;
    messageSuccess = success;
    showMessageFlag = true;
    messageTimer.restart();
}

void MainWindow::setupStyle() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.TabRounding = 6.0f;
    style.WindowPadding = ImVec2(14, 12);
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(10, 8);
    style.FrameBorderSize = 1.0f;
    style.WindowBorderSize = 1.0f;

    applyTheme(themeIndex);
}

void MainWindow::applyTheme(int idx) {
    // Curated light themes. Goal: avoid "blue/black" vibe.
    // idx: 0=Mint, 1=Lavender, 2=Peach, 3=Light Blue & White
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    ImVec4 accent;
    ImVec4 bg;
    ImVec4 panel;
    ImVec4 border;
    ImVec4 text;
    ImVec4 textDisabled;

    if (idx == 3) { // Light Blue & White (requested)
        accent = ImVec4(0.25f, 0.64f, 0.96f, 1.00f);      // sky blue accent
        bg = ImVec4(0.98f, 0.99f, 1.00f, 1.00f);          // near-white blue tint
        panel = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);       // pure white
        border = ImVec4(0.83f, 0.87f, 0.92f, 0.95f);      // cool border
        text = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);        // dark gray (not pure black)
        textDisabled = ImVec4(0.52f, 0.56f, 0.60f, 1.00f);
    } else if (idx == 1) { // Lavender
        accent = ImVec4(0.62f, 0.45f, 0.92f, 1.00f);
        bg = ImVec4(0.98f, 0.97f, 0.99f, 1.00f);
        panel = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        border = ImVec4(0.84f, 0.83f, 0.88f, 0.95f);
        text = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        textDisabled = ImVec4(0.50f, 0.48f, 0.55f, 1.00f);
    } else if (idx == 2) { // Peach
        accent = ImVec4(0.95f, 0.55f, 0.32f, 1.00f);
        bg = ImVec4(0.99f, 0.97f, 0.95f, 1.00f);
        panel = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        border = ImVec4(0.88f, 0.84f, 0.82f, 0.95f);
        text = ImVec4(0.12f, 0.10f, 0.10f, 1.00f);
        textDisabled = ImVec4(0.55f, 0.50f, 0.48f, 1.00f);
    } else { // Mint (default)
        accent = ImVec4(0.16f, 0.73f, 0.55f, 1.00f);
        bg = ImVec4(0.96f, 0.98f, 0.97f, 1.00f);
        panel = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        border = ImVec4(0.82f, 0.86f, 0.84f, 0.95f);
        text = ImVec4(0.07f, 0.09f, 0.10f, 1.00f);
        textDisabled = ImVec4(0.47f, 0.50f, 0.50f, 1.00f);
    }

    colors[ImGuiCol_Text] = text;
    colors[ImGuiCol_TextDisabled] = textDisabled;

    colors[ImGuiCol_WindowBg] = bg;
    colors[ImGuiCol_ChildBg]  = bg;
    colors[ImGuiCol_PopupBg]  = panel;
    colors[ImGuiCol_Border]   = border;

    colors[ImGuiCol_TitleBg]          = bg;
    colors[ImGuiCol_TitleBgActive]    = bg;
    colors[ImGuiCol_TitleBgCollapsed] = bg;
    colors[ImGuiCol_MenuBarBg]        = bg;

    colors[ImGuiCol_FrameBg]        = panel;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(panel.x, panel.y, panel.z, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(panel.x, panel.y, panel.z, 1.00f);
    colors[ImGuiCol_ScrollbarBg]    = bg;

    colors[ImGuiCol_Button]        = ImVec4(accent.x, accent.y, accent.z, 0.88f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(accent.x, accent.y, accent.z, 1.00f);
    colors[ImGuiCol_ButtonActive]  = ImVec4(accent.x * 0.90f, accent.y * 0.90f, accent.z * 0.90f, 1.00f);

    colors[ImGuiCol_Header]        = ImVec4(accent.x, accent.y, accent.z, 0.16f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(accent.x, accent.y, accent.z, 0.26f);
    colors[ImGuiCol_HeaderActive]  = ImVec4(accent.x, accent.y, accent.z, 0.34f);

    colors[ImGuiCol_Tab]           = ImVec4(accent.x, accent.y, accent.z, 0.10f);
    colors[ImGuiCol_TabHovered]    = ImVec4(accent.x, accent.y, accent.z, 0.18f);
    colors[ImGuiCol_TabSelected]   = ImVec4(accent.x, accent.y, accent.z, 0.22f);

    colors[ImGuiCol_CheckMark]        = accent;
    colors[ImGuiCol_SliderGrab]       = accent;
    colors[ImGuiCol_SliderGrabActive] = accent;

    colors[ImGuiCol_TableHeaderBg]       = ImVec4(accent.x, accent.y, accent.z, 0.12f);
    colors[ImGuiCol_TableBorderStrong]   = border;
    colors[ImGuiCol_TableBorderLight]    = ImVec4(border.x, border.y, border.z, 0.70f);
    colors[ImGuiCol_TableRowBg]          = ImVec4(panel.x, panel.y, panel.z, 0.70f);
    colors[ImGuiCol_TableRowBgAlt]       = ImVec4(bg.x, bg.y, bg.z, 0.70f);

    // Keep consistency with existing semantic colors (success/error/warn)
    colors[ImGuiCol_PlotLines] = accent;
    colors[ImGuiCol_PlotHistogram] = accent;
}

void MainWindow::setupFonts() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    // The repo may not contain a font file; fall back safely to ImGui default.
    fontBody = nullptr;
    fontHeading = nullptr;
    if (!LibraryConfig::FONT_PATH.empty()) {
        fontBody = io.Fonts->AddFontFromFileTTF(LibraryConfig::FONT_PATH.c_str(),
                                                static_cast<float>(LibraryConfig::FONT_SIZE));
        fontHeading = io.Fonts->AddFontFromFileTTF(LibraryConfig::FONT_PATH.c_str(), 22.0f);
    }
    if (!fontBody) {
        fontBody = io.Fonts->AddFontDefault();
        fontHeading = fontBody;
    }
    (void)ImGui::SFML::UpdateFontTexture();
}

void MainWindow::showDashboardTab() {
    // Quick KPI cards
    ImGui::Text("Overview");
    ImGui::Separator();

    ImGui::Columns(3, "kpiCols", false);
    ImGui::BeginChild("kpi_books", ImVec2(0, 88), true);
    ImGui::TextDisabled("Total Books");
    if (fontHeading) ImGui::PushFont(fontHeading);
    ImGui::Text("%d", currentStats.totalBooks);
    if (fontHeading) ImGui::PopFont();
    ImGui::EndChild();
    ImGui::NextColumn();

    ImGui::BeginChild("kpi_students", ImVec2(0, 88), true);
    ImGui::TextDisabled("Total Students");
    if (fontHeading) ImGui::PushFont(fontHeading);
    ImGui::Text("%d", currentStats.totalStudents);
    if (fontHeading) ImGui::PopFont();
    ImGui::EndChild();
    ImGui::NextColumn();

    ImGui::BeginChild("kpi_requests", ImVec2(0, 88), true);
    ImGui::TextDisabled("Pending Requests");
    if (fontHeading) ImGui::PushFont(fontHeading);
    ImGui::Text("%d", currentStats.pendingRequests);
    if (fontHeading) ImGui::PopFont();
    ImGui::EndChild();
    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::Separator();

    // Recent transactions snippet (for screenshots)
    if (transactionsDirty) refreshTransactions();
    ImGui::Text("Recent Transactions");
    if (transactions.empty()) {
        ImGui::TextDisabled("No transactions yet.");
    } else if (ImGui::BeginTable("dash_tx", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 90);
        ImGui::TableSetupColumn("Student", ImGuiTableColumnFlags_WidthFixed, 110);
        ImGui::TableSetupColumn("Book", ImGuiTableColumnFlags_WidthFixed, 90);
        ImGui::TableHeadersRow();
        int shown = 0;
        for (const auto& t : transactions) {
            if (shown++ >= 7) break;
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(t.getTransactionId().c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(t.getTypeString().c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", t.getStudentId());
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", t.getBookId());
        }
        ImGui::EndTable();
    }
}

bool MainWindow::inputText(const char* label, char* buffer, size_t bufferSize) {
    return ImGui::InputText(label, buffer, bufferSize);
}

bool MainWindow::centerButton(const char* label, float width) {
    if (width <= 0.0f) width = ImGui::CalcTextSize(label).x + 20.0f;
    float windowWidth = ImGui::GetWindowSize().x;
    float buttonX = (windowWidth - width) / 2.0f;
    ImGui::SetCursorPosX(std::max(0.0f, buttonX));
    return ImGui::Button(label, ImVec2(width, 0));
}

void MainWindow::showMessageBox() {
    if (messageTimer.getElapsedTime().asSeconds() > 3.0f) {
        showMessageFlag = false;
        return;
    }

    ImVec2 windowSize(520, 90);
    ImVec2 windowPos((ImGui::GetIO().DisplaySize.x - windowSize.x) / 2.0f, 24.0f);
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);

    ImGui::PushStyleColor(ImGuiCol_WindowBg,
                          messageSuccess
                              ? ImVec4(LibraryConfig::SUCCESS_COLOR[0], LibraryConfig::SUCCESS_COLOR[1],
                                       LibraryConfig::SUCCESS_COLOR[2], 0.92f)
                              : ImVec4(LibraryConfig::ERROR_COLOR[0], LibraryConfig::ERROR_COLOR[1],
                                       LibraryConfig::ERROR_COLOR[2], 0.92f));

    ImGui::Begin("##Toast", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    ImGui::TextWrapped("%s", lastMessage.c_str());
    ImGui::End();
    ImGui::PopStyleColor();
}

void MainWindow::showShortcutsDialog() {
    ImGui::SetNextWindowSize(ImVec2(550, 450), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), 
                            ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    
    if (!ImGui::Begin("Keyboard Shortcuts", &showShortcutsWindow)) {
        ImGui::End();
        return;
    }
    
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Keyboard Shortcuts Reference");
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::BeginTable("ShortcutsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Shortcut", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        
        // General shortcuts
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "F1");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextWrapped("Show this help dialog");
        
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ctrl+Q");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextWrapped("Quit application");
        
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ctrl+F");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextWrapped("Focus on search (switches to Books tab)");
        
        if (role == UserRole::Librarian) {
            // Librarian-only shortcuts
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Librarian Only:");
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ctrl+S");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextWrapped("Save all data to files");
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ctrl+R");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextWrapped("Reload all data from files");
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ctrl+1");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextWrapped("Switch to Dashboard tab");
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ctrl+2");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextWrapped("Switch to Books tab");
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ctrl+3");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextWrapped("Switch to Students tab");
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ctrl+4");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextWrapped("Switch to Issue/Return tab");
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ctrl+5");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextWrapped("Switch to Requests tab");
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ctrl+6");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextWrapped("Switch to History tab");
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ctrl+7");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextWrapped("Switch to Statistics tab");
        }
        
        ImGui::EndTable();
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextDisabled("Tip: Press F1 anytime to view this help");
    
    ImGui::Spacing();
    if (centerButton("Close", 120.0f)) {
        showShortcutsWindow = false;
    }
    
    ImGui::End();
}

std::string MainWindow::formatBookInfo(const Book& book) {
    return book.getTitle() + " by " + book.getAuthor();
}

std::string MainWindow::formatStudentInfo(const Student& student) {
    return student.getName() + " (" + student.getDepartment() + ")";
}

std::string MainWindow::formatTransactionInfo(const Transaction& trans) {
    return trans.getTransactionId() + ": " + trans.getTypeString();
}
