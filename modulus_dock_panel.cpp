#include "modulus_dock_panel.h"
#include <afxwin.h>

IMPLEMENT_DYNAMIC(ModulusDockPanel, CDockablePane)

// ============================================================================
// MESSAGE MAP
// ============================================================================

BEGIN_MESSAGE_MAP(ModulusDockPanel, CDockablePane)
ON_WM_CREATE()
ON_WM_SIZE()
ON_WM_PAINT()
ON_WM_ERASEBKGND()
ON_BN_CLICKED(IDC_BUTTON_HELLO, &ModulusDockPanel::OnHelloClicked)
ON_BN_CLICKED(IDC_BUTTON_LAYOUT, &ModulusDockPanel::OnLayoutClicked)
ON_BN_CLICKED(IDC_BUTTON_ROUTE, &ModulusDockPanel::OnRouteClicked)
END_MESSAGE_MAP()

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

ModulusDockPanel::ModulusDockPanel()
{
}

ModulusDockPanel::~ModulusDockPanel()
{
}

// ============================================================================
// CREATE
// ============================================================================

BOOL ModulusDockPanel::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
                              DWORD dwStyle, const RECT &rect, CWnd *pParentWnd,
                              UINT nID, CCreateContext *pContext)
{
    if (!CDockablePane::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext))
    {
        return FALSE;
    }

    EnableDocking(CBRS_ALIGN_ANY);
    SetupUI();
    ApplyStyles();

    return TRUE;
}

// ============================================================================
// MESSAGE HANDLERS
// ============================================================================

int ModulusDockPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    SetupUI();
    return 0;
}

void ModulusDockPanel::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);
    AdjustLayout();
}

void ModulusDockPanel::OnPaint()
{
    CPaintDC dc(this);

    CRect rectClient;
    GetClientRect(rectClient);

    // Draw background
    dc.FillSolidRect(rectClient, RGB(245, 246, 250));
}

BOOL ModulusDockPanel::OnEraseBkgnd(CDC *pDC)
{
    CRect rectClient;
    GetClientRect(rectClient);
    pDC->FillSolidRect(rectClient, RGB(245, 246, 250));
    return TRUE;
}

void ModulusDockPanel::AdjustLayout()
{
    if (GetSafeHwnd() == nullptr)
        return;

    CRect rectClient;
    GetClientRect(rectClient);

    int nMargin = 10;
    int nLabelHeight = 30;
    int nButtonHeight = 35;
    int nSpacing = 10;

    // Welcome label
    rectClient.top = nMargin;
    CRect rectLabel = rectClient;
    rectLabel.bottom = rectLabel.top + nLabelHeight;
    m_welcomeLabel.MoveWindow(rectLabel);

    // Status label
    CRect rectStatus = rectClient;
    rectStatus.top = rectLabel.bottom + nSpacing;
    rectStatus.bottom = rectStatus.top + nLabelHeight;
    m_statusLabel.MoveWindow(rectStatus);

    // Buttons
    int nButtonWidth = (rectClient.Width() - 3 * nMargin - 2 * nSpacing) / 3;
    CRect rectButton = rectClient;
    rectButton.top = rectStatus.bottom + nSpacing * 2;
    rectButton.bottom = rectButton.top + nButtonHeight;
    rectButton.left = nMargin;
    rectButton.right = rectButton.left + nButtonWidth;

    m_helloButton.MoveWindow(rectButton);

    rectButton.left += nButtonWidth + nSpacing;
    rectButton.right = rectButton.left + nButtonWidth;
    m_layoutButton.MoveWindow(rectButton);

    rectButton.left += nButtonWidth + nSpacing;
    rectButton.right = rectButton.left + nButtonWidth;
    m_routeButton.MoveWindow(rectButton);
}

// ============================================================================
// UI SETUP
// ============================================================================

void ModulusDockPanel::SetupUI()
{
    // Welcome label
    m_welcomeLabel.Create(_T("Hello World from Modulus Lite!"),
                          WS_CHILD | WS_VISIBLE | SS_CENTER,
                          CRect(0, 0, 100, 30), this, IDC_STATIC);

    LOGFONT lf = {};
    lf.lfHeight = 14;
    lf.lfWeight = FW_BOLD;
    _tcscpy_s(lf.lfFaceName, _T("Segoe UI"));
    CFont fontWelcome;
    fontWelcome.CreateFontIndirect(&lf);
    m_welcomeLabel.SetFont(&fontWelcome);

    // Status label
    m_statusLabel.Create(_T("Ready to design pipelines"),
                         WS_CHILD | WS_VISIBLE | SS_CENTER,
                         CRect(0, 0, 100, 30), this, IDC_STATIC);

    lf.lfHeight = 11;
    lf.lfWeight = FW_NORMAL;
    CFont fontStatus;
    fontStatus.CreateFontIndirect(&lf);
    m_statusLabel.SetFont(&fontStatus);

    // Buttons
    m_helloButton.Create(_T("Hello!"),
                         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                         CRect(0, 0, 100, 35), this, IDC_BUTTON_HELLO);

    m_layoutButton.Create(_T("Generate Layout"),
                          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                          CRect(0, 0, 100, 35), this, IDC_BUTTON_LAYOUT);

    m_routeButton.Create(_T("Route Pipes"),
                         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                         CRect(0, 0, 100, 35), this, IDC_BUTTON_ROUTE);
}

void ModulusDockPanel::ApplyStyles()
{
    // Set text colors
    m_welcomeLabel.SetTextColor(RGB(44, 62, 80));   // Dark blue-gray
    m_statusLabel.SetTextColor(RGB(127, 140, 141)); // Light gray
}

// ============================================================================
// BUTTON HANDLERS
// ============================================================================

void ModulusDockPanel::OnHelloClicked()
{
    m_statusLabel.SetWindowText(_T("Hello from Modulus Lite!"));
}

void ModulusDockPanel::OnLayoutClicked()
{
    m_statusLabel.SetWindowText(_T("Generating machine layout..."));
    // Call plugin command
}

void ModulusDockPanel::OnRouteClicked()
{
    m_statusLabel.SetWindowText(_T("Routing pipes..."));
    // Call plugin command
}