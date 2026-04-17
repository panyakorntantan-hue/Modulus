#define _WINSOCKAPI_
#include <winsock2.h>   // ✅ MUST be first
#include <windows.h>    // then windows
#include <afxwin.h> 
#include "ui_panel.h"
#include "aduiDock.h"
#include <QWidget>
#include <QMainWindow>

using namespace ModulusLite;

ModulusDock::ModulusDock()
    : m_pPanel(nullptr) {}

ModulusDock::~ModulusDock() {
    if (m_pPanel) {
        delete m_pPanel;
        m_pPanel = nullptr;
    }
}

bool ModulusDock::createDock(HWND hWndAcad) {
    if (!Create(
            hWndAcad,
            _T("Modulus Lite"),
            WS_CHILD | WS_VISIBLE,
            ACBRS_RIGHT)) {
        return false;
    }

    // Convert dock HWND → Qt widget
    QWidget* dockQtParent = QWidget::find((WId)m_hWnd);
    if (!dockQtParent) {
        return false;
    }

    // Create your existing UI panel INSIDE the dock
    m_pPanel = new UIPanel(dockQtParent);
    m_pPanel->setWindowFlags(Qt::Widget);
    m_pPanel->setMinimumSize(350, 500);
    m_pPanel->show();

    return true;
}