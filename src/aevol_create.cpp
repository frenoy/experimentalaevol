//*****************************************************************************
//
//                         aevol - Artificial Evolution
//
// Copyright (C) 2004 LIRIS.
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
#ifdef __X11
  #include <ae_exp_manager_X11.h>
#else
  #include <ae_exp_manager.h>
#endif
#include <param_loader.h>



// =================================================================
//                         Function declarations
// =================================================================
void print_help( char* prog_path );
void print_version( void );





int main( int argc, char* argv[] )
{
  // 1) Initialize command-line option variables with default values
  char* param_file_name = NULL;
  char* chromosome_file_name = NULL;
  char* plasmid_file_name = NULL;
  char* sequences_file_name = NULL;
  
  // 2) Define allowed options
  const char * options_list = "hf:Vc:p:s:";
  static struct option long_options_list[] = {
    { "help",     no_argument,        NULL, 'h' },
    { "file",     required_argument,  NULL, 'f' },
    { "version",  no_argument,        NULL, 'V' },
    { "chromosome",   required_argument,  NULL, 'c' },
    { "plasmid",   required_argument,  NULL, 'p' },
    { "sequences", required_argument, NULL, 's'},
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
        param_file_name = new char[strlen(optarg)+1];
        strcpy( param_file_name, optarg );
        break;
      }
      case 'c':
      {
        chromosome_file_name = new char [strlen(optarg)+1];
        strcpy( chromosome_file_name, optarg );
        break;
      }
      case 'p':
      {
        plasmid_file_name = new char [strlen(optarg)+1];
        strcpy( plasmid_file_name, optarg );
        break;
      }
      case 's':
      {
        sequences_file_name = new char [strlen(optarg)+1];
        strcpy( sequences_file_name, optarg );
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
  
  char rawchromosome[1000000];
  int32_t lchromosome=-1;
  char* chromosome;
  
  if ( chromosome_file_name != NULL )
  {
    FILE* chromosome_file = fopen(chromosome_file_name,"r");
    fgets(rawchromosome, 1000000, chromosome_file);
    lchromosome = strlen(rawchromosome)-1;
    chromosome = new char[lchromosome]; // Warning: will become the DNA of the first individual created -> no not delete, will be freed in ~ae_dna.
    strncpy(chromosome, rawchromosome, lchromosome);
    printf("Loading chromosome from text file %s (%"PRId32" base pairs) \n",chromosome_file_name,lchromosome);
    fclose(chromosome_file);
  }

  char rawplasmid[1000000];
  int32_t lplasmid=-1;
  char* plasmid;
  
  if ( plasmid_file_name != NULL )
  {
    FILE* plasmid_file = fopen(plasmid_file_name,"r");
    fgets(rawplasmid, 1000000, plasmid_file);
    lplasmid = strlen(rawplasmid)-1;
    plasmid = new char[lplasmid]; // Warning: will become the DNA of the first individual created -> no not delete, will be freed in ~ae_dna.
    strncpy(plasmid, rawplasmid, lplasmid);
    printf("Loading plasmid from text file %s (%"PRId32" base pairs) \n",plasmid_file_name,lplasmid);
    fclose(plasmid_file);
  }

  // 5) Create a param loader for the parameter file
  param_loader* my_param_loader = new param_loader( param_file_name );
  delete param_file_name;
  
  
  // 6) Initialize the experiment manager
  #ifndef __NO_X
    ae_exp_manager* exp_manager = new ae_exp_manager_X11();
  #else
    ae_exp_manager* exp_manager = new ae_exp_manager();
  #endif

  std::function<std::list<char*>(void)> empty;

  
  // 7) Load the parameter file
  if (sequences_file_name != NULL )
  {
    FILE* source = fopen(sequences_file_name,"r");
    if (source==NULL)
    {
      printf("ERROR: failed to open file %s in reading mode\n",sequences_file_name);
      exit(EXIT_FAILURE);
    }

    my_param_loader->load( exp_manager, true, [&](void)->std::list<char*>{
      char rawdata[1000000];
      std::list<char*> r;
      if ( (fgets(rawdata, 1000000, source) == NULL) || (feof(source)))
      {
        r.push_front(NULL);
        return r;
      }
      char* chr = strtok(rawdata," \n");
      if (chr==NULL)
      {
        r.push_front(NULL);
        return r;
      }
      r.push_front(chr);
      char* pla = strtok(NULL," \n");
      r.push_back(pla);
      return r;
    });

    fclose(source);
  }
  else if (lchromosome > -1)
  {
    if (lplasmid > -1)
    {
      my_param_loader->load( exp_manager, true, empty, chromosome, lchromosome, plasmid, lplasmid );
    }
    else
    {
      my_param_loader->load( exp_manager, true, empty, chromosome, lchromosome );
    }
  }
  else
  {
    my_param_loader->load( exp_manager, true, empty );
  }
  delete my_param_loader;
  
  //~ ((ae_exp_manager_X11*)exp_manager)->toggle_display_on_off();
  //~ exp_manager->display();
  //~ getchar();
  
  // 8) Create the static setup files (experimental setup and output profile)
  exp_manager->write_setup_files();
  
  // 9) Create the initial backups
  exp_manager->save();
  
  delete exp_manager;
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
	printf( "%s: create an experiment with setup as specified in param_file.\n", prog_name );
  printf( "\n" );
	printf( "Usage : %s -h or --help\n", prog_name );
	printf( "   or : %s -V or --version\n", prog_name );
	printf( "   or : %s [-f param_file]\n", prog_name );
	printf( "\nOptions\n" );
	printf( "  -h, --help\n\tprint this help, then exit\n\n" );
	printf( "  -V, --version\n\tprint version number, then exit\n\n" );
	printf( "  -f, --file param_file\n\tspecify parameter file (default: param.in)\n" );
}

/*!
  \brief 
  
*/
void print_version( void ) 
{
	printf( "aevol %s\n", VERSION );
}
