/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2008 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * adplug.cpp - CAdPlug utility class, by Simon Peter <dn.tlp@gmx.net>
 */

#include <cstring>
#include <string>
#include <libbinio/binfile.h>

#include "adplug.h"
#include "debug.h"

/***** Replayer includes *****/

#include "hsc.h"
#include "amd.h"
#include "a2m.h"
#include "imf.h"
#include "sng.h"
#include "adtrack.h"
#include "bam.h"
#include "cmf.h"
#include "d00.h"
#include "dfm.h"
#include "hsp.h"
#include "ksm.h"
#include "mad.h"
#include "mid.h"
#include "mkj.h"
#include "cff.h"
#include "dmo.h"
#include "s3m.h"
#include "dtm.h"
#include "fmc.h"
#include "mtk.h"
#include "rad.h"
#include "raw.h"
#include "sa2.h"
#include "bmf.h"
#include "flash.h"
#include "hybrid.h"
#include "hyp.h"
#include "psi.h"
#include "rat.h"
#include "lds.h"
#include "u6m.h"
#include "rol.h"
#include "xsm.h"
#include "dro.h"
#include "dro2.h"
#include "msc.h"
#include "rix.h"
#include "adl.h"
#include "jbm.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

/***** CAdPlug *****/

// List of all players that come with the standard AdPlug distribution
const CPlayerDesc CAdPlug::allplayers[] = {
    CPlayerDesc(ChscPlayer::factory, "HSC-Tracker", {".hsc"}),
    CPlayerDesc(CsngPlayer::factory, "SNGPlay", {".sng"}),
    CPlayerDesc(CimfPlayer::factory, "Apogee IMF", {".imf", ".wlf", ".adlib"}),
    CPlayerDesc(Ca2mLoader::factory, "Adlib Tracker 2", {".a2m"}),
    CPlayerDesc(CadtrackLoader::factory, "Adlib Tracker", {".sng"}),
    CPlayerDesc(CamdLoader::factory, "AMUSIC", {".amd"}),
    CPlayerDesc(CbamPlayer::factory, "Bob's Adlib Music", {".bam"}),
    CPlayerDesc(CcmfPlayer::factory, "Creative Music File", {".cmf"}),
    CPlayerDesc(Cd00Player::factory, "Packed EdLib", {".d00"}),
    CPlayerDesc(CdfmLoader::factory, "Digital-FM", {".dfm"}),
    CPlayerDesc(ChspLoader::factory, "HSC Packed", {".hsp"}),
    CPlayerDesc(CksmPlayer::factory, "Ken Silverman Music", {".ksm"}),
    CPlayerDesc(CmadLoader::factory, "Mlat Adlib Tracker", {".mad"}),
    CPlayerDesc(CDukePlayer::factory, "Duke", {".mid"}),
    CPlayerDesc(CmidPlayer::factory, "MIDI", {".mid", ".sci", ".laa"}),
    CPlayerDesc(CmkjPlayer::factory, "MKJamz", {".mkj"}),
    CPlayerDesc(CcffLoader::factory, "Boomtracker", {".cff"}),
    CPlayerDesc(CdmoLoader::factory, "TwinTeam", {".dmo"}),
    CPlayerDesc(Cs3mPlayer::factory, "Scream Tracker 3", {".s3m"}),
    CPlayerDesc(CdtmLoader::factory, "DeFy Adlib Tracker", {".dtm"}),
    CPlayerDesc(CfmcLoader::factory, "Faust Music Creator", {".sng"}),
    CPlayerDesc(CmtkLoader::factory, "MPU-401 Trakker", {".mtk"}),
    CPlayerDesc(CradLoader::factory, "Reality Adlib Tracker", {".rad"}),
    CPlayerDesc(CrawPlayer::factory, "RdosPlay RAW", {".raw"}),
    CPlayerDesc(Csa2Loader::factory, "Surprise! Adlib Tracker", {".sat", ".sa2"}),
    CPlayerDesc(CxadbmfPlayer::factory, "BMF Adlib Tracker", {".xad"}),
    CPlayerDesc(CxadflashPlayer::factory, "Flash", {".xad"}),
    CPlayerDesc(CxadhybridPlayer::factory, "Hybrid", {".xad"}),
    CPlayerDesc(CxadhypPlayer::factory, "Hypnosis", {".xad"}),
    CPlayerDesc(CxadpsiPlayer::factory, "PSI", {".xad"}),
    CPlayerDesc(CxadratPlayer::factory, "rat", {".xad"}),
    CPlayerDesc(CldsPlayer::factory, "LOUDNESS Sound System", {".lds"}),
    CPlayerDesc(Cu6mPlayer::factory, "Ultima 6 Music", {".m"}),
    CPlayerDesc(CrolPlayer::factory, "Adlib Visual Composer", {".rol"}),
    CPlayerDesc(CxsmPlayer::factory, "eXtra Simple Music", {".xsm"}),
    CPlayerDesc(CdroPlayer::factory, "DOSBox Raw OPL v0.1", {".dro"}),
    CPlayerDesc(Cdro2Player::factory, "DOSBox Raw OPL v2.0", {".dro"}),
    CPlayerDesc(CmscPlayer::factory, "Adlib MSC Player", {".msc"}),
    CPlayerDesc(CrixPlayer::factory, "Softstar RIX OPL Music", {".rix"}),
    CPlayerDesc(CadlPlayer::factory, "Westwood ADL", {".adl"}),
    CPlayerDesc(CjbmPlayer::factory, "JBM Adlib Music", {".jbm"}),
    CPlayerDesc()
};

const CPlayers &CAdPlug::init_players(const CPlayerDesc pd[]) {
    static CPlayers initplayers;

    for (size_t i = 0; pd[i].factory; i++)
        initplayers.addPlayerDescription(&pd[i]);

    return initplayers;
}

const CPlayers CAdPlug::s_players = CAdPlug::init_players(CAdPlug::allplayers);

std::shared_ptr<CPlayer> CAdPlug::factory(const std::string &fn, const CPlayers &pl) {
    AdPlug_LogWrite("*** CAdPlug::factory(\"%s\",opl,fp) ***\n", fn.c_str());

    // Try a direct hit by file extension
    const auto ext = boost::to_lower_copy(boost::filesystem::path(fn).extension().string());
    for (auto i = pl.begin(); i != pl.end(); i++) {
        for (auto j = 0; !(*i)->get_extension(j).empty(); j++) {
            if(ext == boost::to_lower_copy((*i)->get_extension(j))) {
                AdPlug_LogWrite("Trying direct hit: %s\n", (*i)->filetype.c_str());
                std::shared_ptr<CPlayer> p{ (*i)->factory() };
                if (p && p->load(fn)) {
                    AdPlug_LogWrite("got it!\n");
                    AdPlug_LogWrite("--- CAdPlug::factory ---\n");
                    return p;
                }
            }
        }
    }

    // Try all players, one by one
    for (auto i = pl.begin(); i != pl.end(); i++) {
        AdPlug_LogWrite("Trying: %s\n", (*i)->filetype.c_str());
        std::shared_ptr<CPlayer> p { (*i)->factory() };
        if (p && p->load(fn)) {
            AdPlug_LogWrite("got it!\n");
            AdPlug_LogWrite("--- CAdPlug::factory ---\n");
            return p;
        }
    }

    // Unknown file
    AdPlug_LogWrite("End of list!\n");
    AdPlug_LogWrite("--- CAdPlug::factory ---\n");
    return 0;
}

std::string CAdPlug::get_version() { return "0.1"; }

void CAdPlug::debug_output(const std::string &filename) {
    AdPlug_LogFile(filename.c_str());
    AdPlug_LogWrite("CAdPlug::debug_output(\"%s\"): Redirected.\n", filename.c_str());
}
