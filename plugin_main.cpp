// ============================================================================
// AutoCAD 2027 ObjectARX Plugin - Modulus Lite
// Native MFC Docking Panel (Fixed + Stable)
// ============================================================================

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// Kill bad macros if leaking
#undef BYTE
#undef WORD
#undef FAR
#undef BOOLEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

// MFC AFTER
#include <afxwin.h>
#include <afxext.h>

// Other Windows
#include <tchar.h>
#include <objbase.h>
#include <unknwn.h>
#include <winuser.h>
#include <shlwapi.h>

// 2. Standard library
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

// 3. ObjectARX headers
#include "acdb.h"
#include "aced.h"
#include "acedads.h"
#include "rxregsvc.h"
#include "arxHeaders.h"
#include "acdocman.h" // CAcModuleResourceOverride

// 4. Project headers
#include "core.h"
#include "cad_adapter.h"
#include "modulus_dock_panel.h"
#include "block_library.h"

#pragma comment(lib, "shlwapi.lib")

using namespace ModulusLite;

// ============================================================================
// GLOBALS
// ============================================================================
static ModulusDockPanel* g_pDockPanel = nullptr;

// ============================================================================
// PLUGIN CLASS
// ============================================================================
class ModulusLitePlugin
{
public:
    static ModulusLitePlugin& instance()
    {
        static ModulusLitePlugin s_instance;
        return s_instance;
    }

    bool initialize(HWND hWndAcad);
    void shutdown();

    std::vector<Machine>& getMachines() { return m_machines; }
    std::vector<Pipe>& getPipes() { return m_pipes; }

    Pipeline* getPipeline() { return m_pPipeline; }

    void showUI();
    void hideUI();

private:
    ModulusLitePlugin() : m_pPipeline(nullptr) {}
    ~ModulusLitePlugin();

    std::vector<Machine> m_machines;
    std::vector<Pipe> m_pipes;
    Pipeline* m_pPipeline;

    void setupSampleData();
    void initializeBlockLibrary();
};

ModulusLitePlugin::~ModulusLitePlugin()
{
    delete m_pPipeline;
}

// ============================================================================
// INITIALIZATION
// ============================================================================
bool ModulusLitePlugin::initialize(HWND)
{
    CAcModuleResourceOverride resourceOverride; // CRITICAL

    acutPrintf(_T("\n[Modulus Lite] Initializing...\n"));

    AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
    if (!pDb)
    {
        acutPrintf(_T("[Modulus Lite] ERROR: No working database!\n"));
        return false;
    }

    initializeBlockLibrary();
    m_pPipeline = new Pipeline(pDb);
    setupSampleData();

    try
    {
        // Correct main window retrieval
        CFrameWnd* pMainFrame =
            dynamic_cast<CFrameWnd*>(CWnd::FromHandle(adsw_acadMainWnd()));

        if (!pMainFrame)
        {
            acutPrintf(_T("[Modulus Lite] ERROR: No main frame.\n"));
            return false;
        }

        g_pDockPanel = new ModulusDockPanel();

        if (g_pDockPanel->Create(
                _T("Modulus Lite"),
                pMainFrame,
                CRect(0, 0, 400, 150),
                TRUE,
                AFX_IDW_DOCKBAR_BOTTOM,
                WS_CHILD | WS_VISIBLE | CBRS_BOTTOM))
        {
            g_pDockPanel->EnableDocking(CBRS_ALIGN_ANY);
            pMainFrame->EnableDocking(CBRS_ALIGN_ANY);
            pMainFrame->DockPane(g_pDockPanel);

            acutPrintf(_T("[Modulus Lite] Dock panel created.\n"));
        }
        else
        {
            delete g_pDockPanel;
            g_pDockPanel = nullptr;
            return false;
        }
    }
    catch (...)
    {
        acutPrintf(_T("[Modulus Lite] ERROR: Dock creation failed.\n"));
        return false;
    }

    return true;
}

// ============================================================================
// SHUTDOWN
// ============================================================================
void ModulusLitePlugin::shutdown()
{
    acutPrintf(_T("\n[Modulus Lite] Shutting down...\n"));

    if (g_pDockPanel)
    {
        g_pDockPanel->ShowPane(FALSE, FALSE, FALSE);
        delete g_pDockPanel;
        g_pDockPanel = nullptr;
    }

    delete m_pPipeline;
    m_pPipeline = nullptr;
}

// ============================================================================
// UI CONTROL
// ============================================================================
void ModulusLitePlugin::showUI()
{
    CAcModuleResourceOverride resourceOverride;

    if (g_pDockPanel)
        g_pDockPanel->ShowPane(TRUE, FALSE, FALSE);
}

void ModulusLitePlugin::hideUI()
{
    CAcModuleResourceOverride resourceOverride;

    if (g_pDockPanel)
        g_pDockPanel->ShowPane(FALSE, FALSE, FALSE);
}

// ============================================================================
// DATA SETUP
// ============================================================================
void ModulusLitePlugin::initializeBlockLibrary()
{
    auto& lib = BlockLibraryManager::instance();
    lib.initializeDefaults();
}

void ModulusLitePlugin::setupSampleData()
{
    m_machines.clear();
    m_pipes.clear();

    m_machines.resize(5);

    for (int i = 0; i < 5; ++i)
    {
        auto& m = m_machines[i];
        m.id = i;
        m.name = "Machine" + std::to_string(i);
        m.position = Point3D(2 + i * 3, 2 + (i % 2) * 4, 0.5);

        m.ports = {
            {-0.5,0,0.5},{0.5,0,0.5},{0,-0.5,0.5},{0,0.5,0.5}
        };

        m.modelPath = "MachineBlock";
    }

    m_pipes.resize(1);
}

// ============================================================================
// COMMANDS
// ============================================================================
void cmdShowUI() { ModulusLitePlugin::instance().showUI(); }
void cmdHideUI() { ModulusLitePlugin::instance().hideUI(); }

void cmdGenerateLayout()
{
    auto& machines = ModulusLitePlugin::instance().getMachines();
    FDGLayout::layout(machines);
}

void cmdRoutePipes()
{
    auto& plugin = ModulusLitePlugin::instance();
    plugin.getPipeline()->execute(plugin.getMachines(), plugin.getPipes());
}

// ============================================================================
// ENTRY POINT
// ============================================================================
extern "C"
AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* pkt)
{
    switch (msg)
    {
    case AcRx::kInitAppMsg:
    {
        CAcModuleResourceOverride resourceOverride;

        acrxRegisterAppMDIAware(pkt);

        HWND hWndAcad = adsw_acadMainWnd();

        acedRegCmds->addCommand(_T("MODULUS"), _T("MODUI"), _T("MODUI"), ACRX_CMD_MODAL, cmdShowUI);
        acedRegCmds->addCommand(_T("MODULUS"), _T("MODHIDE"), _T("MODHIDE"), ACRX_CMD_MODAL, cmdHideUI);
        acedRegCmds->addCommand(_T("MODULUS"), _T("MODLAYOUT"), _T("MODLAYOUT"), ACRX_CMD_MODAL, cmdGenerateLayout);
        acedRegCmds->addCommand(_T("MODULUS"), _T("MODROUTE"), _T("MODROUTE"), ACRX_CMD_MODAL, cmdRoutePipes);

        if (!ModulusLitePlugin::instance().initialize(hWndAcad))
            return AcRx::kRetError;

        ModulusLitePlugin::instance().showUI();
    }
    return AcRx::kRetOK;

    case AcRx::kUnloadAppMsg:
        acedRegCmds->removeGroup(_T("MODULUS"));
        ModulusLitePlugin::instance().shutdown();
        return AcRx::kRetOK;
    }

    return AcRx::kRetOK;
}
