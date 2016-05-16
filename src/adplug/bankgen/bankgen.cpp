#include <cstdio>

#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>

#include "stream/filestream.h"

#include "loader.h"

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::cout << "Missing output filename.\n";
        return EXIT_FAILURE;
    }

    BankDatabaseGen db;
    db.loadMiles("opl_files/sc3.opl",
                 "AIL (Star Control 3, Albion, Empire 2, Sensible Soccer, Settlers 2, many others)",
                 "G"); // Our "standard" bank! Same as file22.opl

    db.loadBisqwit("op3_files/bisqwit.adlraw",
                   "Bisqwit (selection of 4op and 2op)",
                   "Bisq");

    db.loadBnk("bnk_files/melodic.bnk", "HMI (Descent, Asterix)", "HMIGM"); // same as file156.bnk
    db.loadBnk("bnk_files/drum.bnk", "HMI (Descent, Asterix)", "HMIGP");
    db.loadBnk("bnk_files/intmelo.bnk", "HMI (Descent:: Int)", "intM");
    db.loadBnk("bnk_files/intdrum.bnk", "HMI (Descent:: Int)", "intP");
    db.loadBnk("bnk_files/hammelo.bnk", "HMI (Descent:: Ham)", "hamM");
    db.loadBnk("bnk_files/hamdrum.bnk", "HMI (Descent:: Ham)", "hamP");
    db.loadBnk("bnk_files/rickmelo.bnk", "HMI (Descent:: Rick)", "rickM");
    db.loadBnk("bnk_files/rickdrum.bnk", "HMI (Descent:: Rick)", "rickP");

    db.loadBnk("bnk_files/d2melo.bnk", "HMI (Descent 2)", "b6M");
    db.loadBnk("bnk_files/d2drum.bnk", "HMI (Descent 2)", "b6P");
    db.loadBnk("bnk_files/normmelo.bnk", "HMI (Normality)", "b7M");
    db.loadBnk("bnk_files/normdrum.bnk", "HMI (Normality)", "b7P"); // same as file122.bnk
    db.loadBnk("bnk_files/ssmelo.bnk", "HMI (Shattered Steel)", "b8M");
    db.loadBnk("bnk_files/ssdrum.bnk", "HMI (Shattered Steel)", "b8P");

    db.loadBnk("bnk_files/file131.bnk", "HMI (Theme Park)", "b9M");
    db.loadBnk("bnk_files/file132.bnk", "HMI (Theme Park)", "b9P");
    db.loadBnk("bnk_files/file133.bnk", "HMI (3d Table Sports, Battle Arena Toshinden)", "b10P");
    db.loadBnk("bnk_files/file134.bnk", "HMI (3d Table Sports, Battle Arena Toshinden)", "b10M");
    db.loadBnk("bnk_files/file142.bnk", "HMI (Aces of the Deep)", "b11P");
    db.loadBnk("bnk_files/file143.bnk", "HMI (Aces of the Deep)", "b11M");
    db.loadBnk("bnk_files/file144.bnk", "HMI (Earthsiege)", "b12M");
    db.loadBnk("bnk_files/file145.bnk", "HMI (Earthsiege)", "b12P");
    db.loadBnk("bnk_files/file167.bnk", "HMI (Anvil of Dawn)", "b13P");
    db.loadBnk("bnk_files/file168.bnk", "HMI (Anvil of Dawn)", "b13M");

    db.loadDoom("doom2/genmidi.op2", "DMX (Doom :: partially pseudo 4op)", "dM");
    db.loadDoom("doom2/genmidi.htc", "DMX (Hexen, Heretic :: partially pseudo 4op)", "hxM"); // same as genmidi.hxn
    db.loadDoom("doom2/default.op2", "DMX (MUS Play :: partially pseudo 4op)", "mus");

    db.loadMiles("opl_files/file17.opl", "AIL (Discworld, Grandest Fleet, Pocahontas, Slob Zone 3d, Ultima 4, Zorro)", "f17G");
    db.loadMiles("opl_files/warcraft.ad", "AIL (Warcraft 2)", "sG"); // same as file44, warcraft.opl
    db.loadMiles("opl_files/file19.opl", "AIL (Syndicate)", "f19G");
    db.loadMiles("opl_files/file20.opl", "AIL (Guilty, Orion Conspiracy, Terra Nova Strike Force Centauri :: 4op)", "f20G");
    db.loadMiles("opl_files/file21.opl", "AIL (Magic Carpet 2)", "f21G");
    db.loadMiles("opl_files/nemesis.opl", "AIL (Nemesis)", "nem");
    db.loadMiles("opl_files/file23.opl", "AIL (Jagged Alliance)", "f23G");
    db.loadMiles("opl_files/file24.opl", "AIL (When Two Worlds War :: 4op, MISSING INSTRUMENTS)", "f24G");
    db.loadMiles("opl_files/file25.opl", "AIL (Bards Tale Construction :: MISSING INSTRUMENTS)", "f25G");
    db.loadMiles("opl_files/file26.opl", "AIL (Return to Zork)", "f26G");
    db.loadMiles("opl_files/file27.opl", "AIL (Theme Hospital)", "f27G");
    db.loadMiles("opl_files/nhlpa.opl", "AIL (National Hockey League PA)", "nhl");
    db.loadMiles("opl_files/file29.opl", "AIL (Inherit The Earth)", "f29G");
    db.loadMiles("opl_files/file30.opl", "AIL (Inherit The Earth, file two)", "f30G");
    db.loadMiles("opl_files/file31.opl", "AIL (Little Big Adventure :: 4op)", "f31G");
    db.loadMiles("opl_files/file32.opl", "AIL (Wreckin Crew)", "f32G");
    db.loadMiles("opl_files/file13.opl", "AIL (Death Gate)", "f13G");
    db.loadMiles("opl_files/file34.opl", "AIL (FIFA International Soccer)", "f34G");
    db.loadMiles("opl_files/file35.opl", "AIL (Starship Invasion)", "f35G");
    db.loadMiles("opl_files/file36.opl", "AIL (Super Street Fighter 2 :: 4op)", "f36G");
    db.loadMiles("opl_files/file37.opl", "AIL (Lords of the Realm :: MISSING INSTRUMENTS)", "f37G");
    db.loadMiles("opl_files/simfarm.opl", "AIL (SimFarm, SimHealth :: 4op)", "qG");
    db.loadMiles("opl_files/simfarm.ad", "AIL (SimFarm, Settlers, Serf City)", "mG"); // same as file18.opl
    db.loadMiles("opl_files/file12.opl", "AIL (Caesar 2 :: partially 4op, MISSING INSTRUMENTS)", "f12G");
    db.loadMiles("opl_files/file41.opl", "AIL (Syndicate Wars)", "f41G");
    db.loadMiles("opl_files/file42.opl", "AIL (Bubble Bobble Feat. Rainbow Islands, Z)", "f42G");
    db.loadMiles("opl_files/file47.opl", "AIL (Warcraft)", "f47G");
    db.loadMiles("opl_files/file48.opl", "AIL (Terra Nova Strike Force Centuri :: partially 4op)", "f48G");
    db.loadMiles("opl_files/file49.opl", "AIL (System Shock :: partially 4op)", "f49G");
    db.loadMiles("opl_files/file50.opl", "AIL (Advanced Civilization)", "f50G");
    db.loadMiles("opl_files/file53.opl", "AIL (Battle Chess 4000 :: partially 4op, melodic only)", "f53G");
    db.loadMiles("opl_files/file54.opl", "AIL (Ultimate Soccer Manager :: partially 4op)", "f54G");

    db.loadMiles("opl_files/sample.ad", "AIL (Air Bucks, Blue And The Gray, America Invades, Terminator 2029)", "MG"); // same as file51.opl
    db.loadMiles("opl_files/sample.opl", "AIL (Ultima Underworld 2)", "oG"); // same as file40.opl
    db.loadMiles("opl_files/file15.opl", "AIL (Kasparov's Gambit)", "f15G");
    db.loadMiles("opl_files/file16.opl", "AIL (High Seas Trader :: MISSING INSTRUMENTS)", "f16G");

    db.loadBnk2("bnk_files/file159.bnk", "AIL (Master of Magic, Master of Orion 2 :: 4op, std percussion)", "b50", "gm", "gps"); // fat-opl3
    db.loadBnk2("bnk_files/file159.bnk", "AIL (Master of Magic, Master of Orion 2 :: 4op, orchestral percussion)", "b51", "gm", "gpo");

    db.loadIBK("ibk_files/soccer-genmidi.ibk", "SB (Action Soccer)", "b55M", false);
    db.loadIBK("ibk_files/soccer-percs.ibk", "SB (Action Soccer)", "b55P", true);
    db.loadIBK("ibk_files/game.ibk", "SB (3d Cyberpuck :: melodic only)", "b56", false);
    db.loadIBK("ibk_files/mt_fm.ibk", "SB (Simon the Sorcerer :: melodic only)", "b57", false);

    db.loadJunglevision("op3_files/fat2.op3", "OP3 (The Fat Man 2op set)", "fat2");
    db.loadJunglevision("op3_files/fat4.op3", "OP3 (The Fat Man 4op set)", "fat4");
    db.loadJunglevision("op3_files/jv_2op.op3", "OP3 (JungleVision 2op set :: melodic only)", "b60");
    db.loadJunglevision("op3_files/wallace.op3", "OP3 (Wallace 2op set, Nitemare 3D :: melodic only)", "b61");

    db.loadTMB("tmb_files/d3dtimbr.tmb", "TMB (Duke Nukem 3D)", "duke");
    db.loadTMB("tmb_files/swtimbr.tmb", "TMB (Shadow Warrior)", "sw");

    db.loadDoom("raptor/genmidi.op2", "DMX (Raptor)", "rapt");

    try
    {
        // db.dump();
        db.save(argv[1]);
    }
    catch(std::runtime_error& ex)
    {
        std::cout << "Failed to create " << argv[1] << ": " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
