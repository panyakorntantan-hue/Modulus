#pragma once

// modulus_dock.h
// AutoCAD-dockable panel host for Modulus Lite.
// Wraps a CAdUiDockControlBar and embeds UIPanel inside it.
#include <aduipalette.h>   // CAdUiPaletteSet / CAdUiPalette (ObjectARX)
#include "panel.h"

namespace ModulusLite
{

// ============================================================================
// ModulusDock
// A thin wrapper that:
//  1. Creates an AdUi palette-set (the dockable chrome AutoCAD provides)
//  2. Hosts a UIPanel child window inside it
//  3. Offers ShowControlBar / hide helpers used by the plugin singleton
// ============================================================================
class ModulusDock
{
public:
    ModulusDock();
    ~ModulusDock();

    // Call once during plugin init.  hWndAcad = adsw_acadMainWnd().hwnd
    bool createDock(HWND hWndAcad);

    // Show / hide the palette set
    void ShowControlBar(BOOL bShow, BOOL bDelay);

    // Access the embedded panel (set machines / pipes / pipeline on it)
    UIPanel* panel() { return m_pPanel.get(); }

private:
    // The ObjectARX palette-set is an MFC CWnd derivative managed by AcAp.
    // We allocate it with new; AutoCAD owns its lifetime after Create().
    CAdUiPaletteSet*        m_pPaletteSet   = nullptr;
    CAdUiPalette*           m_pPalette      = nullptr;
    std::unique_ptr<UIPanel> m_pPanel;

    bool m_created = false;
};

} // namespace ModulusLite