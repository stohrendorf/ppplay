#include "bankdatabase.h"

#include "ymf262/opl3.h"

#include <boost/archive/xml_iarchive.hpp>

#include <boost/serialization/map.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/optional.hpp>

#include <cstring>
#include <fstream>

bool bankdb::SlotSettings::operator==(const bankdb::SlotSettings& b) const
{
    if( !std::equal(data.begin(), data.end(), b.data.begin()) )
    {
        return false;
    }
    return finetune == b.finetune;
}

bool bankdb::SlotSettings::operator!=(const bankdb::SlotSettings& b) const
{
    return !(*this == b);
}

bool bankdb::SlotSettings::operator<(const bankdb::SlotSettings& b) const
{
    if( int c = std::memcmp(data.data(), b.data.data(), 11) )
    {
        return c < 0;
    }
    return finetune < b.finetune;
}

void bankdb::SlotSettings::apply(opl::SlotView& slot) const
{
#if 1
    slot.setC0(data[10]);

    slot.modulator().set20(data[0]);
    slot.modulator().set60(data[2]);
    slot.modulator().set80(data[4]);
    slot.modulator().setE0(data[6]);
    slot.modulator().set40(data[8]);

    slot.carrier().set20(data[1]);
    slot.carrier().set60(data[3]);
    slot.carrier().set80(data[5]);
    slot.carrier().setE0(data[7]);
    slot.carrier().set40(data[9]);
#else
    slot.setFeedback((data[10] >> 1) & 7);
    slot.setCnt(data[10] & 0x01);
    slot.setOutput(data[10] & 0x80, data[10] & 0x40, data[10] & 0x20, data[10] & 0x10);

    slot.modulator().setAttackRate(data[3] >> 4);
    slot.modulator().setDecayRate(data[3] & 0x0f);
    slot.modulator().setSustainLevel(data[5] >> 4);
    slot.modulator().setReleaseRate(data[5] & 0x0f);
    slot.modulator().setAm(data[1] & 0x80);
    slot.modulator().setVib(data[1] & 0x40);
    slot.modulator().setEgt(data[1] & 0x20);
    slot.modulator().setKsr(data[1] & 0x10);
    slot.modulator().setMult(data[1] & 0x0f);
    slot.modulator().setWave(data[7]);
    slot.modulator().setKsl(data[9] >> 6);
    slot.modulator().setTotalLevel(data[9] & 0x3f);

    slot.carrier().setAttackRate(data[2] >> 4);
    slot.carrier().setDecayRate(data[2] & 0x0f);
    slot.carrier().setSustainLevel(data[4] >> 4);
    slot.carrier().setReleaseRate(data[4] & 0x0f);
    slot.carrier().setAm(data[0] & 0x80);
    slot.carrier().setVib(data[0] & 0x40);
    slot.carrier().setEgt(data[0] & 0x20);
    slot.carrier().setKsr(data[0] & 0x10);
    slot.carrier().setMult(data[0] & 0x0f);
    slot.carrier().setWave(data[6]);
    slot.carrier().setKsl(data[8] >> 6);
    slot.carrier().setTotalLevel(data[8] & 0x3f);
#endif
}

bool bankdb::Instrument::operator==(const bankdb::Instrument& b) const
{
    if( first != b.first || *first != *b.first )
    {
        return false;
    }
    if( second != b.second || *second != *b.second )
    {
        return false;
    }
    if( noteOverride != b.noteOverride )
    {
        return false;
    }
    return pseudo4op == b.pseudo4op;
}

bool bankdb::Instrument::operator<(const bankdb::Instrument& b) const
{
    if( first != b.first )
    {
        return SlotSettings::ptrLess(first, b.first);
    }
    if( second != b.second )
    {
        return SlotSettings::ptrLess(second, b.second);
    }
    if( noteOverride != b.noteOverride )
    {
        return noteOverride < b.noteOverride;
    }
    return pseudo4op < b.pseudo4op;
}

void bankdb::BankDatabase::load(const std::string& filename)
{
    std::ifstream ifs(filename);
    boost::archive::xml_iarchive ia(ifs);
    ia >> boost::serialization::make_nvp("database", *this);

    //std::cout << "Banks:\n";
    //for(const auto& x : m_banks)
    //    std::cout << x.first << std::endl;
}
