#pragma once

#include <wx/wx.h>

class TreeFrame;

class TreeApp : public wxApp
{
public:
    virtual bool OnInit() override;

private:
    TreeFrame* m_frame;
};

wxDECLARE_APP(TreeApp);
