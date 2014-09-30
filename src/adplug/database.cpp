/*
 * AdPlug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (c) 1999 - 2006 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * database.cpp - AdPlug database class
 * Copyright (c) 2002 Riven the Mage <riven@ok.ru>
 * Copyright (c) 2002, 2003, 2006 Simon Peter <dn.tlp@gmx.net>
 */

#include <libbinio/binio.h>
#include <libbinio/binfile.h>
#include <string.h>

#include "database.h"

#define DB_FILEID_V10 "AdPlug Module Information Database 1.0\x10"

/***** CAdPlugDatabase *****/

const unsigned short CAdPlugDatabase::hash_radix = 0xfff1; // should be prime

CAdPlugDatabase::CAdPlugDatabase()
    : linear_index(0), linear_logic_length(0), linear_length(0) {
  m_dbLinear = new DB_Bucket *[hash_radix];
  m_dbHashed = new DB_Bucket *[hash_radix];
  memset(m_dbLinear, 0, sizeof(DB_Bucket *) * hash_radix);
  memset(m_dbHashed, 0, sizeof(DB_Bucket *) * hash_radix);
}

CAdPlugDatabase::~CAdPlugDatabase() {
  unsigned long i;

  for (i = 0; i < linear_length; i++)
    delete m_dbLinear[i];

  delete[] m_dbLinear;
  delete[] m_dbHashed;
}

bool CAdPlugDatabase::load(std::string db_name) {
  binifstream f(db_name);
  if (f.error())
    return false;
  return load(f);
}

bool CAdPlugDatabase::load(binistream &f) {
  unsigned int idlen = strlen(DB_FILEID_V10);
  char *id = new char[idlen];
  unsigned long length;

  // Open database as little endian with IEEE floats
  f.setFlag(binio::BigEndian, false);
  f.setFlag(binio::FloatIEEE);

  f.readString(id, idlen);
  if (memcmp(id, DB_FILEID_V10, idlen)) {
    delete[] id;
    return false;
  }
  delete[] id;
  length = f.readInt(4);

  // read records
  for (unsigned long i = 0; i < length; i++)
    insert(CRecord::factory(f));

  return true;
}

bool CAdPlugDatabase::save(std::string db_name) {
  binofstream f(db_name);
  if (f.error())
    return false;
  return save(f);
}

bool CAdPlugDatabase::save(binostream &f) {
  unsigned long i;

  // Save database as little endian with IEEE floats
  f.setFlag(binio::BigEndian, false);
  f.setFlag(binio::FloatIEEE);

  f.writeString(DB_FILEID_V10);
  f.writeInt(linear_logic_length, 4);

  // write records
  for (i = 0; i < linear_length; i++)
    if (!m_dbLinear[i]->deleted)
      m_dbLinear[i]->record->write(f);

  return true;
}

CAdPlugDatabase::CRecord *CAdPlugDatabase::search(CKey const &key) {
  if (lookup(key))
    return get_record();
  else
    return 0;
}

bool CAdPlugDatabase::lookup(CKey const &key) {
  unsigned long index = make_hash(key);
  if (!m_dbHashed[index])
    return false;

  // immediate hit ?
  DB_Bucket *bucket = m_dbHashed[index];

  if (!bucket->deleted && bucket->record->m_key == key) {
    linear_index = bucket->index;
    return true;
  }

  // in-chain hit ?
  bucket = m_dbHashed[index]->chain;

  while (bucket) {
    if (!bucket->deleted && bucket->record->m_key == key) {
      linear_index = bucket->index;
      return true;
    }

    bucket = bucket->chain;
  }

  return false;
}

bool CAdPlugDatabase::insert(CRecord *record) {
  long index;

  // sanity checks
  if (!record)
    return false; // null-pointer given
  if (linear_length == hash_radix)
    return false; // max. db size exceeded
  if (lookup(record->m_key))
    return false; // record already in db

  // make bucket
  DB_Bucket *bucket = new DB_Bucket(linear_length, record);
  if (!bucket)
    return false;

  // add to linear list
  m_dbLinear[linear_length] = bucket;
  linear_logic_length++;
  linear_length++;

  // add to hashed list
  index = make_hash(record->m_key);

  if (!m_dbHashed[index]) // First entry in hashtable
    m_dbHashed[index] = bucket;
  else { // Add entry in chained list
    DB_Bucket *chain = m_dbHashed[index];

    while (chain->chain)
      chain = chain->chain;
    chain->chain = bucket;
  }

  return true;
}

void CAdPlugDatabase::wipe(CRecord *record) {
  if (!lookup(record->m_key))
    return;
  wipe();
}

void CAdPlugDatabase::wipe() {
  if (!linear_length)
    return;

  DB_Bucket *bucket = m_dbLinear[linear_index];

  if (!bucket->deleted) {
    delete bucket->record;
    linear_logic_length--;
    bucket->deleted = true;
  }
}

CAdPlugDatabase::CRecord *CAdPlugDatabase::get_record() {
  if (!linear_length)
    return 0;
  return m_dbLinear[linear_index]->record;
}

bool CAdPlugDatabase::go_forward() {
  if (linear_index + 1 < linear_length) {
    linear_index++;
    return true;
  } else
    return false;
}

bool CAdPlugDatabase::go_backward() {
  if (!linear_index)
    return false;
  linear_index--;
  return true;
}

void CAdPlugDatabase::goto_begin() {
  if (linear_length)
    linear_index = 0;
}

void CAdPlugDatabase::goto_end() {
  if (linear_length)
    linear_index = linear_length - 1;
}

inline unsigned long CAdPlugDatabase::make_hash(CKey const &key) {
  return (key.crc32 + key.crc16) % hash_radix;
}

/***** CAdPlugDatabase::DB_Bucket *****/

CAdPlugDatabase::DB_Bucket::DB_Bucket(unsigned long nindex, CRecord *newrecord,
                                      DB_Bucket *newchain)
    : index(nindex), deleted(false), chain(newchain), record(newrecord) {}

CAdPlugDatabase::DB_Bucket::~DB_Bucket() {
  if (!deleted)
    delete record;
}

/***** CAdPlugDatabase::CRecord *****/

CAdPlugDatabase::CRecord *CAdPlugDatabase::CRecord::factory(RecordType type) {
  switch (type) {
  case Plain:
    return new CPlainRecord;
  case SongInfo:
    return new CInfoRecord;
  case ClockSpeed:
    return new CClockRecord;
  default:
    return 0;
  }
}

CAdPlugDatabase::CRecord *CAdPlugDatabase::CRecord::factory(binistream &in) {
  RecordType type;
  unsigned long size;
  CRecord *rec;

  type = (RecordType) in.readInt(1);
  size = in.readInt(4);
  rec = factory(type);

  if (rec) {
    rec->m_key.crc16 = in.readInt(2);
    rec->m_key.crc32 = in.readInt(4);
    rec->m_filetype = in.readString('\0');
    rec->m_comment = in.readString('\0');
    rec->read_own(in);
    return rec;
  } else {
    // skip this record, cause we don't know about it
    in.seek(size, binio::Add);
    return 0;
  }
}

void CAdPlugDatabase::CRecord::write(binostream &out) {
  out.writeInt(m_type, 1);
  out.writeInt(get_size() + m_filetype.length() + m_comment.length() + 8, 4);
  out.writeInt(m_key.crc16, 2);
  out.writeInt(m_key.crc32, 4);
  out.writeString(m_filetype);
  out.writeInt('\0', 1);
  out.writeString(m_comment);
  out.writeInt('\0', 1);

  write_own(out);
}

bool CAdPlugDatabase::CRecord::user_read(std::istream &in, std::ostream &out) {
  return user_read_own(in, out);
}

bool CAdPlugDatabase::CRecord::user_write(std::ostream &out) {
  out << "Record type: ";
  switch (m_type) {
  case Plain:
    out << "Plain";
    break;
  case SongInfo:
    out << "SongInfo";
    break;
  case ClockSpeed:
    out << "ClockSpeed";
    break;
  default:
    out << "*** Unknown ***";
    break;
  }
  out << std::endl;
  out << "Key: " << std::hex << m_key.crc16 << ":" << m_key.crc32 << std::dec
      << std::endl;
  out << "File type: " << m_filetype << std::endl;
  out << "Comment: " << m_comment << std::endl;

  return user_write_own(out);
}

/***** CAdPlugDatabase::CRecord::CKey *****/

CAdPlugDatabase::CKey::CKey(binistream &buf) { make(buf); }

bool CAdPlugDatabase::CKey::operator==(const CKey &key) {
  return ((crc16 == key.crc16) && (crc32 == key.crc32));
}

void CAdPlugDatabase::CKey::make(binistream &buf)
    // Key is CRC16:CRC32 pair. CRC16 and CRC32 calculation routines (c) Zhengxi
    {
  static const unsigned short magic16 = 0xa001;
  static const unsigned long magic32 = 0xedb88320;

  crc16 = 0;
  crc32 = ~0;

  while (!buf.eof()) {
    unsigned char byte = buf.readInt(1);

    for (int j = 0; j < 8; j++) {
      if ((crc16 ^ byte) & 1)
        crc16 = (crc16 >> 1) ^ magic16;
      else
        crc16 >>= 1;

      if ((crc32 ^ byte) & 1)
        crc32 = (crc32 >> 1) ^ magic32;
      else
        crc32 >>= 1;

      byte >>= 1;
    }
  }

  crc16 &= 0xffff;
  crc32 = ~crc32;
}

/***** CInfoRecord *****/

CInfoRecord::CInfoRecord() { m_type = SongInfo; }

void CInfoRecord::read_own(binistream &in) {
  m_title = in.readString('\0');
  m_author = in.readString('\0');
}

void CInfoRecord::write_own(binostream &out) {
  out.writeString(m_title);
  out.writeInt('\0', 1);
  out.writeString(m_author);
  out.writeInt('\0', 1);
}

unsigned long CInfoRecord::get_size() {
  return m_title.length() + m_author.length() + 2;
}

bool CInfoRecord::user_read_own(std::istream &in, std::ostream &out) {
  out << "Title: ";
  in >> m_title;
  out << "Author: ";
  in >> m_author;
  return true;
}

bool CInfoRecord::user_write_own(std::ostream &out) {
  out << "Title: " << m_title << std::endl;
  out << "Author: " << m_author << std::endl;
  return true;
}

/***** CClockRecord *****/

CClockRecord::CClockRecord() : m_clock(0.0f) { m_type = ClockSpeed; }

void CClockRecord::read_own(binistream &in) {
  m_clock = in.readFloat(binio::Single);
}

void CClockRecord::write_own(binostream &out) {
  out.writeFloat(m_clock, binio::Single);
}

unsigned long CClockRecord::get_size() { return 4; }

bool CClockRecord::user_read_own(std::istream &in, std::ostream &out) {
  out << "Clockspeed: ";
  in >> m_clock;
  return true;
}

bool CClockRecord::user_write_own(std::ostream &out) {
  out << "Clock speed: " << m_clock << " Hz" << std::endl;
  return true;
}
