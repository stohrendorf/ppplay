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
    PlayerDesc(HscPlayer::factory, "HSC-Tracker", {".hsc"}),
    PlayerDesc(SngPlayer::factory, "SNGPlay", {".sng"}),
    PlayerDesc(ImfPlayer::factory, "Apogee IMF", {".imf", ".wlf", ".adlib"}),
    PlayerDesc(A2mPlayer::factory, "Adlib Tracker 2", {".a2m"}),
    PlayerDesc(AdTrackPlayer::factory, "Adlib Tracker", {".sng"}),
    PlayerDesc(AmdPlayer::factory, "AMUSIC", {".amd"}),
    PlayerDesc(BamPlayer::factory, "Bob's Adlib Music", {".bam"}),
    PlayerDesc(CmfPlayer::factory, "Creative Music File", {".cmf"}),
    PlayerDesc(D00Player::factory, "Packed EdLib", {".d00"}),
    PlayerDesc(DfmPlayer::factory, "Digital-FM", {".dfm"}),
    PlayerDesc(HspPlayer::factory, "HSC Packed", {".hsp"}),
    PlayerDesc(KsmPlayer::factory, "Ken Silverman Music", {".ksm"}),
    PlayerDesc(MadPlayer::factory, "Mlat Adlib Tracker", {".mad"}),
    PlayerDesc(CDukePlayer::factory, "Duke", {".mid"}),
    PlayerDesc(MidPlayer::factory, "MIDI", {".mid", ".sci", ".laa"}),
    PlayerDesc(MkjPlayer::factory, "MKJamz", {".mkj"}),
    PlayerDesc(CffPlayer::factory, "Boomtracker", {".cff"}),
    PlayerDesc(DmoPlayer::factory, "TwinTeam", {".dmo"}),
    PlayerDesc(S3mPlayer::factory, "Scream Tracker 3", {".s3m"}),
    PlayerDesc(DtmPlayer::factory, "DeFy Adlib Tracker", {".dtm"}),
    PlayerDesc(FmcPlayer::factory, "Faust Music Creator", {".sng"}),
    PlayerDesc(MtkPlayer::factory, "MPU-401 Trakker", {".mtk"}),
    PlayerDesc(RadPlayer::factory, "Reality Adlib Tracker", {".rad"}),
    PlayerDesc(RawPlayer::factory, "RdosPlay RAW", {".raw"}),
    PlayerDesc(Sa2Player::factory, "Surprise! Adlib Tracker", {".sat", ".sa2"}),
    PlayerDesc(BmfPlayer::factory, "BMF Adlib Tracker", {".xad"}),
    PlayerDesc(FlashPlayer::factory, "Flash", {".xad"}),
    PlayerDesc(HybridPlayer::factory, "Hybrid", {".xad"}),
    PlayerDesc(HypPlayer::factory, "Hypnosis", {".xad"}),
    PlayerDesc(PsiPlayer::factory, "PSI", {".xad"}),
    PlayerDesc(RatPlayer::factory, "rat", {".xad"}),
    PlayerDesc(LdsPlayer::factory, "LOUDNESS Sound System", {".lds"}),
    PlayerDesc(U6mPlayer::factory, "Ultima 6 Music", {".m"}),
    PlayerDesc(RolPlayer::factory, "Adlib Visual Composer", {".rol"}),
    PlayerDesc(XsmPlayer::factory, "eXtra Simple Music", {".xsm"}),
    PlayerDesc(DroPlayer::factory, "DOSBox Raw OPL v0.1", {".dro"}),
    PlayerDesc(Dro2Player::factory, "DOSBox Raw OPL v2.0", {".dro"}),
    PlayerDesc(MscPlayer::factory, "Adlib MSC Player", {".msc"}),
    PlayerDesc(RixPlayer::factory, "Softstar RIX OPL Music", {".rix"}),
    PlayerDesc(AdlPlayer::factory, "Westwood ADL", {".adl"}),
    PlayerDesc(JbmPlayer::factory, "JBM Adlib Music", {".jbm"}),
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