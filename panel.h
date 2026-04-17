#pragma once

#include <QDockWidget>
#include <QPushButton>
#include <QListWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QTableWidget>
#include <vector>
#include <QInputDialog>
#include "core.h"
#include "block_library.h"

namespace ModulusLite {

// Forward declaration
class Pipeline;

/**
 * Main control panel for Modulus Lite plugin
 * Handles layout generation, pipe routing, machine block selection
 */
class UIPanel : public QDockWidget {
    Q_OBJECT
    
public:
    explicit UIPanel(QWidget* parent = nullptr);
    ~UIPanel();
    
    // Set data to display
    void setMachines(std::vector<Machine>* pMachines);
    void setPipes(std::vector<Pipe>* pPipes);
    void setPipeline(Pipeline* pPipeline);
    
private slots:
    // Layout & Routing
    void onGenerateLayout();
    void onRoutePipes();
    
    // Block Library Management
    void onImportDWG();
    void onImportMultipleDWG();
    void onBrowseAndSelectBlock();
    void onRemoveSelectedBlock();
    void onResetToDefaults();
    void onExportLibrary();
    void onImportLibrary();
    
    // Block Selection
    void onCategoryChanged(const QString& category);
    void onBlockSelected(int row);
    void onAssignBlockToMachine();
    
    // Pipe Selection
    void onPipeSelected(int row);
    
    // Info & Statistics
    void onShowBlockStats();
    void onShowLibraryInfo();
    
private:
    // UI Elements - Main Tab
    QPushButton* m_pBtnLayout;
    QPushButton* m_pBtnRoute;
    QLabel* m_pStatusLabel;
    
    // UI Elements - Block Library Tab
    QComboBox* m_pCategoryCombo;
    QListWidget* m_pBlockListWidget;
    QLabel* m_pBlockInfoLabel;
    QPushButton* m_pBtnImportDWG;
    QPushButton* m_pBtnImportMultiple;
    QPushButton* m_pBtnRemoveBlock;
    QPushButton* m_pBtnResetDefaults;
    QPushButton* m_pBtnExportLib;
    QPushButton* m_pBtnImportLib;
    
    // UI Elements - Machine Assignment Tab
    QComboBox* m_pMachineCombo;
    QListWidget* m_pAvailableBlocksList;
    QPushButton* m_pBtnAssignBlock;
    QLabel* m_pMachineInfoLabel;
    
    // UI Elements - Pipes Tab
    QListWidget* m_pPipeListWidget;
    QLabel* m_pPipeInfoLabel;
    
    // UI Elements - Statistics Tab
    QTableWidget* m_pStatsTable;
    QLabel* m_pLibraryStatsLabel;
    
    // Tab widget
    QTabWidget* m_pTabWidget;
    
    // Data pointers
    std::vector<Machine>* m_pMachines;
    std::vector<Pipe>* m_pPipes;
    Pipeline* m_pPipeline;
    
    // Helper methods
    void setupUI();
    void setupMainTab();
    void setupBlockLibraryTab();
    void setupMachineAssignmentTab();
    void setupPipesTab();
    void setupStatisticsTab();
    
    void refreshBlockList();
    void refreshMachineList();
    void refreshPipeList();
    void refreshStatistics();
    
    void updateStatus(const std::string& message);
    void updateBlockInfo(const MachineBlock& block);
    void updateMachineInfo(const Machine& machine);
    void updatePipeInfo(const Pipe& pipe);
    
    // File dialog helpers
    std::string openFileDialog(const std::string& title, const std::string& filter);
    std::vector<std::string> openMultiFileDialog(const std::string& title);
};

}  // namespace ModulusLite