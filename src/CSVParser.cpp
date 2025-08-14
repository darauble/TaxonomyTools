#include "CSVParser.hpp"
#include <wx/regex.h>
#include <wx/tokenzr.h>

CSVParser::CSVParser()
{
}

bool CSVParser::ParseTaxonomyCSV(const wxString& content, std::map<long, TaxonomyEntry>& taxonomyMap)
{
    wxStringTokenizer tokenizer(content, wxT("\n"), wxTOKEN_RET_EMPTY_ALL);
    bool firstLine = true;
    
    while (tokenizer.HasMoreTokens())
    {
        wxString line = tokenizer.GetNextToken().Trim(false).Trim(true);
        if (line.IsEmpty())
            continue;
            
        if (firstLine)
        {
            firstLine = false;
            continue; // Skip header
        }
        
        std::vector<wxString> fields = ParseCSVLine(line);
        if (fields.size() < 14)
            continue;
            
        TaxonomyEntry entry;
        long id;
        if (!fields[0].ToLong(&id))
        {
            printf("Warning: Invalid taxonomy ID '%s', skipping entry\n", fields[0].ToStdString().c_str());
            continue;
        }
        entry.id = id;
        
        // Scientific name is in the 14th field (index 13)
        if (fields.size() > 13)
            entry.scientificName = fields[13];
        else
            entry.scientificName = ""; // Empty if field doesn't exist
            
        entry.kingdom = fields[3];
        entry.phylum = fields[4];
        entry.taxonClass = fields[5];
        entry.order = fields[6];
        entry.family = fields[7];
        if (fields.size() > 8)
            entry.genus = fields[8];
            
        taxonomyMap[entry.id] = entry;
    }
    
    return !taxonomyMap.empty();
}

bool CSVParser::ParseVernacularCSV(const wxString& content, std::vector<VernacularEntry>& vernacularEntries)
{
    wxStringTokenizer tokenizer(content, wxT("\n"), wxTOKEN_RET_EMPTY_ALL);
    bool firstLine = true;
    
    while (tokenizer.HasMoreTokens())
    {
        wxString line = tokenizer.GetNextToken().Trim(false).Trim(true);
        if (line.IsEmpty())
            continue;
            
        if (firstLine)
        {
            firstLine = false;
            continue; // Skip header
        }
        
        std::vector<wxString> fields = ParseCSVLine(line);
        if (fields.size() < 3)
            continue;
            
        VernacularEntry entry;
        long taxonId;
        if (!fields[0].ToLong(&taxonId))
        {
            printf("Warning: Invalid vernacular taxon ID '%s', skipping entry\n", fields[0].ToStdString().c_str());
            continue;
        }
        entry.taxonId = taxonId;
        entry.vernacularName = fields[1];
        entry.language = fields[2];
        if (fields.size() > 3)
            entry.countryCode = fields[3];
            
        vernacularEntries.push_back(entry);
    }
    
    return !vernacularEntries.empty();
}

wxString CSVParser::ExtractLanguageFromFilename(const wxString& filename)
{
    wxRegEx regex("VernacularNames-([^.]+)\\.csv$", wxRE_ADVANCED);
    if (regex.Matches(filename))
    {
        return regex.GetMatch(filename, 1);
    }
    return wxEmptyString;
}

std::vector<wxString> CSVParser::ParseCSVLine(const wxString& line)
{
    std::vector<wxString> fields;
    wxString currentField;
    bool inQuotes = false;
    
    for (size_t i = 0; i < line.length(); ++i)
    {
        wxChar ch = line[i];
        
        if (ch == wxT('"'))
        {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == wxT('"'))
            {
                currentField += wxT('"');
                ++i; // Skip next quote
            }
            else
            {
                inQuotes = !inQuotes;
            }
        }
        else if (ch == wxT(',') && !inQuotes)
        {
            fields.push_back(UnescapeCSVField(currentField));
            currentField.Clear();
        }
        else
        {
            currentField += ch;
        }
    }
    
    fields.push_back(UnescapeCSVField(currentField));
    return fields;
}

wxString CSVParser::UnescapeCSVField(const wxString& field)
{
    wxString result = field;
    result.Trim(false).Trim(true);
    
    if (IsQuoted(result))
    {
        result = result.Mid(1, result.length() - 2);
        result.Replace(wxT("\"\""), wxT("\""));
    }
    
    return result;
}

bool CSVParser::IsQuoted(const wxString& field)
{
    return field.length() >= 2 && field.StartsWith(wxT("\"")) && field.EndsWith(wxT("\""));
}