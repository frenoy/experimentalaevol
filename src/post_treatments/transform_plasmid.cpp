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
  
  // Define allowed options
  const char * options_list = "hr:p:";
  static struct option long_options_list[] = {
    { "help", 1, NULL, 'h' },
    { "resume", 1, NULL, 'r' },
    { "plasmid", 1, NULL, 'p' },
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

  
  // We parse the individuals and transform them
  ae_list_node<ae_individual*>* indiv_node = pop->get_indivs()->get_first();
  ae_individual* indiv      = NULL;
  int32_t nb=0;
  while( indiv_node != NULL )
  {
    indiv = (ae_individual *) indiv_node->get_obj();
    char* plasmid=new char[lplasmid]; // Warning: will become the DNA of the first individual created -> no not delete, will be freed in ~ae_dna.
    strncpy(plasmid, rawplasmid, lplasmid);
    indiv->add_GU(plasmid,lplasmid);
    indiv_node = indiv_node->get_next();
  }

  exp_manager->save();
  delete exp_manager;

  return EXIT_SUCCESS;
}

void print_help( char* prog_name ) 
{
  printf( "\n\
Add a plasmid to each individual \n\
Usage : transform_indiv -h\n\
or :    transform_indiv -r ng -p plasmid_file \n\
\t-h : display this screen\n\
\t-r ng  : modify generation ng (need a full backup), \n\
\t-p plasmid_file : read plasmid sequence from file plasmid_file\n\
");
}
