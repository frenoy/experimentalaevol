// This program adds a unique integer to each individual. It will be inherited by descendants.

 
// =================================================================
//                              Libraries
// =================================================================
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>


// =================================================================
//                            Project Files
// =================================================================
#include <ae_population.h>
#include <ae_individual.h>
#include <ae_list.h>
#include <ae_exp_manager.h>




void print_help( char* prog_name );


int main( int argc, char* argv[] )
{
  // Initialize command-line option variables with default values  
  char* plasmid_file_name  = NULL;
  int32_t num_gener = -1;
  int32_t plasmid_maximal_length = 10000000;
  int32_t plasmid_minimal_length = 40;
  
  // Define allowed options
  const char * options_list = "hr:p:m:M:";
  static struct option long_options_list[] = {
    { "help", 1, NULL, 'h' },
    { "resume", 1, NULL, 'r' },
    { "plasmid", 1, NULL, 'p' },
    { "min", 1, NULL, 'm' },
    { "max", 1, NULL, 'M' },
    { 0, 0, 0, 0 }
  };

  // Get actual values of the command-line options
  int option;
  while ( ( option = getopt_long(argc, argv, options_list, long_options_list, NULL) ) != -1 ) 
  {
    switch ( option )
    {
      case 'h' :
        print_help( argv[0] );
        exit( EXIT_SUCCESS );
        break;
      case 'r':
        num_gener = atol( optarg );
        break;  
      case 'p':
        plasmid_file_name = new char [strlen(optarg)+1];
        strcpy( plasmid_file_name, optarg );
        break;
      case 'm':
        plasmid_minimal_length = atol( optarg );
        break;
      case 'M':
        plasmid_maximal_length = atol( optarg );
        break;
    }
  }
  
  // Open the files

  ae_population* pop = NULL;
  ae_exp_manager* exp_manager = new ae_exp_manager();
  
  // We need a full backup
  if ( num_gener == -1 )
  {
    printf("You must specify a generation number");
    exit(EXIT_FAILURE);
  }
  exp_manager->load( num_gener, false, false, false );
  pop = exp_manager->get_pop();
  
  // And a plasmid
  if (plasmid_file_name==NULL)
  {
    printf("You must specify a plasmid");
    exit(EXIT_FAILURE);
  }
  
  char rawplasmid[1000000];
  int32_t lplasmid=-1;
  FILE* plasmid_file = fopen(plasmid_file_name,"r");
  fgets(rawplasmid, 1000000, plasmid_file);
  lplasmid = strlen(rawplasmid)-1;
  fclose(plasmid_file);

  // We know need to make several changes in experiment so it 'accepts' this new plasmid
  exp_manager->get_exp_s()->set_with_plasmids( true ); // We need to change the allow_plasmid parameter, otherwise aevol_run will crash
  int16_t* _trait_gu_location = new int16_t[NB_FEATURES];
  for (int16_t i=0; i<NB_FEATURES; i++){
    _trait_gu_location[i]=0;
  }
  exp_manager->get_exp_s()->set_trait_gu_location(_trait_gu_location);
  exp_manager->get_output_m()->set_compute_phen_contrib_by_GU(true);
  
  // We parse the individuals and transform them
  ae_list_node<ae_individual*>* indiv_node = pop->get_indivs()->get_first();
  ae_individual* indiv      = NULL;
  while( indiv_node != NULL )
  {
    indiv = (ae_individual *) indiv_node->get_obj();
    char* plasmid=new char[lplasmid+1]; // Warning: will become the DNA of the first individual created -> no not delete, will be freed in ~ae_dna.
    strncpy(plasmid, rawplasmid, lplasmid);
    plasmid[lplasmid]='\0';
    indiv->add_GU(plasmid,lplasmid);
    indiv->set_allow_plasmids( true );
    indiv->get_genetic_unit(1)->set_min_gu_length(plasmid_minimal_length);
    indiv->get_genetic_unit(1)->set_max_gu_length(plasmid_maximal_length);
    indiv->set_replication_report(NULL); // plasmid's DNA should not have replic reports otherwise stat_record will try to access it.
    indiv->get_genetic_unit(1)->get_dna()->set_replic_report(NULL);
    indiv->get_genetic_unit(1)->compute_phenotypic_contribution();
    indiv->reevaluate();
    indiv_node = indiv_node->get_next();
  }
  
  
  
  // We save and exit
  exp_manager->write_setup_files();
  exp_manager->save();
  delete exp_manager;
  delete [] plasmid_file_name;

  return EXIT_SUCCESS;
}

void print_help( char* prog_name ) 
{
  printf( "\n");
  printf("Add a plasmid to each individual \n");
  printf("Usage : transform_indiv -h\n");
  printf("or :    transform_indiv -r ng -p plasmid_file \n");
  printf("\t-h : display this screen\n");
  printf("\t-r ng  : modify generation ng (need a full backup), \n");
  printf("\t-p plasmid_file : read plasmid sequence from file plasmid_file\n");
}
