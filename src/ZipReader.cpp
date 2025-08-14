#include "ZipReader.hpp"
#include <wx/regex.h>
#include <string>

ZipReader::ZipReader()
    : m_archive(nullptr)
{
}

ZipReader::~ZipReader()
{
    CloseArchive();
}

bool ZipReader::OpenArchive(const wxString& archivePath)
{
    CloseArchive();
    
    int error;
    std::string pathStr = archivePath.ToStdString();
    m_archive = zip_open(pathStr.c_str(), ZIP_RDONLY, &error);
    if (!m_archive)
    {
        return false;
    }
    
    m_archivePath = archivePath;
    return true;
}

void ZipReader::CloseArchive()
{
    if (m_archive)
    {
        zip_close(m_archive);
        m_archive = nullptr;
    }
    m_archivePath.Clear();
}

std::vector<ZipFileInfo> ZipReader::GetFileList() const
{
    std::vector<ZipFileInfo> files;
    
    if (!m_archive)
        return files;
    
    zip_int64_t numEntries = zip_get_num_entries(m_archive, 0);
    for (zip_int64_t i = 0; i < numEntries; ++i)
    {
        struct zip_stat stat;
        if (zip_stat_index(m_archive, i, 0, &stat) == 0)
        {
            ZipFileInfo info;
            info.filename = wxString::FromUTF8(stat.name);
            info.size = stat.size;
            info.isDirectory = info.filename.EndsWith("/");
            files.push_back(info);
        }
    }
    
    return files;
}

std::vector<wxString> ZipReader::GetLanguageFiles() const
{
    std::vector<wxString> languageFiles;
    
    if (!m_archive)
        return languageFiles;
    
    wxRegEx regex("VernacularNames-([^.]+)\\.csv$", wxRE_ADVANCED);
    
    std::vector<ZipFileInfo> files = GetFileList();
    for (const auto& file : files)
    {
        if (!file.isDirectory && regex.Matches(file.filename))
        {
            languageFiles.push_back(file.filename);
        }
    }
    
    return languageFiles;
}

bool ZipReader::ReadFileToString(const wxString& filename, wxString& content)
{
    if (!m_archive)
        return false;
    
    std::string filenameStr = filename.ToStdString();
    zip_file_t* file = zip_fopen(m_archive, filenameStr.c_str(), 0);
    if (!file)
        return false;
    
    struct zip_stat stat;
    if (zip_stat(m_archive, filenameStr.c_str(), 0, &stat) != 0)
    {
        zip_fclose(file);
        return false;
    }
    
    std::vector<char> buffer(stat.size + 1);
    zip_int64_t bytesRead = zip_fread(file, buffer.data(), stat.size);
    zip_fclose(file);
    
    if (bytesRead < 0)
        return false;
    
    buffer[bytesRead] = '\0';
    content = wxString::FromUTF8(buffer.data());
    return true;
}

bool ZipReader::ReadFileToBuffer(const wxString& filename, std::vector<char>& buffer)
{
    if (!m_archive)
        return false;
    
    std::string filenameStr = filename.ToStdString();
    zip_file_t* file = zip_fopen(m_archive, filenameStr.c_str(), 0);
    if (!file)
        return false;
    
    struct zip_stat stat;
    if (zip_stat(m_archive, filenameStr.c_str(), 0, &stat) != 0)
    {
        zip_fclose(file);
        return false;
    }
    
    buffer.resize(stat.size);
    zip_int64_t bytesRead = zip_fread(file, buffer.data(), stat.size);
    zip_fclose(file);
    
    if (bytesRead < 0)
    {
        buffer.clear();
        return false;
    }
    
    buffer.resize(bytesRead);
    return true;
}