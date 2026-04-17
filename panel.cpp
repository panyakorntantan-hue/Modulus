#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

// Guard against Winsock 1.0
#define _WINSOCKAPI_ 

// These two must come first
#include <winsock2.h>
#include <ws2tcpip.h>

// Now include your project headers
#include "base.h"
#include "panel.h"

namespace ModulusLite
{

// ============================================================================
// Colour palette — industrial/dark theme matching AutoCAD's dark UI
// ============================================================================
namespace Colors
{
    static const COLORREF Background   = RGB(30,  30,  30);
    static const COLORREF HeaderBg     = RGB(20,  20,  20);
    static const COLORREF AccentBlue   = RGB(0,  122, 204);
    static const COLORREF TextPrimary  = RGB(220, 220, 220);
    static const COLORREF TextMuted    = RGB(130, 130, 130);
    static const COLORREF ButtonBg     = RGB(50,  50,  50);
    static const COLORREF ButtonHover  = RGB(0,  100, 180);
}

// ============================================================================
// IMPLEMENT_DYNAMIC + message map
// ============================================================================
IMPLEMENT_DYNAMIC(UIPanel, CWnd)

BEGIN_MESSAGE_MAP(UIPanel, CWnd)
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_BN_CLICKED(ID_BTN_LAYOUT, OnBtnLayout)
    ON_BN_CLICKED(ID_BTN_ROUTE,  OnBtnRoute)
END_MESSAGE_MAP()

// ============================================================================
// Construction / Destruction
// ============================================================================
UIPanel::UIPanel()  = default;
UIPanel::~UIPanel() = default;

// ============================================================================
// OnPaint — draw the header band
// ============================================================================
void UIPanel::OnPaint()
{
    CPaintDC dc(this);

    CRect rcClient;
    GetClientRect(&rcClient);

    // ---- Full background ----
    dc.FillSolidRect(&rcClient, Colors::Background);

    // ---- Header band (top 48 px) ----
    CRect rcHeader(rcClient.left, rcClient.top,
                   rcClient.right, rcClient.top + 48);
    dc.FillSolidRect(&rcHeader, Colors::HeaderBg);

    // ---- Accent stripe (3 px bottom of header) ----
    CRect rcStripe(rcHeader.left, rcHeader.bottom - 3,
                   rcHeader.right, rcHeader.bottom);
    dc.FillSolidRect(&rcStripe, Colors::AccentBlue);

    // ---- Title text ----
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(Colors::TextPrimary);

    CFont fontTitle;
    fontTitle.CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                         CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                         DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));
    CFont* pOld = dc.SelectObject(&fontTitle);

    CRect rcText(rcHeader.left + 12, rcHeader.top,
                 rcHeader.right, rcHeader.bottom - 3);
    dc.DrawText(_T("Modulus Lite"), &rcText,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // ---- Version sub-text ----
    dc.SelectObject(pOld);
    CFont fontSub;
    fontSub.CreateFont(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                       DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));
    dc.SelectObject(&fontSub);
    dc.SetTextColor(Colors::TextMuted);

    CRect rcVer(rcHeader.left + 12, rcHeader.top + 26,
                rcHeader.right, rcHeader.bottom - 3);
    dc.DrawText(_T("v1.0  |  Hello World"), &rcVer,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    dc.SelectObject(pOld);
}

// ============================================================================
// OnSize — position child controls
// ============================================================================
void UIPanel::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    layoutControls(cx, cy);
}

// ============================================================================
// layoutControls — called on WM_SIZE and after Create()
// ============================================================================
void UIPanel::layoutControls(int cx, int cy)
{
    const int kMargin   = 10;
    const int kHeaderH  = 48;
    const int kBtnH     = 30;
    const int kGap      = 8;
    const int kStatusH  = 20;

    // Guard: controls may not exist yet on first OnSize before children created
    if (!m_btnLayout.GetSafeHwnd())
        return;

    int y = kHeaderH + kGap + kGap;

    // Status label
    m_lblStatus.MoveWindow(kMargin, y,
                           cx - kMargin * 2, kStatusH);
    y += kStatusH + kGap;

    // Layout button
    m_btnLayout.MoveWindow(kMargin, y,
                           cx - kMargin * 2, kBtnH);
    y += kBtnH + kGap;

    // Route button
    m_btnRoute.MoveWindow(kMargin, y,
                          cx - kMargin * 2, kBtnH);
    y += kBtnH + kGap * 2;

    // Log list — fills remaining space
    int listH = cy - y - kMargin;
    if (listH < 40) listH = 40;
    m_listLog.MoveWindow(kMargin, y,
                         cx - kMargin * 2, listH);
}

// ============================================================================
// PreCreateWindow / PostNcDestroy not needed for a plain CWnd child.
// We override Create() implicitly by calling the base version in modulus_dock.cpp.
// But we DO need to create child controls at the right time — hook PreSubclassWindow
// or, simpler, override OnCreate (requires registering WM_CREATE).
// We add WM_CREATE to the message map inline here.
// ============================================================================

// Add WM_CREATE entry dynamically by inserting a plain afx_msg:
// (We add it here rather than the header to keep the header clean.)
// Trick: use CWnd::PreSubclassWindow which is guaranteed to run
//        before any WM_SIZE arrives.

void UIPanel::PreSubclassWindow()
{
    CWnd::PreSubclassWindow();

    // ---- Create child controls ----
    CRect rc(0, 0, 0, 0); // will be sized by layoutControls on WM_SIZE

    // Status label
    m_lblStatus.Create(_T("Ready."),
                       WS_CHILD | WS_VISIBLE | SS_LEFT,
                       rc, this);

    // Buttons
    m_btnLayout.Create(_T("▶  Generate Layout (FDG)"),
                       WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                       rc, this, ID_BTN_LAYOUT);

    m_btnRoute.Create(_T("⟳  Route Pipes (A*)"),
                      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      rc, this, ID_BTN_ROUTE);

    // Log list
    m_listLog.Create(WS_CHILD | WS_VISIBLE | WS_BORDER |
                     WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
                     rc, this, ID_LIST_LOG);

    // Apply dark-ish font to all controls
    CFont* pFont = GetFont();
    if (pFont)
    {
        m_lblStatus.SetFont(pFont);
        m_btnLayout.SetFont(pFont);
        m_btnRoute.SetFont(pFont);
        m_listLog.SetFont(pFont);
    }

    m_listLog.AddString(_T("Modulus Lite panel ready."));
    m_listLog.AddString(_T("Use buttons above or AutoCAD commands."));
    m_listLog.AddString(_T("-------------------------------------------"));
}

// ============================================================================
// Button handlers
// ============================================================================

void UIPanel::OnBtnLayout()
{
    if (!m_pMachines)
    {
        m_listLog.AddString(_T("[Layout] No machine data available."));
        return;
    }

    m_lblStatus.SetWindowText(_T("Running FDG layout..."));
    FDGLayout::layout(*m_pMachines);

    CString msg;
    msg.Format(_T("[Layout] Positioned %zu machines."), m_pMachines->size());
    m_listLog.AddString(msg);

    for (const auto& m : *m_pMachines)
    {
        CString line;
        line.Format(_T("  Machine %d  (%.2f, %.2f, %.2f)"),
                    m.id, m.position.x, m.position.y, m.position.z);
        m_listLog.AddString(line);
    }

    m_listLog.SetCurSel(m_listLog.GetCount() - 1);
    m_lblStatus.SetWindowText(_T("Layout complete."));
}

void UIPanel::OnBtnRoute()
{
    if (!m_pPipeline || !m_pMachines || !m_pPipes)
    {
        m_listLog.AddString(_T("[Route] Pipeline not ready."));
        return;
    }

    m_lblStatus.SetWindowText(_T("Routing pipes..."));

    bool ok = m_pPipeline->execute(*m_pMachines, *m_pPipes);
    if (ok)
    {
        CString msg;
        msg.Format(_T("[Route] Routed %zu pipes."), m_pPipes->size());
        m_listLog.AddString(msg);

        for (const auto& p : *m_pPipes)
        {
            CString line;
            line.Format(_T("  Pipe %d  L=%.2fm  $%.2f  %s"),
                        p.id, p.length, p.cost, CString(p.tag.c_str()));
            m_listLog.AddString(line);
        }
        m_lblStatus.SetWindowText(_T("Routing complete."));
    }
    else
    {
        m_listLog.AddString(_T("[Route] ERROR: Pipeline execution failed."));
        m_lblStatus.SetWindowText(_T("Routing failed."));
    }

    m_listLog.SetCurSel(m_listLog.GetCount() - 1);
}

} // namespace ModulusLite