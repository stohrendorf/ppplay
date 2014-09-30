/*
 * AdPlug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (c) 1999 - 2003 Simon Peter <dn.tlp@gmx.net>, et al.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * database.h - AdPlug database class
 * Copyright (c) 2002 Riven the Mage <riven@ok.ru>
 * Copyright (c) 2002, 2003 Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_ADPLUG_DATABASE
#define H_ADPLUG_DATABASE

#include <iostream>
#include <string>
#include <libbinio/binio.h>
#include "stuff/utils.h"

class CAdPlugDatabase {
  DISABLE_COPY(CAdPlugDatabase)
public:
  struct CKey {
    unsigned short crc16 = 0;
    unsigned long crc32 = 0;

    CKey() = default;
    CKey(binistream &in);

    bool operator==(const CKey &key);

  private:
    void make(binistream &in);
  };

  class CRecord {
  public:
    enum RecordType {
      Plain,
      SongInfo,
      ClockSpeed
    };

    RecordType m_type = Plain;
    CKey m_key {}
    ;
    std::string m_filetype {}
    , m_comment {}
    ;

    static CRecord *factory(RecordType m_type);
    static CRecord *factory(binistream &in);

    CRecord() = default;
    virtual ~CRecord() = default;

    void write(binostream &out);

    bool user_read(std::istream &in, std::ostream &out);
    bool user_write(std::ostream &out);

  protected:
    virtual void read_own(binistream &in) = 0;
    virtual void write_own(binostream &out) = 0;
    virtual unsigned long get_size() = 0;
    virtual bool user_read_own(std::istream &in, std::ostream &out) = 0;
    virtual bool user_write_own(std::ostream &out) = 0;
  };

  CAdPlugDatabase();
  ~CAdPlugDatabase();

  bool load(std::string db_name);
  bool load(binistream &f);
  bool save(std::string db_name);
  bool save(binostream &f);

  bool insert(CRecord *record);

  void wipe(CRecord *record);
  void wipe();

  CRecord *search(CKey const &key);
  bool lookup(CKey const &key);

  CRecord *get_record();

  bool go_forward();
  bool go_backward();

  void goto_begin();
  void goto_end();

private:
  static const unsigned short hash_radix;

  struct DB_Bucket {
    DISABLE_COPY(DB_Bucket)
    unsigned long index = 0;
    bool deleted = false;
    DB_Bucket *chain = nullptr;

    CRecord *record = nullptr;

    DB_Bucket(unsigned long nindex, CRecord *newrecord,
              DB_Bucket *newchain = 0);
    ~DB_Bucket();
  };

  DB_Bucket **m_dbLinear = nullptr;
  DB_Bucket **m_dbHashed = nullptr;

  unsigned long linear_index = 0, linear_logic_length = 0, linear_length = 0;

  unsigned long make_hash(CKey const &key);
};

class CPlainRecord : public CAdPlugDatabase::CRecord {
public:
  CPlainRecord() { m_type = Plain; }

protected:
  virtual void read_own(binistream &) {}
  virtual void write_own(binostream &) {}
  virtual unsigned long get_size() { return 0; }
  virtual bool user_read_own(std::istream &, std::ostream &) { return true; }
  virtual bool user_write_own(std::ostream &) { return true; }
};

class CInfoRecord : public CAdPlugDatabase::CRecord {
public:
  std::string m_title {}
  ;
  std::string m_author {}
  ;

  CInfoRecord();

protected:
  virtual void read_own(binistream &in);
  virtual void write_own(binostream &out);
  virtual unsigned long get_size();
  virtual bool user_read_own(std::istream &in, std::ostream &out);
  virtual bool user_write_own(std::ostream &out);
};

class CClockRecord : public CAdPlugDatabase::CRecord {
public:
  float m_clock = 0;

  CClockRecord();

protected:
  virtual void read_own(binistream &in);
  virtual void write_own(binostream &out);
  virtual unsigned long get_size();
  virtual bool user_read_own(std::istream &in, std::ostream &out);
  virtual bool user_write_own(std::ostream &out);
};

#endif
