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
 
 
#ifndef __AE_MUTATION_H__
#define __AE_MUTATION_H__
 
 
// =================================================================
//                              Libraries
// =================================================================
#include <inttypes.h>




// =================================================================
//                            Project Files
// =================================================================
#include <ae_object.h>
#include <ae_enums.h>
#include <zlib.h>


// =================================================================
//                          Class declarations
// =================================================================


enum ae_mutation_type
{
  SWITCH  = 0,
  S_INS   = 1,
  S_DEL   = 2,
  DUPL    = 3,
  DEL     = 4,
  TRANS   = 5,
  INV     = 6,
  INSERT  = 7,
  INS_HT  = 8,
  REPL_HT = 9
};





 
class ae_mutation : public ae_object
{  
  public :
  
    // =================================================================
    //                             Constructors
    // =================================================================
    ae_mutation( void );
    ae_mutation( const ae_mutation &model );
    ae_mutation( gzFile backup_file );
  
    // =================================================================
    //                             Destructors
    // =================================================================
    virtual ~ae_mutation( void );
  
    // =================================================================
    //                        Accessors: Getters
    // =================================================================
    inline ae_mutation_type get_mut_type( void );
    inline int32_t          get_length( void );

    void get_infos_point_mutation( int32_t* pos );
    void get_infos_small_insertion( int32_t* pos, int32_t* length ); // everything except the sequence
    void get_sequence_small_insertion( char* seq ); // seq must be a char array, large enough to contain _length+1 characters
    void get_infos_small_deletion( int32_t* pos, int32_t* length );
    void get_infos_duplication( int32_t* pos1, int32_t* pos2, int32_t* pos3, int16_t* align_score = NULL );
    void get_infos_deletion( int32_t* pos1, int32_t* pos2, int16_t* align_score = NULL );
    void get_infos_translocation( int32_t* pos1, int32_t* pos2, int32_t* pos3, int32_t* pos4, bool* invert,
                                  int16_t* align_score_1 = NULL, int16_t* align_score_2 = NULL );
    void get_infos_inversion( int32_t* pos1, int32_t* pos2, int16_t* align_score = NULL );
    void get_infos_insertion( int32_t* pos, int32_t* length );
    void get_sequence_insertion( char* seq );
    void get_infos_ins_HT( int32_t* pos1, int32_t* pos2, int32_t* pos3, int32_t* pos4, ae_sense* sense, int32_t* length );
    void get_sequence_ins_HT( char* seq ); 
    void get_infos_repl_HT( int32_t* pos1, int32_t* pos2, int32_t* pos3, int32_t* pos4, ae_sense* sense, int32_t* length );
    void get_sequence_repl_HT( char* seq ); 
    
    // =================================================================
    //                        Accessors: Setters
    // =================================================================

  
    // =================================================================
    //                            Public Methods
    // =================================================================
    void report_point_mutation( int32_t pos );
    void report_small_insertion( int32_t pos, int32_t length, const char* seq );
    void report_small_deletion( int32_t pos, int32_t length );
    void report_duplication( int32_t pos_1, int32_t pos_2, int32_t pos_3, int32_t length, int16_t align_score = -1 );
    void report_deletion( int32_t pos_1, int32_t pos_2, int32_t length, int16_t align_score = -1 );
    void report_translocation( int32_t pos_1, int32_t pos_2, int32_t pos_3, int32_t pos_4, int32_t length,
                                bool invert, int16_t align_score_1 = -1, int16_t align_score_2 = -1 );
    void report_inversion( int32_t pos_1, int32_t pos_2, int32_t length, int16_t align_score = -1 );
    void report_insertion( int32_t pos, int32_t length, const char* seq );
    void report_ins_HT(int32_t alignment_1_donor_pos_1, int32_t alignment_1_donor_pos_2, int32_t alignment_2_ind_pos, int32_t alignment_2_donor_pos, int32_t length, int16_t alignment_1_score, int16_t alignment_2_score, int32_t donor_id, ae_sense sense, const char* seq);
    void report_repl_HT(int32_t alignment_1_donor_pos_1, int32_t alignment_1_donor_pos_2, int32_t alignment_2_ind_pos, int32_t alignment_2_donor_pos, int32_t repl_seq_length, int32_t donor_seq_length, int16_t alignment_1_score, int16_t alignment_2_score, int32_t donor_id, ae_sense sense, const char* seq);

    void get_generic_description_string( char * str );
    
    /* DEPRECATED, use get_length instead */
    int32_t segment_length( int32_t gen_unit_len );

    void save( gzFile backup_file );
  
    // =================================================================
    //                           Public Attributes
    // =================================================================
  
  
  
  
  
  protected :
  
    // =================================================================
    //                         Forbidden Constructors
    // =================================================================
    //~ ae_mutation( void )
    //~ {
      //~ printf( "ERROR : Call to forbidden constructor in file %s : l%d\n", __FILE__, __LINE__ );
      //~ exit( EXIT_FAILURE );
    //~ };

  
    // =================================================================
    //                           Protected Methods
    // =================================================================
  
    // =================================================================
    //                          Protected Attributes
    // =================================================================
    ae_mutation_type  _mut_type;
    int32_t*          _pos;
    int32_t*           _length;
    char*             _seq;
    bool              _invert;
    int16_t*          _align_score;
    int32_t           _donor_id;   // CK: There should also be another int to indicate which genetic unit in the donor 
    ae_sense          _sense;
};


// =====================================================================
//                          Accessors' definitions
// =====================================================================
ae_mutation_type ae_mutation::get_mut_type( void )
{
  return _mut_type;
}

inline int32_t ae_mutation::get_length( void )
{
  return _length[0];
}



// =====================================================================
//                       Inline functions' definition
// =====================================================================


#endif // __AE_MUTATION_H__
