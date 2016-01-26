#pragma once

#include "bankdatabase.h"

class BankDatabaseGen : public bankdb::BankDatabase
{
public:
    void loadBnk(const char* filename, const char* bankname, const char* prefix);

    void loadBnk2(const char* fn, const char* bankname, const char* prefix,
                         const std::string& melo_filter,
                         const std::string& perc_filter);

    void loadDoom(const char* fn, const char* bankname, const char* prefix);

    void loadMiles(const char* fn, const char* bankname, const char* prefix);

    void loadIBK(const char* fn, const char* bankname, const char* prefix, bool percussive);

    void loadJunglevision(const char* fn, const char* bankname, const char* prefix);

    void loadTMB(const char* fn, const char* bankname, const char* prefix);

    void loadBisqwit(const char* fn, const char* bankname, const char* prefix);

    void save(const std::string& filename);
    void dump();
};
