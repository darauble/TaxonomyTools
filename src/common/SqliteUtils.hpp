#pragma once

#include <wx/wx.h>
#include <sqlite3.h>
#include <memory>
#include <stdexcept>

// Exception class for SQLite errors
class SqliteException : public std::runtime_error
{
public:
    explicit SqliteException(const wxString& message, int errorCode = 0)
        : std::runtime_error(message.ToStdString()), m_errorCode(errorCode)
    {
    }

    int GetErrorCode() const { return m_errorCode; }

private:
    int m_errorCode;
};

// RAII wrapper for sqlite3_stmt (prepared statement)
class SqliteStatement
{
public:
    SqliteStatement() : m_stmt(nullptr) {}

    explicit SqliteStatement(sqlite3* db, const wxString& sql)
        : m_stmt(nullptr)
    {
        Prepare(db, sql);
    }

    ~SqliteStatement()
    {
        if (m_stmt)
        {
            sqlite3_finalize(m_stmt);
            m_stmt = nullptr;
        }
    }

    // No copying
    SqliteStatement(const SqliteStatement&) = delete;
    SqliteStatement& operator=(const SqliteStatement&) = delete;

    // Move operations
    SqliteStatement(SqliteStatement&& other) noexcept
        : m_stmt(other.m_stmt)
    {
        other.m_stmt = nullptr;
    }

    SqliteStatement& operator=(SqliteStatement&& other) noexcept
    {
        if (this != &other)
        {
            if (m_stmt)
                sqlite3_finalize(m_stmt);
            m_stmt = other.m_stmt;
            other.m_stmt = nullptr;
        }
        return *this;
    }

    void Prepare(sqlite3* db, const wxString& sql)
    {
        if (m_stmt)
        {
            sqlite3_finalize(m_stmt);
            m_stmt = nullptr;
        }

        int result = sqlite3_prepare_v2(db, sql.utf8_str(), -1, &m_stmt, nullptr);
        if (result != SQLITE_OK)
        {
            wxString error = wxString::Format("Failed to prepare SQL: %s\nSQL: %s",
                sqlite3_errmsg(db), sql);
            throw SqliteException(error, result);
        }
    }

    // Binding functions
    void BindInt64(int index, long long value)
    {
        int result = sqlite3_bind_int64(m_stmt, index, value);
        if (result != SQLITE_OK)
        {
            throw SqliteException("Failed to bind int64", result);
        }
    }

    void BindText(int index, const wxString& value)
    {
        int result = sqlite3_bind_text(m_stmt, index, value.utf8_str(), -1, SQLITE_TRANSIENT);
        if (result != SQLITE_OK)
        {
            throw SqliteException("Failed to bind text", result);
        }
    }

    void BindNull(int index)
    {
        int result = sqlite3_bind_null(m_stmt, index);
        if (result != SQLITE_OK)
        {
            throw SqliteException("Failed to bind null", result);
        }
    }

    // Execution
    bool Step()
    {
        int result = sqlite3_step(m_stmt);
        if (result == SQLITE_ROW)
        {
            return true;
        }
        else if (result == SQLITE_DONE)
        {
            return false;
        }
        else
        {
            throw SqliteException("Step failed", result);
        }
    }

    void Execute()
    {
        int result = sqlite3_step(m_stmt);
        if (result != SQLITE_DONE && result != SQLITE_ROW)
        {
            throw SqliteException("Execute failed", result);
        }
    }

    void Reset()
    {
        int result = sqlite3_reset(m_stmt);
        if (result != SQLITE_OK)
        {
            throw SqliteException("Reset failed", result);
        }
    }

    // Column access
    long long GetColumnInt64(int column) const
    {
        return sqlite3_column_int64(m_stmt, column);
    }

    wxString GetColumnText(int column) const
    {
        const unsigned char* text = sqlite3_column_text(m_stmt, column);
        if (text)
        {
            return wxString::FromUTF8(reinterpret_cast<const char*>(text));
        }
        return wxEmptyString;
    }

    int GetColumnCount() const
    {
        return sqlite3_column_count(m_stmt);
    }

    sqlite3_stmt* Get() { return m_stmt; }
    const sqlite3_stmt* Get() const { return m_stmt; }

private:
    sqlite3_stmt* m_stmt;
};

// RAII wrapper for sqlite3 database
class SqliteDatabase
{
public:
    SqliteDatabase() : m_db(nullptr) {}

    explicit SqliteDatabase(const wxString& path, int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)
        : m_db(nullptr)
    {
        Open(path, flags);
    }

    ~SqliteDatabase()
    {
        Close();
    }

    // No copying
    SqliteDatabase(const SqliteDatabase&) = delete;
    SqliteDatabase& operator=(const SqliteDatabase&) = delete;

    // Move operations
    SqliteDatabase(SqliteDatabase&& other) noexcept
        : m_db(other.m_db)
    {
        other.m_db = nullptr;
    }

    SqliteDatabase& operator=(SqliteDatabase&& other) noexcept
    {
        if (this != &other)
        {
            Close();
            m_db = other.m_db;
            other.m_db = nullptr;
        }
        return *this;
    }

    void Open(const wxString& path, int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)
    {
        Close();

        int result = sqlite3_open_v2(path.utf8_str(), &m_db, flags, nullptr);
        if (result != SQLITE_OK)
        {
            wxString error = wxString::Format("Failed to open database: %s\nPath: %s",
                sqlite3_errmsg(m_db), path);
            sqlite3_close(m_db);
            m_db = nullptr;
            throw SqliteException(error, result);
        }
    }

    void Close()
    {
        if (m_db)
        {
            sqlite3_close(m_db);
            m_db = nullptr;
        }
    }

    bool IsOpen() const
    {
        return m_db != nullptr;
    }

    // Execute simple SQL without parameters
    void Execute(const wxString& sql)
    {
        char* errorMsg = nullptr;
        int result = sqlite3_exec(m_db, sql.utf8_str(), nullptr, nullptr, &errorMsg);
        if (result != SQLITE_OK)
        {
            wxString error = wxString::Format("Failed to execute SQL: %s\nSQL: %s",
                errorMsg ? errorMsg : "Unknown error", sql);
            if (errorMsg)
                sqlite3_free(errorMsg);
            throw SqliteException(error, result);
        }
    }

    // Get last insert rowid
    long long GetLastInsertRowId() const
    {
        return sqlite3_last_insert_rowid(m_db);
    }

    // Get number of rows changed
    int GetChanges() const
    {
        return sqlite3_changes(m_db);
    }

    // Get error message
    wxString GetErrorMessage() const
    {
        if (m_db)
        {
            return wxString::FromUTF8(sqlite3_errmsg(m_db));
        }
        return wxEmptyString;
    }

    sqlite3* Get() { return m_db; }
    const sqlite3* Get() const { return m_db; }

private:
    sqlite3* m_db;
};

// RAII transaction helper
class SqliteTransaction
{
public:
    explicit SqliteTransaction(SqliteDatabase& db)
        : m_db(db), m_committed(false)
    {
        m_db.Execute("BEGIN TRANSACTION");
    }

    ~SqliteTransaction()
    {
        if (!m_committed)
        {
            try
            {
                m_db.Execute("ROLLBACK");
            }
            catch (...)
            {
                // Suppress exceptions in destructor
            }
        }
    }

    // No copying or moving
    SqliteTransaction(const SqliteTransaction&) = delete;
    SqliteTransaction& operator=(const SqliteTransaction&) = delete;
    SqliteTransaction(SqliteTransaction&&) = delete;
    SqliteTransaction& operator=(SqliteTransaction&&) = delete;

    void Commit()
    {
        if (!m_committed)
        {
            m_db.Execute("COMMIT");
            m_committed = true;
        }
    }

    void Rollback()
    {
        if (!m_committed)
        {
            m_db.Execute("ROLLBACK");
            m_committed = true;
        }
    }

private:
    SqliteDatabase& m_db;
    bool m_committed;
};
