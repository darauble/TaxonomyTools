#pragma once

#include <wx/wx.h>

class TaxonomyApp : public wxApp
{
public:
    virtual bool OnInit() override;
    virtual int OnExit() override;
};

wxDECLARE_APP(TaxonomyApp);