#pragma once

#include <wx/wx.h>
#include <zip.h>
#include <vector>
#include <string>
#include <map>

struct ZipFileInfo
{
    wxString filename;
    zip_uint64_t size;
    bool isDirectory;
};

class ZipReader
{
public:
    ZipReader();
    ~ZipReader();

    bool OpenArchive(const wxString& archivePath);
    void CloseArchive();
    
    std::vector<ZipFileInfo> GetFileList() const;
    std::vector<wxString> GetLanguageFiles() const;
    
    bool ReadFileToString(const wxString& filename, wxString& content);
    bool ReadFileToBuffer(const wxString& filename, std::vector<char>& buffer);
    
    bool IsOpen() const { return m_archive != nullptr; }

private:
    zip_t* m_archive;
    wxString m_archivePath;
};