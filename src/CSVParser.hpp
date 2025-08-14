#pragma once

#include <wx/wx.h>
#include <vector>
#include <map>

struct TaxonomyEntry
{
    long id;
    wxString scientificName;
    wxString kingdom;
    wxString phylum;
    wxString taxonClass;
    wxString order;
    wxString family;
    wxString genus;
};

struct VernacularEntry
{
    long taxonId;
    wxString vernacularName;
    wxString language;
    wxString countryCode;
};

class CSVParser
{
public:
    CSVParser();
    
    bool ParseTaxonomyCSV(const wxString& content, std::map<long, TaxonomyEntry>& taxonomyMap);
    bool ParseVernacularCSV(const wxString& content, std::vector<VernacularEntry>& vernacularEntries);
    
    static wxString ExtractLanguageFromFilename(const wxString& filename);
    
private:
    std::vector<wxString> ParseCSVLine(const wxString& line);
    wxString UnescapeCSVField(const wxString& field);
    bool IsQuoted(const wxString& field);
};