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

 
const char* DEFAULT_PARAM_FILE_NAME = "param.in";
 
 
// =================================================================
//                              Libraries
// =================================================================
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



// =================================================================
//                            Project Files
// =================================================================
#include <f_line.h>
#include <ae_population.h>
#ifdef __X11
  #include <ae_exp_manager_X11.h>
#else
  #include <ae_exp_manager.h>
#endif

// =================================================================
//                         Function declarations
// =================================================================
enum population_change_type
{
  SUBPOPULATIONS_BASED_ON_NON_CODING_BASES = 0,
  REMOVE_NON_CODING_BASES_BEST_IND = 1,
  REMOVE_NON_CODING_BASES_POPULATION = 2,
  DOUBLE_NON_CODING_BASES_BEST_IND = 3,
  DOUBLE_NON_CODING_BASES_POPULATION = 4
};

void print_help( char* prog_path );
void print_version( void );

f_line* get_line( FILE* param_file );
void format_line( f_line* formated_line, char* line, bool* line_is_interpretable );
void change_based_on_non_coding_bases_of_best_individual_ancestor(ae_population* pop, ae_exp_manager* exp_m, population_change_type type);
void change_based_on_non_coding_bases_in_population(ae_population* pop, ae_exp_manager* exp_m, population_change_type type);
ae_individual* create_clone( ae_individual* dolly, int32_t id );



int main( int argc, char* argv[] )
{
  // 1) Initialize command-line option variables with default values
  char* param_file_name = NULL;
  bool verbose          = false;
  int32_t num_gener = -1;  
  
  // 2) Define allowed options
  const char * options_list = "hf:g:V";
  static struct option long_options_list[] = {
    { "help",     no_argument,        NULL, 'h' },
    { "file",     required_argument,  NULL, 'f' }, // Provide file with parameters to change
    { "gener",    required_argument,  NULL, 'g' },
    { "version",  no_argument,        NULL, 'V' },
    { 0, 0, 0, 0 }
  };
      
  // 3) Get actual values of the command-line options
  int option;
  while ( ( option = getopt_long(argc, argv, options_list, long_options_list, NULL) ) != -1 ) 
  {
    switch ( option ) 
    {
      case 'h' :
      {
        print_help( argv[0] );
        exit( EXIT_SUCCESS );
      }
      case 'V' :
      {
        print_version();
        exit( EXIT_SUCCESS );
      }
      case 'f' :
      {
        if ( strcmp( optarg, "" ) == 0 )
        {
          printf( "%s: error: Option -f or --file : missing argument.\n", argv[0] );
          exit( EXIT_FAILURE );
        }
        
        param_file_name = optarg;
        break;
      }
      case 'g' :
      {
        num_gener = atoi( optarg );
        break;
      }
      default :
      {
        // An error message is printed in getopt_long, we just need to exit
        exit( EXIT_FAILURE );
      }
    }
  }  
  
  // 4) Set undefined command line parameters to default values
  if ( param_file_name == NULL )
  {
    param_file_name = new char[strlen(DEFAULT_PARAM_FILE_NAME)+1];
    sprintf( param_file_name, "%s", DEFAULT_PARAM_FILE_NAME );
  }
  
  // 5) Check the consistancy of the command-line options
  if ( num_gener == -1 )
  {
    printf( "%s: error: You must provide a generation number.\n", argv[0] );
    exit( EXIT_FAILURE );
  }
  
  // 6) Initialize the experiment manager
  #ifndef __NO_X
    ae_exp_manager* exp_manager = new ae_exp_manager_X11();
  #else
    ae_exp_manager* exp_manager = new ae_exp_manager();
  #endif
  exp_manager->load( num_gener, false, verbose );

  // 7) Retrieve the population, the environment, the selection,...
  ae_population* pop = exp_manager->get_pop();
  ae_environment* env = exp_manager->get_env();
  ae_selection* sel = exp_manager->get_sel();
  
  // 8) Interpret and apply changes
  printf("Interpret and apply changes\n");
  FILE* param_file  = fopen( param_file_name,  "r" );
  if ( param_file == NULL )
  {
    printf( "%s:%d: error: could not open parameter file %s\n", __FILE__, __LINE__, param_file_name );
    exit( EXIT_FAILURE );
  }
  
  bool env_change = false;
  bool env_hasbeenmodified = false;
  bool trait_gu_location_hasbeenmodified = false;
  bool record_tree = false;
  int32_t tree_step = 100;
  ae_tree_mode tree_mode = NORMAL;
  int16_t* new_trait_gu_location = new int16_t[NB_FEATURES];
  int16_t i;
  for (i=0; i<NB_FEATURES; i++)
  {
    new_trait_gu_location[i]=0;
  }

  f_line* line;
  int32_t cur_line = 0;
  while ( ( line = get_line(param_file) ) != NULL ) 
  {
    cur_line++;
    if ( strcmp( line->words[0], "ENV_AXIS_FEATURES" ) == 0 )
    {
      int16_t env_axis_nb_segments = line->nb_words / 2;
      double* env_axis_segment_boundaries = new double [env_axis_nb_segments + 1];
      env_axis_segment_boundaries[0] = X_MIN;
      for ( int16_t i = 1 ; i < env_axis_nb_segments ; i++ )
      {
        env_axis_segment_boundaries[i] = atof( line->words[2*i] );
      }
      env_axis_segment_boundaries[env_axis_nb_segments] = X_MAX;
      
      // Set segment features
      ae_env_axis_feature* env_axis_features = new ae_env_axis_feature[env_axis_nb_segments];
      for ( int16_t i = 0 ; i < env_axis_nb_segments ; i++ )
      {
        if ( strcmp( line->words[2*i+1], "NEUTRAL" ) == 0 )
        {
          env_axis_features[i] = NEUTRAL;
        }
        else if ( strcmp( line->words[2*i+1], "METABOLISM" ) == 0 )
        {
          env_axis_features[i] = METABOLISM;
        }
        else if ( strcmp( line->words[2*i+1], "SECRETION" ) == 0 )
        {
          exp_manager->get_exp_s()->set_with_secretion( true );
          env_axis_features[i] = SECRETION;
        }
        else if ( strcmp( line->words[2*i+1], "DONOR" ) == 0 )
        {
          env_axis_features[i] = DONOR;
        }
        else if ( strcmp( line->words[2*i+1], "RECIPIENT" ) == 0 )
        {
          env_axis_features[i] = RECIPIENT;
        }
        else
        {
          printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown axis feature \"%s\".\n",
                  param_file_name, cur_line, line->words[2*i+1] );
          exit( EXIT_FAILURE );
        }
      }
      env->set_segmentation( env_axis_nb_segments,
                             env_axis_segment_boundaries,
                             env_axis_features );
      env_hasbeenmodified = true;
      delete env_axis_segment_boundaries;
      delete env_axis_features;
    }
    else if ( strcmp( line->words[0], "TRAIT_GU_LOCATION" ) == 0)
    {
      if ( strcmp( line->words[1], "NEUTRAL" ) == 0 )
      {
        new_trait_gu_location[NEUTRAL] = atoi( line->words[2] );
      }
      else if ( strcmp( line->words[1], "METABOLISM" ) == 0 )
      {
        new_trait_gu_location[METABOLISM] = atoi( line->words[2] );
      }
      else if ( strcmp( line->words[1], "SECRETION" ) == 0 )
      {
        new_trait_gu_location[SECRETION] = atoi( line->words[2] );
      }
      else if ( strcmp( line->words[1], "DONOR" ) == 0 )
      {
        new_trait_gu_location[DONOR] = atoi( line->words[2] );
      }
      else if ( strcmp( line->words[1], "RECIPIENT" ) == 0 )
      {
        new_trait_gu_location[RECIPIENT] = atoi( line->words[2] );
      }
      else
      {
        printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown axis feature \"%s\".\n",
               param_file_name, cur_line, line->words[2] );
        exit( EXIT_FAILURE );
      }
      trait_gu_location_hasbeenmodified=true;
    }
    else if ( strcmp( line->words[0], "ISOLATE_GUS" ) == 0 )
    {
      exp_manager->get_exp_s()->set_isolate_GUs(true);
    }
    else if (strcmp(line->words[0], "BREAK_LINKAGE") == 0 )
    {
      exp_manager->get_exp_s()->set_break_linkage(atoi( line->words[1] ), atoi( line->words[2] ));
    }
    else if ( strcmp( line->words[0], "RECORD_TREE" ) == 0 )
    {
      if ( strncmp( line->words[1], "true", 4 ) == 0 )
      {
        record_tree = true;
      }
      else if ( strncmp( line->words[1], "false", 5 ) == 0 )
      {
        printf( "ERROR stop recording tree is not implemented yet.\n");
        exit(EXIT_FAILURE);
      }
      else
      {
        printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown tree recording option (use true/false).\n",
               param_file_name, cur_line );
        exit( EXIT_FAILURE );
      }
      if (exp_manager->get_output_m()->get_record_tree())
      {
        printf( "ERROR modification of already existing tree not impemented yet\n" );
        exit(EXIT_FAILURE);
      }
    }
    else if ( strcmp( line->words[0], "TREE_STEP" ) == 0 )
    {
      tree_step = atol( line->words[1] );
    }
    else if ( strcmp( line->words[0], "TREE_MODE" ) == 0 )
    {
      if ( strcmp( line->words[1], "light" ) == 0 )
      {
        tree_mode = LIGHT;
      }
      else if ( strcmp( line->words[1], "normal" ) == 0 )
      {
        tree_mode = NORMAL;
      }
      else
      {
        printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown tree mode option (use normal/light).\n",
               param_file_name, cur_line );
        exit( EXIT_FAILURE );
      }
    }
    else if ( strcmp( line->words[0], "BACKUP_STEP" ) == 0 )
    {
      exp_manager->get_output_m()->set_backup_step( atol( line->words[1] ) );
    }
    else if ( strcmp( line->words[0], "BIG_BACKUP_STEP" ) == 0 )
    {
      exp_manager->get_output_m()->set_big_backup_step( atol( line->words[1] ) );
    }
    else if ( strcmp( line->words[0], "POPULATION_SIZE") == 0 )
    {
    	pop->set_nb_indivs(atol( line->words[1] ) );
      printf("\tChange of population size to %ld\n",atol( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "SELECTION_SCHEME" ) == 0 )
    {
      if ( strncmp( line->words[1], "lin", 3 ) == 0 )
      {
        sel->set_selection_scheme(RANK_LINEAR);
      }
      else if ( strncmp( line->words[1], "exp", 3 ) == 0 )
      {
        sel->set_selection_scheme(RANK_EXPONENTIAL);
      }
      else if ( strncmp( line->words[1], "fitness", 7 ) == 0 )
      {
        sel->set_selection_scheme(FITNESS_PROPORTIONATE);
      }
      else if ( strcmp( line->words[1], "fittest" ) == 0 )
      {
        sel->set_selection_scheme(FITTEST);
      }
      else
      {
        printf( "ERROR in param file \"%s\" on line %"PRId32" : unknown selection scheme \"%s\".\n",
               param_file_name, cur_line, line->words[1] );
        exit( EXIT_FAILURE );
      }
    }
    else if ( strcmp( line->words[0], "SELECTION_PRESSURE") == 0 )
    {
    	sel->set_selection_pressure(atof( line->words[1] ) );
      printf("\tChange of selection pressure to %f\n",atof( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "POINT_MUTATION_RATE" ) == 0 )
    {
      pop->set_overall_point_mutation_rate( atof( line->words[1] ) );
      printf("\tChange of overall point mutation rate to %f\n",atof( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "SMALL_INSERTION_RATE" ) == 0 )
    {
      pop->set_overall_small_insertion_rate( atof( line->words[1] ) );
      printf("\tChange of overall small insertion rate to %f\n",atof( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "SMALL_DELETION_RATE" ) == 0 )
    {
      pop->set_overall_small_deletion_rate( atof( line->words[1] ) );
      printf("\tChange of overall small deletion rate to %f\n",atof( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "MAX_INDEL_SIZE" ) == 0 )
    {
      pop->set_overall_max_indel_size( atol( line->words[1] ) );
      printf("\tChange of overall maximum indel size to %f\n",atof( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "DUPLICATION_RATE" ) == 0 )
    {
      pop->set_overall_duplication_rate( atof( line->words[1] ) );
      printf("\tChange of overall duplication rate to %f\n",atof( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "DELETION_RATE" ) == 0 )
    {
      pop->set_overall_deletion_rate( atof( line->words[1] ) );
      printf("\tChange of overall deletion rate to %f\n",atof( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "TRANSLOCATION_RATE" ) == 0 )
    {
      pop->set_overall_translocation_rate( atof( line->words[1] ) );
      printf("\tChange of overall translocation rate to %f\n",atof( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "INVERSION_RATE" ) == 0 )
    {
      pop->set_overall_inversion_rate( atof( line->words[1] ) );
      printf("\tChange of overall inversion to %f\n",atof( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "TRANSFER_INS_RATE" ) == 0 )
    {
      pop->set_overall_transfer_ins_rate( atof( line->words[1] ) );
      printf("\tChange of overall transfer insertion rate to %f\n",atof( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "TRANSFER_REPL_RATE" ) == 0 )
    {
      pop->set_overall_transfer_repl_rate( atof( line->words[1] ) );
      printf("\tChange of overall transfer replacement rate to %f\n",atof( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "ENV_ADD_GAUSSIAN" ) == 0 )
    {
      if ( env_change)
      {
        env->add_gaussian( atof(line->words[1]), atof(line->words[2]), atof(line->words[3]));
        printf("\tAddition of a gaussian with %f, %f, %f \n",atof(line->words[1]), atof(line->words[2]), atof(line->words[3]));
      }
      else
      {
        env->clear_gaussians();
        env->clear_initial_gaussians();
        env->set_gaussians(new ae_list<ae_gaussian*>());
        env->add_gaussian( atof(line->words[1]), atof(line->words[2]), atof(line->words[3]));
        printf("\tChange of the environment: first gaussian with %f, %f, %f \n",atof(line->words[1]), atof(line->words[2]), atof(line->words[3]));
        env_change = true;
      }
      env_hasbeenmodified = true;
    }
    else if ( strcmp( line->words[0], "ENV_GAUSSIAN" ) == 0 )
    {
      if ( env_change)
      {
        env->add_gaussian( atof(line->words[1]), atof(line->words[2]), atof(line->words[3]));
        printf("\tAddition of a gaussian with %f, %f, %f \n",atof(line->words[1]), atof(line->words[2]), atof(line->words[3]));
      }
      else
      {
        env->clear_gaussians();
        env->clear_initial_gaussians();
        env->set_gaussians(new ae_list<ae_gaussian*>());
        env->add_gaussian( atof(line->words[1]), atof(line->words[2]), atof(line->words[3]));
        printf("\tChange of the environment: first gaussian with %f, %f, %f \n",atof(line->words[1]), atof(line->words[2]), atof(line->words[3]));
        env_change = true;
      }
      env_hasbeenmodified = true;
    }
    else if ( strcmp( line->words[0], "ENV_VARIATION" ) == 0 )
    {
      static bool env_var_already_set = false;
      if ( env_var_already_set )
      {
        printf( "%s:%d: ERROR in param file : duplicate entry for %s.\n", __FILE__, __LINE__, line->words[0] );
        exit( EXIT_FAILURE );
      }
      env_var_already_set = true;
      
      if ( strcmp( line->words[1], "none" ) == 0 )
      {
        assert( line->nb_words == 2 );
        env->set_var_method( NO_VAR );
        printf("\tNo more environmental variation\n");
      }
      else if ( strcmp( line->words[1], "autoregressive_mean_variation" ) == 0 )
      {
        assert( line->nb_words == 5 );
        env->set_var_method( AUTOREGRESSIVE_MEAN_VAR );
        env->set_var_sigma( atof( line->words[2] ) );
        env->set_var_tau( atol( line->words[3] ) );
        env->set_var_prng( new ae_jumping_mt(atoi( line->words[4])));
        printf("\tChange of environmental variation to a autoregressive mean variation with sigma=%f, tau=%ld and seed=%d\n", atof( line->words[2] ),atol( line->words[3] ),atoi( line->words[4]));
      }
      else if ( strcmp( line->words[1], "autoregressive_height_variation" ) == 0 )
      {
        assert( line->nb_words == 5 );
        env->set_var_method( AUTOREGRESSIVE_HEIGHT_VAR );
        env->set_var_sigma( atof( line->words[2] ) );
        env->set_var_tau( atol( line->words[3] ) );
        env->set_var_prng( new ae_jumping_mt(atoi( line->words[4])));
        printf("\tChange of environmental variation to a autoregressive height variation with sigma=%f, tau=%ld and seed=%d\n", atof( line->words[2] ),atol( line->words[3] ),atoi( line->words[4]));
      }
      else if ( strcmp( line->words[1], "add_local_gaussians" ) == 0 )
      {
        assert( line->nb_words == 3 );
        env->set_var_method( LOCAL_GAUSSIANS_VAR );
        env->set_var_prng( new ae_jumping_mt(atoi(line->words[2])));
        printf("\tChange of environmental variation to a local gaussians variation with seed=%d\n", atoi( line->words[2]));
      }
      else
      {
        printf( "%s:%d: ERROR in param file : unknown environment variation method.\n", __FILE__, __LINE__ );
        exit( EXIT_FAILURE );
      }
    }
    else if ( strcmp( line->words[0], "SECRETION_CONTRIB_TO_FITNESS" ) == 0 )
    {
        exp_manager->get_exp_s()->set_secretion_contrib_to_fitness( atof( line->words[1] ) );
    }
    else if ( strcmp( line->words[0], "SECRETION_COST" ) == 0 )
    {
      exp_manager->get_exp_s()->set_secretion_cost( atof( line->words[1] ) );
    }
    else if ( strcmp( line->words[0], "PLASMID_MINIMAL_LENGTH" ) == 0 )
    {
      if (!exp_manager->get_with_plasmids()){
        printf("ERROR: option PLASMID_MINIMAL_LENGTH has no sense because there are no plasmids in this population.\n");
        exit( EXIT_FAILURE );
      }
      int32_t plasmid_minimal_length = atoi( line->words[1] );
      ae_list_node<ae_individual*>* indiv_node = pop->get_indivs()->get_first();
      ae_individual* indiv      = NULL;
      while( indiv_node != NULL )
      {
        indiv = (ae_individual *) indiv_node->get_obj();
        if (indiv->get_genetic_unit(1)->get_seq_length()<plasmid_minimal_length){
          printf("ERROR: there is one genetic unit with a smaller length than the new minimum.\n");
          exit( EXIT_FAILURE );
        }
        indiv->get_genetic_unit(1)->set_min_gu_length(plasmid_minimal_length);
        indiv_node = indiv_node->get_next();
      }
    }
    else if ( strcmp( line->words[0], "PLASMID_MAXIMAL_LENGTH" ) == 0 )
    {
      if (!exp_manager->get_with_plasmids()){
        printf("ERROR: option PLASMID_MAXIMAL_LENGTH has no sense because there are no plasmids in this population.\n");
        exit( EXIT_FAILURE );
      }
      int32_t plasmid_maximal_length = atoi( line->words[1] );
      ae_list_node<ae_individual*>* indiv_node = pop->get_indivs()->get_first();
      ae_individual* indiv      = NULL;
      while( indiv_node != NULL )
      {
        indiv = (ae_individual *) indiv_node->get_obj();
        if (indiv->get_genetic_unit(1)->get_seq_length()>plasmid_maximal_length){
          printf("ERROR: there is one genetic unit with a higher length than the new maximum.\n");
          exit( EXIT_FAILURE );
        }
        indiv->get_genetic_unit(1)->set_max_gu_length(plasmid_maximal_length);
        indiv_node = indiv_node->get_next();
      }
    }
    else if ( strcmp( line->words[0], "CHROMOSOME_MINIMAL_LENGTH" ) == 0 )
    {
      int32_t chromosome_minimal_length = atoi( line->words[1] );
      ae_list_node<ae_individual*>* indiv_node = pop->get_indivs()->get_first();
      ae_individual* indiv      = NULL;
      while( indiv_node != NULL )
      {
        indiv = (ae_individual *) indiv_node->get_obj();
        if (indiv->get_genetic_unit(0)->get_seq_length()<chromosome_minimal_length){
          printf("ERROR: there is one genetic unit with a smaller length than the new minimum.\n");
          exit( EXIT_FAILURE );
        }
        indiv->get_genetic_unit(0)->set_min_gu_length(chromosome_minimal_length);
        indiv_node = indiv_node->get_next();
      }
    }
    else if ( strcmp( line->words[0], "CHROMOSOME_MAXIMAL_LENGTH" ) == 0 )
    {
      int32_t chromosome_maximal_length = atoi( line->words[1] );
      ae_list_node<ae_individual*>* indiv_node = pop->get_indivs()->get_first();
      ae_individual* indiv      = NULL;
      while( indiv_node != NULL )
      {
        indiv = (ae_individual *) indiv_node->get_obj();
        if (indiv->get_genetic_unit(0)->get_seq_length()>chromosome_maximal_length){
          printf("ERROR: there is one genetic unit with a higher length than the new maximum.\n");
          exit( EXIT_FAILURE );
        }
        indiv->get_genetic_unit(0)->set_max_gu_length(chromosome_maximal_length);
        indiv_node = indiv_node->get_next();
      }

    }
    else if ( strcmp( line->words[0], "SEED" ) == 0 )
    {
      int32_t seed = atoi( line->words[1] ) ;
      
      ae_jumping_mt* prng = new ae_jumping_mt( seed );
      
      // Change prng in ae_selection 
      sel->set_prng( new ae_jumping_mt(*prng) );
      
      if( exp_manager->is_spatially_structured())
      {
        ae_spatial_structure* sp_struct = exp_manager->get_spatial_structure();
        sp_struct->set_prng(new ae_jumping_mt(*prng) );
      }
      
      printf("\tChange of the seed to %d in selection \n",atoi( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "MUT_SEED" ) == 0 )
    {
      int32_t mut_seed = atoi( line->words[1] ) ;
      
      ae_jumping_mt* mut_prng = new ae_jumping_mt( mut_seed );
      
      // Change prng of the population
      pop->set_mut_prng( new ae_jumping_mt(*mut_prng) );
      printf("\tChange of the seed to %d in mutations \n",atoi( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "STOCH_SEED" ) == 0 )
    {
      int32_t stoch_seed = atoi( line->words[1] ) ;
      
      ae_jumping_mt* stoch_prng = new ae_jumping_mt( stoch_seed );
      
      // Change prng of the population
      pop->set_stoch_prng( new ae_jumping_mt(*stoch_prng) );
      printf("\tChange of the seed to %d in individuals' stochasticity \n",atoi( line->words[1] ));
    }
    else if ( strcmp( line->words[0], "CREATE_3_SUBPOPULATIONS_BASED_ON_NON_CODING_BASES" ) == 0 )
    {
      change_based_on_non_coding_bases_of_best_individual_ancestor(pop, exp_manager, SUBPOPULATIONS_BASED_ON_NON_CODING_BASES);
      printf("\tChange of the population for a population with %"PRId32" individuals in 3 equal subpopulations (A: clones of the previous best individual, B: clones of the previous best individual without any non coding bases, C: clones of the previous best individual with twice non bases\n",pop->get_nb_indivs());
    }
    else if ( strcmp( line->words[0], "REMOVE_NON_CODING_BASES_BEST" ) == 0 )
    {
      change_based_on_non_coding_bases_of_best_individual_ancestor(pop, exp_manager, REMOVE_NON_CODING_BASES_BEST_IND);
      printf("\tChange of the population for a population with %"PRId32" clones of the best individual ancestor without any non coding bases\n",pop->get_nb_indivs());
    }
    else if ( strcmp( line->words[0], "REMOVE_NON_CODING_BASES_POP" ) == 0 )
    {
      change_based_on_non_coding_bases_in_population(pop, exp_manager, REMOVE_NON_CODING_BASES_POPULATION);
      printf("\tChange of the population for a population with %"PRId32" individuals without any non coding bases\n",pop->get_nb_indivs());
    }
    else if ( strcmp( line->words[0], "DOUBLE_NON_CODING_BASES_BEST" ) == 0 )
    {
      change_based_on_non_coding_bases_of_best_individual_ancestor(pop, exp_manager, DOUBLE_NON_CODING_BASES_BEST_IND);
      printf("\tChange of the population for a population with %"PRId32" clones of the best individual ancestor with twice the non coding bases number \n",pop->get_nb_indivs());
    }
    else if ( strcmp( line->words[0], "DOUBLE_NON_CODING_BASES_POP" ) == 0 )
    {
      change_based_on_non_coding_bases_in_population(pop, exp_manager, DOUBLE_NON_CODING_BASES_POPULATION);
      printf("\tChange of the population for a population with %"PRId32" individuals with twice the non coding bases number\n",pop->get_nb_indivs());
    }
    else if ( strcmp( line->words[0], "DUMP_STEP" ) == 0 )
    {
      int step = atoi( line->words[1] );
      if (step>0)
      {
        exp_manager->get_output_m()->set_dump_step( step );
      }
    }
#ifdef BINARY_SECRETION
    else if ( strcmp( line->words[0], "SWITCH_DC") == 0 )
    {
      sel->set_mutdc(atof(line->words[1]));
    }
    else if ( strcmp( line->words[0], "SWITCH_CD") == 0 )
    {
      sel->set_mutcd(atof(line->words[1]));
    }
#endif
    else
    {
      printf( "%s:%d: error: the change %s is not implemented yet \n", __FILE__, __LINE__, line->words[0] );
      exit( EXIT_FAILURE );
    }

    delete line;
  }
  fclose( param_file );
  printf("Ok\n");
  if (env_hasbeenmodified)
  {
    env->build();
  }
  if (trait_gu_location_hasbeenmodified){
    exp_manager->get_exp_s()->set_trait_gu_location(new_trait_gu_location);
    delete [] new_trait_gu_location;
  }
  if (record_tree)
  {
    printf("WARNING: if TREE_STEP in not specified, it will be set to default value\n");
    exp_manager->get_output_m()->init_tree( exp_manager, tree_mode, tree_step );
  }
  
  // 9) Save the modified experiment
  printf("Save the modified experiment into backup...\t");
  exp_manager->write_setup_files();
  exp_manager->save();
  printf("OK\n");
  
  delete exp_manager;
}


/*!
  \brief Get a line in a file and format it
  
  \param param_file file with param in which a line is reading
  \return line (pointer)
  
  \see format_line(f_line* formated_line, char* line, bool* line_is_interpretable )
*/
f_line* get_line( FILE* param_file )
{
  char line[255];
  f_line* formated_line = new f_line();

  bool found_interpretable_line = false; 

  while ( !feof( param_file ) && !found_interpretable_line )
  {
    if ( !fgets( line, 255, param_file ) )
    {
      delete formated_line;
      return NULL;
    }
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
  \brief Format a line by parsing it and the words inside
  
  \param formated_line the resulted formated line
  \param line original line in char*
  \param line_is_interpretable boolean with about the possible intrepretation of the line
*/
void format_line( f_line* formated_line, char* line, bool* line_is_interpretable )
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
  \brief Change in the population based on non coding bases on the best individual ancestor. 3 types of changes
  
  SUBPOPULATIONS_BASED_ON_NON_CODING_BASES:
    Create the 3 subpopulations in the population. The definition of 3 subpopulations is based on non coding bases.
    
    The subpopulation are clonal and based on the ancestor of best individual of pop at begin.
    The individuals in first subpopulation are clone of the best individual ancestor. 
    The individuals in second subpopulation are clone of the best individual ancestor without any bases that are not in coding RNA.  
    The individuals in third subpopulation are clone of the best individual ancestor with addition of bases that are not in coding RNA to double them.
    
    pop is changed into the new population with the 3 subpopulations
    
  REMOVE_NON_CODING_BASES_BEST_IND: 
    The individual of the new population are clone of the best individual ancestor without any bases that are not in coding RNA.  
    
  DOUBLE_NON_CODING_BASES_BEST_IND:
    The individual of the new population are clone of the best individual ancestor with addition of bases that are not in coding RNA to double them.
    
  \param pop population to change
  \param exp_m global exp_manager
  \param type type of change in the population
*/
void change_based_on_non_coding_bases_of_best_individual_ancestor(ae_population* pop, ae_exp_manager* exp_m, population_change_type type)
{
  if(type == SUBPOPULATIONS_BASED_ON_NON_CODING_BASES || type == REMOVE_NON_CODING_BASES_BEST_IND || type == DOUBLE_NON_CODING_BASES_BEST_IND)
  {
    // 1) Compute the population size
    int32_t subpopulation_size = (int)floor(pop->get_nb_indivs()/3);
    int32_t population_size = subpopulation_size*3;
    
    // 2) Retrieve the ancestor (at last backup) of the best individual
    int32_t tree_step = exp_m->get_tree_step();
    int32_t backup_step = exp_m->get_backup_step();
    int32_t num_gener = exp_m->get_num_gener();
    if(num_gener-backup_step <= 0)
    {
      printf( "Error: You must provide a generation number higher to have at least one backup file before.\n");
      exit( EXIT_FAILURE );
    }

    ae_tree * tree = NULL;
    int32_t ancestor_index = 0 ;
    char tree_file_name[50];
    
    
    #ifdef __REGUL
      sprintf( tree_file_name,"tree/tree_%06"PRId32".rae", num_gener ); 
    #else
      sprintf( tree_file_name,"tree/tree_%06"PRId32".ae", num_gener ); 
    #endif
    char pop_file_name[255];
    sprintf( pop_file_name, POP_FNAME_FORMAT, num_gener  );
    tree = new ae_tree( exp_m, tree_file_name );
    ae_replication_report * reports = new ae_replication_report( *(tree->get_report_by_rank(num_gener, tree->get_nb_indivs( num_gener )) ));
    ancestor_index = reports->get_id();
    for (int32_t i = 1; i <= backup_step ; i++ )
    {
      //printf( "Getting the replication report for the ancestor at generation %"PRId32"\n", num_gener-i );

      if ( ae_utils::mod( i, tree_step ) == 0 ) 
      {
        //printf(" New tree file\n");
        #ifdef __REGUL
          sprintf( tree_file_name,"tree/tree_%06"PRId32".rae", num_gener-i ); 
        #else
          sprintf( tree_file_name,"tree/tree_%06"PRId32".ae",  num_gener-i ); 
        #endif
        if ( ae_utils::mod( i, backup_step ) == 0 ) 
        {
          sprintf( pop_file_name,       POP_FNAME_FORMAT,       num_gener-i );
        }
        
        delete tree;
        tree = new ae_tree( exp_m, tree_file_name );
      }

      reports = new ae_replication_report( *(tree->get_report_by_index(num_gener-i, ancestor_index)) );

      ancestor_index = reports->get_parent_id();
      //printf("%"PRId32"\n", ancestor_index);
    }
    
    ae_exp_manager* exp_m_tmp = NULL;
    #ifndef __NO_X
      exp_m_tmp = new ae_exp_manager_X11();
    #else
      exp_m_tmp = new ae_exp_manager();
    #endif
    exp_m_tmp->load( num_gener-backup_step, false, true );
    
    ae_individual* indiv = new ae_individual(*exp_m_tmp->get_indiv_by_id( ancestor_index ));
            
    // 3) Create the new population with 1/3 being clones of the choosen individual, 1/3 being clones of the choosen individual without non coding bases and 
    // 1/3 being clones of the choosen individual with twice coding bases          
    ae_list<ae_individual*>*      new_generation  = new ae_list<ae_individual*>();
    int32_t         index_new_indiv = 0;

    ae_individual* only_coding_indiv = create_clone( indiv, index_new_indiv++ ); //one individual being the clone of the choosen individual but without any non coding bases
    only_coding_indiv->remove_non_coding_bases();
    
    ae_individual* twice_non_coding_indiv = create_clone( indiv, index_new_indiv++ ); //one individual being the clone of the choosen individual but without any non coding bases
    twice_non_coding_indiv->double_non_coding_bases();
    
    //printf("%"PRId32" %"PRId32"\n", indiv->get_total_genome_size(), twice_non_coding_indiv->get_total_genome_size());
    
    int32_t* probe_A = new int32_t[5];
    int32_t* probe_B = new int32_t[5];
    int32_t* probe_C = new int32_t[5];
    for( int32_t i = 0 ; i<5; i++)
    {
      probe_A[i] = 1;
      probe_B[i] = 10;
      probe_C[i] = 100;
    }
    indiv->set_int_probes(probe_A);
    only_coding_indiv->set_int_probes(probe_B);
    twice_non_coding_indiv->set_int_probes(probe_C);
    
    double* probe_double_A = new double[5];
    double* probe_double_B = new double[5];
    double* probe_double_C = new double[5];
    for( int32_t i = 0 ; i<5; i++)
    {
      probe_double_A[i] = 1;
      probe_double_B[i] = 10;
      probe_double_C[i] = 100;
    }
    indiv->set_double_probes(probe_double_A);
    only_coding_indiv->set_double_probes(probe_double_B);
    twice_non_coding_indiv->set_double_probes(probe_double_C);
    
    
    if(type == SUBPOPULATIONS_BASED_ON_NON_CODING_BASES)
    {
      new_generation->add(indiv);
      new_generation->add(only_coding_indiv);
      new_generation->add(twice_non_coding_indiv);
      for ( int32_t i = 0 ; i < subpopulation_size-1 ; i++ ) // clones of the 3 individuals
      {
        new_generation->add(create_clone( indiv, index_new_indiv++ ));
        new_generation->add(create_clone( only_coding_indiv, index_new_indiv++ ));
        new_generation->add(create_clone( twice_non_coding_indiv, index_new_indiv++ ));
      }
    }
    else if(type == REMOVE_NON_CODING_BASES_BEST_IND)
    {
      for ( int32_t i = 0 ; i < pop->get_nb_indivs() ; i++ )
      {
        new_generation->add(create_clone( only_coding_indiv, i ));
      }
    }
    else
    {
      for ( int32_t i = 0 ; i < pop->get_nb_indivs() ; i++ )
      {
        new_generation->add(create_clone( twice_non_coding_indiv, i ));
      }
    }

    //  4) Replace the current population by the new one
    ae_list<ae_individual*>*      old_generation  = pop->get_indivs();
    ae_list_node<ae_individual*>* indiv_node      = old_generation->get_first();
    if ( (not exp_m->get_with_HT()) and (not exp_m->get_with_plasmids()) )
    {
      for ( int32_t i = 0 ; i < pop->get_nb_indivs() ; i++ )
      {
        old_generation->remove( indiv_node, true, true );
        indiv_node = indiv_node->get_next();
      }
    }
    else
    {
      old_generation->erase( true );
    }

    assert( pop->get_indivs()->is_empty() );
    pop->replace_population( new_generation );
    
    if(type == SUBPOPULATIONS_BASED_ON_NON_CODING_BASES)
    {
      pop->set_nb_indivs( population_size );
      exp_m->get_output_m()->get_tree()->set_nb_indivs(population_size, num_gener);
    }
    
    pop->sort_individuals();
  }
  else
  {
    printf( "%s:%d: error: wrong population_change_type %d\n", __FILE__, __LINE__, type );
    exit( EXIT_FAILURE );
  }
}

/*!
  \brief Change in the population based on non coding bases. 2 types of changes
  
  REMOVE_NON_CODING_BASES_POPULATION:
    The individual of the new population are the individuals without any bases that are not in coding RNA.
    
  DOUBLE_NON_CODING_BASES_POPULATION:
    The individual of the new population are the individuals with addition of bases that are not in coding RNA to double them.
    
  \param pop population to change
  \param exp_m global exp_manager
  \param type type of change in the population
*/
void change_based_on_non_coding_bases_in_population(ae_population* pop, ae_exp_manager* exp_m, population_change_type type)
{
  if(type == REMOVE_NON_CODING_BASES_POPULATION || type == DOUBLE_NON_CODING_BASES_POPULATION)
  {
    ae_list_node<ae_individual*>*   indiv_node = pop->get_indivs()->get_first();
    ae_individual*  indiv           = NULL;
    for ( int32_t i = 0 ; i < pop->get_nb_indivs() ; i++ )
    {
      indiv = indiv_node->get_obj();
      if(type ==  REMOVE_NON_CODING_BASES_POPULATION)
      {
        indiv->remove_non_coding_bases();
      }
      else
      {
        indiv->double_non_coding_bases();
      }
      indiv_node = indiv_node->get_next();
    }
  }
  else
  {
    printf( "%s:%d: error: wrong population_change_type %d\n", __FILE__, __LINE__, type );
    exit( EXIT_FAILURE );
  }
}

/*!
  \brief Create of clone of an ae_individual 
  
  \param dolly original individual that would be cloned
  \param id index of the clone in the population
  \return clone of dolly
*/
ae_individual* create_clone( ae_individual* dolly, int32_t id )
{
  ae_individual* indiv;
  
  indiv = new ae_individual( *dolly );
  
  indiv->set_id( id );
  //printf( "metabolic error of the clonal individual : %f (%"PRId32" gene(s), %"PRId32" non coding bases)\n",
  //        indiv->get_dist_to_target_by_feature(METABOLISM), indiv->get_protein_list()->get_nb_elts(), indiv->get_nb_bases_in_0_coding_RNA());
  return indiv;
}







/*!
  \brief 
  
*/
void print_help( char* prog_path ) 
{
  // Get the program file-name in prog_name (strip prog_path of the path)
  char* prog_name; // No new, it will point to somewhere inside prog_path
  if ( prog_name = strrchr( prog_path, '/' ) ) prog_name++;
  else prog_name = prog_path;
  
	printf( "******************************************************************************\n" );
	printf( "*                                                                            *\n" );
	printf( "*                        aevol - Artificial Evolution                        *\n" );
	printf( "*                                                                            *\n" );
	printf( "* Aevol is a simulation platform that allows one to let populations of       *\n" );
  printf( "* digital organisms evolve in different conditions and study experimentally  *\n" );
  printf( "* the mechanisms responsible for the structuration of the genome and the     *\n" );
  printf( "* transcriptome.                                                             *\n" );
	printf( "*                                                                            *\n" );
	printf( "******************************************************************************\n" );
  printf( "\n" );
	printf( "%s: modify an experiment as specified in param_file.\n", prog_name );
  printf( "\n" );
	printf( "Usage : %s -h or --help\n", prog_name );
	printf( "   or : %s -V or --version\n", prog_name );
	printf( "   or : %s -g GENER [-f param_file]\n", prog_name );
	printf( "\nOptions\n" );
	printf( "  -h, --help\n\tprint this help, then exit\n\n" );
	printf( "  -V, --version\n\tprint version number, then exit\n\n" );
  printf( "  -g, --gener GENER\n\tspecify generation number\n\n" );
  printf( "  -f, --file param_file\n\tspecify parameter file (default: param.in)\n");
}

/*!
  \brief 
  
*/
void print_version( void ) 
{
	printf( "aevol %s\n", VERSION );
}
