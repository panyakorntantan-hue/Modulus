// modulus_dock.cpp
// AutoCAD dockable panel implementation for Modulus Lite.

#include <tchar.h>
#include <aduipalette.h>
#include "aced.h"        // acedGetAcadDwgView, adsw_acadMainWnd
#include "acedads.h"
#include "modulus_dock.h"
#include "panel.h"

namespace ModulusLite
{

// ============================================================================
// ModulusDock
// ============================================================================

ModulusDock::ModulusDock()  = default;
ModulusDock::~ModulusDock() = default;

bool ModulusDock::createDock(HWND hWndAcad)
{
    if (m_created)
        return true;

    // ------------------------------------------------------------------ //
    // 1.  Create the palette-set  (AutoCAD's docking chrome)
    // ------------------------------------------------------------------ //
    m_pPaletteSet = new CAdUiPaletteSet();

    // Dock styles:  PS_SNAP enables snapping to document edges,
    //              PS_ONTOP keeps it above drawing area,
    //              PS_EDIT_NAME lets user rename (optional)
    DWORD dwStyle = PS_SNAP | PS_ONTOP | PS_EDIT_NAME;

    CWnd* pAcadWnd = CWnd::FromHandle(hWndAcad);
    if (!pAcadWnd)
    {
        acutPrintf(_T("[Modulus Lite] ERROR: Could not get AutoCAD CWnd.\n"));
        delete m_pPaletteSet;
        m_pPaletteSet = nullptr;
        return false;
    }

    // PaletteSet Create(title, style, rect, parent)
    // Start docked to the right edge — rect is ignored when docked.
    CRect rcInit(0, 0, 280, 480);
    if (!m_pPaletteSet->Create(_T("Modulus Lite"), dwStyle, rcInit, pAcadWnd))
    {
        acutPrintf(_T("[Modulus Lite] ERROR: CAdUiPaletteSet::Create failed.\n"));
        delete m_pPaletteSet;
        m_pPaletteSet = nullptr;
        return false;
    }

    // ------------------------------------------------------------------ //
    // 2.  Add a palette (tab) inside the set
    // ------------------------------------------------------------------ //
    m_pPalette = new CAdUiPalette();
    if (!m_pPalette->Create(_T("Panel"), m_pPaletteSet))
    {
        acutPrintf(_T("[Modulus Lite] ERROR: CAdUiPalette::Create failed.\n"));
        return false;
    }
    m_pPaletteSet->AddPalette(m_pPalette);

    // ------------------------------------------------------------------ //
    // 3.  Create UIPanel as a child of the palette
    // ------------------------------------------------------------------ //
    m_pPanel = std::make_unique<UIPanel>();

    CRect rcPanel;
    m_pPalette->GetClientRect(&rcPanel);

    if (!m_pPanel->Create(nullptr,                     // window class (auto-registered)
                          _T(""),
                          WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                          rcPanel,
                          m_pPalette,
                          IDC_STATIC))
    {
        acutPrintf(_T("[Modulus Lite] ERROR: UIPanel::Create failed.\n"));
        return false;
    }

    // ------------------------------------------------------------------ //
    // 4.  Dock to the right side of AutoCAD frame by default
    // ------------------------------------------------------------------ //
    m_pPaletteSet->SetDockSide(AC_DB_DOCT_RIGHT);
    m_pPaletteSet->ShowWindow(SW_SHOW);

    m_created = true;
    acutPrintf(_T("[Modulus Lite] Dock panel created and docked to right edge.\n"));
    return true;
}

void ModulusDock::ShowControlBar(BOOL bShow, BOOL /*bDelay*/)
{
    if (m_pPaletteSet && m_pPaletteSet->GetSafeHwnd())
    {
        m_pPaletteSet->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
    }
}

} // namespace ModulusLite