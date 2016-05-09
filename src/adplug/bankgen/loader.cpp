#include "loader.h"

#include "stream/filestream.h"
#include "stuff/stringutils.h"

#include <boost/archive/xml_oarchive.hpp>

#include <boost/serialization/map.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/range/adaptors.hpp>

namespace
{
boost::property_tree::ptree toPtree(const bankdb::SlotSettings& ss)
{
    boost::property_tree::ptree entry;

    entry.put("carrier.waveform", int(ss.data[6]));
    entry.put("carrier.sustainLevel", int(ss.data[4] >> 4));
    entry.put("carrier.releaseRate", int(ss.data[4] & 0x0f));
    entry.put("carrier.attackRate", int(ss.data[2] >> 4));
    entry.put("carrier.decayRate", int(ss.data[2] & 0x0f));
    entry.put("carrier.tremolo", (ss.data[0] & 0x80) != 0);
    entry.put("carrier.vibrato", (ss.data[0] & 0x40) != 0);
    entry.put("carrier.noSustain", (ss.data[0] & 0x20) != 0);
    entry.put("carrier.keyScaleRate", (ss.data[0] & 0x10) != 0);
    entry.put("carrier.multiplier", int(ss.data[0] & 0x0f));
    entry.put("carrier.keyScaleLevel", int(ss.data[8] >> 6));
    entry.put("carrier.totalLevel", int(ss.data[8] & 0x3f));

    entry.put("modulator.waveform", int(ss.data[7]));
    entry.put("modulator.sustainLevel", int(ss.data[5] >> 4));
    entry.put("modulator.releaseRate", int(ss.data[5] & 0x0f));
    entry.put("modulator.attackRate", int(ss.data[3] >> 4));
    entry.put("modulator.decayRate", int(ss.data[3] & 0x0f));
    entry.put("modulator.tremolo", (ss.data[1] & 0x80) != 0);
    entry.put("modulator.vibrato", (ss.data[1] & 0x40) != 0);
    entry.put("modulator.noSustain", (ss.data[1] & 0x20) != 0);
    entry.put("modulator.keyScaleRate", (ss.data[1] & 0x10) != 0);
    entry.put("modulator.multiplier", int(ss.data[1] & 0x0f));
    entry.put("modulator.keyScaleLevel", int(ss.data[9] >> 6));
    entry.put("modulator.totalLevel", int(ss.data[9] & 0x3f));

    entry.put("additive", (ss.data[10] & 0x01) != 0);
    entry.put("feedback", int((ss.data[10] >> 1) & 7));
    entry.put("connection.a", (ss.data[10] & 0x80) != 0);
    entry.put("connection.b", (ss.data[10] & 0x40) != 0);
    entry.put("connection.c", (ss.data[10] & 0x20) != 0);
    entry.put("connection.d", (ss.data[10] & 0x10) != 0);

    return entry;
}

boost::property_tree::ptree toPtree(const bankdb::Instrument& ins)
{
    boost::property_tree::ptree entry;

    entry.add_child("first", toPtree(*ins.first));

    if(ins.second)
        entry.add_child("second", toPtree(*ins.second));

    if(ins.noteOverride)
        entry.put("noteOverride", static_cast<int>(*ins.noteOverride));

    entry.put("pseudo4op", ins.pseudo4op);
    entry.put("localName", ins.localName);
    entry.put("generatedName", ins.generatedName);

    return entry;
}

const std::string MidiInsName[] = {
    "AcouGrandPiano",
    "BrightAcouGrand",
    "ElecGrandPiano",
    "Honky-tonkPiano",
    "Rhodes Piano",
    "Chorused Piano",
    "Harpsichord",
    "Clavinet",
    "Celesta",
    "Glockenspiel",
    "Music box",
    "Vibraphone",
    "Marimba",
    "Xylophone",
    "Tubular Bells",
    "Dulcimer",
    "Hammond Organ",
    "Percussive Organ",
    "Rock Organ",
    "Church Organ",
    "Reed Organ",
    "Accordion",
    "Harmonica",
    "Tango Accordion",
    "Acoustic Guitar1",
    "Acoustic Guitar2",
    "Electric Guitar1",
    "Electric Guitar2",
    "Electric Guitar3",
    "Overdrive Guitar",
    "Distorton Guitar",
    "Guitar Harmonics",
    "Acoustic Bass",
    "Electric Bass 1",
    "Electric Bass 2",
    "Fretless Bass",
    "Slap Bass 1",
    "Slap Bass 2",
    "Synth Bass 1",
    "Synth Bass 2",
    "Violin",
    "Viola",
    "Cello",
    "Contrabass",
    "Tremulo Strings",
    "Pizzicato String",
    "Orchestral Harp",
    "Timpany",
    "String Ensemble1",
    "String Ensemble2",
    "Synth Strings 1",
    "SynthStrings 2",
    "Choir Aahs",
    "Voice Oohs",
    "Synth Voice",
    "Orchestra Hit",
    "Trumpet",
    "Trombone",
    "Tuba",
    "Muted Trumpet",
    "French Horn",
    "Brass Section",
    "Synth Brass 1",
    "Synth Brass 2",
    "Soprano Sax",
    "Alto Sax",
    "Tenor Sax",
    "Baritone Sax",
    "Oboe",
    "English Horn",
    "Bassoon",
    "Clarinet",
    "Piccolo",
    "Flute",
    "Recorder",
    "Pan Flute",
    "Bottle Blow",
    "Shakuhachi",
    "Whistle",
    "Ocarina",
    "Lead 1 squareea",
    "Lead 2 sawtooth",
    "Lead 3 calliope",
    "Lead 4 chiff",
    "Lead 5 charang",
    "Lead 6 voice",
    "Lead 7 fifths",
    "Lead 8 brass",
    "Pad 1 new age",
    "Pad 2 warm",
    "Pad 3 polysynth",
    "Pad 4 choir",
    "Pad 5 bowedpad",
    "Pad 6 metallic",
    "Pad 7 halo",
    "Pad 8 sweep",
    "FX 1 rain",
    "FX 2 soundtrack",
    "FX 3 crystal",
    "FX 4 atmosphere",
    "FX 5 brightness",
    "FX 6 goblins",
    "FX 7 echoes",
    "FX 8 sci-fi",
    "Sitar",
    "Banjo",
    "Shamisen",
    "Koto",
    "Kalimba",
    "Bagpipe",
    "Fiddle",
    "Shanai",
    "Tinkle Bell",
    "Agogo Bells",
    "Steel Drums",
    "Woodblock",
    "Taiko Drum",
    "Melodic Tom",
    "Synth Drum",
    "Reverse Cymbal",
    "Guitar FretNoise",
    "Breath Noise",
    "Seashore",
    "Bird Tweet",
    "Telephone",
    "Helicopter",
    "Applause/Noise",
    "Gunshot",
    // 27..34:  High Q; Slap; Scratch Push; Scratch Pull; Sticks;
    //          Square Click; Metronome Click; Metronome Bell
    "Ac Bass Drum",
    "Bass Drum 1",
    "Side Stick",
    "Acoustic Snare",
    "Hand Clap",
    "Electric Snare",
    "Low Floor Tom",
    "Closed High Hat",
    "High Floor Tom",
    "Pedal High Hat",
    "Low Tom",
    "Open High Hat",
    "Low-Mid Tom",
    "High-Mid Tom",
    "Crash Cymbal 1",
    "High Tom",
    "Ride Cymbal 1",
    "Chinese Cymbal",
    "Ride Bell",
    "Tambourine",
    "Splash Cymbal",
    "Cow Bell",
    "Crash Cymbal 2",
    "Vibraslap",
    "Ride Cymbal 2",
    "High Bongo",
    "Low Bongo",
    "Mute High Conga",
    "Open High Conga",
    "Low Conga",
    "High Timbale",
    "Low Timbale",
    "High Agogo",
    "Low Agogo",
    "Cabasa",
    "Maracas",
    "Short Whistle",
    "Long Whistle",
    "Short Guiro",
    "Long Guiro",
    "Claves",
    "High Wood Block",
    "Low Wood Block",
    "Mute Cuica",
    "Open Cuica",
    "Mute Triangle",
    "Open Triangle",
    "Shaker","Jingle Bell","Bell Tree","Castanets","Mute Surdu","Open Surdu","" };

#pragma pack(push,1)
struct Doom_OPL2instrument
{
    uint8_t trem_vibr_1;    /* OP 1: tremolo/vibrato/sustain/KSR/multi */
    uint8_t att_dec_1;      /* OP 1: attack rate/decay rate */
    uint8_t sust_rel_1;     /* OP 1: sustain level/release rate */
    uint8_t wave_1;         /* OP 1: waveform select */
    uint8_t scale_1;        /* OP 1: key scale level */
    uint8_t level_1;        /* OP 1: output level */
    uint8_t feedback;       /* feedback/AM-FM (both operators) */
    uint8_t trem_vibr_2;    /* OP 2: tremolo/vibrato/sustain/KSR/multi */
    uint8_t att_dec_2;      /* OP 2: attack rate/decay rate */
    uint8_t sust_rel_2;     /* OP 2: sustain level/release rate */
    uint8_t wave_2;         /* OP 2: waveform select */
    uint8_t scale_2;        /* OP 2: key scale level */
    uint8_t level_2;        /* OP 2: output level */
    uint8_t unused;
    int16_t basenote;       /* base note offset */
};

struct Doom_opl_instr
{
    static constexpr uint16_t FL_FIXED_PITCH = 0x0001;          // note has fixed pitch (drum note)
    static constexpr uint16_t FL_UNKNOWN = 0x0002;          // ??? (used in instrument #65 only)
    static constexpr uint16_t FL_DOUBLE_VOICE = 0x0004;          // use two voices instead of one

    uint16_t flags;

    uint8_t         finetune;
    uint8_t         note;
    Doom_OPL2instrument patchdata[2];
};
#pragma pack(pop)
} // anonymous namespace

void BankDatabaseGen::loadBnk(const char* filename, const char* bankname, const char* prefix)
{
    registerBank(prefix, bankname);

    FileStream fp(filename);
    std::vector<uint8_t> data(fp.size());
    fp.read(data.data(), data.size());

    // unsigned short version = *(short*)&data[0]; // major,minor (2 bytes)
    //                                             "ADLIB-"    (6 bytes)
    uint16_t entries_used = *reinterpret_cast<uint16_t*>(&data[8]);   // entries used
    // unsigned short total_entries = *(short*)&data[10]; // total entries
    uint32_t name_offset = *reinterpret_cast<uint32_t*>(&data[12]);// name_offset
    uint32_t data_offset = *reinterpret_cast<uint32_t*>(&data[16]);// data_offset
    // 16..23: 8 byte sof filler

    for(unsigned n = 0; n < entries_used; ++n)
    {
        const size_t offset1 = name_offset + n * 12;

        unsigned short data_index = data[offset1 + 0] + data[offset1 + 1] * 256;
        unsigned char usage_flag = data[offset1 + 2];
        std::string name = stringncpy(reinterpret_cast<const char*>(&data[offset1 + 3]), 9);

        const size_t offset2 = data_offset + data_index * 30;
        // const unsigned char mode      = data[offset2+0];
        //const unsigned char voice_num = data[offset2+1];
        const uint8_t* op1 = &data[offset2 + 2];  // 13 bytes
        const uint8_t* op2 = &data[offset2 + 15];
        const uint8_t waveform_mod = data[offset2 + 28];
        const uint8_t waveform_car = data[offset2 + 29];

        bool percussive = (usage_flag >= 16);

        int gmno = (n + percussive * 128);

        bankdb::Instrument& instrument = addInstrument(prefix, gmno);
        instrument.first = std::make_shared<bankdb::SlotSettings>();

        instrument.first->data[0] = (op1[9] << 7) // TREMOLO FLAG
            + (op1[10] << 6) // VIBRATO FLAG
            + (op1[5] << 5) // SUSTAIN FLAG
            + (op1[11] << 4) // SCALING FLAG
            + op1[1];      // FREQMUL
        instrument.first->data[1] = (op2[9] << 7)
            + (op2[10] << 6)
            + (op2[5] << 5)
            + (op2[11] << 4)
            + op2[1];
        instrument.first->data[2] = op1[3] * 0x10 + op1[6]; // ATTACK, DECAY
        instrument.first->data[3] = op2[3] * 0x10 + op2[6];
        instrument.first->data[4] = op1[4] * 0x10 + op1[7]; // SUSTAIN, RELEASE
        instrument.first->data[5] = op2[4] * 0x10 + op2[7];
        instrument.first->data[6] = waveform_mod;
        instrument.first->data[7] = waveform_car;
        instrument.first->data[8] = op1[0] * 0x40 + op1[8]; // KSL , LEVEL
        instrument.first->data[9] = op2[0] * 0x40 + op2[8]; // KSL , LEVEL
        instrument.first->data[10] = op1[2] * 2 + op1[12]; // FEEDBACK, ADDITIVEFLAG
        instrument.first->finetune = 0;
        // Note: op2[2] and op2[12] are unused and contain garbage.
        if(percussive && usage_flag != 0)
            instrument.noteOverride = usage_flag;
        instrument.pseudo4op = false;

        instrument.localName = name;
        instrument.generatedName = stringFmt("%s%u", prefix, n);
    }
}

void BankDatabaseGen::loadBnk2(const char* fn, const char* bankname, const char* prefix, const std::string& melo_filter, const std::string& perc_filter)
{
    registerBank(prefix, bankname);

    FileStream fp(fn);
    std::vector<uint8_t> data(fp.size());
    fp.read(data.data(), data.size());

    uint16_t ins_entries = *reinterpret_cast<uint16_t*>(&data[28 + 2 + 10]);
    uint8_t* records = &data[48];

    for(unsigned n = 0; n < ins_entries; ++n)
    {
        const size_t offset1 = n * 28;
        int used = records[offset1 + 15];
        // int attrib   = *(int*)&records[offset1 + 16];
        int32_t offset2 = *reinterpret_cast<int32_t*>(&records[offset1 + 20]);
        if(used == 0)
            continue;

        std::string name = stringncpy(reinterpret_cast<const char*>(&records[offset1 + 3]), 12);

        int gmno = 0;
        if(name.substr(0, melo_filter.size()) == melo_filter)
            gmno = std::stoi(name.substr(melo_filter.size()));
        else if(name.substr(0, perc_filter.size()) == perc_filter)
            gmno = std::stoi(name.substr(perc_filter.size())) + 128;
        else
            continue;

        const unsigned char* insdata = &data[offset2];
        const unsigned char* ops[4] = { insdata + 0, insdata + 6, insdata + 12, insdata + 18 };
        unsigned char C4xxxFFFC = insdata[24];
        unsigned char xxP24NNN = insdata[25];
        unsigned char TTTTTTTT = insdata[26];
        // unsigned char xxxxxxxx = insdata[27];

        bankdb::SlotSettings slotSettings[2];
        for(unsigned a = 0; a < 2; ++a)
        {
            slotSettings[a].data[0] = ops[a * 2 + 0][0];
            slotSettings[a].data[1] = ops[a * 2 + 1][0];
            slotSettings[a].data[2] = ops[a * 2 + 0][2];
            slotSettings[a].data[3] = ops[a * 2 + 1][2];
            slotSettings[a].data[4] = ops[a * 2 + 0][3];
            slotSettings[a].data[5] = ops[a * 2 + 1][3];
            slotSettings[a].data[6] = ops[a * 2 + 0][4] & 0x07;
            slotSettings[a].data[7] = ops[a * 2 + 1][4] & 0x07;
            slotSettings[a].data[8] = ops[a * 2 + 0][1];
            slotSettings[a].data[9] = ops[a * 2 + 1][1];
            slotSettings[a].finetune = TTTTTTTT;
        }
        slotSettings[0].data[10] = C4xxxFFFC & 0x0F;
        slotSettings[1].data[10] = (slotSettings[0].data[10] & 0x0E) | (C4xxxFFFC >> 7);

        bankdb::Instrument& instrument = addInstrument(prefix, gmno);
        if(gmno >= 128)
            instrument.noteOverride = 35;
        instrument.pseudo4op = false;

        instrument.first = std::make_shared<bankdb::SlotSettings>(slotSettings[0]);
        instrument.localName = name;
        instrument.generatedName = stringFmt("%s%c%u", prefix, (gmno & 128) ? 'P' : 'M', int(gmno & 127));

        if(xxP24NNN & 8)
        {
            // dual-op
            instrument.second = std::make_shared<bankdb::SlotSettings>(slotSettings[1]);
        }
    }
}

void BankDatabaseGen::loadDoom(const char* fn, const char* bankname, const char* prefix)
{
    registerBank(prefix, bankname);

    FileStream fp(fn);
    std::vector<uint8_t> data(fp.size());
    fp.read(data.data(), data.size());

    for(int a = 0; a < 175; ++a)
    {
        const size_t offset1 = 0x18A4 + a * 32;
        const size_t offset2 = 8 + a * 36;

        std::string name = stringncpy(reinterpret_cast<const char*>(&data[offset1]), 32);

        int gmno = a < 128 ? a : ((a | 128) + 35);

        const Doom_opl_instr& ins = *reinterpret_cast<const Doom_opl_instr*>(&data[offset2]);

        bankdb::SlotSettings slotSettings[2];
        for(unsigned index = 0; index < 2; ++index)
        {
            const Doom_OPL2instrument& src = ins.patchdata[index];
            slotSettings[index].data[0] = src.trem_vibr_1;
            slotSettings[index].data[1] = src.trem_vibr_2;
            slotSettings[index].data[2] = src.att_dec_1;
            slotSettings[index].data[3] = src.att_dec_2;
            slotSettings[index].data[4] = src.sust_rel_1;
            slotSettings[index].data[5] = src.sust_rel_2;
            slotSettings[index].data[6] = src.wave_1;
            slotSettings[index].data[7] = src.wave_2;
            slotSettings[index].data[8] = src.scale_1 | src.level_1;
            slotSettings[index].data[9] = src.scale_2 | src.level_2;
            slotSettings[index].data[10] = src.feedback;
            slotSettings[index].finetune = src.basenote + 12;
        }
        bankdb::Instrument& instrument = addInstrument(prefix, gmno);
        if(gmno >= 128)
            instrument.noteOverride = ins.note;
        instrument.pseudo4op = false;
        while(instrument.noteOverride && *instrument.noteOverride < 20)
        {
            *instrument.noteOverride += 12;
            slotSettings[0].finetune -= 12;
            slotSettings[1].finetune -= 12;
        }
        instrument.first = std::make_shared<bankdb::SlotSettings>(slotSettings[0]);
        instrument.localName = name;
        instrument.generatedName = stringFmt("%s%c%u", prefix, (gmno < 128 ? 'M' : 'P'), int(gmno & 127));

        if((ins.flags & 4) != 0) // Double instrument
        {
            instrument.pseudo4op = true;
            instrument.second = std::make_shared<bankdb::SlotSettings>(slotSettings[1]);
        }
    }
}

void BankDatabaseGen::loadMiles(const char* fn, const char* bankname, const char* prefix)
{
    registerBank(prefix, bankname);

    FileStream fp(fn);
    std::vector<uint8_t> data(fp.size());
    fp.read(data.data(), data.size());

    for(unsigned a = 0; a < 2000; ++a)
    {
        unsigned gmnumber = data[a * 6 + 0];
        if(gmnumber == 0xFF)
            break;

        unsigned gmnumber2 = data[a * 6 + 1];
        if(gmnumber2 != 0 && gmnumber2 != 0x7F)
            continue;

        int gmno = gmnumber2 == 0x7F ? gmnumber + 0x80 : gmnumber;
        int midi_index = gmno < 128 ? gmno
            : gmno < 128 + 35 ? -1
            : gmno < 128 + 88 ? gmno - 35
            : -1;

        uint32_t offset = *reinterpret_cast<uint32_t*>(&data[a * 6 + 2]);

        unsigned length = data[offset] + data[offset + 1] * 256;
        signed char notenum = data[offset + 2];

        bankdb::SlotSettings slotSettings[200];

        const unsigned inscount = (length - 3) / 11;
        for(unsigned i = 0; i < inscount; ++i)
        {
            unsigned o = offset + 3 + i * 11;
            slotSettings[i].finetune = (gmno < 128 && i == 0) ? notenum : 0;
            const uint8_t mapping[10] = { 0,8,2,4,6,1,9,3,5,7 };
            for(int m = 0; m < 10; ++m)
                slotSettings[i].data[mapping[m]] = data[o + m];

            unsigned fb_c = data[offset + 3 + 5];
            slotSettings[i].data[10] = fb_c;
            if(i == 1)
            {
                slotSettings[0].data[10] = fb_c & 0x0F;
                slotSettings[1].data[10] = (fb_c & 0x0E) | (fb_c >> 7);
            }
        }
        if(inscount > 2)
            continue;

        bankdb::Instrument& instrument = addInstrument(prefix, gmno);
        if(gmno >= 128)
            instrument.noteOverride = data[offset + 3] != 0 ? data[offset + 3] : 35;
        instrument.pseudo4op = false;
        if(midi_index >= 0)
            instrument.localName = MidiInsName[midi_index];

        instrument.first = std::make_shared<bankdb::SlotSettings>(slotSettings[0]);

        if(inscount > 1)
            instrument.second = std::make_shared<bankdb::SlotSettings>(slotSettings[1]);

        instrument.generatedName = stringFmt("%s%c%u", prefix, (gmno < 128 ? 'M' : 'P'), int(gmno & 127));
    }
}

void BankDatabaseGen::loadIBK(const char* fn, const char* bankname, const char* prefix, bool percussive)
{
    registerBank(prefix, bankname);

    FileStream fp(fn);
    std::vector<uint8_t> data(fp.size());
    fp.read(data.data(), data.size());

    unsigned offs1_base = 0x804, offs1_len = 9;
    unsigned offs2_base = 0x004, offs2_len = 16;

    for(unsigned a = 0; a < 128; ++a)
    {
        unsigned offset1 = offs1_base + a*offs1_len;
        unsigned offset2 = offs2_base + a*offs2_len;

        std::string name;
        for(unsigned p = 0; p < 9; ++p)
            if(data[offset1 + p] >= 0x20 && data[offset1 + p] <= 0x7e)
                name += char(data[offset1 + p]);

        int gmno = a + 128 * percussive;
        // int midi_index = gmno < 128 ? gmno
        //               : gmno < 128+35 ? -1
        //               : gmno < 128+88 ? gmno-35
        //               : -1;

        bankdb::SlotSettings slotSettings;
        // [+11] seems to be used also, what is it for?
        const uint8_t mapping[11] = { 0,1,8,9,2,3,4,5,6,7,10 };
        for(int m = 0; m < 11; ++m)
            slotSettings.data[mapping[m]] = data[offset2 + m];
        slotSettings.finetune = 0;
        bankdb::Instrument& instrument = addInstrument(prefix, gmno);
        if(gmno >= 128)
            instrument.noteOverride = 35;
        instrument.pseudo4op = false;
        instrument.first = std::make_shared<bankdb::SlotSettings>(slotSettings);
        instrument.localName = name;
        instrument.generatedName = stringFmt("%s%c%u", prefix, (gmno < 128 ? 'M' : 'P'), int(gmno & 127));
    }
}

void BankDatabaseGen::loadJunglevision(const char* fn, const char* bankname, const char* prefix)
{
    registerBank(prefix, bankname);

    FileStream fp(fn);
    std::vector<uint8_t> data(fp.size());
    fp.read(data.data(), data.size());

    unsigned ins_count = data[0x20] + (data[0x21] << 8);
    unsigned drum_count = data[0x22] + (data[0x23] << 8);
    unsigned first_ins = data[0x24] + (data[0x25] << 8);
    unsigned first_drum = data[0x26] + (data[0x27] << 8);

    unsigned total_ins = ins_count + drum_count;

    for(unsigned a = 0; a < total_ins; ++a)
    {
        unsigned offset = 0x28 + a * 0x18;
        unsigned gmno = (a < ins_count) ? (a + first_ins) : (a + first_drum);
        int midi_index = gmno < 128 ? gmno
            : gmno < 128 + 35 ? -1
            : gmno < 128 + 88 ? gmno - 35
            : -1;

        bankdb::SlotSettings slotSettings[2];

        const uint8_t mapping[10] = { 2,8,4,10,5,11,6,12,3,9 };
        for(int m = 0; m < 10; ++m)
        {
            slotSettings[0].data[m] = data[offset + mapping[m]];
            slotSettings[1].data[m] = data[offset + mapping[m] + 11];
        }

        slotSettings[0].data[10] = data[offset + 7] & ~0x30;
        slotSettings[0].finetune = 0;

        slotSettings[1].data[10] = data[offset + 7 + 11] & ~0x30;
        slotSettings[1].finetune = 0;

        bankdb::Instrument& instrument = addInstrument(prefix, gmno);
        instrument.noteOverride = data[offset + 1];
        instrument.pseudo4op = false;

        while(instrument.noteOverride && *instrument.noteOverride < 20)
        {
            *instrument.noteOverride += 12;
            slotSettings[0].finetune -= 12;
            slotSettings[1].finetune -= 12;
        }

        if(midi_index >= 0)
            instrument.localName = MidiInsName[midi_index];

        instrument.generatedName = stringFmt("%s%c%u", prefix, (gmno < 128 ? 'M' : 'P'), int(gmno & 127));

        instrument.first = std::make_shared<bankdb::SlotSettings>(slotSettings[0]);
        if(data[offset] != 0)
        {
            instrument.second = std::make_shared<bankdb::SlotSettings>(slotSettings[1]);
        }
    }
}

void BankDatabaseGen::loadTMB(const char* fn, const char* bankname, const char* prefix)
{
    registerBank(prefix, bankname);

    FileStream fp(fn);
    std::vector<uint8_t> data(fp.size(), 0);
    fp.read(data.data(), data.size());

    for(unsigned a = 0; a < 256; ++a)
    {
        unsigned offset = a * 0x0D;
        unsigned gmno = a;
        int midi_index = gmno < 128 ? gmno
            : gmno < 128 + 35 ? -1
            : gmno < 128 + 88 ? gmno - 35
            : -1;

        bankdb::Instrument& instrument = addInstrument(prefix, gmno);
        instrument.first = std::make_shared<bankdb::SlotSettings>();
        const uint8_t mapping[11] = { 0,1,4,5,6,7,8,9,2,3,10 };
        for(int m = 0; m < 11; ++m)
            instrument.first->data[m] = data[offset + mapping[m]];
        instrument.first->finetune = 0; //data[offset + 12];

        if(gmno >= 128 && data[offset + 11] != 0)
            instrument.noteOverride = data[offset + 11];
        instrument.pseudo4op = false;

        if(midi_index >= 0)
            instrument.localName = MidiInsName[midi_index];

        instrument.generatedName = stringFmt("%s%c%u", prefix, (gmno < 128 ? 'M' : 'P'), int(gmno & 127));
    }
}

void BankDatabaseGen::loadBisqwit(const char* fn, const char* bankname, const char* prefix)
{
    registerBank(prefix, bankname);

    FileStream fp(fn);
    for(unsigned a = 0; a < 256; ++a)
    {
        // unsigned offset = a * 25;
        unsigned gmno = a;
        int midi_index = gmno < 128 ? gmno
            : gmno < 128 + 35 ? -1
            : gmno < 128 + 88 ? gmno - 35
            : -1;

        bankdb::Instrument& instrument = addInstrument(prefix, gmno);
        instrument.noteOverride = 0;
        fp >> *instrument.noteOverride;
        instrument.pseudo4op = false;

        instrument.first = std::make_shared<bankdb::SlotSettings>();
        fp >> instrument.first->finetune;
        fp.read(instrument.first->data.data(), 11);
        instrument.second = std::make_shared<bankdb::SlotSettings>();
        fp >> instrument.second->finetune;
        fp.read(instrument.second->data.data(), 11);

        if(midi_index >= 0)
            instrument.localName = MidiInsName[midi_index];

        instrument.generatedName = stringFmt("%s%c%u", prefix, (gmno < 128 ? 'M' : 'P'), int(gmno & 127));
    }
}

void BankDatabaseGen::dump()
{
    boost::property_tree::ptree jsonBanks;

    for(const auto& bank : banks())
    {
        boost::property_tree::ptree jsonBank;

        int instrId = 0;
        for(const auto& instrument : bank.second.instruments)
        {
            boost::property_tree::ptree jsonInstr = toPtree(instrument.second);
            jsonInstr.put("index", instrument.first);
            jsonBank.add_child(boost::lexical_cast<std::string>(instrId++), jsonInstr);
        }

        jsonBanks.add_child(bank.first, jsonBank);
    }

    boost::property_tree::ptree json;
    json.add_child("banks", jsonBanks);

    boost::property_tree::write_json(std::cout, json);
    std::cout << std::endl;
}

namespace
{
struct PtrCompare
{
    bool operator()(const bankdb::SlotSettings::Ptr& a, const bankdb::SlotSettings::Ptr& b) const
    {
        return bankdb::SlotSettings::ptrLess(a, b);
    }
};
}

void BankDatabaseGen::save(const std::string& filename)
{
    // Make slot settings unique (and verify the instruments if enabled)
    {
        std::cout << "Compressing... ";
        std::set<bankdb::SlotSettings::Ptr, PtrCompare> slots;
        size_t processed = 0;
        for(auto& bank : banks())
        {
            for(bankdb::Instrument& instrument : bank.second.instruments | boost::adaptors::map_values)
            {
                instrument.first = *slots.insert(instrument.first).first;
                ++processed;
                if(instrument.second)
                {
                    instrument.second = *slots.insert(instrument.second).first;
                    ++processed;
                    if(!instrument.pseudo4op)
                        bank.second.uses4op = true;
                }
            }

            bank.second.onlyPercussion = true;

            for(size_t i = 0; i < 128; ++i)
            {
                auto it = bank.second.instruments.find(i);
                if(it != bank.second.instruments.end())
                {
                    bank.second.onlyPercussion = false;
                    break;
                }
            }

#ifdef VERIFY_INSTRUMENTS
            if(bank.second.onlyPercussion)
                continue;

            for(size_t i = 0; i < 128; ++i)
            {
                auto it = bank.second.instruments.find(i);
                if(it == bank.second.instruments.end())
                {
                    throw std::runtime_error(stringFmt("Missing instrument #%d in bank %s", i, bank.first));
                }
                if(!it->second.first)
                {
                    throw std::runtime_error(stringFmt("Missing first slot settings in instrument #%d in bank %s", i, bank.first));
                }
            }
#endif
        }
        std::cout << "done " << processed << "=>" << slots.size() << "\n";
    }

    std::ofstream ofs(filename);
    boost::archive::xml_oarchive oa(ofs);
    oa << boost::serialization::make_nvp("database", *this);
}