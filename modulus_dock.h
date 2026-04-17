#pragma once

#define _WINSOCKAPI_
#include <winsock2.h>
#include <windows.h>
#include <afxwin.h>
#include "adui.h"
#include "aduiDock.h"

namespace ModulusLite {

class UIPanel;

class ModulusDock : public AdUiDockControlBar {
public:
    ModulusDock() : m_pPanel(nullptr) {}

    bool createDock(HWND hWndAcad)
    {
        CWnd* pParent = CWnd::FromHandle(hWndAcad);

        if (!Create(
            pParent,
            _T("Modulus Lite"),
            WS_CHILD | WS_VISIBLE,
            ACBRS_RIGHT))
        {
            return false;
        }

        EnableDocking(CBRS_ALIGN_ANY);
        pParent->DockControlBar(this);

        return true;
    }

private:
    UIPanel* m_pPanel;
};

}