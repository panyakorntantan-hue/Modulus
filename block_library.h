#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace ModulusLite {

// ============================================================================
// MACHINE BLOCK LIBRARY
// ============================================================================

/**
 * Represents a single DWG block available for use as a machine symbol
 */
struct MachineBlock {
    std::string blockName;        // Name in DWG (e.g., "Pump_Centrifugal")
    std::string displayName;      // User-friendly name (e.g., "Centrifugal Pump")
    std::string filePath;         // Full path to source DWG file
    std::string category;         // Category: "Pump", "Tank", "Valve", "Heater", etc.
    std::string description;      // Optional description
    double defaultScale;          // Default scale factor (1.0 = 1:1)
    bool isImported;              // True if from user upload; false if built-in
    
    MachineBlock() 
        : blockName(""), displayName(""), filePath(""), category(""), 
          description(""), defaultScale(1.0), isImported(false) {}
};

/**
 * Manages library of available machine blocks
 * Handles import, storage, and retrieval of DWG blocks
 */
class BlockLibraryManager {
public:

    std::string getLibraryDirectoryPublic() const {
        return getLibraryDirectory();
    }

    static BlockLibraryManager& instance();
    
    // ========================================================================
    // USER IMPORT OPERATIONS
    // ========================================================================
    
    /**
     * Import a single DWG file and extract block definitions
     * 
     * @param dwgFilePath Full path to DWG file (e.g., /Users/john/pumps.dwg)
     * @param category Category for organization (e.g., "Pump", "Tank")
     * @return List of blocks found in the DWG file
     */
    std::vector<MachineBlock> importDWGFile(
        const std::string& dwgFilePath,
        const std::string& category
    );
    
    /**
     * Import multiple DWG files at once
     * 
     * @param dwgFilePaths Vector of paths to DWG files
     * @param categories Vector of categories (parallel to dwgFilePaths)
     * @return Total count of blocks imported
     */
    int importMultipleDWGFiles(
        const std::vector<std::string>& dwgFilePaths,
        const std::vector<std::string>& categories
    );
    
    /**
     * Remove an imported block from library
     * 
     * @param blockName Block to remove
     * @return True if removed; false if not found
     */
    bool removeBlock(const std::string& blockName);
    
    // ========================================================================
    // LIBRARY QUERIES
    // ========================================================================
    
    /**
     * Get all available blocks
     * @return Vector of all imported and built-in blocks
     */
    const std::vector<MachineBlock>& getAllBlocks() const;
    
    /**
     * Get blocks by category
     * @param category Filter (e.g., "Pump", "Tank")
     * @return Blocks matching category
     */
    std::vector<MachineBlock> getBlocksByCategory(const std::string& category) const;
    
    /**
     * Get all available categories
     * @return Vector of unique category names
     */
    std::vector<std::string> getCategories() const;
    
    /**
     * Get a specific block by name
     * @param blockName Name of block
     * @return Block if found; empty MachineBlock if not
     */
    MachineBlock getBlock(const std::string& blockName) const;
    
    /**
     * Check if block exists in library
     * @param blockName Name to check
     * @return True if block available
     */
    bool blockExists(const std::string& blockName) const;
    
    // ========================================================================
    // LIBRARY PERSISTENCE
    // ========================================================================
    
    /**
     * Save library manifest to disk
     * Stores user's imported blocks so they persist across sessions
     * 
     * @param manifestPath Path to save manifest (e.g., ~/.moduluslite/blocks.json)
     * @return True if saved successfully
     */
    bool saveLibraryManifest(const std::string& manifestPath);
    
    /**
     * Load library manifest from disk
     * Restores previously imported blocks
     * 
     * @param manifestPath Path to load from
     * @return True if loaded successfully
     */
    bool loadLibraryManifest(const std::string& manifestPath);
    
    /**
     * Export library to file (for sharing with team)
     * @param exportPath Where to save library bundle
     * @return True if exported successfully
     */
    bool exportLibrary(const std::string& exportPath);
    
    /**
     * Import a library bundle (shared from another user)
     * @param importPath Path to library bundle
     * @return Count of blocks imported
     */
    int importLibraryBundle(const std::string& importPath);
    
    // ========================================================================
    // BUILT-IN BLOCKS (DEFAULTS)
    // ========================================================================
    
    /**
     * Initialize with built-in default blocks
     * (Simple geometric placeholders)
     */
    void initializeDefaults();
    
    /**
     * Clear library and reset to defaults
     */
    void resetToDefaults();
    
    // ========================================================================
    // STATISTICS & INFO
    // ========================================================================
    
    /**
     * Get total count of available blocks
     */
    int getTotalBlockCount() const;
    
    /**
     * Get count of user-imported blocks
     */
    int getImportedBlockCount() const;
    
    /**
     * Get count of built-in blocks
     */
    int getBuiltInBlockCount() const;
    
    /**
     * Get library statistics as formatted string
     * @return String with total, imported, categories, etc.
     */
    std::string getStatistics() const;

private:
    BlockLibraryManager();
    ~BlockLibraryManager();
    
    std::vector<MachineBlock> m_blocks;
    std::unordered_map<std::string, size_t> m_blockIndex;  // blockName -> index
    
    // Helper: extract blocks from a DWG file
    std::vector<MachineBlock> extractBlocksFromDWG(
        const std::string& dwgPath,
        const std::string& category
    );
    
    // Helper: validate block accessibility in DWG
    bool validateBlockInDWG(const std::string& blockName, const std::string& dwgPath);
    
    // Helper: update internal index after changes
    void rebuildIndex();
    
    // Helper: get user's library directory
    static std::string getLibraryDirectory();
};

// ============================================================================
// DWG FILE UTILITY
// ============================================================================

/**
 * Utilities for working with DWG files
 */
class DWGFileUtil {
public:
    /**
     * Get list of all blocks defined in a DWG file
     * 
     * @param dwgPath Path to DWG file
     * @return Vector of block names (or empty if file invalid)
     */
    static std::vector<std::string> getBlocksInDWG(const std::string& dwgPath);
    
    /**
     * Get metadata about a DWG file
     * 
     * @param dwgPath Path to DWG file
     * @return Map with keys: "filename", "created", "modified", "blockCount", "version"
     */
    static std::unordered_map<std::string, std::string> getDWGMetadata(
        const std::string& dwgPath
    );
    
    /**
     * Check if file is valid DWG
     * 
     * @param dwgPath Path to check
     * @return True if valid DWG file
     */
    static bool isValidDWG(const std::string& dwgPath);
    
    /**
     * Copy DWG file to library storage
     * 
     * @param sourcePath Original file path
     * @param destName Name to store as (without .dwg)
     * @return Full path where stored, or empty if failed
     */
    static std::string copyToLibraryStorage(
        const std::string& sourcePath,
        const std::string& destName
    );
};

}  // namespace ModulusLite
