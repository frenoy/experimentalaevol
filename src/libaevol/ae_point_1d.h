// ****************************************************************************
//
//          Aevol - An in silico experimental evolution platform
//
// ****************************************************************************
// 
// Copyright: See the AUTHORS file provided with the package or <www.aevol.fr>
// Web: http://www.aevol.fr/
// E-mail: See <http://www.aevol.fr/contact/>
// Original Authors : Guillaume Beslon, Carole Knibbe, David Parsons
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// ****************************************************************************
 
 
#ifndef __AE_POINT_1D_H__
#define __AE_POINT_1D_H__
 
 
// =================================================================
//                              Libraries
// =================================================================
#include <inttypes.h>



// =================================================================
//                            Project Files
// =================================================================




// =================================================================
//                          Class declarations
// =================================================================





 
class ae_point_1d : public ae_object
{  
  public :
  
    // =================================================================
    //                             Constructors
    // =================================================================
    inline ae_point_1d( void );
    inline ae_point_1d( double x );
    inline ae_point_1d( const ae_point_1d& source );
    inline ae_point_1d( gzFile backup_file );
  
    // =================================================================
    //                             Destructors
    // =================================================================
    inline virtual ~ae_point_1d( void );
  
    // =================================================================
    //                              Accessors
    // =================================================================
  
    // =================================================================
    //                            Public Methods
    // =================================================================
    inline void save( gzFile backup_file );
  
    // =================================================================
    //                           Public Attributes
    // =================================================================
    double x;
  
  
  
  
  
  protected :
  
    // =================================================================
    //                         Forbidden Constructors
    // =================================================================
    //~ ae_point_1d( void )
    //~ {
      //~ printf( "ERROR : Call to forbidden constructor in file %s : l%d\n", __FILE__, __LINE__ );
      //~ exit( EXIT_FAILURE );
    //~ };
    /*    ae_point_1d( const ae_point_1d &model )
    {
      printf( "ERROR : Call to forbidden constructor in file %s : l%d\n", __FILE__, __LINE__ );
      exit( EXIT_FAILURE );
      };*/

  
    // =================================================================
    //                           Protected Methods
    // =================================================================
  
    // =================================================================
    //                          Protected Attributes
    // =================================================================
};




//##############################################################################
//                                                                             #
//                              Class ae_point_1d                              #
//                                                                             #
//##############################################################################

// =================================================================
//                    Definition of static attributes
// =================================================================

// =================================================================
//                             Constructors
// =================================================================
inline ae_point_1d::ae_point_1d( void )
{
  x = -1;
}

inline ae_point_1d::ae_point_1d( double x )
{
  this->x = x;
}

inline ae_point_1d::ae_point_1d( const ae_point_1d& source )
{
  this->x = source.x;
}

inline ae_point_1d::ae_point_1d( gzFile backup_file )
{
  gzread( backup_file, &x, sizeof(x) );
}

// =================================================================
//                             Destructors
// =================================================================
inline ae_point_1d::~ae_point_1d( void )
{
}

// =====================================================================
//                          Accessors definitions
// =====================================================================

// =================================================================
//                            Public Methods
// =================================================================
inline void ae_point_1d::save( gzFile backup_file )
{
  gzwrite( backup_file, &x, sizeof(x) );
}

// =================================================================
//                           Protected Methods
// =================================================================



#endif // __AE_POINT_1D_H__
