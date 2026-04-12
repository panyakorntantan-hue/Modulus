#include "block_library.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

// ObjectARX headers
#include "acdb.h"
#include "acdbabb.h"
#include "dbsymtb.h"

namespace fs = std::filesystem;

namespace ModulusLite {

// ============================================================================
// SINGLETON INSTANCE
// ============================================================================

BlockLibraryManager& BlockLibraryManager::instance() {
    static BlockLibraryManager s_instance;
    return s_instance;
}

BlockLibraryManager::BlockLibraryManager() {
    // Load persisted library on startup
    std::string libPath = getLibraryDirectory() + "/blocks.json";
    if (fs::exists(libPath)) {
        loadLibraryManifest(libPath);
    } else {
        initializeDefaults();
    }
}

BlockLibraryManager::~BlockLibraryManager() {
    // Save library on shutdown
    std::string libPath = getLibraryDirectory() + "/blocks.json";
    saveLibraryManifest(libPath);
}

// ============================================================================
// USER IMPORT OPERATIONS
// ============================================================================

std::vector<MachineBlock> BlockLibraryManager::importDWGFile(
    const std::string& dwgFilePath,
    const std::string& category
) {
    std::vector<MachineBlock> importedBlocks;
    
    // Validate file exists and is valid DWG
    if (!fs::exists(dwgFilePath)) {
        std::cerr << "ERROR: File not found: " << dwgFilePath << std::endl;
        return importedBlocks;
    }
    
    if (!DWGFileUtil::isValidDWG(dwgFilePath)) {
        std::cerr << "ERROR: Not a valid DWG file: " << dwgFilePath << std::endl;
        return importedBlocks;
    }
    
    // Extract blocks from DWG
    importedBlocks = extractBlocksFromDWG(dwgFilePath, category);
    
    if (importedBlocks.empty()) {
        std::cerr << "WARNING: No blocks found in " << dwgFilePath << std::endl;
        return importedBlocks;
    }
    
    // Copy DWG to library storage
    fs::path sourcePath(dwgFilePath);
    std::string storageDir = getLibraryDirectory();
    fs::create_directories(storageDir);
    
    std::string destFileName = sourcePath.stem().string() + "_imported.dwg";
    std::string destPath = storageDir + "/" + destFileName;
    
    try {
        fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);
    } catch (const std::exception& e) {
        std::cerr << "WARNING: Could not copy file to library: " << e.what() << std::endl;
        // Continue anyway; blocks can still be referenced at original location
    }
    
    // Update file path to storage location (or keep original if copy failed)
    for (auto& block : importedBlocks) {
        block.filePath = fs::exists(destPath) ? destPath : dwgFilePath;
        block.isImported = true;
    }
    
    // Add to library
    for (const auto& block : importedBlocks) {
        m_blocks.push_back(block);
    }
    
    rebuildIndex();
    
    std::cout << "Imported " << importedBlocks.size() << " blocks from " 
              << sourcePath.filename().string() << std::endl;
    
    return importedBlocks;
}

int BlockLibraryManager::importMultipleDWGFiles(
    const std::vector<std::string>& dwgFilePaths,
    const std::vector<std::string>& categories
) {
    if (dwgFilePaths.size() != categories.size()) {
        std::cerr << "ERROR: File paths and categories must have same size" << std::endl;
        return 0;
    }
    
    int totalImported = 0;
    
    for (size_t i = 0; i < dwgFilePaths.size(); ++i) {
        auto blocks = importDWGFile(dwgFilePaths[i], categories[i]);
        totalImported += blocks.size();
    }
    
    return totalImported;
}

bool BlockLibraryManager::removeBlock(const std::string& blockName) {
    auto it = m_blockIndex.find(blockName);
    if (it == m_blockIndex.end()) {
        return false;
    }
    
    size_t idx = it->second;
    m_blocks.erase(m_blocks.begin() + idx);
    rebuildIndex();
    
    std::cout << "Removed block: " << blockName << std::endl;
    return true;
}

// ============================================================================
// LIBRARY QUERIES
// ============================================================================

const std::vector<MachineBlock>& BlockLibraryManager::getAllBlocks() const {
    return m_blocks;
}

std::vector<MachineBlock> BlockLibraryManager::getBlocksByCategory(
    const std::string& category
) const {
    std::vector<MachineBlock> result;
    
    for (const auto& block : m_blocks) {
        if (block.category == category) {
            result.push_back(block);
        }
    }
    
    return result;
}

std::vector<std::string> BlockLibraryManager::getCategories() const {
    std::vector<std::string> categories;
    
    for (const auto& block : m_blocks) {
        auto it = std::find(categories.begin(), categories.end(), block.category);
        if (it == categories.end()) {
            categories.push_back(block.category);
        }
    }
    
    std::sort(categories.begin(), categories.end());
    return categories;
}

MachineBlock BlockLibraryManager::getBlock(const std::string& blockName) const {
    auto it = m_blockIndex.find(blockName);
    if (it == m_blockIndex.end()) {
        return MachineBlock();
    }
    
    return m_blocks[it->second];
}

bool BlockLibraryManager::blockExists(const std::string& blockName) const {
    return m_blockIndex.find(blockName) != m_blockIndex.end();
}

// ============================================================================
// LIBRARY PERSISTENCE
// ============================================================================

bool BlockLibraryManager::saveLibraryManifest(const std::string& manifestPath) {
    std::ofstream file(manifestPath);
    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot write to " << manifestPath << std::endl;
        return false;
    }
    
    // Simple JSON format
    file << "{\n";
    file << "  \"blocks\": [\n";
    
    for (size_t i = 0; i < m_blocks.size(); ++i) {
        const auto& block = m_blocks[i];
        
        file << "    {\n";
        file << "      \"blockName\": \"" << block.blockName << "\",\n";
        file << "      \"displayName\": \"" << block.displayName << "\",\n";
        file << "      \"filePath\": \"" << block.filePath << "\",\n";
        file << "      \"category\": \"" << block.category << "\",\n";
        file << "      \"description\": \"" << block.description << "\",\n";
        file << "      \"defaultScale\": " << block.defaultScale << ",\n";
        file << "      \"isImported\": " << (block.isImported ? "true" : "false") << "\n";
        file << "    }";
        
        if (i < m_blocks.size() - 1) {
            file << ",\n";
        } else {
            file << "\n";
        }
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    std::cout << "Library manifest saved to " << manifestPath << std::endl;
    return true;
}

bool BlockLibraryManager::loadLibraryManifest(const std::string& manifestPath) {
    std::ifstream file(manifestPath);
    if (!file.is_open()) {
        std::cerr << "WARNING: Cannot load manifest from " << manifestPath << std::endl;
        return false;
    }
    
    // Simple JSON parsing (production would use a JSON library)
    std::string line;
    std::vector<MachineBlock> loaded;
    MachineBlock current;
    
    while (std::getline(file, line)) {
        // Parse simple key: "value" patterns
        if (line.find("\"blockName\"") != std::string::npos) {
            size_t start = line.find("\"") + 1;
            size_t end = line.rfind("\"");
            if (start < end) {
                current.blockName = line.substr(start, end - start);
            }
        }
        else if (line.find("\"displayName\"") != std::string::npos) {
            size_t start = line.find("\"", line.find(":")) + 1;
            size_t end = line.rfind("\"");
            if (start < end) {
                current.displayName = line.substr(start, end - start);
            }
        }
        else if (line.find("\"filePath\"") != std::string::npos) {
            size_t start = line.find("\"", line.find(":")) + 1;
            size_t end = line.rfind("\"");
            if (start < end) {
                current.filePath = line.substr(start, end - start);
            }
        }
        else if (line.find("\"category\"") != std::string::npos) {
            size_t start = line.find("\"", line.find(":")) + 1;
            size_t end = line.rfind("\"");
            if (start < end) {
                current.category = line.substr(start, end - start);
            }
        }
        else if (line.find("\"isImported\"") != std::string::npos) {
            current.isImported = line.find("true") != std::string::npos;
            loaded.push_back(current);
            current = MachineBlock();
        }
    }
    
    m_blocks = loaded;
    rebuildIndex();
    file.close();
    
    std::cout << "Loaded " << m_blocks.size() << " blocks from manifest" << std::endl;
    return true;
}

bool BlockLibraryManager::exportLibrary(const std::string& exportPath) {
    // Create a zip or tar bundle with manifest + DWG files
    // For MVP, just copy manifest
    
    std::string manifestName = "library_manifest.json";
    std::string manifestPath = exportPath + "/" + manifestName;
    
    fs::create_directories(exportPath);
    
    // Save manifest
    saveLibraryManifest(manifestPath);
    
    // Copy all DWG files referenced in library
    std::vector<std::string> copiedFiles;
    for (const auto& block : m_blocks) {
        if (!fs::exists(block.filePath)) {
            continue;  // Skip missing files
        }
        
        std::string destFileName = fs::path(block.filePath).filename().string();
        std::string destPath = exportPath + "/" + destFileName;
        
        try {
            fs::copy_file(block.filePath, destPath, fs::copy_options::overwrite_existing);
            copiedFiles.push_back(destFileName);
        } catch (const std::exception& e) {
            std::cerr << "WARNING: Could not copy " << destFileName << std::endl;
        }
    }
    
    std::cout << "Exported library to " << exportPath 
              << " (" << copiedFiles.size() << " DWG files)" << std::endl;
    
    return true;
}

int BlockLibraryManager::importLibraryBundle(const std::string& importPath) {
    // Find and load manifest from bundle
    std::string manifestPath = importPath + "/library_manifest.json";
    
    if (!fs::exists(manifestPath)) {
        std::cerr << "ERROR: No library manifest found in " << importPath << std::endl;
        return 0;
    }
    
    // Load manifest
    loadLibraryManifest(manifestPath);
    
    // Update file paths to point to bundle location
    int imported = 0;
    for (auto& block : m_blocks) {
        std::string fileName = fs::path(block.filePath).filename().string();
        std::string newPath = importPath + "/" + fileName;
        
        if (fs::exists(newPath)) {
            block.filePath = newPath;
            imported++;
        }
    }
    
    std::cout << "Imported library bundle with " << imported << " blocks" << std::endl;
    return imported;
}

// ============================================================================
// BUILT-IN BLOCKS (DEFAULTS)
// ============================================================================

void BlockLibraryManager::initializeDefaults() {
    m_blocks.clear();
    
    // Built-in default blocks (geometric placeholders)
    
    MachineBlock pump;
    pump.blockName = "PUMP_CENTRIFUGAL";
    pump.displayName = "Centrifugal Pump";
    pump.filePath = "";
    pump.category = "Pump";
    pump.description = "Standard centrifugal pump (built-in placeholder)";
    pump.defaultScale = 1.0;
    pump.isImported = false;
    m_blocks.push_back(pump);
    
    MachineBlock tank;
    tank.blockName = "TANK_VERTICAL";
    tank.displayName = "Vertical Tank";
    tank.filePath = "";
    tank.category = "Tank";
    tank.description = "Vertical cylindrical tank (built-in placeholder)";
    tank.defaultScale = 1.0;
    tank.isImported = false;
    m_blocks.push_back(tank);
    
    MachineBlock valve;
    valve.blockName = "VALVE_BALL";
    valve.displayName = "Ball Valve";
    valve.filePath = "";
    valve.category = "Valve";
    valve.description = "Ball valve (built-in placeholder)";
    valve.defaultScale = 0.5;
    valve.isImported = false;
    m_blocks.push_back(valve);
    
    MachineBlock heater;
    heater.blockName = "HEATER_SHELL_TUBE";
    heater.displayName = "Shell & Tube Heater";
    heater.filePath = "";
    heater.category = "Heat Exchanger";
    heater.description = "Shell and tube heat exchanger (built-in placeholder)";
    heater.defaultScale = 1.0;
    heater.isImported = false;
    m_blocks.push_back(heater);
    
    MachineBlock reactor;
    reactor.blockName = "REACTOR_JACKETED";
    reactor.displayName = "Jacketed Reactor";
    reactor.filePath = "";
    reactor.category = "Reactor";
    reactor.description = "Jacketed reaction vessel (built-in placeholder)";
    reactor.defaultScale = 1.0;
    reactor.isImported = false;
    m_blocks.push_back(reactor);
    
    rebuildIndex();
    std::cout << "Initialized " << m_blocks.size() << " default blocks" << std::endl;
}

void BlockLibraryManager::resetToDefaults() {
    m_blocks.clear();
    m_blockIndex.clear();
    initializeDefaults();
    std::cout << "Reset library to defaults" << std::endl;
}

// ============================================================================
// STATISTICS & INFO
// ============================================================================

int BlockLibraryManager::getTotalBlockCount() const {
    return m_blocks.size();
}

int BlockLibraryManager::getImportedBlockCount() const {
    int count = 0;
    for (const auto& block : m_blocks) {
        if (block.isImported) count++;
    }
    return count;
}

int BlockLibraryManager::getBuiltInBlockCount() const {
    return getTotalBlockCount() - getImportedBlockCount();
}

std::string BlockLibraryManager::getStatistics() const {
    std::ostringstream ss;
    ss << "Block Library Statistics:\n"
       << "  Total Blocks: " << getTotalBlockCount() << "\n"
       << "  Built-in Blocks: " << getBuiltInBlockCount() << "\n"
       << "  Imported Blocks: " << getImportedBlockCount() << "\n"
       << "  Categories: " << getCategories().size() << "\n";
    
    auto cats = getCategories();
    for (const auto& cat : cats) {
        int catCount = getBlocksByCategory(cat).size();
        ss << "    - " << cat << " (" << catCount << ")\n";
    }
    
    return ss.str();
}

// ============================================================================
// PRIVATE HELPERS
// ============================================================================

std::vector<MachineBlock> BlockLibraryManager::extractBlocksFromDWG(
    const std::string& dwgPath,
    const std::string& category
) {
    std::vector<MachineBlock> blocks;
    
    // Use ObjectARX to read DWG and extract block definitions
    AcDbDatabase* pDb = nullptr;
    
    // Open DWG file
    Acad::ErrorStatus es = acdbOpenFile(dwgPath.c_str(), false, &pDb);
    if (es != Acad::eOk || !pDb) {
        std::cerr << "ERROR: Cannot open DWG file: " << dwgPath << std::endl;
        return blocks;
    }
    
    // Get block table
    AcDbBlockTable* pBlkTable = nullptr;
    es = acdbOpenObject(pBlkTable, pDb->blockTableId(), AcDb::kForRead);
    
    if (es == Acad::eOk && pBlkTable) {
        AcDbBlockTableIterator* pIterator = nullptr;
        es = pBlkTable->newIterator(pIterator);
        
        if (es == Acad::eOk && pIterator) {
            for (pIterator->start(); !pIterator->done(); pIterator->step()) {
                AcDbBlockTableRecord* pBlock = nullptr;
                es = pIterator->getRecord(pBlock, AcDb::kForRead);
                
                if (es == Acad::eOk && pBlock) {
                    // Skip model space, paper space, and unnamed blocks
                    const char* pName = pBlock->getName();
                    if (pName && 
                        std::string(pName) != "*MODEL_SPACE" &&
                        std::string(pName) != "*PAPER_SPACE" &&
                        std::string(pName).at(0) != '*') {
                        
                        MachineBlock block;
                        block.blockName = pName;
                        block.displayName = pName;
                        block.filePath = dwgPath;
                        block.category = category;
                        block.description = "Imported from " + fs::path(dwgPath).filename().string();
                        block.defaultScale = 1.0;
                        block.isImported = true;
                        
                        blocks.push_back(block);
                    }
                    
                    pBlock->close();
                }
            }
            
            delete pIterator;
        }
        
        pBlkTable->close();
    }
    
    // Close the database
    acdbCloseFile(pDb);
    delete pDb;
    
    return blocks;
}

bool BlockLibraryManager::validateBlockInDWG(
    const std::string& blockName,
    const std::string& dwgPath
) {
    auto blocks = getBlocksInDWG(dwgPath);
    return std::find(blocks.begin(), blocks.end(), blockName) != blocks.end();
}

void BlockLibraryManager::rebuildIndex() {
    m_blockIndex.clear();
    for (size_t i = 0; i < m_blocks.size(); ++i) {
        m_blockIndex[m_blocks[i].blockName] = i;
    }
}

std::string BlockLibraryManager::getLibraryDirectory() {
    // macOS: ~/.moduluslite/blocks
    // Linux: ~/.moduluslite/blocks
    // Windows: %APPDATA%\ModulusLite\blocks
    
    const char* homeDir = nullptr;
    #ifdef _WIN32
        homeDir = std::getenv("APPDATA");
        if (!homeDir) homeDir = std::getenv("USERPROFILE");
    #else
        homeDir = std::getenv("HOME");
    #endif
    
    if (!homeDir) {
        return "./moduluslite_blocks";
    }
    
    std::string libDir = std::string(homeDir) + "/.moduluslite/blocks";
    fs::create_directories(libDir);
    
    return libDir;
}

// ============================================================================
// DWG FILE UTILITIES
// ============================================================================

std::vector<std::string> DWGFileUtil::getBlocksInDWG(const std::string& dwgPath) {
    std::vector<std::string> blocks;
    
    AcDbDatabase* pDb = nullptr;
    Acad::ErrorStatus es = acdbOpenFile(dwgPath.c_str(), false, &pDb);
    
    if (es != Acad::eOk || !pDb) {
        return blocks;
    }
    
    AcDbBlockTable* pBlkTable = nullptr;
    es = acdbOpenObject(pBlkTable, pDb->blockTableId(), AcDb::kForRead);
    
    if (es == Acad::eOk && pBlkTable) {
        AcDbBlockTableIterator* pIterator = nullptr;
        pBlkTable->newIterator(pIterator);
        
        for (pIterator->start(); !pIterator->done(); pIterator->step()) {
            AcDbBlockTableRecord* pBlock = nullptr;
            if (pIterator->getRecord(pBlock, AcDb::kForRead) == Acad::eOk && pBlock) {
                const char* pName = pBlock->getName();
                if (pName && std::string(pName).at(0) != '*') {
                    blocks.push_back(pName);
                }
                pBlock->close();
            }
        }
        
        delete pIterator;
        pBlkTable->close();
    }
    
    acdbCloseFile(pDb);
    delete pDb;
    
    return blocks;
}

std::unordered_map<std::string, std::string> DWGFileUtil::getDWGMetadata(
    const std::string& dwgPath
) {
    std::unordered_map<std::string, std::string> metadata;
    
    if (!fs::exists(dwgPath)) {
        return metadata;
    }
    
    metadata["filename"] = fs::path(dwgPath).filename().string();
    
    auto lastWrite = fs::last_write_time(dwgPath);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        lastWrite - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    auto tt = std::chrono::system_clock::to_time_t(sctp);
    metadata["modified"] = std::string(std::ctime(&tt));
    
    auto blocks = getBlocksInDWG(dwgPath);
    metadata["blockCount"] = std::to_string(blocks.size());
    
    return metadata;
}

bool DWGFileUtil::isValidDWG(const std::string& dwgPath) {
    if (!fs::exists(dwgPath)) {
        return false;
    }
    
    // Check file extension
    std::string ext = fs::path(dwgPath).extension().string();
    if (ext != ".dwg" && ext != ".DWG") {
        return false;
    }
    
    // Try to open with ObjectARX
    AcDbDatabase* pDb = nullptr;
    Acad::ErrorStatus es = acdbOpenFile(dwgPath.c_str(), false, &pDb);
    
    bool valid = (es == Acad::eOk && pDb != nullptr);
    
    if (pDb) {
        acdbCloseFile(pDb);
        delete pDb;
    }
    
    return valid;
}

std::string DWGFileUtil::copyToLibraryStorage(
    const std::string& sourcePath,
    const std::string& destName
) {
    std::string storageDir = BlockLibraryManager::instance().instance().getLibraryDirectory();
    fs::create_directories(storageDir);
    
    std::string destPath = storageDir + "/" + destName + ".dwg";
    
    try {
        fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);
        return destPath;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Cannot copy to library storage: " << e.what() << std::endl;
        return "";
    }
}

}  // namespace ModulusLite
