//*****************************************************************************
//
//                         aevol - Artificial Evolution
//
// Copyright (C) 2004  LIRIS.
// Web: https://liris.cnrs.fr/
// E-mail: carole.knibbe@liris.cnrs.fr
// Original Authors : Guillaume Beslon, Carole Knibbe, Virginie Lefort
//                    David Parsons
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//*****************************************************************************


/** \class
 *  \brief
 */

// =================================================================
//                              Libraries
// =================================================================

// =================================================================
//                            Project Files
// =================================================================
#include <ae_individual_R_X11.h>

//##############################################################################
//                                                                             #
//                           Class ae_individual_R_X11                         #
//                                                                             #
//##############################################################################

// =================================================================
//                    Definition of static attributes
// =================================================================

// =================================================================
//                             Constructors
// =================================================================
ae_individual_R_X11::ae_individual_R_X11( const ae_individual_R_X11 &model ) :
  ae_individual( model ), ae_individual_R( model ), ae_individual_X11( model )
{
  //printf("ae_individual_R_X11( model )");
}

ae_individual_R_X11::ae_individual_R_X11( void )  :
ae_individual(), ae_individual_R(), ae_individual_X11()
{
  //printf("ae_individual_R_X11( void )");
}

ae_individual_R_X11::ae_individual_R_X11( ae_individual_R_X11* parent, int32_t id,
                                          ae_jumping_mt* mut_prng, ae_jumping_mt* stoch_prng ) :
        ae_individual( parent, id, mut_prng, stoch_prng ),
        ae_individual_R( parent, id, mut_prng, stoch_prng  ),
        ae_individual_X11( parent, id, mut_prng, stoch_prng  )
{
  //printf("ae_individual_R_X11( parent )");
}

ae_individual_R_X11::ae_individual_R_X11( gzFile backup_file ) :
ae_individual( backup_file ), ae_individual_R( backup_file ), ae_individual_X11( backup_file )
{
}

// =================================================================
//                             Destructors
// =================================================================
ae_individual_R_X11::~ae_individual_R_X11( void )
{
}

// =================================================================
//                            Public Methods
// =================================================================

// =================================================================
//                           Protected Methods
// =================================================================