#ifndef MODULUS_DOCK_PANEL_H
#define MODULUS_DOCK_PANEL_H

#include <afxwin.h>
#include <afxtempl.h>
#include <afxdockablepane.h> // Required for CDockablePane
#include <afxframewndex.h>

class ModulusDockPanel : public CDockablePane
{
    DECLARE_DYNAMIC(ModulusDockPanel)

public:
    ModulusDockPanel();
    virtual ~ModulusDockPanel();

    // Initialize the panel
    BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, 
                DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, 
                UINT nID, CCreateContext *pContext = nullptr);

    // Size and layout
    virtual void AdjustLayout();

protected:
    // Message handlers
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC *pDC);

    DECLARE_MESSAGE_MAP()

private:
    // UI Controls
    CStatic m_welcomeLabel;
    CStatic m_statusLabel;
    CButton m_helloButton;
    CButton m_layoutButton;
    CButton m_routeButton;

    // Handlers
    afx_msg void OnHelloClicked();
    afx_msg void OnLayoutClicked();
    afx_msg void OnRouteClicked();

    // Helper
    void SetupUI();
    void ApplyStyles();
};

#endif // MODULUS_DOCK_PANEL_H