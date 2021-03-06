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
 
 
#ifndef __AE_POINT_2D_H__
#define __AE_POINT_2D_H__
 
 
// =================================================================
//                              Libraries
// =================================================================
#include <inttypes.h>
#include <zlib.h>



// =================================================================
//                            Project Files
// =================================================================
#include <ae_object.h>




// =================================================================
//                          Class declarations
// =================================================================





 
class ae_point_2d : public ae_object
{  
  public :
  
    // =================================================================
    //                             Constructors
    // =================================================================
    inline ae_point_2d( void );
    inline ae_point_2d( double x, double y );
    inline ae_point_2d( const ae_point_2d& source );
    inline ae_point_2d( gzFile backup_file );
  
    // =================================================================
    //                             Destructors
    // =================================================================
    inline virtual ~ae_point_2d( void );
  
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
    double y;
  
  
  
  
  
  protected :
  
    // =================================================================
    //                         Forbidden Constructors
    // =================================================================
    //~ ae_point_2d( void )
    //~ {
      //~ printf( "ERROR : Call to forbidden constructor in file %s : l%d\n", __FILE__, __LINE__ );
      //~ exit( EXIT_FAILURE );
    //~ };
    /*    ae_point_2d( const ae_point_2d &model )
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
//                              Class ae_point_2d                              #
//                                                                             #
//##############################################################################

// =================================================================
//                    Definition of static attributes
// =================================================================

// =================================================================
//                             Constructors
// =================================================================
inline ae_point_2d::ae_point_2d( void )
{
  x = -1;
  y = -1;
}

inline ae_point_2d::ae_point_2d( double x, double y )
{
  this->x = x;
  this->y = y;
}

inline ae_point_2d::ae_point_2d( const ae_point_2d& source )
{
  this->x = source.x;
  this->y = source.y;
}

inline ae_point_2d::ae_point_2d( gzFile backup_file )
{
  gzread( backup_file, &x, sizeof(x) );
  gzread( backup_file, &y, sizeof(y) );
}

// =================================================================
//                             Destructors
// =================================================================
inline ae_point_2d::~ae_point_2d( void )
{
}

// =====================================================================
//                          Accessors definitions
// =====================================================================

// =================================================================
//                            Public Methods
// =================================================================
inline void ae_point_2d::save( gzFile backup_file )
{
  gzwrite( backup_file, &x, sizeof(x) );
  gzwrite( backup_file, &y, sizeof(y) );
}

// =================================================================
//                           Protected Methods
// =================================================================



#endif // __AE_POINT_2D_H__
