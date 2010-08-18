/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "modmodule.h"

#include <iostream>

using namespace std;
using namespace ppp;
using namespace ppp::mod;

/**
* @file
* @ingroup ModMod
* @brief Module definitions for ScreamTracker 3 Modules
*/

/**
 * @brief MOD Module Header
 * @ingroup ModMod
 */
typedef struct __attribute__(( packed ) ) {
	char title[20]; //!< @brief Module's title
	//SAMPLE INST[31]            ; instrument headers
	//unsigned char length; //!< @brief Song length
	//unsigned char patCount; //!< @brief Pattern count
	//unsigned char orders[128]; //!< @brief Order list
} modModuleHeader;

ModModule::ModModule( const unsigned int frq, const unsigned short maxRpt ) throw( PppException ) : GenModule( frq, maxRpt ),
		aBreakRow( -1 ), aBreakOrder( -1 ), aPatLoopRow( -1 ), aPatLoopCount( -1 ), aPatDelayCount( -1 ),
		aCustomData( false ) {
	try {
		for ( int i = 0; i < 256; i++ ) {
			aOrders.push_back( GenOrder::CP( new GenOrder( modOrderEnd ) ) );
			aPatterns.push_back( ModPattern::CP() );
			aSamples->push_back( ModSample::CP( static_cast<ModSample*>( 0 ) ) );
		}
		for ( int i = 0; i < 32; i++ ) {
			aChannels.push_back( ModChannel::CP( new ModChannel( aPlaybackFrequency, aSamples ) ) );
		}
	}
	PPP_CATCH_ALL();
}

ModModule::~ModModule() throw() {
}

bool ModModule::load( const string fn ) throw( PppException ) {
	try {
		LOG_BEGIN();
		aFileName = fn;
		modModuleHeader modHdr;
		FBinStream str( fn );
		LOG_MESSAGE( "Opening '" + fn + "'" );
		try {
			str.seek( 0 );
			str.read( modHdr );
		}
		catch ( ... ) {
			PPP_THROW( "Header could not be read" );
		}
		if ( str.fail() ) { // header read completely?
			LOG_WARNING( "Header Error" );
			return false;
		}
	}
	PPP_CATCH_ALL();
}

bool ModModule::existsSample( int idx ) throw() {
	idx--;
	if ( !inRange<int>( idx, 0, aSamples->size() - 1 ) )
		return false;
	return ( *aSamples )[idx];
}

string ModModule::getSampleName( int idx ) throw() {
	if ( !existsSample( idx ) )
		return "";
	return ( *aSamples )[idx-1]->getTitle();
}

bool ModModule::existsInstr( int idx ) const throw() {
	return false;
}

string ModModule::getInstrName( int idx ) const throw() {
	return "";
}

void ModModule::checkGlobalFx() throw( PppException ) {
	try {
	}
	PPP_RETHROW()
	catch ( ... ) {
		PPP_THROW( "Unknown Exception" );
	}
}

bool ModModule::adjustPosition( const bool increaseTick, const bool doStore ) throw( PppException ) {
	LOG_BEGIN();
	PPP_TEST( aOrders.size() == 0 );
	bool orderChanged = false;
	//! @todo Implement saving of the last order for back-jumping
//	short lastOrder = aPlaybackInfo.order;
	if ( increaseTick ) {
		aPlaybackInfo.tick++;
		PPP_TEST( aPlaybackInfo.speed == 0 );
		aPlaybackInfo.tick %= aPlaybackInfo.speed;
	}
	if (( aPlaybackInfo.tick == 0 ) && increaseTick ) {
		aPatDelayCount = -1;
		if ( aBreakOrder != -1 ) {
			mapOrder( aPlaybackInfo.order )->incCount();
			if ( aBreakOrder < static_cast<signed>( aOrders.size() ) ) {
				aPlaybackInfo.order = aBreakOrder;
				orderChanged = true;
			}
			aPlaybackInfo.row = 0;
		}
		if ( aBreakRow != -1 ) {
			if ( aBreakRow <= 63 ) {
				aPlaybackInfo.row = aBreakRow;
			}
			if ( aBreakOrder == -1 ) {
				if ( aPatLoopCount == -1 ) {
					mapOrder( aPlaybackInfo.order )->incCount();
					aPlaybackInfo.order++;
					orderChanged = true;
				}
			}
		}
		if (( aBreakRow == -1 ) && ( aBreakOrder == -1 ) && ( aPatDelayCount == -1 ) ) {
			aPlaybackInfo.row++;
			aPlaybackInfo.row %= 64;
			if ( aPlaybackInfo.row == 0 ) {
				mapOrder( aPlaybackInfo.order )->incCount();
				aPlaybackInfo.order++;
				orderChanged = true;
				aPlaybackInfo.order %= aOrders.size();
			}
		}
		aBreakRow = aBreakOrder = -1;
	}
	aPlaybackInfo.pattern = mapOrder( aPlaybackInfo.order )->getIndex();
	// skip "--" and "++" marks
	while ( aPlaybackInfo.pattern >= 254 ) {
		if ( aPlaybackInfo.pattern == modOrderEnd ) {
			LOG_MESSAGE( "aPlaybackInfo.pattern == modOrderEnd" );
			return false;
		}
		if ( !mapOrder( aPlaybackInfo.order ) )
			return false;
		mapOrder( aPlaybackInfo.order )->incCount();
		aPlaybackInfo.order++;
		orderChanged = true;
		if ( aPlaybackInfo.order >= static_cast<signed>( aOrders.size() ) ) {
			LOG_MESSAGE( "Song end reached: End of orders" );
			return false;
		}
		aPlaybackInfo.pattern = mapOrder( aPlaybackInfo.order )->getIndex();
	}
	if ( orderChanged ) {
		aPatLoopRow = 0;
		aPatLoopCount = -1;
		try {
			if ( doStore )
				saveState();
			else {
				PPP_TEST( !mapOrder( aPlaybackInfo.order ) );
				restoreState( aPlaybackInfo.order, mapOrder( aPlaybackInfo.order )->getCount() );
			}
		}
		PPP_CATCH_ALL()
	}
	return true;
}

void ModModule::getTick( AudioFifo::AudioBuffer &buf, unsigned int &bufLen ) throw( PppException ) {
	LOG_BEGIN();
	try {
	}
	catch ( ... ) {
		LOG_ERROR( "EXCEPTION" );
	}
}

void ModModule::getTickNoMixing( unsigned int& bufLen ) throw( PppException ) {
	try {
	}
	PPP_CATCH_ALL()
}

bool ModModule::jumpNextOrder() throw() {
	int currOrder = aPlaybackInfo.order;
	while ( currOrder == aPlaybackInfo.order ) {
		if ( !adjustPosition( true, false ) )
			return false;
	}
	return true;
}

GenOrder::CP ModModule::mapOrder( int order ) throw() {
	static GenOrder::CP xxx( new GenOrder( modOrderEnd ) );
	xxx->setCount( 0xff );
	if ( !inRange<int>( order, 0, aOrders.size() - 1 ) )
		return xxx;
	return aOrders[order];
}

GenPattern::CP ModModule::getPattern( int n ) throw() {
	if ( !inRange<int>( n, 0, aPatterns.size() - 1 ) )
		return GenPattern::CP();
	return aPatterns[n];
}

GenChannel::CP ModModule::getChannel( int n ) throw() {
	if ( !inRange( n, 0, aChannelCount - 1 ) )
		return GenChannel::CP();
}

GenSample::CP ModModule::getSmp( int n ) throw() {
	if ( !existsSample( n ) )
		return GenSample::CP();
	return ( *aSamples )[n];
}

string ModModule::getChanStatus( int idx ) throw() {
	GenChannel::CP x = getChannel( idx );
	if ( !x )
		return "";
	return x->getStatus();
}

bool ModModule::jumpNextTrack() throw( PppException ) {
	LOG_BEGIN();
	if ( !isMultiTrack() ) {
		LOG_MESSAGE( "This is not a multi-track" );
		return false;
	}
	PPP_TEST( !mapOrder( aPlaybackInfo.order ) );
	mapOrder( aPlaybackInfo.order )->incCount();
	aCurrentTrack++;
	if ( aCurrentTrack >= aTracks.size() ) {
		GenMultiTrack nulltrack;
		for ( unsigned int i = 0; i < aOrders.size(); i++ ) {
			PPP_TEST( !aOrders[i] );
			if (( aOrders[i]->getIndex() != modOrderEnd ) && ( aOrders[i]->getIndex() != modOrderSkip ) && ( aOrders[i]->getCount() == 0 ) ) {
				PPP_TEST( !mapOrder( i ) );
				aPlaybackInfo.pattern = mapOrder( i )->getIndex();
				aPlaybackInfo.order = i;
				nulltrack.startOrder = i;
				aTracks.push_back( nulltrack );
				aPlayedFrames = 0;
				saveState();
				return true;
			}
		}
		nulltrack.startOrder = GenMultiTrack::stopHere;
		aTracks.push_back( nulltrack );
		return false;
	}
	else {
		if ( aTracks[aCurrentTrack].startOrder == GenMultiTrack::stopHere ) {
			LOG_MESSAGE( "No more tracks" );
			return false;
		}
		aPlaybackInfo.order = aTracks[aCurrentTrack].startOrder;
		PPP_TEST( !mapOrder( aPlaybackInfo.order ) );
		aPlaybackInfo.pattern = mapOrder( aPlaybackInfo.order )->getIndex();
		aPlayedFrames = 0;
		restoreState( aPlaybackInfo.order, 0 );
		return true;
	}
	LOG_ERROR( "This should definitively NOT have happened..." );
	return false;
}

bool ModModule::jumpPrevTrack() throw( PppException ) {
	LOG_BEGIN();
	if ( !isMultiTrack() ) {
		LOG_MESSAGE( "This is not a multi-track" );
		return false;
	}
	if ( aCurrentTrack == 0 ) {
		LOG_MESSAGE( "Already on first track" );
		return false;
	}
	aCurrentTrack--;
	aPlaybackInfo.order = aTracks[aCurrentTrack].startOrder;
	PPP_TEST( !mapOrder( aPlaybackInfo.order ) );
	aPlaybackInfo.pattern = mapOrder( aPlaybackInfo.order )->getIndex();
	aPlayedFrames = 0;
	restoreState( aPlaybackInfo.order, 0 );
	return true;
}

BinStream &ModModule::saveState() throw( PppException ) {
	try {
		BinStream &str = GenModule::saveState();
		str.write( aBreakRow )
		.write( aBreakOrder )
		.write( aPatLoopRow )
		.write( aPatLoopCount )
		.write( aPatDelayCount )
		.write( aCustomData );
		return str;
	}
	PPP_CATCH_ALL();
}

BinStream &ModModule::restoreState( unsigned short ordindex, unsigned char cnt ) throw( PppException ) {
	try {
		BinStream &str = GenModule::restoreState( ordindex, cnt );
		str.read( aBreakRow )
		.read( aBreakOrder )
		.read( aPatLoopRow )
		.read( aPatLoopCount )
		.read( aPatDelayCount )
		.read( aCustomData );
		return str;
	}
	PPP_CATCH_ALL();
}
