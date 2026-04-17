#pragma once

#include "base.h"

// then MFC / ARX stuff
#include <vector>

#ifdef _WINSOCK_H
#error "winsock.h got included — include order is broken"
#endif

// Forward declarations so the header compiles without pulling in core.h
namespace ModulusLite { struct Machine; struct Pipe; class Pipeline; }

namespace ModulusLite
{

class UIPanel : public CWnd
{
    DECLARE_DYNAMIC(UIPanel)

public:
    UIPanel();
    virtual ~UIPanel();

    // Data setters — call these after createDock()
    void setMachines(std::vector<Machine>* p)  { m_pMachines = p; }
    void setPipes   (std::vector<Pipe>*    p)  { m_pPipes    = p; }
    void setPipeline(Pipeline*             p)  { m_pPipeline = p; }

protected:
    // MFC message map
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBtnLayout();
    afx_msg void OnBtnRoute();
    DECLARE_MESSAGE_MAP()

private:
    void layoutControls(int cx, int cy);

    // Child controls
    CStatic  m_lblTitle;
    CStatic  m_lblStatus;
    CButton  m_btnLayout;
    CButton  m_btnRoute;
    CListBox m_listLog;

    // Plugin data (non-owning pointers)
    std::vector<Machine>* m_pMachines = nullptr;
    std::vector<Pipe>*    m_pPipes    = nullptr;
    Pipeline*             m_pPipeline = nullptr;

    // Control IDs
    enum : UINT {
        ID_BTN_LAYOUT = 1001,
        ID_BTN_ROUTE  = 1002,
        ID_LIST_LOG   = 1003,
    };
};

} // namespace ModulusLite