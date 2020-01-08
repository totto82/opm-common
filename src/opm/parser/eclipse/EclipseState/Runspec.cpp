/*
  Copyright 2016  Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms
  of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  OPM is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ostream>
#include <type_traits>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/W.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>

namespace Opm {

Phase get_phase( const std::string& str ) {
    if( str == "OIL" ) return Phase::OIL;
    if( str == "GAS" ) return Phase::GAS;
    if( str == "WAT" ) return Phase::WATER;
    if( str == "WATER" )   return Phase::WATER;
    if( str == "SOLVENT" ) return Phase::SOLVENT;
    if( str == "POLYMER" ) return Phase::POLYMER;
    if( str == "ENERGY" ) return Phase::ENERGY;
    if( str == "POLYMW" ) return Phase::POLYMW;
    if( str == "FOAM" ) return Phase::FOAM;
    if( str == "SALTWATER" ) return Phase::SALTWATER;

    throw std::invalid_argument( "Unknown phase '" + str + "'" );
}

std::ostream& operator<<( std::ostream& stream, const Phase& p ) {
    switch( p ) {
        case Phase::OIL:     return stream << "OIL";
        case Phase::GAS:     return stream << "GAS";
        case Phase::WATER:   return stream << "WATER";
        case Phase::SOLVENT: return stream << "SOLVENT";
        case Phase::POLYMER: return stream << "POLYMER";
        case Phase::ENERGY:  return stream << "ENERGY";
        case Phase::POLYMW:  return stream << "POLYMW";
        case Phase::FOAM:    return stream << "FOAM";
        case Phase::SALTWATER:    return stream << "SALTWATER";

    }

    return stream;
}

using un = std::underlying_type< Phase >::type;

Phases::Phases( bool oil, bool gas, bool wat, bool sol, bool pol, bool energy, bool polymw, bool foam, bool saltwater) noexcept :
    bits( (oil ? (1 << static_cast< un >( Phase::OIL ) )     : 0) |
          (gas ? (1 << static_cast< un >( Phase::GAS ) )     : 0) |
          (wat ? (1 << static_cast< un >( Phase::WATER ) )   : 0) |
          (sol ? (1 << static_cast< un >( Phase::SOLVENT ) ) : 0) |
          (pol ? (1 << static_cast< un >( Phase::POLYMER ) ) : 0) |
          (energy ? (1 << static_cast< un >( Phase::ENERGY ) ) : 0) |
          (polymw ? (1 << static_cast< un >( Phase::POLYMW ) ) : 0) |
          (foam ? (1 << static_cast< un >( Phase::FOAM ) ) : 0) |
          (saltwater ? (1 << static_cast< un >( Phase::SALTWATER ) ) : 0) )

{}

Phases::Phases(const std::bitset<NUM_PHASES_IN_ENUM>& bset) :
    bits(bset)
{
}

unsigned long Phases::getBits() const {
    return bits.to_ulong();
}

bool Phases::active( Phase p ) const noexcept {
    return this->bits[ static_cast< int >( p ) ];
}

size_t Phases::size() const noexcept {
    return this->bits.count();
}

bool Phases::operator==(const Phases& data) const {
    return bits == data.bits;
}

Welldims::Welldims(const Deck& deck)
{
    if (deck.hasKeyword("WELLDIMS")) {
        const auto& wd = deck.getKeyword("WELLDIMS", 0).getRecord(0);

        this->nCWMax = wd.getItem("MAXCONN")      .get<int>(0);
        this->nWGMax = wd.getItem("MAX_GROUPSIZE").get<int>(0);

        // This is the E100 definition.  E300 instead uses
        //
        //   Max{ "MAXGROUPS", "MAXWELLS" }
        //
        // i.e., the maximum of item 1 and item 4 here.
        this->nGMax = wd.getItem("MAXGROUPS").get<int>(0);
	this->nWMax = wd.getItem("MAXWELLS").get<int>(0);
    }
}

WellSegmentDims::WellSegmentDims() :
    nSegWellMax( ParserKeywords::WSEGDIMS::NSWLMX::defaultValue ),
    nSegmentMax( ParserKeywords::WSEGDIMS::NSEGMX::defaultValue ),
    nLatBranchMax( ParserKeywords::WSEGDIMS::NLBRMX::defaultValue )
{}

WellSegmentDims::WellSegmentDims(const Deck& deck) : WellSegmentDims()
{
    if (deck.hasKeyword("WSEGDIMS")) {
        const auto& wsd = deck.getKeyword("WSEGDIMS", 0).getRecord(0);

        this->nSegWellMax   = wsd.getItem("NSWLMX").get<int>(0);
        this->nSegmentMax   = wsd.getItem("NSEGMX").get<int>(0);
        this->nLatBranchMax = wsd.getItem("NLBRMX").get<int>(0);
    }
}

WellSegmentDims::WellSegmentDims(int segWellMax, int segMax, int latBranchMax) :
    nSegWellMax(segWellMax),
    nSegmentMax(segMax),
    nLatBranchMax(latBranchMax)
{
}

bool WellSegmentDims::operator==(const WellSegmentDims& data) const
{
    return this->maxSegmentedWells() == data.maxSegmentedWells() &&
           this->maxSegmentsPerWell() == data.maxSegmentsPerWell() &&
           this->maxLateralBranchesPerWell() == data.maxLateralBranchesPerWell();
}

EclHysterConfig::EclHysterConfig(const Opm::Deck& deck)
    {

        if (!deck.hasKeyword("SATOPTS"))
            return;

        const auto& satoptsItem = deck.getKeyword("SATOPTS").getRecord(0).getItem(0);
        for (unsigned i = 0; i < satoptsItem.data_size(); ++i) {
            std::string satoptsValue = satoptsItem.get< std::string >(0);
            std::transform(satoptsValue.begin(),
                           satoptsValue.end(),
                           satoptsValue.begin(),
                           ::toupper);

            if (satoptsValue == "HYSTER")
                activeHyst = true;
        }

        // check for the (deprecated) HYST keyword
        if (deck.hasKeyword("HYST"))
            activeHyst = true;

        if (!activeHyst)
	      return;

        if (!deck.hasKeyword("EHYSTR"))
            throw std::runtime_error("Enabling hysteresis via the HYST parameter for SATOPTS requires the "
                                     "presence of the EHYSTR keyword");
	    /*!
	* \brief Set the type of the hysteresis model which is used for relative permeability.
	*
	* -1: relperm hysteresis is disabled
	* 0: use the Carlson model for relative permeability hysteresis of the non-wetting
	*    phase and the drainage curve for the relperm of the wetting phase
	* 1: use the Carlson model for relative permeability hysteresis of the non-wetting
	*    phase and the imbibition curve for the relperm of the wetting phase
	*/
        const auto& ehystrKeyword = deck.getKeyword("EHYSTR");
        if (deck.hasKeyword("NOHYKR"))
            krHystMod = -1;
        else {
            krHystMod = ehystrKeyword.getRecord(0).getItem("relative_perm_hyst").get<int>(0);

            if (krHystMod != 0 && krHystMod != 1)
                throw std::runtime_error(
                    "Only the Carlson relative permeability hysteresis models (indicated by '0' or "
                    "'1' for the second item of the 'EHYSTR' keyword) are supported");
        }

        // this is slightly screwed: it is possible to specify contradicting hysteresis
        // models with HYPC/NOHYPC and the fifth item of EHYSTR. Let's ignore that for
        // now.
            /*!
	* \brief Return the type of the hysteresis model which is used for capillary pressure.
	*
	* -1: capillary pressure hysteresis is disabled
	* 0: use the Killough model for capillary pressure hysteresis
	*/
        std::string whereFlag =
            ehystrKeyword.getRecord(0).getItem("limiting_hyst_flag").getTrimmedString(0);
        if (deck.hasKeyword("NOHYPC") || whereFlag == "KR")
            pcHystMod = -1;
        else {
            // if capillary pressure hysteresis is enabled, Eclipse always uses the
            // Killough model
            pcHystMod = 0;

            throw std::runtime_error("Capillary pressure hysteresis is not supported yet");
        }
    }

EclHysterConfig::EclHysterConfig(bool active, int pcMod, int krMod) :
    activeHyst(active), pcHystMod(pcMod), krHystMod(krMod)
    {
    }
    

bool EclHysterConfig::active() const 
    { return activeHyst; }
    
int EclHysterConfig::pcHysteresisModel() const
    { return pcHystMod; }
        
int EclHysterConfig::krHysteresisModel() const
    { return krHystMod; }

bool EclHysterConfig::operator==(const EclHysterConfig& data) const {
    return this->active() == data.active() &&
           this->pcHysteresisModel() == data.pcHysteresisModel() &&
           this->krHysteresisModel() == data.krHysteresisModel();
}

Runspec::Runspec( const Deck& deck ) :
    active_phases( Phases( deck.hasKeyword( "OIL" ),
                           deck.hasKeyword( "GAS" ),
                           deck.hasKeyword( "WATER" ),
                           deck.hasKeyword( "SOLVENT" ),
                           deck.hasKeyword( "POLYMER" ),
                           deck.hasKeyword( "THERMAL" ),
                           deck.hasKeyword( "POLYMW"  ),
                           deck.hasKeyword( "FOAM" ),
                           deck.hasKeyword( "BRINE" ) ) ),
    m_tabdims( deck ),
    endscale( deck ),
    welldims( deck ),
    wsegdims( deck ),
    udq_params( deck ),
    hystpar( deck ),
    m_actdims( deck )
{}

Runspec::Runspec(const Phases& act_phases,
                 const Tabdims& tabdims,
                 const EndpointScaling& endScale,
                 const Welldims& wellDims,
                 const WellSegmentDims& wsegDims,
                 const UDQParams& udqparams,
                 const EclHysterConfig& hystPar,
                 const Actdims& actDims) :
  active_phases(act_phases),
  m_tabdims(tabdims),
  endscale(endScale),
  welldims(wellDims),
  wsegdims(wsegDims),
  udq_params(udqparams),
  hystpar(hystPar),
  m_actdims(actDims)
{}

const Phases& Runspec::phases() const noexcept {
    return this->active_phases;
}

const Tabdims& Runspec::tabdims() const noexcept {
    return this->m_tabdims;
}

const Actdims& Runspec::actdims() const noexcept {
    return this->m_actdims;
}

const EndpointScaling& Runspec::endpointScaling() const noexcept {
    return this->endscale;
}

const Welldims& Runspec::wellDimensions() const noexcept
{
    return this->welldims;
}

const WellSegmentDims& Runspec::wellSegmentDimensions() const noexcept
{
    return this->wsegdims;
}

const EclHysterConfig& Runspec::hysterPar() const noexcept
{
    return this->hystpar;
}

/*
  Returns an integer in the range 0...7 which can be used to indicate
  available phases in Eclipse restart and init files.
*/
int Runspec::eclPhaseMask( ) const noexcept {
    const int water = 1 << 2;
    const int oil   = 1 << 0;
    const int gas   = 1 << 1;

    return ( active_phases.active( Phase::WATER ) ? water : 0 )
         | ( active_phases.active( Phase::OIL ) ? oil : 0 )
         | ( active_phases.active( Phase::GAS ) ? gas : 0 );
}


const UDQParams& Runspec::udqParams() const noexcept {
    return this->udq_params;
}


bool Runspec::operator==(const Runspec& data) const {
    return this->phases() == data.phases() &&
           this->tabdims() == data.tabdims() &&
           this->endpointScaling() == data.endpointScaling() &&
           this->wellDimensions() == data.wellDimensions() &&
           this->wellSegmentDimensions() == data.wellSegmentDimensions() &&
           this->hysterPar() == data.hysterPar() &&
           this->actdims() == data.actdims();
}

}
