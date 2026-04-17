#include "panel.h"
#include "cad_adapter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <sstream>
#include <iomanip>
#include <QInputDialog>

namespace ModulusLite {

UIPanel::UIPanel(QWidget* parent)
    : QDockWidget("Modulus Lite - Block Manager", parent),
      m_pMachines(nullptr),
      m_pPipes(nullptr),
      m_pPipeline(nullptr) {
    
    setupUI();
}

UIPanel::~UIPanel() {
    // Qt cleanup is automatic
}

void UIPanel::setupUI() {
    // Create central widget
    QWidget* pCentral = new QWidget(this);
    QVBoxLayout* pMainLayout = new QVBoxLayout(pCentral);
    
    // Title
    QLabel* pTitle = new QLabel("Modulus Lite - Block Manager & Layout Tool", this);
    QFont titleFont = pTitle->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    pTitle->setFont(titleFont);
    pMainLayout->addWidget(pTitle);
    
    // Create tab widget
    m_pTabWidget = new QTabWidget(this);
    pMainLayout->addWidget(m_pTabWidget);
    
    // Setup individual tabs
    setupMainTab();
    setupBlockLibraryTab();
    setupMachineAssignmentTab();
    setupPipesTab();
    setupStatisticsTab();
    
    pMainLayout->addStretch();
    setWidget(pCentral);
    
    // Set initial state
    m_pBtnLayout->setEnabled(false);
    m_pBtnRoute->setEnabled(false);
}

void UIPanel::setupMainTab() {
    QWidget* pTab = new QWidget();
    QVBoxLayout* pLayout = new QVBoxLayout(pTab);
    
    QLabel* pLabel = new QLabel("Pipeline Generation", this);
    QFont font = pLabel->font();
    font.setBold(true);
    pLabel->setFont(font);
    pLayout->addWidget(pLabel);
    
    // Buttons
    QHBoxLayout* pBtnLayout = new QHBoxLayout();
    m_pBtnLayout = new QPushButton("1. Generate Layout (FDG)", this);
    m_pBtnRoute = new QPushButton("2. Route Pipes (A*)", this);
    
    pBtnLayout->addWidget(m_pBtnLayout);
    pBtnLayout->addWidget(m_pBtnRoute);
    pLayout->addLayout(pBtnLayout);
    
    // Status
    m_pStatusLabel = new QLabel("Ready", this);
    m_pStatusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    pLayout->addWidget(m_pStatusLabel);
    
    // Separator
    QFrame* pLine = new QFrame(this);
    pLine->setFrameShape(QFrame::HLine);
    pLayout->addWidget(pLine);
    
    // Instructions
    QLabel* pInstructions = new QLabel(
        "1. Import machine DWG blocks using the 'Block Library' tab\n"
        "2. Assign blocks to machines using 'Machine Assignment' tab\n"
        "3. Click 'Generate Layout' to position machines\n"
        "4. Click 'Route Pipes' to auto-route and draw pipes\n"
        "5. View results in AutoCAD drawing",
        this
    );
    pInstructions->setWordWrap(true);
    pInstructions->setStyleSheet("QLabel { background-color: #f5f5f5; padding: 10px; }");
    pLayout->addWidget(pInstructions);
    
    pLayout->addStretch();
    m_pTabWidget->addTab(pTab, "Layout & Routing");
    
    // Connect signals
    connect(m_pBtnLayout, &QPushButton::clicked, this, &UIPanel::onGenerateLayout);
    connect(m_pBtnRoute, &QPushButton::clicked, this, &UIPanel::onRoutePipes);
}

void UIPanel::setupBlockLibraryTab() {
    QWidget* pTab = new QWidget();
    QVBoxLayout* pLayout = new QVBoxLayout(pTab);
    
    // Title
    QLabel* pTitle = new QLabel("DWG Block Library Management", this);
    QFont font = pTitle->font();
    font.setBold(true);
    pTitle->setFont(font);
    pLayout->addWidget(pTitle);
    
    // Import buttons
    QHBoxLayout* pImportLayout = new QHBoxLayout();
    m_pBtnImportDWG = new QPushButton("Import DWG File...", this);
    m_pBtnImportMultiple = new QPushButton("Import Multiple DWGs...", this);
    m_pBtnExportLib = new QPushButton("Export Library...", this);
    m_pBtnImportLib = new QPushButton("Import Library Bundle...", this);
    
    pImportLayout->addWidget(m_pBtnImportDWG);
    pImportLayout->addWidget(m_pBtnImportMultiple);
    pImportLayout->addWidget(m_pBtnExportLib);
    pImportLayout->addWidget(m_pBtnImportLib);
    pLayout->addLayout(pImportLayout);
    
    // Separator
    QFrame* pLine1 = new QFrame(this);
    pLine1->setFrameShape(QFrame::HLine);
    pLayout->addWidget(pLine1);
    
    // Category filter
    QHBoxLayout* pCatLayout = new QHBoxLayout();
    pCatLayout->addWidget(new QLabel("Filter by Category:", this));
    m_pCategoryCombo = new QComboBox(this);
    m_pCategoryCombo->addItem("All Categories");
    pCatLayout->addWidget(m_pCategoryCombo);
    pCatLayout->addStretch();
    pLayout->addLayout(pCatLayout);
    
    // Block list
    QLabel* pBlockLabel = new QLabel("Available Blocks:", this);
    pLayout->addWidget(pBlockLabel);
    
    m_pBlockListWidget = new QListWidget(this);
    m_pBlockListWidget->setMaximumHeight(200);
    pLayout->addWidget(m_pBlockListWidget);
    
    // Block info
    m_pBlockInfoLabel = new QLabel("Select a block to view details", this);
    m_pBlockInfoLabel->setWordWrap(true);
    m_pBlockInfoLabel->setStyleSheet("QLabel { background-color: #e8f4f8; padding: 8px; }");
    pLayout->addWidget(m_pBlockInfoLabel);
    
    // Management buttons
    QHBoxLayout* pMgmtLayout = new QHBoxLayout();
    m_pBtnRemoveBlock = new QPushButton("Remove Selected Block", this);
    m_pBtnResetDefaults = new QPushButton("Reset to Defaults", this);
    pMgmtLayout->addWidget(m_pBtnRemoveBlock);
    pMgmtLayout->addWidget(m_pBtnResetDefaults);
    pMgmtLayout->addStretch();
    pLayout->addLayout(pMgmtLayout);
    
    pLayout->addStretch();
    m_pTabWidget->addTab(pTab, "Block Library");
    
    // Connect signals
    connect(m_pBtnImportDWG, &QPushButton::clicked, this, &UIPanel::onImportDWG);
    connect(m_pBtnImportMultiple, &QPushButton::clicked, this, &UIPanel::onImportMultipleDWG);
    connect(m_pBtnExportLib, &QPushButton::clicked, this, &UIPanel::onExportLibrary);
    connect(m_pBtnImportLib, &QPushButton::clicked, this, &UIPanel::onImportLibrary);
    connect(m_pBtnRemoveBlock, &QPushButton::clicked, this, &UIPanel::onRemoveSelectedBlock);
    connect(m_pBtnResetDefaults, &QPushButton::clicked, this, &UIPanel::onResetToDefaults);
    connect(m_pCategoryCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &UIPanel::onCategoryChanged);
    connect(m_pBlockListWidget, &QListWidget::itemSelectionChanged,
            this, [this]() {
                int row = m_pBlockListWidget->currentRow();
                if (row >= 0) {
                    auto& lib = BlockLibraryManager::instance();
                    auto blocks = lib.getAllBlocks();
                    if (row < (int)blocks.size()) {
                        updateBlockInfo(blocks[row]);
                    }
                }
            });
}

void UIPanel::setupMachineAssignmentTab() {
    QWidget* pTab = new QWidget();
    QVBoxLayout* pLayout = new QVBoxLayout(pTab);
    
    // Title
    QLabel* pTitle = new QLabel("Assign DWG Blocks to Machines", this);
    QFont font = pTitle->font();
    font.setBold(true);
    pTitle->setFont(font);
    pLayout->addWidget(pTitle);
    
    // Machine selector
    QHBoxLayout* pMachineLayout = new QHBoxLayout();
    pMachineLayout->addWidget(new QLabel("Select Machine:", this));
    m_pMachineCombo = new QComboBox(this);
    pMachineLayout->addWidget(m_pMachineCombo);
    pMachineLayout->addStretch();
    pLayout->addLayout(pMachineLayout);
    
    // Machine info
    m_pMachineInfoLabel = new QLabel("Select a machine to view details", this);
    m_pMachineInfoLabel->setWordWrap(true);
    m_pMachineInfoLabel->setStyleSheet("QLabel { background-color: #f0e8f4; padding: 8px; }");
    pLayout->addWidget(m_pMachineInfoLabel);
    
    // Separator
    QFrame* pLine = new QFrame(this);
    pLine->setFrameShape(QFrame::HLine);
    pLayout->addWidget(pLine);
    
    // Available blocks
    QLabel* pBlockLabel = new QLabel("Available Blocks to Assign:", this);
    pLayout->addWidget(pBlockLabel);
    
    m_pAvailableBlocksList = new QListWidget(this);
    m_pAvailableBlocksList->setMaximumHeight(200);
    pLayout->addWidget(m_pAvailableBlocksList);
    
    // Assign button
    m_pBtnAssignBlock = new QPushButton("Assign Selected Block to Machine", this);
    m_pBtnAssignBlock->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }");
    pLayout->addWidget(m_pBtnAssignBlock);
    
    pLayout->addStretch();
    m_pTabWidget->addTab(pTab, "Machine Assignment");
    
    // Connect signals
    connect(m_pMachineCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                if (idx >= 0 && m_pMachines && idx < (int)m_pMachines->size()) {
                    updateMachineInfo((*m_pMachines)[idx]);
                    refreshBlockList();
                }
            });
    connect(m_pBtnAssignBlock, &QPushButton::clicked, this, &UIPanel::onAssignBlockToMachine);
}

void UIPanel::setupPipesTab() {
    QWidget* pTab = new QWidget();
    QVBoxLayout* pLayout = new QVBoxLayout(pTab);
    
    // Title
    QLabel* pTitle = new QLabel("Pipe Information & Details", this);
    QFont font = pTitle->font();
    font.setBold(true);
    pTitle->setFont(font);
    pLayout->addWidget(pTitle);
    
    // Pipe list
    QLabel* pListLabel = new QLabel("Routed Pipes:", this);
    pLayout->addWidget(pListLabel);
    
    m_pPipeListWidget = new QListWidget(this);
    m_pPipeListWidget->setMaximumHeight(200);
    pLayout->addWidget(m_pPipeListWidget);
    
    // Pipe info
    m_pPipeInfoLabel = new QLabel("Select a pipe to view details", this);
    m_pPipeInfoLabel->setWordWrap(true);
    m_pPipeInfoLabel->setStyleSheet("QLabel { background-color: #f4f8e8; padding: 8px; }");
    pLayout->addWidget(m_pPipeInfoLabel);
    
    pLayout->addStretch();
    m_pTabWidget->addTab(pTab, "Pipes");
    
    // Connect signals
    connect(m_pPipeListWidget, &QListWidget::itemSelectionChanged,
            this, [this]() {
                int row = m_pPipeListWidget->currentRow();
                if (row >= 0 && m_pPipes && row < (int)m_pPipes->size()) {
                    updatePipeInfo((*m_pPipes)[row]);
                }
            });
}

void UIPanel::setupStatisticsTab() {
    QWidget* pTab = new QWidget();
    QVBoxLayout* pLayout = new QVBoxLayout(pTab);
    
    // Title
    QLabel* pTitle = new QLabel("Library Statistics & Summary", this);
    QFont font = pTitle->font();
    font.setBold(true);
    pTitle->setFont(font);
    pLayout->addWidget(pTitle);
    
    // Stats table
    m_pStatsTable = new QTableWidget(this);
    m_pStatsTable->setColumnCount(2);
    m_pStatsTable->setHorizontalHeaderLabels({"Metric", "Value"});
    m_pStatsTable->horizontalHeader()->setStretchLastSection(true);
    pLayout->addWidget(m_pStatsTable);
    
    // Library info
    m_pLibraryStatsLabel = new QLabel();
    m_pLibraryStatsLabel->setWordWrap(true);
    m_pLibraryStatsLabel->setStyleSheet("QLabel { background-color: #f9f9f9; padding: 10px; }");
    pLayout->addWidget(m_pLibraryStatsLabel);
    
    pLayout->addStretch();
    m_pTabWidget->addTab(pTab, "Statistics");
}

// ========================================================================
// SLOT IMPLEMENTATIONS
// ========================================================================

void UIPanel::onGenerateLayout() {
    if (!m_pPipeline || !m_pMachines) {
        updateStatus("No pipeline or machines available");
        return;
    }
    
    updateStatus("Generating layout...");
    
    // Run FDG layout
    FDGLayout::layout(*m_pMachines);
    
    updateStatus("Layout generated successfully");
    refreshMachineList();
}

void UIPanel::onRoutePipes() {
    if (!m_pPipeline || !m_pPipes) {
        updateStatus("No pipeline or pipes available");
        return;
    }
    
    updateStatus("Routing pipes...");
    
    // Execute pipeline
    if (m_pPipeline && m_pMachines && m_pPipes) {
        if (m_pPipeline->execute(*m_pMachines, *m_pPipes)) {
            updateStatus("Pipes routed and drawn successfully");
            refreshPipeList();
        } else {
            updateStatus("ERROR: Pipe routing failed");
        }
    }
}

void UIPanel::onImportDWG() {
    std::string filePath = openFileDialog("Import DWG File", "DWG Files (*.dwg);;All Files (*)");
    if (filePath.empty()) return;
    
    // Show category dialog
    QInputDialog* pDialog = new QInputDialog(this);
    pDialog->setWindowTitle("Enter Category");
    pDialog->setLabelText("Category for this block (e.g., Pump, Tank, Valve):");
    pDialog->setTextValue("Equipment");
    
    if (pDialog->exec() == QDialog::Accepted) {
        std::string category = pDialog->textValue().toStdString();
        
        auto& lib = BlockLibraryManager::instance();
        auto blocks = lib.importDWGFile(filePath, category);
        
        updateStatus(std::string("Imported ") + std::to_string(blocks.size()) + " blocks");
        refreshBlockList();
        refreshStatistics();
    }
    
    delete pDialog;
}

void UIPanel::onImportMultipleDWG() {
    auto filePaths = openMultiFileDialog("Import Multiple DWG Files");
    if (filePaths.empty()) return;
    
    // Show category dialog
    QInputDialog* pDialog = new QInputDialog(this);
    pDialog->setWindowTitle("Enter Category");
    pDialog->setLabelText("Category for all blocks:");
    pDialog->setTextValue("Equipment");
    
    if (pDialog->exec() == QDialog::Accepted) {
        std::string category = pDialog->textValue().toStdString();
        
        auto& lib = BlockLibraryManager::instance();
        std::vector<std::string> categories(filePaths.size(), category);
        int imported = lib.importMultipleDWGFiles(filePaths, categories);
        
        updateStatus(std::string("Imported ") + std::to_string(imported) + " blocks from " +
                    std::to_string(filePaths.size()) + " files");
        refreshBlockList();
        refreshStatistics();
    }
    
    delete pDialog;
}

void UIPanel::onRemoveSelectedBlock() {
    int row = m_pBlockListWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a block to remove");
        return;
    }
    
    auto& lib = BlockLibraryManager::instance();
    auto blocks = lib.getAllBlocks();
    if (row >= (int)blocks.size()) return;
    
    if (QMessageBox::question(this, "Confirm Removal",
        "Remove block: " + QString::fromStdString(blocks[row].displayName) + "?") 
        == QMessageBox::Yes) {
        
        lib.removeBlock(blocks[row].blockName);
        refreshBlockList();
        refreshStatistics();
    }
}

void UIPanel::onResetToDefaults() {
    if (QMessageBox::question(this, "Confirm Reset",
        "Reset library to default blocks? (All imports will be lost)") 
        == QMessageBox::Yes) {
        
        auto& lib = BlockLibraryManager::instance();
        lib.resetToDefaults();
        
        refreshBlockList();
        refreshStatistics();
        updateStatus("Library reset to defaults");
    }
}

void UIPanel::onExportLibrary() {
    // Show folder selection dialog
    QString folderPath = QFileDialog::getExistingDirectory(this, "Select Export Folder");
    if (folderPath.isEmpty()) return;
    
    auto& lib = BlockLibraryManager::instance();
    if (lib.exportLibrary(folderPath.toStdString())) {
        updateStatus("Library exported successfully");
        QMessageBox::information(this, "Export Complete",
            "Library exported to:\n" + folderPath);
    } else {
        QMessageBox::critical(this, "Export Failed", "Could not export library");
    }
}

void UIPanel::onImportLibrary() {
    QString folderPath = QFileDialog::getExistingDirectory(this, "Select Library Bundle");
    if (folderPath.isEmpty()) return;
    
    auto& lib = BlockLibraryManager::instance();
    int imported = lib.importLibraryBundle(folderPath.toStdString());
    
    if (imported > 0) {
        updateStatus(std::string("Imported ") + std::to_string(imported) + " blocks from bundle");
        refreshBlockList();
        refreshStatistics();
    } else {
        QMessageBox::warning(this, "Import Failed", "No valid library bundle found");
    }
}

void UIPanel::onCategoryChanged(const QString& category) {
    refreshBlockList();
}

void UIPanel::onAssignBlockToMachine() {
    int machineIdx = m_pMachineCombo->currentIndex();
    int blockRow = m_pAvailableBlocksList->currentRow();
    
    if (machineIdx < 0 || blockRow < 0 || !m_pMachines) {
        QMessageBox::warning(this, "No Selection", "Select both machine and block");
        return;
    }
    
    auto& lib = BlockLibraryManager::instance();
    auto blocks = lib.getAllBlocks();
    if (blockRow >= (int)blocks.size()) return;
    
    // Assign block to machine
    (*m_pMachines)[machineIdx].modelPath = blocks[blockRow].blockName;
    
    updateStatus(std::string("Assigned ") + blocks[blockRow].displayName +
                " to Machine " + std::to_string((*m_pMachines)[machineIdx].id));
    
    updateMachineInfo((*m_pMachines)[machineIdx]);
}

void UIPanel::onShowBlockStats() {
    auto& lib = BlockLibraryManager::instance();
    QMessageBox::information(this, "Block Library Statistics",
        QString::fromStdString(lib.getStatistics()));
}

void UIPanel::onShowLibraryInfo() {
    onShowBlockStats();
}

// ========================================================================
// HELPER METHODS
// ========================================================================

void UIPanel::setMachines(std::vector<Machine>* pMachines) {
    m_pMachines = pMachines;
    if (m_pMachines && !m_pMachines->empty()) {
        m_pBtnLayout->setEnabled(true);
        refreshMachineList();
    }
}

void UIPanel::setPipes(std::vector<Pipe>* pPipes) {
    m_pPipes = pPipes;
    refreshPipeList();
    if (m_pPipes && !m_pPipes->empty()) {
        m_pBtnRoute->setEnabled(true);
    }
}

void UIPanel::setPipeline(Pipeline* pPipeline) {
    m_pPipeline = pPipeline;
}

void UIPanel::updateStatus(const std::string& message) {
    m_pStatusLabel->setText(QString::fromStdString(message));
}

void UIPanel::updateBlockInfo(const MachineBlock& block) {
    std::ostringstream ss;
    ss << "<b>Block Name:</b> " << block.blockName << "<br>"
       << "<b>Display Name:</b> " << block.displayName << "<br>"
       << "<b>Category:</b> " << block.category << "<br>"
       << "<b>Description:</b> " << block.description << "<br>"
       << "<b>Default Scale:</b> " << block.defaultScale << "<br>"
       << "<b>Type:</b> " << (block.isImported ? "User Imported" : "Built-in") << "<br>"
       << "<b>File:</b> " << block.filePath;
    
    m_pBlockInfoLabel->setText(QString::fromStdString(ss.str()));
}

void UIPanel::updateMachineInfo(const Machine& machine) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "<b>Machine ID:</b> " << machine.id << "<br>"
       << "<b>Name:</b> " << machine.name << "<br>"
       << "<b>Position:</b> (" << machine.position.x << ", " 
       << machine.position.y << ", " << machine.position.z << ")<br>"
       << "<b>Size:</b> (" << machine.size.x << ", "
       << machine.size.y << ", " << machine.size.z << ")<br>"
       << "<b>Assigned Block:</b> " << machine.modelPath << "<br>"
       << "<b>Ports:</b> " << machine.ports.size();
    
    m_pMachineInfoLabel->setText(QString::fromStdString(ss.str()));
}

void UIPanel::updatePipeInfo(const Pipe& pipe) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "<b>Pipe ID:</b> " << pipe.id << "<br>"
       << "<b>Tag:</b> " << pipe.tag << "<br>"
       << "<b>From Machine:</b> " << pipe.fromMachineId << "<br>"
       << "<b>To Machine:</b> " << pipe.toMachineId << "<br>"
       << "<b>Diameter:</b> " << (pipe.diameter * 1000.0) << " mm<br>"
       << "<b>Length:</b> " << pipe.length << " m<br>"
       << "<b>Cost:</b> $" << pipe.cost << "<br>"
       << "<b>Spec:</b> " << pipe.specCode;
    
    m_pPipeInfoLabel->setText(QString::fromStdString(ss.str()));
}

void UIPanel::refreshBlockList() {
    m_pBlockListWidget->clear();
    
    auto& lib = BlockLibraryManager::instance();
    std::vector<MachineBlock> blocks;
    
    QString currentCategory = m_pCategoryCombo->currentText();
    if (currentCategory == "All Categories") {
        blocks = lib.getAllBlocks();
    } else {
        blocks = lib.getBlocksByCategory(currentCategory.toStdString());
    }
    
    for (const auto& block : blocks) {
        QString itemText = QString::fromStdString(block.displayName);
        if (!block.category.empty()) {
            itemText += " [" + QString::fromStdString(block.category) + "]";
        }
        m_pBlockListWidget->addItem(itemText);
    }
    
    // Update category combo
    m_pCategoryCombo->blockSignals(true);
    m_pCategoryCombo->clear();
    m_pCategoryCombo->addItem("All Categories");
    for (const auto& cat : lib.getCategories()) {
        m_pCategoryCombo->addItem(QString::fromStdString(cat));
    }
    m_pCategoryCombo->blockSignals(false);
}

void UIPanel::refreshMachineList() {
    m_pMachineCombo->clear();
    m_pAvailableBlocksList->clear();
    
    if (m_pMachines) {
        for (const auto& m : *m_pMachines) {
            m_pMachineCombo->addItem(QString::fromStdString(
                "Machine " + std::to_string(m.id) + " - " + m.name));
        }
    }
}

void UIPanel::refreshPipeList() {
    m_pPipeListWidget->clear();
    
    if (m_pPipes) {
        for (const auto& pipe : *m_pPipes) {
            QString itemText = QString::fromStdString(
                "Pipe " + std::to_string(pipe.id) + " (" + pipe.tag + ")");
            m_pPipeListWidget->addItem(itemText);
        }
    }
}

void UIPanel::refreshStatistics() {
    auto& lib = BlockLibraryManager::instance();
    
    m_pStatsTable->setRowCount(5);
    m_pStatsTable->setItem(0, 0, new QTableWidgetItem("Total Blocks"));
    m_pStatsTable->setItem(0, 1, new QTableWidgetItem(
        QString::number(lib.getTotalBlockCount())));
    
    m_pStatsTable->setItem(1, 0, new QTableWidgetItem("Built-in Blocks"));
    m_pStatsTable->setItem(1, 1, new QTableWidgetItem(
        QString::number(lib.getBuiltInBlockCount())));
    
    m_pStatsTable->setItem(2, 0, new QTableWidgetItem("Imported Blocks"));
    m_pStatsTable->setItem(2, 1, new QTableWidgetItem(
        QString::number(lib.getImportedBlockCount())));
    
    m_pStatsTable->setItem(3, 0, new QTableWidgetItem("Categories"));
    m_pStatsTable->setItem(3, 1, new QTableWidgetItem(
        QString::number(lib.getCategories().size())));
    
    m_pStatsTable->setItem(4, 0, new QTableWidgetItem("Library Directory"));
    m_pStatsTable->setItem(4, 1, new QTableWidgetItem(
        QString::fromStdString(BlockLibraryManager::instance().getLibraryDirectoryPublic())));
    
    m_pLibraryStatsLabel->setText(QString::fromStdString(lib.getStatistics()));
}

std::string UIPanel::openFileDialog(const std::string& title, const std::string& filter) {
    QString filePath = QFileDialog::getOpenFileName(this,
        QString::fromStdString(title),
        "",
        QString::fromStdString(filter));
    
    return filePath.toStdString();
}

std::vector<std::string> UIPanel::openMultiFileDialog(const std::string& title) {
    QStringList filePathsQt = QFileDialog::getOpenFileNames(this,
        QString::fromStdString(title),
        "",
        "DWG Files (*.dwg);;All Files (*)");
    
    std::vector<std::string> filePaths;
    for (const auto& path : filePathsQt) {
        filePaths.push_back(path.toStdString());
    }
    
    return filePaths;
}

}  // namespace ModulusLite