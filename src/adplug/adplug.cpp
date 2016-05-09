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

#include "adplug.h"

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

#include "light4cxx/logger.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace
{
light4cxx::Logger* logger = light4cxx::Logger::get("badplay.factory");
}

// List of all players that come with the standard AdPlug distribution
const PlayerDesc AdPlug::allplayers[] = {
    PlayerDesc(ChscPlayer::factory, "HSC-Tracker", {".hsc"}),
    PlayerDesc(CsngPlayer::factory, "SNGPlay", {".sng"}),
    PlayerDesc(CimfPlayer::factory, "Apogee IMF", {".imf", ".wlf", ".adlib"}),
    PlayerDesc(Ca2mLoader::factory, "Adlib Tracker 2", {".a2m"}),
    PlayerDesc(CadtrackLoader::factory, "Adlib Tracker", {".sng"}),
    PlayerDesc(CamdLoader::factory, "AMUSIC", {".amd"}),
    PlayerDesc(CbamPlayer::factory, "Bob's Adlib Music", {".bam"}),
    PlayerDesc(CcmfPlayer::factory, "Creative Music File", {".cmf"}),
    PlayerDesc(Cd00Player::factory, "Packed EdLib", {".d00"}),
    PlayerDesc(CdfmLoader::factory, "Digital-FM", {".dfm"}),
    PlayerDesc(ChspLoader::factory, "HSC Packed", {".hsp"}),
    PlayerDesc(CksmPlayer::factory, "Ken Silverman Music", {".ksm"}),
    PlayerDesc(CmadLoader::factory, "Mlat Adlib Tracker", {".mad"}),
    PlayerDesc(CDukePlayer::factory, "Duke", {".mid"}),
    PlayerDesc(CmidPlayer::factory, "MIDI", {".mid", ".sci", ".laa"}),
    PlayerDesc(CmkjPlayer::factory, "MKJamz", {".mkj"}),
    PlayerDesc(CcffLoader::factory, "Boomtracker", {".cff"}),
    PlayerDesc(CdmoLoader::factory, "TwinTeam", {".dmo"}),
    PlayerDesc(Cs3mPlayer::factory, "Scream Tracker 3", {".s3m"}),
    PlayerDesc(CdtmLoader::factory, "DeFy Adlib Tracker", {".dtm"}),
    PlayerDesc(CfmcLoader::factory, "Faust Music Creator", {".sng"}),
    PlayerDesc(CmtkLoader::factory, "MPU-401 Trakker", {".mtk"}),
    PlayerDesc(CradLoader::factory, "Reality Adlib Tracker", {".rad"}),
    PlayerDesc(CrawPlayer::factory, "RdosPlay RAW", {".raw"}),
    PlayerDesc(Csa2Loader::factory, "Surprise! Adlib Tracker", {".sat", ".sa2"}),
    PlayerDesc(CxadbmfPlayer::factory, "BMF Adlib Tracker", {".xad"}),
    PlayerDesc(CxadflashPlayer::factory, "Flash", {".xad"}),
    PlayerDesc(CxadhybridPlayer::factory, "Hybrid", {".xad"}),
    PlayerDesc(CxadhypPlayer::factory, "Hypnosis", {".xad"}),
    PlayerDesc(CxadpsiPlayer::factory, "PSI", {".xad"}),
    PlayerDesc(CxadratPlayer::factory, "rat", {".xad"}),
    PlayerDesc(CldsPlayer::factory, "LOUDNESS Sound System", {".lds"}),
    PlayerDesc(Cu6mPlayer::factory, "Ultima 6 Music", {".m"}),
    PlayerDesc(CrolPlayer::factory, "Adlib Visual Composer", {".rol"}),
    PlayerDesc(CxsmPlayer::factory, "eXtra Simple Music", {".xsm"}),
    PlayerDesc(CdroPlayer::factory, "DOSBox Raw OPL v0.1", {".dro"}),
    PlayerDesc(Cdro2Player::factory, "DOSBox Raw OPL v2.0", {".dro"}),
    PlayerDesc(CmscPlayer::factory, "Adlib MSC Player", {".msc"}),
    PlayerDesc(CrixPlayer::factory, "Softstar RIX OPL Music", {".rix"}),
    PlayerDesc(CadlPlayer::factory, "Westwood ADL", {".adl"}),
    PlayerDesc(CjbmPlayer::factory, "JBM Adlib Music", {".jbm"}),
    PlayerDesc()
};

const Players& AdPlug::init_players(const PlayerDesc pd[])
{
    static Players initplayers;

    for(size_t i = 0; pd[i].factory; i++)
        initplayers.addPlayerDescription(&pd[i]);

    return initplayers;
}

const Players AdPlug::s_players = AdPlug::init_players(AdPlug::allplayers);

std::shared_ptr<Player> AdPlug::factory(const std::string& fn, const Players& pl)
{
    logger->info(L4CXX_LOCATION, "Trying to load %s", fn);

    // Try a direct hit by file extension
    const auto ext = boost::to_lower_copy(boost::filesystem::path(fn).extension().string());
    for(auto i = pl.begin(); i != pl.end(); ++i)
    {
        for(auto j = 0; !(*i)->get_extension(j).empty(); j++)
        {
            if(ext == boost::to_lower_copy((*i)->get_extension(j)))
            {
                logger->debug(L4CXX_LOCATION, "Trying direct hit: %s\n", (*i)->filetype);
                std::shared_ptr<Player> p{ (*i)->factory() };
                if(p && p->load(fn))
                {
                    return p;
                }
            }
        }
    }

    // Try all players, one by one
    for(auto i = pl.begin(); i != pl.end(); ++i)
    {
        logger->debug(L4CXX_LOCATION, "Trying: %s\n", (*i)->filetype);
        std::shared_ptr<Player> p{ (*i)->factory() };
        if(p && p->load(fn))
        {
            return p;
        }
    }

    // Unknown file
    return nullptr;
}

std::string AdPlug::get_version()
{
    return "0.1";
}