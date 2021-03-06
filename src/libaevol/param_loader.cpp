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
//*****************************************************************************




// =================================================================
//                              Libraries
// =================================================================
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// =================================================================
//                            Project Files
// =================================================================
#include <param_loader.h>

#include <ae_exp_manager.h>
#include <ae_exp_setup.h>
#include <ae_output_manager.h>
#include <ae_population.h>
#include <ae_individual.h>

#include <ae_jumping_mt.h>
#include <ae_gaussian.h>
#include <ae_env_segment.h>
#include <ae_point_2d.h>
#include <ae_align.h>

//~ #ifdef __X11
  //~ #include <ae_individual_X11.h>
//~ #endif

#ifdef __REGUL
  #include <ae_array_short.h>
#endif


// =================================================================
//                          Class declarations
// =================================================================
class ae_environment;




//##############################################################################
//                                                                             #
//                             Class param_loader                              #
//                                                                             #
//##############################################################################

// =================================================================
//                    Definition of static attributes
// =================================================================

// =================================================================
//                             Constructors
// =================================================================


param_loader::param_loader( const char* file_name )
{
  // Give default values to parameters
  
  // ----------------------------------------- PseudoRandom Number Generators
  _seed           = 0;
  _mut_seed       = 0;
  _stoch_seed     = 0;
  _env_var_seed   = 0;
  _env_noise_seed = 0;
  
  // ------------------------------------------------------------ Constraints
  _min_genome_length  = 10;
  _max_genome_length  = 10000000;
  _w_max              = 0.033333333;
  
  // ----------------------------------------------------- Initial conditions
  _chromosome_initial_length  = 5000;
  _init_method            = ONE_GOOD_GENE | CLONE;
  _init_pop_size          = 1000;
  
  // ------------------------------------------------------------ Environment
  _env_gaussians      = NULL;
  _env_custom_points  = NULL;
  _env_sampling       = 300;
  
  // ---------------------------------------- Environment x-axis segmentation
  _env_axis_nb_segments         = 1;
  _env_axis_segment_boundaries  = NULL;
  _env_axis_features            = NULL;
  _env_axis_separate_segments   = false;
  
  // -------------------------------------------------- Environment variation
  _env_var_method = NO_VAR;
  _env_var_sigma  = 0;
  _env_var_tau    = 0;
  
  // ------------------------------------------------------ Environment noise
  _env_noise_method       = NO_NOISE;
  _env_noise_alpha        = 0;
  _env_noise_sigma        = 0;
  _env_noise_prob         = 0;
  _env_noise_sampling_log = 0;
  
  // --------------------------------------------------------- Mutation rates
  _point_mutation_rate  = 1e-5;
  _small_insertion_rate = 1e-5;
  _small_deletion_rate  = 1e-5;
  _max_indel_size       = 6;
  
  // -------------------------------------------- Rearrangements and Transfer
  _with_4pts_trans            = true;
  _with_alignments            = false;
  _with_HT                    = false;
  _repl_HT_with_close_points  = false;
  _HT_ins_rate                = 0.0;
  _HT_repl_rate               = 0.0;
  _repl_HT_detach_rate        = 0.0;
  
  // ------------------------------ Rearrangement rates (without alignements)
  _duplication_rate   = 5e-5;
  _deletion_rate      = 5e-5;
  _translocation_rate = 5e-5;
  _inversion_rate     = 5e-5;
  
  // --------------------------------- Rearrangement rates (with alignements)
  _neighbourhood_rate       = 5e-5;
  _duplication_proportion   = 0.3;
  _deletion_proportion      = 0.3;
  _translocation_proportion = 0.3;
  _inversion_proportion     = 0.3;
  
  // ------------------------------------------------------------ Alignements
  _align_fun_shape    = SIGMOID;
  _align_sigm_lambda  = 4;
  _align_sigm_mean    = 50;
  _align_lin_min      = 0;
  _align_lin_max      = 100;
  
  _align_max_shift      = 20;
  _align_w_zone_h_len   = 50;
  _align_match_bonus    = 1;
  _align_mismatch_cost  = 2;
  
  // ----------------------------------------------- Phenotypic Stochasticity
  _with_stochasticity = false;
  
  // -------------------------------------------------------------- Selection
  _selection_scheme   = RANK_EXPONENTIAL;
  _selection_pressure = 0.998;
  
  // ------------------------------------------------------ Spatial structure
  _spatially_structured       = false;
  _grid_width                 = 0;
  _grid_height                = 0;
  _migration_number           = 0;
  
  // -------------------------------------------------------------- Secretion
  _with_secretion               = false;
  _secretion_contrib_to_fitness = 0;
  _secretion_diffusion_prop     = 0;
  _secretion_degradation_prop   = 0;
  _secretion_cost               = 0;
  _secretion_init               = 0;
  
#ifdef BINARY_SECRETION
  _mutdc=0.;
  _mutcd=0.;
#endif
  
  // --------------------------------------------------------------- Plasmids
  _allow_plasmids             = false;
  _plasmid_initial_length     = -1;
  _plasmid_initial_gene       = 0;
  _plasmid_minimal_length     = 40;
  _plasmid_maximal_length     = -1;
  _chromosome_minimal_length  = -1;
  _chromosome_maximal_length  = -1;
  _prob_plasmid_HT            = 0;
  _tune_donor_ability         = 0;
  _tune_recipient_ability     = 0;
  _donor_cost                 = 0;
  _recipient_cost             = 0;
  _compute_phen_contrib_by_GU = false;
  _swap_GUs         = false;

  // ------------------------------------------------------------ Hitchhiking
  _trait_gu_location = new int16_t[NB_FEATURES];
  int16_t i;
  for (i=0; i<NB_FEATURES; i++)
  {
    _trait_gu_location[i]=0;
  }
  _isolate_GUs = false;
  _break_linkage_period = 0;
  _break_linkage_nb_gu = -1;
  
  // ------------------------------------------------------- Translation cost
  _translation_cost = 0;
  
  // ---------------------------------------------------------------- Outputs
  _stats            = 0;
  _delete_old_stats = false;
  
  // Backups
  _backup_step      = 500;
  _big_backup_step  = 10000;
  
  // Tree
  _record_tree  = false;
  _tree_step    = 100;
  _tree_mode    = NORMAL;
  
  // Dumps
  _make_dumps = false;
  _dump_step  = 1000;
  
  // Logs
  _logs = 0;
  
  // Other
  _more_stats = false;
  
#ifdef __REGUL
  // ------------------------------------------------------- Binding matrix
  _binding_zeros_percentage = 75;
#endif
  
  // Read parameter file
  _param_file_name = strdup( file_name );
  _param_file  = fopen( _param_file_name,  "r" );
  
  if ( _param_file == NULL )
  {
    printf( "ERROR : couldn't open file %s\n", file_name );
    exit( EXIT_FAILURE );
  }
  
  assert( _param_file );
  
  read_file();
}

// =================================================================
//                             Destructors
// =================================================================
param_loader::~param_loader( void )
{
  free( _param_file_name );
  fclose( _param_file );
  
  if ( _env_axis_segment_boundaries != NULL ) delete [] _env_axis_segment_boundaries;
  if ( _env_axis_features != NULL ) delete [] _env_axis_features;
  delete _trait_gu_location;

}

// =================================================================
//                            Public Methods
// =================================================================

void param_loader::interpret_line( f_line* line, int32_t cur_line )
{
  // Interpret line
  if ( strcmp( line->words[0], "MAX_TRIANGLE_WIDTH" ) == 0 )
  {
    _w_max=atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "ENV_AXIS_FEATURES" ) == 0 )
  {
    // Set general segmentation data
    _env_axis_nb_segments = line->nb_words / 2;
    
    // Set segmentation boundaries
    _env_axis_segment_boundaries = new double [_env_axis_nb_segments + 1];
    _env_axis_segment_boundaries[0] = X_MIN;
    for ( int16_t i = 1 ; i < _env_axis_nb_segments ; i++ )
    {
      _env_axis_segment_boundaries[i] = atof( line->words[2*i] );
    }
    _env_axis_segment_boundaries[_env_axis_nb_segments] = X_MAX;
    
    // Set segment features
    _env_axis_features = new ae_env_axis_feature[_env_axis_nb_segments];
    for ( int16_t i = 0 ; i < _env_axis_nb_segments ; i++ )
    {
      if ( strcmp( line->words[2*i+1], "NEUTRAL" ) == 0 )
      {
        _env_axis_features[i] = NEUTRAL;
      }
      else if ( strcmp( line->words[2*i+1], "METABOLISM" ) == 0 )
      {
        _env_axis_features[i] = METABOLISM;
      }
      else if ( strcmp( line->words[2*i+1], "SECRETION" ) == 0 )
      {
        _with_secretion = true;
        _env_axis_features[i] = SECRETION;
      }
      else if ( strcmp( line->words[2*i+1], "DONOR" ) == 0 )
      {
        _env_axis_features[i] = DONOR;
      }
      else if ( strcmp( line->words[2*i+1], "RECIPIENT" ) == 0 )
      {
        _env_axis_features[i] = RECIPIENT;
      }
      else
      {
        printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown axis feature \"%s\".\n",
                _param_file_name, cur_line, line->words[2*i+1] );
        exit( EXIT_FAILURE );
      }
    }
  }
  else if ( strcmp( line->words[0], "TRAIT_GU_LOCATION" ) == 0)
  {
    if ( strcmp( line->words[1], "NEUTRAL" ) == 0 )
    {
      _trait_gu_location[NEUTRAL] = atoi( line->words[2] );
    }
    else if ( strcmp( line->words[1], "METABOLISM" ) == 0 )
    {
      _trait_gu_location[METABOLISM] = atoi( line->words[2] );
    }
    else if ( strcmp( line->words[1], "SECRETION" ) == 0 )
    {
      _trait_gu_location[SECRETION] = atoi( line->words[2] );
    }
    else if ( strcmp( line->words[1], "DONOR" ) == 0 )
    {
      _trait_gu_location[DONOR] = atoi( line->words[2] );
    }
    else if ( strcmp( line->words[1], "RECIPIENT" ) == 0 )
    {
      _trait_gu_location[RECIPIENT] = atoi( line->words[2] );
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown axis feature \"%s\".\n",
              _param_file_name, cur_line, line->words[2] );
      exit( EXIT_FAILURE );
    }
  }
  else if ( strcmp( line->words[0], "ISOLATE_GUS" ) == 0 )
  {
    _isolate_GUs = true;
  }
  else if (strcmp(line->words[0], "BREAK_LINKAGE") == 0 )
  {
    _break_linkage_period = atoi( line->words[1] );
    _break_linkage_nb_gu = atoi( line->words[2] );
  }
  else if ( strcmp( line->words[0], "ENV_SEPARATE_SEGMENTS" ) == 0 )
  {
    _env_axis_separate_segments = true;
  }
  else if ( strcmp( line->words[0], "RECORD_TREE" ) == 0 )
  {
    if ( strncmp( line->words[1], "true", 4 ) == 0 )
    {
      _record_tree = true;
    }
    else if ( strncmp( line->words[1], "false", 5 ) == 0 )
    {
      _record_tree = false;
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown tree recording option (use true/false).\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE ); 
    }
  }
  else if ( strcmp( line->words[0], "TREE_MODE" ) == 0 )
  {
    if ( strcmp( line->words[1], "light" ) == 0 )
    {
      _tree_mode = LIGHT;
    }
    else if ( strcmp( line->words[1], "normal" ) == 0 )
    {
      _tree_mode = NORMAL;
    }       
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown tree mode option (use normal/light).\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE );
    }
  }
  else if ( strcmp( line->words[0], "MORE_STATS" ) == 0 )
  {
    if ( strncmp( line->words[1], "true", 4 ) == 0 )
    {
      _more_stats = true;
    }
    else if ( strncmp( line->words[1], "false", 5 ) == 0 )
    {
      _more_stats = false;
    }       
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown more stats option (use true/false).\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE ); 
    }
  }
  else if ( strcmp( line->words[0], "DUMP_STEP" ) == 0 )
  {
    _dump_step = atol( line->words[1] );
    if (_dump_step>0) _make_dumps = true;
  }
  else if ( strcmp( line->words[0], "BACKUP_STEP" ) == 0 )
  {
    _backup_step = atol( line->words[1] );
  }
  else if ( strcmp( line->words[0], "BIG_BACKUP_STEP" ) == 0 )
  {
    _big_backup_step = atol( line->words[1] );
  }
  else if ( strcmp( line->words[0], "TREE_STEP" ) == 0 )
  {
    _tree_step = atol( line->words[1] );
  }
  else if ( strcmp( line->words[0], "CHROMOSOME_INITIAL_LENGTH" ) == 0 )
  {
    _chromosome_initial_length = atol( line->words[1] );
  }
  else if ( strcmp( line->words[0], "MIN_GENOME_LENGTH" ) == 0 )
  {
    if ( strncmp( line->words[1], "NONE", 4 ) == 0 )
    {
      _min_genome_length = 1; // Must not be 0
    }
    else
    {
      _min_genome_length = atol( line->words[1] );
      if (_min_genome_length == 0 )
      {
        printf( "ERROR in param file \"%s\" on line %"PRId32" : MIN_GENOME_LENGTH must be > 0.\n",
                _param_file_name, cur_line );
        exit( EXIT_FAILURE ); 
      }
    }
  }
  else if ( strcmp( line->words[0], "MAX_GENOME_LENGTH" ) == 0 )
  {
    if ( strncmp( line->words[1], "NONE", 4 ) == 0 )
    {
      _max_genome_length = INT32_MAX;
    }
    else
    {
      _max_genome_length = atol( line->words[1] );
    }
  }
  else if ( strcmp( line->words[0], "INIT_POP_SIZE" ) == 0 )
  {
    _init_pop_size = atol( line->words[1] );
  }
  else if ( strcmp( line->words[0], "POP_STRUCTURE" ) == 0 )
  {
    if ( strcmp( line->words[1], "grid" ) == 0 )
    {
      _spatially_structured = true;
      _grid_width = atol( line->words[2] );
      _grid_height = atol( line->words[3] );
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown population structure.\n", _param_file_name, cur_line );
      exit( EXIT_FAILURE ); 
    }
  }
  else if ( strcmp( line->words[0], "MIGRATION_NUMBER" ) == 0 )
  {
    _migration_number = atol( line->words[1] );
  }
  else if ( strcmp( line->words[0], "INIT_METHOD" ) == 0 )
  {
    for ( int8_t i = 1 ; i < line->nb_words ; i++ )
    {
      if ( strcmp( line->words[i], "ONE_GOOD_GENE" ) == 0 )
      {
        _init_method |= ONE_GOOD_GENE;
      }
      else if ( strcmp( line->words[i], "CLONE" ) == 0 )
      {
        _init_method |= CLONE;
      }
      else if ( strcmp( line->words[i], "WITH_INS_SEQ" ) == 0 )
      {
        _init_method |= WITH_INS_SEQ;
      }   
      else
      {
        printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown initialization method %s.\n",
                _param_file_name, cur_line, line->words[1] );
        exit( EXIT_FAILURE ); 
      }
    }
  }
  else if ( strcmp( line->words[0], "POINT_MUTATION_RATE" ) == 0 )
  {
    _point_mutation_rate = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "SMALL_INSERTION_RATE" ) == 0 )
  {
    _small_insertion_rate = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "SMALL_DELETION_RATE" ) == 0 )
  {
    _small_deletion_rate = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "MAX_INDEL_SIZE" ) == 0 )
  {
    _max_indel_size = atol( line->words[1] );
  }
  else if ( strcmp( line->words[0], "DUPLICATION_RATE" ) == 0 )
  {
    _duplication_rate = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "DELETION_RATE" ) == 0 )
  {
    _deletion_rate = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "TRANSLOCATION_RATE" ) == 0 )
  {
    _translocation_rate = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "INVERSION_RATE" ) == 0 )
  {
    _inversion_rate = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "NEIGHBOURHOOD_RATE" ) == 0 )
  {
    _neighbourhood_rate = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "DUPLICATION_PROPORTION" ) == 0 )
  {
    _duplication_proportion = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "DELETION_PROPORTION" ) == 0 )
  {
    _deletion_proportion = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "TRANSLOCATION_PROPORTION" ) == 0 )
  {
    _translocation_proportion = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "INVERSION_PROPORTION" ) == 0 )
  {
    _inversion_proportion = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "ALIGN_FUNCTION" ) == 0 )
  {
    if ( line->nb_words != 2 && line->nb_words != 4 )
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : incorrect number of parameters for keyword \"%s\".\n",
              _param_file_name, cur_line, line->words[0] );
      exit( EXIT_FAILURE );
    }
    
    if ( strcmp( line->words[1], "LINEAR" ) == 0 )
    {
      _align_fun_shape = LINEAR;
      
      if ( line->nb_words == 4 )
      {
        _align_lin_min = atol( line->words[2] );
        _align_lin_max = atol( line->words[3] );
      }
    }
    else if ( strcmp( line->words[1], "SIGMOID" ) == 0 )
    {
      _align_fun_shape = SIGMOID;
      
      if ( line->nb_words == 4 )
      {
        _align_sigm_lambda = atol( line->words[2] );
        _align_sigm_mean = atol( line->words[3] );
      }
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown align function shape \"%s\".\n",
              _param_file_name, cur_line, line->words[1] );
      exit( EXIT_FAILURE );
    }
  }
  else if ( strcmp( line->words[0], "ALIGN_MAX_SHIFT" ) == 0 )
  {
    _align_max_shift = atol( line->words[1] );
  }
  else if ( strcmp( line->words[0], "ALIGN_W_ZONE_H_LEN" ) == 0 )
  {
    _align_w_zone_h_len = atol( line->words[1] );
  }
  else if ( strcmp( line->words[0], "ALIGN_MATCH_BONUS" ) == 0 )
  {
    _align_match_bonus = atol( line->words[1] );
  }
  else if ( strcmp( line->words[0], "ALIGN_MISMATCH_COST" ) == 0 )
  {
    _align_mismatch_cost = atol( line->words[1] );
  }
  else if ( strcmp( line->words[0], "STOCHASTICITY" ) == 0 )
  {
    if ( strncmp( line->words[1], "true", 4 ) == 0 )
    {
      _with_stochasticity = true;
    }
  }
  else if ( strcmp( line->words[0], "SELECTION_SCHEME" ) == 0 )
  {
    if ( strncmp( line->words[1], "lin", 3 ) == 0 )
    {
      _selection_scheme = RANK_LINEAR;
    }
    else if ( strncmp( line->words[1], "exp", 3 ) == 0 )
    {
      _selection_scheme = RANK_EXPONENTIAL;
    }
    else if ( strncmp( line->words[1], "fitness", 7 ) == 0 )
    {
      _selection_scheme = FITNESS_PROPORTIONATE;
    }
    else if ( strcmp( line->words[1], "fittest" ) == 0 )
    {
      _selection_scheme = FITTEST;
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown selection scheme \"%s\".\n",
              _param_file_name, cur_line, line->words[1] );
      exit( EXIT_FAILURE );
    }
  }
  else if ( strcmp( line->words[0], "SELECTION_PRESSURE" ) == 0 )
  {
    _selection_pressure = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "SEED" ) == 0 )
  {
    static bool seed_already_set = false; // ??? TODO understand this !
    if ( seed_already_set )
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : duplicate entry for SEED.\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE );
    }
    _seed = atol( line->words[1] );
    seed_already_set = true;
  }
  else if ( strcmp( line->words[0], "MUT_SEED" ) == 0 )
  {
    static bool mut_seed_already_set = false;
    if ( mut_seed_already_set )
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : duplicate entry for MUT_SEED.\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE );
    }
    _mut_seed = atol( line->words[1] );
    mut_seed_already_set = true;
  }
  else if ( strcmp( line->words[0], "STOCH_SEED" ) == 0 )
  {
    static bool stoch_seed_already_set = false;
    if ( stoch_seed_already_set )
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : duplicate entry for STOCH_SEED.\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE );
    }
    _stoch_seed = atol( line->words[1] );
    stoch_seed_already_set = true;
  }
  else if ( strcmp( line->words[0], "WITH_4PTS_TRANS" ) == 0 )
  {
    if ( strncmp( line->words[1], "true", 4 ) == 0 )
    {
      _with_4pts_trans = true;
    }
    else if ( strncmp( line->words[1], "false", 5 ) == 0 )
    {
      printf( "ERROR: 3 points translocation hasn't been implemented yet\n" );
      exit( EXIT_FAILURE );
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown 4pts_trans option (use true/false).\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE ); 
    }
  }
  else if ( strcmp( line->words[0], "WITH_ALIGNMENTS" ) == 0 )
  {
    if ( strncmp( line->words[1], "true", 4 ) == 0 )
    {
      _with_alignments = true;
    }
    else if ( strncmp( line->words[1], "false", 5 ) == 0 )
    {
      _with_alignments = false;
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown alignement option (use true/false).\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE ); 
    }
  }
  else if ( strcmp( line->words[0], "WITH_TRANSFER" ) == 0 )
  {
    if ( strncmp( line->words[1], "true", 4 ) == 0 )
    {
      _with_HT = true;
    }
    else if ( strncmp( line->words[1], "false", 5 ) == 0 )
    {
      _with_HT = false;
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown transfer option (use true/false).\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE ); 
    }
  }
  else if ( strcmp( line->words[0], "REPL_TRANSFER_WITH_CLOSE_POINTS" ) == 0 )
  {
    if ( strncmp( line->words[1], "true", 4 ) == 0 )
    {
      _repl_HT_with_close_points = true;
    }
    else if ( strncmp( line->words[1], "false", 5 ) == 0 )
    {
      _repl_HT_with_close_points = false;
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown transfer option (use true/false).\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE ); 
    }
  }
  else if ( strcmp( line->words[0], "SWAP_GUS" ) == 0 )
  {
    if ( strncmp( line->words[1], "true", 4 ) == 0 )
    {
      _swap_GUs = true;
    }
    else if ( strncmp( line->words[1], "false", 5 ) == 0 )
    {
      _swap_GUs = false;
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown swap option (use true/false).\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE ); 
    }
  }
  else if ( strcmp( line->words[0], "TRANSFER_INS_RATE" ) == 0 )
  {
    _HT_ins_rate = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "TRANSFER_REPL_RATE" ) == 0 )
  {
    _HT_repl_rate = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "REPL_TRANSFER_DETACH_RATE" ) == 0 )
  {
    _repl_HT_detach_rate = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "TRANSLATION_COST" ) == 0 )
  {
    _translation_cost = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "ENV_ADD_POINT" ) == 0 )
  {
    _env_custom_points->add( new ae_point_2d(  atof( line->words[1] ), atof( line->words[2] ) ) );
  }
  else if ( strcmp( line->words[0], "ENV_ADD_GAUSSIAN" ) == 0 )
  {
    if ( _env_gaussians == NULL ) _env_gaussians = new ae_list<ae_gaussian*>();
    
    _env_gaussians->add( new ae_gaussian( atof( line->words[1] ), atof( line->words[2] ), atof( line->words[3] ) ) );
  }
  else if ( strcmp( line->words[0], "ENV_GAUSSIAN" ) == 0 )
  {
    if ( _env_gaussians == NULL ) _env_gaussians = new ae_list<ae_gaussian*>();
    
    _env_gaussians->add( new ae_gaussian( atof( line->words[1] ), atof( line->words[2] ), atof( line->words[3] ) ) );
  }
  else if ( strcmp( line->words[0], "ENV_SAMPLING" ) == 0 )
  {
    _env_sampling = atoi( line->words[1] );
  }
  else if ( strcmp( line->words[0], "ENV_VARIATION" ) == 0 )
  {
    static bool env_var_already_set = false;
    if ( env_var_already_set )
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : duplicate entry for %s.\n",
              _param_file_name, cur_line, line->words[0] );
      exit( EXIT_FAILURE );
    }
    env_var_already_set = true;
    
    if ( strcmp( line->words[1], "none" ) == 0 )
    {
      assert( line->nb_words == 2 );
      _env_var_method = NO_VAR;
    }
    else if ( strcmp( line->words[1], "autoregressive_mean_variation" ) == 0 )
    {
      assert( line->nb_words == 5 );
      _env_var_method = AUTOREGRESSIVE_MEAN_VAR;
      _env_var_sigma = atof( line->words[2] );
      _env_var_tau = atol( line->words[3] );
      _env_var_seed = atoi( line->words[4] );
    }
    else if ( strcmp( line->words[1], "autoregressive_height_variation" ) == 0 )
    {
      assert( line->nb_words == 5 );
      _env_var_method = AUTOREGRESSIVE_HEIGHT_VAR;
      _env_var_sigma = atof( line->words[2] );
      _env_var_tau = atol( line->words[3] );
      _env_var_seed = atoi( line->words[4] );
    }
    else if ( strcmp( line->words[1], "add_local_gaussians" ) == 0 )
    {
      assert( line->nb_words == 3 );
      _env_var_method = LOCAL_GAUSSIANS_VAR;
      _env_var_seed = atoi( line->words[2] );
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown environment variation method.\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE );
    }
  }
  else if ( strcmp( line->words[0], "ENV_NOISE" ) == 0 )
  {
    static bool env_noise_already_set = false;
    if ( env_noise_already_set )
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : duplicate entry for %s.\n",
              _param_file_name, cur_line, line->words[0] );
      exit( EXIT_FAILURE );
    }
    env_noise_already_set = true;
    
    if ( strcmp( line->words[1], "none" ) == 0 )
    {
      assert( line->nb_words == 2 );
      _env_noise_method = NO_NOISE;
    }
    else if ( strcmp( line->words[1], "FRACTAL" ) == 0 )
    {
			assert( line->nb_words == 6 );
      _env_noise_method = FRACTAL;
      _env_noise_sampling_log = atoi( line->words[2] );
      _env_noise_sigma = atof( line->words[3] );
      _env_noise_alpha = atof( line->words[4] );
      _env_noise_prob = atof( line->words[5] );
      _env_noise_seed = atoi( line->words[6] );
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown environment noise method.\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE );
    }
  }
  else if ( strcmp( line->words[0], "SECRETION_CONTRIB_TO_FITNESS" ) == 0 )
  {
    _secretion_contrib_to_fitness = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "SECRETION_DIFFUSION_PROP" ) == 0 )
  {
    _secretion_diffusion_prop = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "SECRETION_DEGRADATION_PROP" ) == 0 )
  {
    _secretion_degradation_prop = atof( line->words[1] );
    if ( _secretion_degradation_prop > 1 || _secretion_degradation_prop < 0 )
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : degradation must be in (0,1).\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE );
    }
  }
  else if ( strcmp( line->words[0], "SECRETION_INITIAL" ) == 0 )
  {
    _secretion_init = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "SECRETION_COST" ) == 0 )
  {
    _secretion_cost = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "ALLOW_PLASMIDS" ) == 0 )
  {
    if ( strncmp( line->words[1], "true", 4 ) == 0 )
    {
      _allow_plasmids = true;
    }
    else if ( strncmp( line->words[1], "false", 5 ) == 0 )
    {
      _allow_plasmids = false;
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown allow_plasmids option (use true/false).\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE ); 
    }
  }
  else if ( strcmp( line->words[0], "PLASMID_INITIAL_LENGTH" ) == 0 )
  {
    _plasmid_initial_length = atoi( line->words[1] );
  }
  else if ( strcmp( line->words[0], "PLASMID_INITIAL_GENE" ) == 0 )
  {
    _plasmid_initial_gene = atoi( line->words[1] );
  }
  else if ( strcmp( line->words[0], "PLASMID_MINIMAL_LENGTH" ) == 0 )
  {
    _plasmid_minimal_length = atoi( line->words[1] );
  }
  else if ( strcmp( line->words[0], "PLASMID_MAXIMAL_LENGTH" ) == 0 )
  {
    _plasmid_maximal_length = atoi( line->words[1] );
  }
  else if ( strcmp( line->words[0], "CHROMOSOME_MINIMAL_LENGTH" ) == 0 )
  {
    _chromosome_minimal_length = atoi( line->words[1] );
  }
  else if ( strcmp( line->words[0], "CHROMOSOME_MAXIMAL_LENGTH" ) == 0 )
  {
    _chromosome_maximal_length = atoi( line->words[1] );
  }
  else if ( strcmp( line->words[0], "PROB_PLASMID_HT" ) == 0 )
  {
    _prob_plasmid_HT = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "TUNE_DONOR_ABILITY" ) == 0 )
  {
    _tune_donor_ability = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "TUNE_RECIPIENT_ABILITY" ) == 0 )
  {
    _tune_recipient_ability = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "DONOR_COST" ) == 0 )
  {
    _donor_cost = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "RECIPIENT_COST" ) == 0 )
  {
    _recipient_cost = atof( line->words[1] );
  }
  else if ( strcmp( line->words[0], "COMPUTE_PHEN_CONTRIB_BY_GU" ) == 0 )
  {
    if ( strncmp( line->words[1], "true", 4 ) == 0 )
    {
      _compute_phen_contrib_by_GU = true;
    }
    else if ( strncmp( line->words[1], "false", 5 ) == 0 )
    {
      _compute_phen_contrib_by_GU = false;
    }
    else
    {
      printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown compute_phen_contrib_by_GU option (use true/false).\n",
              _param_file_name, cur_line );
      exit( EXIT_FAILURE ); 
    }
  }
  else if ( strcmp( line->words[0], "LOG" ) == 0 )
  {
    for ( int8_t i = 1 ; i < line->nb_words ; i++ )
    {
      if ( strcmp( line->words[i], "TRANSFER" ) == 0 )
      {
        _logs |= LOG_TRANSFER;
      }
      else if ( strcmp( line->words[i], "REAR" ) == 0 )
      {
        _logs |= LOG_REAR;
      }
      else if ( strcmp( line->words[i], "BARRIER" ) == 0 )
      {
        _logs |= LOG_BARRIER;
      }
      /*else if ( strcmp( line->words[i], "LOADS" ) == 0 )
      {
        tmp_to_be_logged |= LOG_LOADS;
      }   */
      else
      {
        printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown log option %s.\n",
                _param_file_name, cur_line, line->words[1] );
        exit( EXIT_FAILURE ); 
      }
    }
  }
#ifdef BINARY_SECRETION
  else if ( strcmp( line->words[0], "SWITCH_DC") == 0 )
  {
    _mutdc=atof(line->words[1]);
  }
  else if ( strcmp( line->words[0], "SWITCH_CD") == 0 )
  {
    _mutcd=atof(line->words[1]);
  }
#endif
  else
  {
    printf( "ERROR in param file \"%s\" on line %"PRId32" : undefined key word \"%s\"\n", _param_file_name, cur_line, line->words[0] );
    exit( EXIT_FAILURE );
  }
}

void param_loader::read_file( void )
{
  // The rewind is only necessary when using multiple param files
  rewind( _param_file );

  int32_t cur_line = 0;
  f_line* line;
  

  while ( ( line = get_line(&cur_line) ) != NULL ) // TODO : write line = new f_line( _param_file ) => f_line::f_line( char* )
  {
    interpret_line( line, cur_line );
    delete line;
  }
}


void param_loader::load( ae_exp_manager* exp_m, bool verbose, std::function<std::list<char*>(void)> creator, char* chromosome, int32_t lchromosome, char* plasmid, int32_t lplasmid )
{
  // Check consistency of min, max and initial length of chromosome and plasmid
  // Default for by GU minimal or maximal size is -1.
  // If equal to -1, maximal sizes of each GU will be replaced by total maximal size for the whole genome
  
  if (_allow_plasmids)
  {
    if (_plasmid_initial_gene!=1) // the plasmid will be copied from the chromosome
    {
      if (_plasmid_initial_length != -1)
      {
        printf("WARNING: PLASMID_INITIAL_LENGTH is not taken into account because PLASMID_INITIAL_GENE is set to 0 (copy from chromosome)\n");
        _plasmid_initial_length = _chromosome_initial_length;
      }
    }
    if (_plasmid_maximal_length == -1)
    {
      _plasmid_maximal_length = _max_genome_length;
    }
    if(_plasmid_minimal_length > _plasmid_initial_length)
    {
      printf("ERROR: PLASMID_INITIAL_LENGTH is lower than PLASMID_MINIMAL_LENGTH\n");
      exit( EXIT_FAILURE );
    }
    if (_plasmid_maximal_length < _plasmid_initial_length)
    {
      printf("ERROR: PLASMID_INITIAL_LENGTH is higher than PLASMID_MAXIMAL_LENGTH\n");
      exit( EXIT_FAILURE );
    }
  }
  if (_chromosome_maximal_length == -1)
  {
    _chromosome_maximal_length = _max_genome_length;
  }
  if (_chromosome_minimal_length > _chromosome_initial_length)
  {
    printf("ERROR: CHROMOSOME_INITIAL_LENGTH is lower than CHROMOSOME_MINIMAL_LENGTH\n");
    exit( EXIT_FAILURE );
  }
  if (_chromosome_maximal_length < _chromosome_initial_length)
  {
    printf("ERROR: CHROMOSOME_INITIAL_LENGTH is higher than PLASMID_MAXIMAL_LENGTH\n");
    exit( EXIT_FAILURE );
  }
  
  // Initialize _prng
  _prng = new ae_jumping_mt( _seed );
  
  // Initialize _mut_prng and _stoch_prng :
  // if mut_seed (respectively stoch_seed) not given in param.in, choose it at random
  if ( _mut_seed == 0 ) {
    _mut_seed=_prng->random( 1000000 );
  }
  if ( _stoch_seed == 0 ) {
    _stoch_seed=_prng->random( 1000000 );
  }
  _mut_prng = new ae_jumping_mt( _mut_seed );
  _stoch_prng = new ae_jumping_mt( _stoch_seed );
  
  // Create aliases (syntaxic sugars)
  ae_exp_setup*       exp_s     = exp_m->get_exp_s();
  ae_population*      pop       = exp_m->get_pop();
  ae_environment*     env       = exp_m->get_env();
  ae_selection*       sel       = exp_m->get_sel();
  ae_output_manager*  output_m  = exp_m->get_output_m();
  
  // If the population is spatially structured,
  // check that the population fits in the spatial structure
  if ( _spatially_structured )
  {
    if ( _init_pop_size != _grid_width * _grid_height )
    {
      printf( "ERROR: the number of individuals does not match the size of the grid\n" );
      exit( EXIT_FAILURE );
    }
  }
  
  // 1) ------------------------------------- Initialize the experimental setup
  sel->set_prng( new ae_jumping_mt(*_prng) );

  // ---------------------------------------------------------------- Selection
  sel->set_selection_scheme( _selection_scheme );
  sel->set_selection_pressure( _selection_pressure );
  
  // ----------------------------------------------------------------- Transfer
  exp_s->set_with_HT( _with_HT );
  exp_s->set_repl_HT_with_close_points( _repl_HT_with_close_points );
  exp_s->set_HT_ins_rate( _HT_ins_rate );
  exp_s->set_HT_repl_rate( _HT_repl_rate );
  exp_s->set_repl_HT_detach_rate( _repl_HT_detach_rate );
  
  // ----------------------------------------------------------------- Plasmids
  exp_s->set_with_plasmids( _allow_plasmids );
  exp_s->set_prob_plasmid_HT( _prob_plasmid_HT );
  exp_s->set_tune_donor_ability( _tune_donor_ability );
  exp_s->set_tune_recipient_ability( _tune_recipient_ability );
  exp_s->set_donor_cost( _donor_cost );
  exp_s->set_recipient_cost( _recipient_cost );
  exp_s->set_swap_GUs( _swap_GUs );

  // -------------------------------------------------------------- Hitchhiking
  exp_s->set_trait_gu_location( _trait_gu_location);
  output_m->set_compute_phen_contrib_by_GU( _compute_phen_contrib_by_GU );
  exp_s->set_isolate_GUs( _isolate_GUs );
  exp_s->set_break_linkage( _break_linkage_period, _break_linkage_nb_gu );
  
  // -------------------------------------------------------- Spatial structure
  if ( _spatially_structured )
  {
    ae_jumping_mt* sp_struct_prng = new ae_jumping_mt(*_prng);
    exp_m->set_spatial_structure( _grid_width,
                                  _grid_height,
                                  sp_struct_prng );
    ae_spatial_structure* sp_struct = exp_m->get_spatial_structure();
    sp_struct->set_secretion_degradation_prop( _secretion_degradation_prop );
    sp_struct->set_secretion_diffusion_prop( _secretion_diffusion_prop );
    sp_struct->set_migration_number( _migration_number );
  }
  
  // ---------------------------------------------------------------- Secretion
  exp_s->set_with_secretion( _with_secretion );
#ifdef BINARY_SECRETION
  if (exp_s->get_with_secretion()) // If we us a binary secretion we can not use a 'classical' secretion feature
  {
    printf( "ERROR: classical secretion can not be used when aevol is compiled with --enable-binary-secretion\n" );
    exit(EXIT_FAILURE);
  }
  if ((_mutdc>0) || (_mutcd>0))
  {
    exp_s->set_with_secretion(true);
    exp_s->get_sel()->set_mutdc(_mutdc);
    exp_s->get_sel()->set_mutcd(_mutcd);
  }
#endif
  exp_s->set_secretion_contrib_to_fitness( _secretion_contrib_to_fitness );
  exp_s->set_secretion_cost( _secretion_cost );
  
  
  // 2) ------------------------------------------------ Create the environment
  // Move the gaussian list and the list of custom points from the parameters
  // to the environment
  env->set_gaussians( _env_gaussians );
  _env_gaussians = NULL;
  env->set_custom_points( _env_custom_points );
  _env_custom_points = NULL;
  
  // Copy the sampling
  env->set_sampling( _env_sampling );
  
  // Set the environment segmentation
  if( (_env_axis_features != NULL) && (_env_axis_segment_boundaries != NULL)  )
    {
      // if param.in contained a line starting with ENV_AXIS_FEATURES,
      // we use the values indicated on this line
      env->set_segmentation( _env_axis_nb_segments,
                             _env_axis_segment_boundaries,
                             _env_axis_features,
                             _env_axis_separate_segments);
    } // else we leave the ae_environment as it is by default (one "metabolic" segment from X_MIN to XMAX)


  // Set environmental variation
  if ( _env_var_method != NO_VAR )
  {
    env->set_var_method( _env_var_method );
    env->set_var_prng( new ae_jumping_mt( _env_var_seed ) );
    env->set_var_sigma_tau( _env_var_sigma, _env_var_tau );
  }
  
  // Set environmental noise
  if ( _env_noise_method != NO_NOISE )
  {
    env->set_noise_method( _env_noise_method );
    env->set_noise_sampling_log( _env_noise_sampling_log );
    env->set_noise_prng( new ae_jumping_mt( _env_noise_seed ) );
    env->set_noise_alpha( _env_noise_alpha );
    env->set_noise_sigma( _env_noise_sigma );
    env->set_noise_prob( _env_noise_prob );
  }
  
  // Build the environment
  env->build();
  
  if ( verbose )
  {
    printf( "Entire geometric area of the environment : %f\n", env->get_geometric_area() );
  }
  
  
  // 3) --------------------------------------------- Create the new population
  pop->set_mut_prng( new ae_jumping_mt(*_mut_prng) );
  pop->set_stoch_prng( new ae_jumping_mt(*_stoch_prng) );
  
  // Generate a model ae_mut_param object
  ae_params_mut* param_mut = new ae_params_mut();
  param_mut->set_point_mutation_rate( _point_mutation_rate );
  param_mut->set_small_insertion_rate( _small_insertion_rate );
  param_mut->set_small_deletion_rate( _small_deletion_rate );
  param_mut->set_max_indel_size( _max_indel_size );
  param_mut->set_with_4pts_trans( _with_4pts_trans );
  param_mut->set_with_alignments( _with_alignments );
  param_mut->set_with_HT( _with_HT );
  param_mut->set_repl_HT_with_close_points( _repl_HT_with_close_points );
  param_mut->set_HT_ins_rate( _HT_ins_rate );
  param_mut->set_HT_repl_rate( _HT_repl_rate );
  param_mut->set_repl_HT_detach_rate( _repl_HT_detach_rate );
  param_mut->set_duplication_rate( _duplication_rate );
  param_mut->set_deletion_rate( _deletion_rate );
  param_mut->set_translocation_rate( _translocation_rate );
  param_mut->set_inversion_rate( _inversion_rate );
  param_mut->set_neighbourhood_rate( _neighbourhood_rate );
  param_mut->set_duplication_proportion( _duplication_proportion );
  param_mut->set_deletion_proportion( _deletion_proportion );
  param_mut->set_translocation_proportion( _translocation_proportion );
  param_mut->set_inversion_proportion( _inversion_proportion );

  if (creator)
  {
    int32_t id_new_indiv = 0;
    int nblines=0;
    while (true){
      std::list<char*> gi = creator();
      char* chr = gi.front();
      if (chr==NULL) break;
      int lchromosome = strlen(chr);
      char* chromosome = new char[lchromosome+1];
      strncpy(chromosome, chr, lchromosome+1);

      ae_individual* indiv = new ae_individual( exp_m,
                                               _prng,
                                               _prng,
                                               param_mut,
                                               _w_max,
                                               _min_genome_length,
                                               _max_genome_length,
                                               _allow_plasmids,
                                               id_new_indiv,
                                               0 );

      indiv->add_GU( chromosome, lchromosome );
      indiv->get_genetic_unit(0)->set_min_gu_length(_chromosome_minimal_length);
      indiv->get_genetic_unit(0)->set_max_gu_length(_chromosome_maximal_length);

      char* pla = gi.back();
      if (pla!=NULL)
      {
        if (!_allow_plasmids){
          printf("ERROR: a plasmid was specified but ALLOW_PLASMIDS was set to false\n");
          exit(EXIT_FAILURE);
        }
        int lplasmid = strlen(plasmid);
        char* plasmid = new char[lplasmid+1];
        strncpy(plasmid, pla, lplasmid+1);;
        indiv->add_GU( plasmid, lplasmid );
        indiv->get_genetic_unit(1)->set_min_gu_length(_plasmid_minimal_length);
        indiv->get_genetic_unit(1)->set_max_gu_length(_plasmid_maximal_length);
      }
      indiv->set_with_stochasticity( _with_stochasticity );
      indiv->compute_statistical_data();
      indiv->evaluate( exp_m->get_env() );
      pop->add_indiv( indiv );

      id_new_indiv++;
    }
    pop->sort_individuals();
    printf("Read %d individuals from text file\n",id_new_indiv);
    if (id_new_indiv!=_init_pop_size)
    {
      printf("ERROR: INIT_POP_SIZE incompatible with number of genomes present in input file\n");
      exit(EXIT_FAILURE);
    }
  }
  else if (chromosome != NULL)
  {
    int32_t        id_new_indiv = 0;

    ae_individual* indiv = new ae_individual( exp_m,
                                             _prng,
                                             _prng,
                                             param_mut,
                                             _w_max,
                                             _min_genome_length,
                                             _max_genome_length,
                                             _allow_plasmids,
                                             id_new_indiv++, 0 );

    indiv->add_GU( chromosome, lchromosome );
    indiv->get_genetic_unit(0)->set_min_gu_length(_chromosome_minimal_length);
    indiv->get_genetic_unit(0)->set_max_gu_length(_chromosome_maximal_length);
    
    if (plasmid != NULL)
    {
      if (!_allow_plasmids){
        printf("ERROR: option -p was used but ALLOW_PLASMIDS was set to false\n");
        exit(EXIT_FAILURE);
      }
      indiv->add_GU( plasmid, lplasmid );
      indiv->get_genetic_unit(1)->set_min_gu_length(_plasmid_minimal_length);
      indiv->get_genetic_unit(1)->set_max_gu_length(_plasmid_maximal_length);
    }
    
    indiv->set_with_stochasticity( _with_stochasticity );
    indiv->compute_statistical_data();
    indiv->evaluate( exp_m->get_env() );
    printf("Starting with a clonal population of individual with metabolic error %f and secretion error %f \n",indiv->get_dist_to_target_by_feature(METABOLISM),indiv->get_dist_to_target_by_feature(SECRETION));
    pop->add_indiv( indiv );
    
    // Make the clones and add them to the list of individuals
    ae_individual* clone = NULL;
    for ( int32_t i = 1 ; i < _init_pop_size ; i++ )
    {
      clone = create_clone( indiv, id_new_indiv++ );
      pop->add_indiv( clone );
    }
    pop->sort_individuals();
  }
  else if ( _init_method & ONE_GOOD_GENE )
  {
    ae_individual* indiv        = NULL;
    int32_t        id_new_indiv = 0;

    if ( _init_method & CLONE )
    {
      // Create an individual with a "good" gene (in fact, make an indiv whose
      // fitness is better than that corresponding to a flat phenotype)
      // and set its id
      indiv = create_random_individual_with_good_gene( exp_m, param_mut, id_new_indiv++ );
      indiv->get_genetic_unit(0)->set_min_gu_length(_chromosome_minimal_length);
      indiv->get_genetic_unit(0)->set_max_gu_length(_chromosome_maximal_length);
      if (_allow_plasmids)
      {
        indiv->get_genetic_unit(1)->set_min_gu_length(_plasmid_minimal_length);
        indiv->get_genetic_unit(1)->set_max_gu_length(_plasmid_maximal_length);
      }

      indiv->set_with_stochasticity( _with_stochasticity );
      
      // Add it to the list
      pop->add_indiv( indiv );
    
      // Make the clones and add them to the list of individuals
      ae_individual* clone = NULL;
      for ( int32_t i = 1 ; i < _init_pop_size ; i++ )
      {
        // Create a clone, setting its id
        clone = create_clone( indiv, id_new_indiv++ );
        
        #ifdef DISTRIBUTED_PRNG
          #error Not implemented yet !
          indiv->do_prng_jump();
        #endif
        
        // Add it to the list
        pop->add_indiv( clone );
      }
      pop->sort_individuals();
    }
    else // if ( ! CLONE )
    {
      for ( int32_t i = 0 ; i < _init_pop_size ; i++ )
      {
        // Create an individual and set its id
        indiv = create_random_individual_with_good_gene( exp_m, param_mut, id_new_indiv++ );
        indiv->get_genetic_unit(0)->set_min_gu_length(_chromosome_minimal_length);
        indiv->get_genetic_unit(0)->set_max_gu_length(_chromosome_maximal_length);
        if (_allow_plasmids)
        {
          indiv->get_genetic_unit(1)->set_min_gu_length(_plasmid_minimal_length);
          indiv->get_genetic_unit(1)->set_max_gu_length(_plasmid_maximal_length);
        }

        // Add it to the list
        pop->add_indiv( indiv );
      }
      
      pop->sort_individuals();
    }
  }
  else // if ( ! ONE_GOOD_GENE )
  {
    ae_individual* indiv        = NULL;
    int32_t        id_new_indiv = 0;

    if ( _init_method & CLONE )
    {
      // Create a random individual and set its id
      indiv = create_random_individual( exp_m, param_mut, id_new_indiv++ );
      indiv->get_genetic_unit(0)->set_min_gu_length(_chromosome_minimal_length);
      indiv->get_genetic_unit(0)->set_max_gu_length(_chromosome_maximal_length);
      if (_allow_plasmids)
      {
        indiv->get_genetic_unit(1)->set_min_gu_length(_plasmid_minimal_length);
        indiv->get_genetic_unit(1)->set_max_gu_length(_plasmid_maximal_length);
      }

      // Add it to the list
      pop->add_indiv( indiv );
      
      // Make the clones and add them to the list of individuals
      ae_individual* clone = NULL;
      for ( int32_t i = 1 ; i < _init_pop_size ; i++ )
      {
        // Create a clone, setting its id
        clone = create_clone( indiv, id_new_indiv++ );

        #ifdef DISTRIBUTED_PRNG
          #error Not implemented yet !
          indiv->do_prng_jump();
        #endif
        
        // Add it to the list
        pop->add_indiv( clone );
      }
      pop->sort_individuals();
    }
    else // if ( ! CLONE )
    {
      for ( int32_t i = 0 ; i < _init_pop_size ; i++ )
      {
        // Create a random individual and set its id
        indiv = create_random_individual( exp_m, param_mut, id_new_indiv++ );
        indiv->get_genetic_unit(0)->set_min_gu_length(_chromosome_minimal_length);
        indiv->get_genetic_unit(0)->set_max_gu_length(_chromosome_maximal_length);
        if (_allow_plasmids)
        {
          indiv->get_genetic_unit(1)->set_min_gu_length(_plasmid_minimal_length);
          indiv->get_genetic_unit(1)->set_max_gu_length(_plasmid_maximal_length);
        }

        // Add it to the list
        pop->add_indiv( indiv );
      }
      
      pop->sort_individuals();
    }
  }
  
  // If the population is spatially structured, set each individual's position
  if ( exp_m->is_spatially_structured() )
  {
    int16_t x, y;
    int16_t x_max = exp_m->get_grid_width();
    int16_t y_max = exp_m->get_grid_height();
    ae_grid_cell* grid_cell = NULL;
    
    ae_list_node<ae_individual*>* indiv_node = pop->get_indivs()->get_first();
    ae_individual*  indiv = NULL;
    
    while ( indiv_node != NULL )
    {
      indiv = indiv_node->get_obj();
      
      do
      {
        x = _prng->random( x_max );
        y = _prng->random( y_max );
        grid_cell = exp_m->get_grid_cell( x, y );
      } while ( grid_cell->get_individual() != NULL );
      
      grid_cell->set_individual( indiv );
      
      indiv_node = indiv_node->get_next();
    }
  }
  
  
  
  // 4) ------------------------------------------ Set the recording parameters
  output_m->set_backup_step( _backup_step );
  output_m->set_big_backup_step( _big_backup_step );
  
  if ( _record_tree )
  {
    output_m->init_tree( exp_m, _tree_mode, _tree_step );
  }
  
  if ( _make_dumps )
  {
    output_m->set_dump_step( _dump_step );
  }
  output_m->set_logs( _logs );
  
  delete param_mut;
  delete _prng; // Each class that needed it has now its own copy
  delete _mut_prng;
  delete _stoch_prng;
}



// =================================================================
//                           Protected Methods
// =================================================================
/*!
  \brief Format a line by parsing it and the words inside
  
  \param formated_line the resulted formated line
  \param line original line in char*
  \param line_is_interpretable boolean with about the possible intrepretation of the line
*/
void param_loader::format_line( f_line* formated_line, char* line, bool* line_is_interpretable )
{
  int16_t i = 0;
  int16_t j;

  // Parse line
  while ( line[i] != '\n' && line[i] != '\0' && line[i] != '\r' )
  {
    j = 0;

    // Flush white spaces and tabs
    while ( line[i] == ' ' || line[i] == 0x09 ) i++; // 0x09 is the ASCII code for TAB

    // Check comments
    if ( line[i] == '#' ) break;

    // If we got this far, there is content in the line
    *line_is_interpretable = true;

    // Parse word
    while ( line[i] != ' '  && line[i] != '\n' && line[i] != '\0' && line[i] != '\r' )
    {
      formated_line->words[formated_line->nb_words][j++] = line[i++];
    }

    // Add '\0' at end of word if it's not empty (line ending with space or tab)
    if ( j != 0 )
    {
      formated_line->words[formated_line->nb_words++][j] = '\0';
    }
  }
}

/*!
  \brief Get a line in a file and format it
  
  \return line (pointer)
  
  \see format_line(f_line* formated_line, char* line, bool* line_is_interpretable )
*/
f_line* param_loader::get_line( int32_t* cur_line_ptr ) // void
{
  char line[255];
  f_line* formated_line = new f_line();

  bool found_interpretable_line = false; // Found line that is neither a comment nor empty

  while ( !feof( _param_file ) && !found_interpretable_line )
  {
    if ( !fgets( line, 255, _param_file ) )
    {
      delete formated_line;
      return NULL;
    }
    (*cur_line_ptr)++;
    format_line( formated_line, line, &found_interpretable_line );
  }

  if ( found_interpretable_line )
  {
    return formated_line;
  }
  else
  {
    delete formated_line;
    return NULL;
  }
}

/*!
  \brief Create an individual with random sequences
  
  \param exp_m global exp_manager
  \param param_mut mutation parameter of the newly created individual
  \param id index of newly created individual in the population
  \return clone of dolly
*/
ae_individual* param_loader::create_random_individual( ae_exp_manager* exp_m, ae_params_mut* param_mut, int32_t id ) const
{
  // Generate a random genome
  char * random_genome = new char [_chromosome_initial_length + 1];
  for ( int32_t i = 0 ; i < _chromosome_initial_length ; i++ )
  {
    random_genome[i] = '0' + _prng->random( NB_BASE );
  }
  random_genome[_chromosome_initial_length] = 0;
  
  
  // ------------------------------------------------------- Global constraints
  // Create an individual with this genome and set its id
  #ifdef DISTRIBUTED_PRNG
    #error Not implemented yet !
  #endif
  ae_individual* indiv = new ae_individual( exp_m,
                                            _prng,
                                            _prng,
                                            param_mut,
                                            _w_max,
                                            _min_genome_length,
                                            _max_genome_length,
                                            _allow_plasmids,
                                            id, 0 );
                                            
  // <Graphical debug>
  //~ #ifdef __X11
    //~ indiv = new ae_individual_X11( exp_m, new ae_jumping_mt(*_prng), param_mut, _param_values->_w_max, id, 0 );
  //~ #else
    //~ indiv = new ae_individual( exp_m, new ae_jumping_mt(*_prng), param_mut, _param_values->_w_max, id, 0 );
  //~ #endif
  // </Graphical debug>
  indiv->add_GU( random_genome, _chromosome_initial_length );
  
  if (_allow_plasmids) // We create a plasmid
  {
    char * plasmid_genome;
    if ( _plasmid_initial_gene == 1 ) // Then the plasmid is generated independently from the chromosome
    {
      plasmid_genome = new char [_plasmid_initial_length + 1]; // As ae_dna constructor do not allocate memory but directly use the provided string, we allocate the memory here.
      for ( int32_t i = 0 ; i < _plasmid_initial_length ; i++ )
      {
        plasmid_genome[i] = '0' + _prng->random( NB_BASE );
      }
      plasmid_genome[_plasmid_initial_length] = 0;
      indiv->add_GU( plasmid_genome, _plasmid_initial_length );
    }
    else // The plasmid has the same genome than the chromosome
    {
      plasmid_genome = new char [_chromosome_initial_length + 1]; // As ae_dna constructor do not allocate memory but directly use the provided string, we allocate the memory here.
      strncpy(plasmid_genome,random_genome,_chromosome_initial_length+1);
      indiv->add_GU( plasmid_genome, _chromosome_initial_length );
    }
    plasmid_genome = NULL; // should not be deleted since it is now the plasmid dna
  }
  
  random_genome = NULL; // should not be deleted since it is now the chromosomal dna
  
  // Insert a few IS in the sequence
  /*if ( ae_common::init_params->get_init_method() & WITH_INS_SEQ )
  {
    // Create a random sequence
    int32_t seq_len = 50;
    char* ins_seq = new char[seq_len+1];
    int16_t nb_insert = 50;
    int16_t nb_invert = 50;
    
    for ( int32_t i = 0 ; i < seq_len ; i++ )
    {
      ins_seq[i] = '0' + ae_common::sim->prng->random( NB_BASE );
    }
    ins_seq[seq_len] = '\0';
    
    
    // Insert the sequence at random positions
    ae_mutation* mut1 = NULL;
    for ( int16_t i = 0 ; i < nb_insert ; i++ )
    {
      mut1 = indiv->get_genetic_unit(0)->get_dna()->do_insertion( ins_seq, seq_len );
      delete mut1;
    }
    
    
    // Invert the sequence and insert it at random positions
    char* inverted_seq = new char[seq_len+1];
    for ( int32_t i = 0 ; i < seq_len ; i++ )
    {
      inverted_seq[i] = (ins_seq[seq_len-1-i] == '1') ? '0' : '1';
    }
    inverted_seq[seq_len] = '\0';
    
    for ( int16_t i = 0 ; i < nb_invert ; i++ )
    {
      mut1 = indiv->get_genetic_unit(0)->get_dna()->do_insertion( inverted_seq, seq_len );
      delete mut1;
    }
    
    delete [] ins_seq;
    delete [] inverted_seq;
  }*/
  
  // Evaluate the newly created individual
  indiv->evaluate( exp_m->get_env() );
  
  return indiv;
}

/*!
  \brief Create an individual with random sequences. The individual have to have at least one good functional gene
  
  \param exp_m global exp_manager
  \param param_mut mutation parameter of the newly created individual
  \param id index of newly created individual in the population
  \return clone of dolly
*/
ae_individual* param_loader::create_random_individual_with_good_gene( ae_exp_manager* exp_m, ae_params_mut* param_mut, int32_t id ) const
{
  // Create a random individual and evaluate it
  ae_individual* indiv = create_random_individual( exp_m, param_mut, id );
  
  // While the created individual is not better than the flat individual (indiv whith no metabolic gene),
  // we delete it and replace it by another random individual
  double env_metabolic_area;
  
  env_metabolic_area = exp_m->get_env()->get_area_by_feature( METABOLISM );

  // If there are plasmids, make sure there is at least one metabolic gene on each genetic units
  if ( _allow_plasmids )
  {
    if ( _plasmid_initial_gene == 1 ) // two independently generated genetic units
    {
      while ( indiv->get_genetic_unit(0)->get_dist_to_target_by_feature( METABOLISM ) >= env_metabolic_area  ||
              indiv->get_genetic_unit(1)->get_dist_to_target_by_feature( METABOLISM ) >= env_metabolic_area  )
      {
        delete indiv;
        indiv = create_random_individual( exp_m, param_mut, id );
      }
      // TODO: not very smart, because the chances of randomly creating two good genes at the same time are very low! We should totally change the way plasmids are handled in this part of the initialization code.
    }
    else // it is either 0 (generate one good gene (metabolic if there is metabolism, secretion otherwise) on the chromosome and copy it to the plasmid), either 2 (generate one good metabolic gene on the chromosome and one secretion gene on the plasmid)
    {
      // here things work the same as before, but in the constructor of the individual, 
      // a single genetic unit is created and then copied from the chromosome to the plasmid
      if ( exp_m->get_env()->get_area_by_feature(METABOLISM)!=0.0 )
      {
        while ( indiv->get_dist_to_target_by_feature( METABOLISM ) >= env_metabolic_area )
        {
          delete indiv;
          indiv = create_random_individual( exp_m, param_mut, id );
        }
      }
      else // In case there is no metabolic target, we create a secretion gene
      {
        while ( indiv->get_dist_to_target_by_feature( SECRETION ) >= exp_m->get_env()->get_area_by_feature( SECRETION ) )
        {
          delete indiv;
          indiv = create_random_individual( exp_m, param_mut, id );
        }
      }
    }
    if ( _plasmid_initial_gene == 2 ) // only valid if there is a metabolic and a secretion target
    {
      if (exp_m->get_env()->get_area_by_feature( METABOLISM )==0.0)
      {
        printf( "ERROR : plasmid_initial_gene is 2 but there is no metabolic target\n" );
        exit( EXIT_FAILURE );
      }
      if (exp_m->get_env()->get_area_by_feature( SECRETION )==0.0)
      {
        printf( "ERROR : plasmid_initial_gene is 2 but there is no secretion target\n" );
        exit( EXIT_FAILURE );
      }
      // We already have one chromosome with a good metabolic gene
      
      // We now create one other genetic unit with a good secretion gene in an independant individual
      double env_secretion_area = exp_m->get_env()->get_area_by_feature( SECRETION );
      ae_individual* indivbis = create_random_individual( exp_m, param_mut, id );
      while ( indivbis->get_dist_to_target_by_feature(SECRETION) >= env_secretion_area )
      {
        delete indivbis;
        indivbis = create_random_individual( exp_m, param_mut, id );
      }
      
      // We remove plasmid from indiv, extract plasmid from indivbis and insert it as plasmid in indiv
      indiv->get_genetic_unit_list()->remove(indiv->get_genetic_unit_list()->get_last(),true,true);
      indiv->inject_GU(indivbis);
      
      // We check that there is no negative epistasis between our two genes
      indiv->evaluate( exp_m->get_env() );
      assert( (indiv->get_dist_to_target_by_feature(SECRETION) < env_secretion_area) && (indiv->get_dist_to_target_by_feature(METABOLISM) < env_metabolic_area) );
      delete indivbis;
    }
  }
  else
  {
    if ( exp_m->get_env()->get_area_by_feature(METABOLISM)!=0.0 )
    {
      while ( indiv->get_dist_to_target_by_feature( METABOLISM ) >= env_metabolic_area )
      {
        delete indiv;
        indiv = create_random_individual( exp_m, param_mut, id );
      }
    }
    else
    {
      while ( indiv->get_dist_to_target_by_feature( SECRETION ) >= exp_m->get_env()->get_area_by_feature( SECRETION ) )
      {
        delete indiv;
        indiv = create_random_individual( exp_m, param_mut, id );
      }
    }
    
  }
  
  // Compute the "good" individual's statistics
  indiv->compute_statistical_data();
  
  //~ printf( "metabolic error of the generated individual : %f (%"PRId32" gene(s))\n",
          //~ indiv->get_dist_to_target_by_feature(METABOLISM), indiv->get_protein_list()->get_nb_elts() );
  
  return indiv;
}

/*!
  \brief Create of clone of an ae_individual 
  
  \param dolly original individual that would be cloned
  \param id index of the clone in the population
  \return clone of dolly
*/
ae_individual* param_loader::create_clone( ae_individual* dolly, int32_t id ) const
{
  ae_individual* indiv;
  
  indiv = new ae_individual( *dolly );

  
  //~ #ifdef __X11
    //~ indiv = new ae_individual_X11( *(dynamic_cast<ae_individual_X11*>(dolly)) );
  //~ #else
    //~ indiv = new ae_individual( *dolly );
  //~ #endif
  
  indiv->set_id( id );
  //~ printf( "metabolic error of the clonal individual : %f (%"PRId32" gene(s))\n",
          //~ indiv->get_dist_to_target_by_feature(METABOLISM), indiv->get_protein_list()->get_nb_elts());
  return indiv;
}

void param_loader::print_to_file( FILE* file )
{
  // ------------------------------------------------------------ Constraints
  fprintf( file, "\nConstraints ---------------------------------------------\n" );
  fprintf( file, "min_genome_length :          %"PRId32"\n", _min_genome_length       );
  fprintf( file, "max_genome_length :          %"PRId32"\n", _max_genome_length       );
  fprintf( file, "W_MAX :                      %f\n",        _w_max                   );
  
  // --------------------------------------------------------- Mutation rates
  fprintf( file, "\nMutation rates ------------------------------------------\n" );
  fprintf( file, "point_mutation_rate :        %e\n",  _point_mutation_rate        );
  fprintf( file, "small_insertion_rate :       %e\n",  _small_insertion_rate       );
  fprintf( file, "small_deletion_rate :        %e\n",  _small_deletion_rate        );
  fprintf( file, "max_indel_size :             %"PRId16"\n", _max_indel_size       );
  
  // -------------------------------------------- Rearrangements and Transfer
  fprintf( file, "\nRearrangements and Transfer -----------------------------\n" );
  fprintf( file, "with_4pts_trans :            %s\n",  _with_4pts_trans? "true" : "false" );
  fprintf( file, "with_alignments :            %s\n",  _with_alignments? "true" : "false" );
  fprintf( file, "with_HT :                    %s\n",  _with_HT? "true" : "false"   );
  fprintf( file, "repl_HT_with_close_points :  %s\n",  _repl_HT_with_close_points? "true" : "false"   );
  fprintf( file, "HT_ins_rate :                %e\n",  _HT_ins_rate );
  fprintf( file, "HT_repl_rate :               %e\n",  _HT_repl_rate );
  
  // ---------------------------------------------------- Rearrangement rates
  if ( _with_alignments )
  {
    fprintf( file, "\nRearrangement rates (with alignements) ------------------\n" );
    fprintf( file, "neighbourhood_rate :         %e\n",  _neighbourhood_rate         );
    fprintf( file, "duplication_proportion :     %e\n",  _duplication_proportion     );
    fprintf( file, "deletion_proportion :        %e\n",  _deletion_proportion        );
    fprintf( file, "translocation_proportion :   %e\n",  _translocation_proportion   );
    fprintf( file, "inversion_proportion :       %e\n",  _inversion_proportion       );
  }
  else
  {
    fprintf( file, "\nRearrangement rates (without alignements) ----------------\n" );
    fprintf( file, "duplication_rate :           %e\n",  _duplication_rate           );
    fprintf( file, "deletion_rate :              %e\n",  _deletion_rate              );
    fprintf( file, "translocation_rate :         %e\n",  _translocation_rate         );
    fprintf( file, "inversion_rate :             %e\n",  _inversion_rate             );
  }
  
  // ------------------------------------------------------------ Alignements
  fprintf( file, "\nAlignements ---------------------------------------------\n" );
  fprintf( file, "align_fun_shape :            %"PRId16"\n", (int16_t) _align_fun_shape       );
  fprintf( file, "align_sigm_lambda :          %f\n",        _align_sigm_lambda     );
  fprintf( file, "align_sigm_mean :            %"PRId16"\n", _align_sigm_mean       );
  fprintf( file, "align_lin_min :              %"PRId16"\n", _align_lin_min         );
  fprintf( file, "align_lin_max :              %"PRId16"\n", _align_lin_max         );
  fprintf( file, "align_max_shift :            %"PRId16"\n", _align_max_shift       );
  fprintf( file, "align_w_zone_h_len :         %"PRId16"\n", _align_w_zone_h_len    );
  fprintf( file, "align_match_bonus :          %"PRId16"\n", _align_match_bonus     );
  fprintf( file, "align_mismatch_cost :        %"PRId16"\n", _align_mismatch_cost   );
  
  // -------------------------------------------------------------- Selection
  fprintf( file, "\nSelection -----------------------------------------------\n" );
  switch ( _selection_scheme )
  {
    case RANK_LINEAR :
    {
      fprintf( file, "selection_scheme :           RANK_LINEAR\n" );
      break;
    }
    case RANK_EXPONENTIAL :
    {
      fprintf( file, "selection_scheme :           RANK_EXPONENTIAL\n" );
      break;
    }
    case FITNESS_PROPORTIONATE :
    {
      fprintf( file, "selection_scheme :           FITNESS_PROPORTIONATE\n" );
      break;
    }
    case FITTEST :
    {
      fprintf( file, "selection_scheme :           FITTEST\n" );
      break;
    }
    default :
    {
      fprintf( file, "selection_scheme :           UNKNOWN\n" );
      break;
    }
  }
  fprintf( file, "selection_pressure :         %e\n",  _selection_pressure );
  
  
  // -------------------------------------------------------------- Secretion
  fprintf( file, "\nSecretion -----------------------------------------------\n" );
  fprintf( file, "with_secretion :                %s\n", _with_secretion? "true" : "false" );
  fprintf( file, "secretion_contrib_to_fitness :  %e\n", _secretion_contrib_to_fitness    );
  fprintf( file, "secretion_diffusion_prop :      %e\n", _secretion_diffusion_prop        );
  fprintf( file, "secretion_degradation_prop :    %e\n", _secretion_degradation_prop      );
  fprintf( file, "secretion_cost :                %e\n", _secretion_cost                  );
  
  // --------------------------------------------------------------- Plasmids
  fprintf( file, "\nPlasmids ------------------------------------------------\n" );
  fprintf( file, "allow_plasmids :             %s\n", _allow_plasmids? "true" : "false"              );
  fprintf( file, "plasmid_minimal_length :     %"PRId32"\n", _plasmid_minimal_length                 );
  fprintf( file, "plasmid_maximal_length :     %"PRId32"\n", _plasmid_maximal_length                 );
  fprintf( file, "chromosome_minimal_length :  %"PRId32"\n", _chromosome_minimal_length              );
  fprintf( file, "chromosome_maximal_length :  %"PRId32"\n", _chromosome_maximal_length              );
  fprintf( file, "prob_plasmid_HT :            %e\n", _prob_plasmid_HT                               );
  fprintf( file, "tune_donor_ability :         %e\n", _tune_donor_ability                            );
  fprintf( file, "tune_recipient_ability :     %e\n", _tune_recipient_ability                        );
  fprintf( file, "donor_cost :                 %e\n", _donor_cost                                    );
  fprintf( file, "recipient_cost :             %e\n", _recipient_cost                                );
  fprintf( file, "compute_phen_contrib_by_GU : %s\n", _compute_phen_contrib_by_GU? "true" : "false"  );
  fprintf( file, "swap_GUs :                   %s\n",  _swap_GUs? "true" : "false"   );
  
  // ------------------------------------------------------- Translation cost
  fprintf( file, "\nTranslation cost ----------------------------------------\n" );
  fprintf( file, "translation_cost :           %e\n",  _translation_cost           );
}

