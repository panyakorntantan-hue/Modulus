// ============================================================================
// AutoCAD 2027 ObjectARX Plugin - Modulus Lite
// Win32 Native UI - No Qt dependencies
// ============================================================================

// 1. Define Unicode before any includes (required for _T macro)
#define _WINSOCKAPI_

// 2. Windows and COM headers (MUST come first
#include <tchar.h>
#include <objbase.h>
#include <unknwn.h>
#include <winuser.h>
#include <shlwapi.h>
#include <afxwin.h>    

// 3. Standard library
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>


// 4. ObjectARX headers (depend on Windows headers above)
#include "acdb.h"
#include "aced.h"
#include "acedads.h"
#include "rxregsvc.h"
#include "arxHeaders.h"

// 5. Project headers (depend on everything above)
#include "core.h"
#include "cad_adapter.h"
#include "ui_panel.h"
#include "block_library.h"
#include "modulus_dock.h"

#pragma comment(lib, "shlwapi.lib")

using namespace ModulusLite;

// ============================================================================
// PLUGIN STATE
// ============================================================================

class ModulusLitePlugin
{
public:
    static ModulusLitePlugin &instance()
    {
        static ModulusLitePlugin s_instance;
        return s_instance;
    }

    bool initialize(HWND hWndAcad);
    void shutdown();

    std::vector<Machine> &getMachines() { return m_machines; }
    std::vector<Pipe> &getPipes() { return m_pipes; }

    Pipeline *getPipeline() { return m_pPipeline; }
    UIPanel *getUIPanel() { return m_pUIPanel.get(); }

    void showUI();
    void hideUI();

private:
    ModulusLitePlugin();
    ~ModulusLitePlugin();

    std::vector<Machine> m_machines;
    std::vector<Pipe> m_pipes;
    Pipeline *m_pPipeline;
    std::unique_ptr<ModulusDock> m_pDock;

    void setupSampleData();
    void initializeBlockLibrary();
};

ModulusLitePlugin::ModulusLitePlugin()
    : m_pPipeline(nullptr)
{
}

ModulusLitePlugin::~ModulusLitePlugin()
{
    if (m_pPipeline)
    {
        delete m_pPipeline;
        m_pPipeline = nullptr;
    }
}

bool ModulusLitePlugin::initialize(HWND hWndAcad)
{
    acutPrintf(_T("\n[Modulus Lite] Initializing...\n"));

    AcDbDatabase *pDb = acdbHostApplicationServices()->workingDatabase();
    if (!pDb)
    {
        acutPrintf(_T("[Modulus Lite] ERROR: No working database!\n"));
        return false;
    }

    // Initialize block library with defaults
    initializeBlockLibrary();

    // Create pipeline
    m_pPipeline = new Pipeline(pDb);

    // Setup sample data (in real app, this would come from user input)
    setupSampleData();

    // Create UI panel
    m_pDock = std::make_unique<ModulusDock>();

    if (!m_pDock->createDock(hWndAcad))
    {
        acutPrintf(_T("[Modulus Lite] ERROR: Failed to create dock panel!\n"));
        return false;
    }

    UIPanel *panel = m_pDock->panel();

    panel->setMachines(&m_machines);
    panel->setPipes(&m_pipes);
    panel->setPipeline(m_pPipeline);

    acutPrintf(_T("[Modulus Lite] Initialization complete.\n"));
    return true;
}

void ModulusLitePlugin::shutdown()
{
    if (m_pDock)
    {
        m_pDock->ShowControlBar(FALSE, FALSE);
    }

    if (m_pPipeline)
    {
        delete m_pPipeline;
        m_pPipeline = nullptr;
    }
}

void ModulusLitePlugin::showUI()
{
    if (m_pDock)
        m_pDock->ShowControlBar(TRUE, FALSE);
}

void ModulusLitePlugin::hideUI()
{
    if (m_pDock)
        m_pDock->ShowControlBar(FALSE, FALSE);
}

void ModulusLitePlugin::initializeBlockLibrary()
{
    auto &lib = BlockLibraryManager::instance();
    lib.initializeDefaults();
    acutPrintf(_T("[Modulus Lite] Block library initialized with %d default blocks.\n"),
               lib.getTotalBlockCount());
}

void ModulusLitePlugin::setupSampleData()
{
    acutPrintf(_T("\n[Modulus Lite] Setting up sample data...\n"));

    // Create 5 machines
    m_machines.resize(5);

    for (int i = 0; i < 5; ++i)
    {
        m_machines[i].id = i;
        m_machines[i].name = "Machine" + std::to_string(i);
        m_machines[i].position = Point3D(2.0 + i * 3.0, 2.0 + (i % 2) * 4.0, 0.5);
        m_machines[i].size = Point3D(1.0, 1.0, 1.0);
        m_machines[i].velocity = Point3D(0, 0, 0);

        // Add 4 ports to each machine
        m_machines[i].ports.push_back(Point3D(-0.5, 0, 0.5)); // Left
        m_machines[i].ports.push_back(Point3D(0.5, 0, 0.5));  // Right
        m_machines[i].ports.push_back(Point3D(0, -0.5, 0.5)); // Back
        m_machines[i].ports.push_back(Point3D(0, 0.5, 0.5));  // Front

        m_machines[i].modelPath = "MachineBlock";

        acutPrintf(_T("  Created Machine %d: %s at (%.1f, %.1f, %.1f)\n"),
                   i, m_machines[i].name.c_str(),
                   m_machines[i].position.x, m_machines[i].position.y, m_machines[i].position.z);
    }

    // Create pipes connecting machines
    m_pipes.resize(3);

    m_pipes[0].id = 0;
    m_pipes[0].fromMachineId = 0;
    m_pipes[0].toMachineId = 1;
    m_pipes[0].fromPortIndex = 1;
    m_pipes[0].toPortIndex = 0;
    m_pipes[0].material = MaterialType::Liquid;
    m_pipes[0].service = ServiceType::Water;
    m_pipes[0].diameter = PipeProperties::getDiameter(MaterialType::Liquid);
    m_pipes[0].specCode = "CS150";
    m_pipes[0].lineNumber = 100;

    m_pipes[1].id = 1;
    m_pipes[1].fromMachineId = 1;
    m_pipes[1].toMachineId = 3;
    m_pipes[1].fromPortIndex = 2;
    m_pipes[1].toPortIndex = 3;
    m_pipes[1].material = MaterialType::Gas;
    m_pipes[1].service = ServiceType::Air;
    m_pipes[1].diameter = PipeProperties::getDiameter(MaterialType::Gas);
    m_pipes[1].specCode = "SS316";
    m_pipes[1].lineNumber = 200;

    m_pipes[2].id = 2;
    m_pipes[2].fromMachineId = 2;
    m_pipes[2].toMachineId = 4;
    m_pipes[2].fromPortIndex = 1;
    m_pipes[2].toPortIndex = 0;
    m_pipes[2].material = MaterialType::Sludge;
    m_pipes[2].service = ServiceType::Slurry;
    m_pipes[2].diameter = PipeProperties::getDiameter(MaterialType::Sludge);
    m_pipes[2].specCode = "HDPE";
    m_pipes[2].lineNumber = 300;

    acutPrintf(_T("  Created %zu sample pipes.\n"), m_pipes.size());
    acutPrintf(_T("\n[Modulus Lite] Sample data ready.\n"));
}

// ============================================================================
// COMMAND HANDLERS
// ============================================================================

void cmdShowUI()
{
    auto &plugin = ModulusLitePlugin::instance();
    plugin.showUI();
    acutPrintf(_T("\n[Modulus Lite] UI panel opened.\n"));
}

void cmdHideUI()
{
    auto &plugin = ModulusLitePlugin::instance();
    plugin.hideUI();
    acutPrintf(_T("\n[Modulus Lite] UI panel closed.\n"));
}

void cmdGenerateLayout()
{
    auto &plugin = ModulusLitePlugin::instance();
    auto &machines = plugin.getMachines();

    acutPrintf(_T("\n[Modulus Lite] Generating layout...\n"));

    // Run layout
    FDGLayout::layout(machines);

    acutPrintf(_T("[Modulus Lite] Layout generated. %zu machines positioned.\n"), machines.size());

    for (const auto &m : machines)
    {
        acutPrintf(_T("  Machine %d at (%.2f, %.2f, %.2f)\n"),
                   m.id, m.position.x, m.position.y, m.position.z);
    }
}

void cmdRoutePipes()
{
    auto &plugin = ModulusLitePlugin::instance();
    auto pPipeline = plugin.getPipeline();
    auto &machines = plugin.getMachines();
    auto &pipes = plugin.getPipes();

    acutPrintf(_T("\n[Modulus Lite] Executing pipeline (routing pipes)...\n"));

    // Execute full pipeline
    if (pPipeline && pPipeline->execute(machines, pipes))
    {
        acutPrintf(_T("[Modulus Lite] Pipeline executed successfully.\n"));
        acutPrintf(_T("[Modulus Lite] Routed %zu pipes.\n"), pipes.size());

        // Print pipe info
        for (const auto &pipe : pipes)
        {
            acutPrintf(_T("  Pipe %d: %s, Length=%.2fm, Cost=$%.2f\n"),
                       pipe.id, pipe.tag.c_str(), pipe.length, pipe.cost);
        }
    }
    else
    {
        acutPrintf(_T("[Modulus Lite] ERROR: Pipeline execution failed.\n"));
    }
}

void cmdGetStats()
{
    auto &lib = BlockLibraryManager::instance();

    acutPrintf(_T("\n[Modulus Lite] Block Library Statistics:\n"));
    acutPrintf(_T("%s\n"), lib.getStatistics().c_str());
}

void cmdReloadLibrary()
{
    auto &lib = BlockLibraryManager::instance();

    std::string manifestPath = BlockLibraryManager::instance().getLibraryDirectoryPublic() + "/blocks.json";

    if (lib.loadLibraryManifest(manifestPath))
    {
        acutPrintf(_T("\n[Modulus Lite] Library reloaded from disk.\n"));
    }
    else
    {
        acutPrintf(_T("\n[Modulus Lite] WARNING: Could not load manifest. Using defaults.\n"));
        lib.initializeDefaults();
    }
}

// ============================================================================
// ENTRY POINTS
// ============================================================================

extern "C"
{

    AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void *pkt)
    {
        switch (msg)
        {
        case AcRx::kInitAppMsg:
        {
            acutPrintf(_T("\n========================================\n"));
            acutPrintf(_T("Modulus Lite v1.0 Loading...\n"));
            acutPrintf(_T("========================================\n"));

            // Register as MDI-aware
            if (!acrxRegisterAppMDIAware(pkt))
            {
                acutPrintf(_T("ERROR: Failed to register MDI awareness.\n"));
                return AcRx::kRetError;
            }

            // Get AutoCAD window handle
            HWND hWndAcad = adsw_acadMainWnd().hwnd;

            // Register commands
            // UI Management
            acedRegCmds->addCommand(_T("MODULUSLITE"), _T("MODUI"),
                                    _T("MODUI"), ACRX_CMD_MODAL, cmdShowUI);
            acedRegCmds->addCommand(_T("MODULUSLITE"), _T("MODHIDE"),
                                    _T("MODHIDE"), ACRX_CMD_MODAL, cmdHideUI);

            // Layout & Routing
            acedRegCmds->addCommand(_T("MODULUSLITE"), _T("MODLAYOUT"),
                                    _T("MODLAYOUT"), ACRX_CMD_MODAL, cmdGenerateLayout);
            acedRegCmds->addCommand(_T("MODULUSLITE"), _T("MODROUTE"),
                                    _T("MODROUTE"), ACRX_CMD_MODAL, cmdRoutePipes);

            // Utilities
            acedRegCmds->addCommand(_T("MODULUSLITE"), _T("MODSTATS"),
                                    _T("MODSTATS"), ACRX_CMD_MODAL, cmdGetStats);
            acedRegCmds->addCommand(_T("MODULUSLITE"), _T("MODRELOAD"),
                                    _T("MODRELOAD"), ACRX_CMD_MODAL, cmdReloadLibrary);

            // Initialize plugin
            auto &plugin = ModulusLitePlugin::instance();
            if (!plugin.initialize(hWndAcad))
            {
                acutPrintf(_T("ERROR: Failed to initialize plugin.\n"));
                return AcRx::kRetError;
            }

            // Show UI automatically (can be commented out)
            plugin.showUI();

            acutPrintf(_T("\n========================================\n"));
            acutPrintf(_T("Modulus Lite v1.0 Loaded Successfully!\n"));
            acutPrintf(_T("========================================\n"));
            acutPrintf(_T("\nAvailable Commands:\n"));
            acutPrintf(_T("  MODUI        - Show UI panel\n"));
            acutPrintf(_T("  MODHIDE      - Hide UI panel\n"));
            acutPrintf(_T("  MODLAYOUT    - Generate machine layout (FDG)\n"));
            acutPrintf(_T("  MODROUTE     - Route pipes (A*)\n"));
            acutPrintf(_T("  MODSTATS     - Show block library statistics\n"));
            acutPrintf(_T("  MODRELOAD    - Reload library from disk\n"));
            acutPrintf(_T("\n"));
        }
            return AcRx::kRetOK;

        case AcRx::kUnloadAppMsg:
        {
            acutPrintf(_T("\n[Modulus Lite] Unloading...\n"));

            // Unregister commands
            acedRegCmds->removeGroup(_T("MODULUSLITE"));

            // Shutdown plugin
            ModulusLitePlugin::instance().shutdown();

            acutPrintf(_T("[Modulus Lite] Unloaded successfully.\n\n"));
        }
            return AcRx::kRetOK;

        default:
            return AcRx::kRetOK;
        }
    }

} // extern "C"