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
#include <stdio.h>
#include <math.h>


// =================================================================
//                            Project Files
// =================================================================
#include <ae_simulation.h>
#include <ae_population.h>
#include <ae_individual.h>
#include <ae_common.h>

#ifdef __NO_X
  #ifndef __REGUL

  #else
    #include <ae_individual_R.h>
  #endif
#elif defined __X11
  #ifndef __REGUL
    #include <ae_individual_X11.h>
  #else
    #include <ae_individual_R_X11.h>
  #endif
#endif

#include <ae_vis_a_vis.h>
#include <ae_align.h>

//##############################################################################
//                                                                             #
//                             Class ae_population                             #
//                                                                             #
//##############################################################################

// =================================================================
//                    Definition of static attributes
// =================================================================

// =================================================================
//                             Constructors
// =================================================================
ae_population::ae_population( void )
{
  _nb_indivs  = ae_common::init_pop_size;
  _indivs     = new ae_list();
  _prob_reprod = NULL;
  _prob_reprod_previous_best = 0; // TODO
  
  ae_individual* indiv            = NULL;
  int32_t        index_new_indiv  = 0;

  printf( "Entire geometric area of the environment : %f\n", ae_common::sim->get_env()->get_geometric_area() );
  
  if ( ae_common::init_method & ONE_GOOD_GENE )
  {
    if ( ae_common::init_method & CLONE )
    {
      // Create an individual with a "good" gene (in fact, make an indiv whose fitness is better than that corresponding to a flat phenotype)
      // and set its index in the population
      indiv = create_random_individual_with_good_gene( index_new_indiv++ );
      
      // Add it to the list
      _indivs->add( indiv );
      
    
      // Make the clones and add them to the list of individuals
      ae_individual* clone = NULL;
      for ( int32_t i = 1 ; i < _nb_indivs ; i++ )
      {
        // Create a clone, setting its index
        clone = create_clone( indiv, index_new_indiv++ );
        
        // Add it to the list
        _indivs->add( clone );
      }
    }
    else
    {
      for ( int32_t i = 0 ; i < _nb_indivs ; i++ )
      {
        // Create an individual and set its index
        indiv = create_random_individual_with_good_gene( index_new_indiv++ );
        
        // Add it to the list
        _indivs->add( indiv );
      }
      
      sort_individuals();
    }
  }
  else
  {
    printf( "ERROR, the initialization method you have chosen has not been implemented yet. in file %s:%d\n", __FILE__, __LINE__ );
  }
  
  #ifdef FIXED_POPULATION_SIZE
    if ( ae_common::selection_scheme == RANK_LINEAR || ae_common::selection_scheme == RANK_EXPONENTIAL )
    {
      if ( ae_common::pop_structure == false )
      {
        compute_prob_reprod();
      }
    }
  #endif
  
  if ( ae_common::pop_structure == true )
  {
    _pop_grid = new ae_grid_cell** [ae_common::grid_x];
    _output_grid = new double*[ae_common::grid_x];
    for ( int16_t i = 0 ; i < ae_common::grid_x ; i++ )
    {
      _pop_grid[i] = new ae_grid_cell* [ae_common::grid_y];
      _output_grid[i] = new double[ae_common::grid_y];
    }

    ae_list_node*  indiv_node = _indivs->get_first();

    for ( int16_t x = 0 ; x < ae_common::grid_x; x++ ) 
    {
      for ( int16_t y = 0 ; y < ae_common::grid_y; y++ ) 
      {         
        _pop_grid[x][y] = new ae_grid_cell( x, y, (ae_individual*) indiv_node->get_obj() );
        _output_grid[x][y] = 0; 
        _pop_grid[x][y]->get_individual()->set_grid_cell( _pop_grid[x][y] );
        indiv_node = indiv_node->get_next();        
      }
    }
    
    // start with a point source of secreted compound
    if ( ae_common::secretion_init > 0 )
    {
      _pop_grid[(int16_t)ae_common::grid_x/2][(int16_t)ae_common::grid_y/2]->set_compound_amount( ae_common::secretion_init );
    }
  }
  
  sort_individuals();
}

// This constructor starts a population with full of clones of a single individual, 
// loaded from a file
ae_population::ae_population( char* organism_file_name )
{
  printf( "Creating a population from a saved individual genome...\n"); 
  
  _nb_indivs  = ae_common::init_pop_size;
  _indivs     = new ae_list();
  _prob_reprod = NULL;
  _prob_reprod_previous_best = 0; // TODO
  
  ae_individual* indiv            = NULL;
  int32_t        index_new_indiv  = 0;

  printf( "geometric area of the environment : %f\n", ae_common::sim->get_env()->get_geometric_area() );

  // Create an individual from the file
  indiv = create_individual_from_file( organism_file_name, index_new_indiv++ );
  
  // Add it to the list
  _indivs->add( indiv );
      
  // Make the clones and add them to the list of individuals
  ae_individual* clone = NULL;
  for ( int32_t i = 1 ; i < _nb_indivs ; i++ )
  {
    // Create a clone, setting its index
    clone = create_clone( indiv, index_new_indiv++ );
    
    // Add it to the list
    _indivs->add( clone );
  }
    
  #ifdef FIXED_POPULATION_SIZE
    if ( ae_common::selection_scheme == RANK_LINEAR || ae_common::selection_scheme == RANK_EXPONENTIAL )
    {
      if ( ae_common::pop_structure == false )
      {
        compute_prob_reprod();
      }
    }
  #endif
  
  if ( ae_common::pop_structure == true )
  {
    _pop_grid = new ae_grid_cell** [ae_common::grid_x];
    _output_grid = new double*[ae_common::grid_x];
    for ( int16_t i = 0 ; i < ae_common::grid_x ; i++ )
    {
      _pop_grid[i] = new ae_grid_cell* [ae_common::grid_y];
      _output_grid[i] = new double[ae_common::grid_y];
    }

    ae_list_node*  indiv_node = _indivs->get_first();

    for ( int16_t x = 0 ; x < ae_common::grid_x; x++ ) 
    {
      for ( int16_t y = 0 ; y < ae_common::grid_y; y++ ) 
      {         
        _pop_grid[x][y] = new ae_grid_cell( x, y, (ae_individual*) indiv_node->get_obj() );
        _output_grid[x][y] = 0; 
        _pop_grid[x][y]->get_individual()->set_grid_cell( _pop_grid[x][y] );
        indiv_node = indiv_node->get_next();        
      }
    }
    
    // start with a point source of secreted compound
    if ( ae_common::secretion_init > 0 )
    {
      _pop_grid[(int16_t)ae_common::grid_x/2][(int16_t)ae_common::grid_y/2]->set_compound_amount( ae_common::secretion_init );
    }
  }
  
  sort_individuals();
}

ae_population::ae_population( gzFile* backup_file )
{
  // Retreive population intrinsic data
  gzread( backup_file, &_nb_indivs,                  sizeof(_nb_indivs) );
  gzread( backup_file, &_prob_reprod_previous_best,  sizeof(_prob_reprod_previous_best) );

  // Retreive individuals
  _indivs = new ae_list();
  ae_individual* indiv = NULL;
  
  printf( "  Loading individuals " );
  for ( int32_t i = 0 ; i < _nb_indivs ; i++ )
  {
    if ( i && i % 100 == 0 )
    {
      putchar( '*' );
      fflush( stdout );
    }
#ifdef __NO_X
  #ifndef __REGUL
    //~ printf( "new ae_individual( backup_file );\n" );
    indiv = new ae_individual( backup_file );
  #else
    indiv = new ae_individual_R( backup_file );
  #endif
#elif defined __X11
  #ifndef __REGUL
    indiv = new ae_individual_X11( backup_file );
  #else
    indiv = new ae_individual_R_X11( backup_file );
  #endif
#endif
    
    _indivs->add( indiv );
  }
  putchar( '\n' );

  // When reloading from a backup, we can change the population size by overloading INIT_POP_SIZE,
  // so we have to check if we need to create or delete some individuals
  
  // If population size has increased, then we create new individuals
  if ( _nb_indivs < ae_common::init_pop_size )
  {
    int nbclone = ae_common::init_pop_size / _nb_indivs;
    ae_common::grid_x = ae_common::grid_x * nbclone;
    
    if ( (ae_common::init_pop_size % _nb_indivs) != 0 )
    {
      printf( "ERROR : The new number of individual has to be divisible be the old one.\n" );
      exit( EXIT_FAILURE );
    }
    if ( ae_common::init_pop_size == 0 )
    {
      printf( "ERROR : The new number of individual has to be greater than 0.\n" );
      exit( EXIT_FAILURE );
    }
    printf( "    Expanding the population %d times \n",nbclone );
    
    // Create clones of the old individuals
    for ( int c = 1 ; c < nbclone ; c++ )
    {
      for ( int i = 0; i < _nb_indivs; i++ )
      {
        #ifdef __NO_X
          #ifndef __REGUL
            indiv = new ae_individual( dynamic_cast <ae_individual*> (_indivs->get_object(i)), nbclone * _nb_indivs + i );
          #else
            indiv = new ae_individual_R( dynamic_cast <ae_individual_R*> (_indivs->get_object(i)), nbclone * _nb_indivs + i );
          #endif
        #elif defined __X11
          #ifndef __REGUL
            indiv = new ae_individual_X11( dynamic_cast <ae_individual_X11*> (_indivs->get_object(i)), nbclone * _nb_indivs + i );
          #else
            indiv = new ae_individual_R_X11( dynamic_cast <ae_individual_R_X11*> (_indivs->get_object(i)), nbclone * _nb_indivs + i );
          #endif
        #endif
        _indivs->add( indiv );
      }
    }
    
    // create the grid and put the individuals on it
    if ( ae_common::pop_structure == true )
    {
      _pop_grid = new ae_grid_cell** [ae_common::grid_x];
      _output_grid = new double*[ae_common::grid_x];
      for ( int16_t i = 0 ; i < ae_common::grid_x ; i++ )
      {
       _pop_grid[i] = new ae_grid_cell* [ae_common::grid_y];
       _output_grid[i] = new double[ae_common::grid_y];
      }

      ae_list_node*  indiv_node = _indivs->get_first();

      for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
      {
        for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
        {         
          _pop_grid[x][y] = new ae_grid_cell( x, y, (ae_individual*) indiv_node->get_obj() );
          _output_grid[x][y] = 0; 
          _pop_grid[x][y]->get_individual()->set_grid_cell( _pop_grid[x][y] );  
          _pop_grid[x][y]->get_individual()->set_placed_in_population(true);
          indiv_node = indiv_node->get_next();
        }
      }
    }
    
    // Reload the secretion grid and expand it
    if ( ae_common::use_secretion ) 
    {
      double tmp_comp;
      for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
      {
        for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
        {
          if (x >= (ae_common::grid_x/nbclone))
          {
            tmp_comp = _pop_grid[x%(ae_common::grid_x/nbclone)][y]->get_compound_amount();
          }
          else
          {
            gzread( backup_file, &tmp_comp, sizeof(tmp_comp) );
          }
          _pop_grid[x][y]->set_compound_amount(tmp_comp); 
        }
      }
    }

    // update the number of individuals
    _nb_indivs = ae_common::init_pop_size;
  }
  // If population size has decreased, then we only keep some individuals
  else if ( _nb_indivs > ae_common::init_pop_size )
  {
    // save the old list and delete it
    ae_list* _old_indivs = new ae_list(*_indivs);
    delete _indivs;
    
    _indivs = new ae_list();
    
    int32_t k = (int32_t) round( sqrt(ae_common::init_pop_size) );
    int32_t i = k;
    bool    b = false;
    
    // Find a rectangle that has the wanted area
    while( (i > 0) && (!b) )
    {
      if ((ae_common::init_pop_size/i)==(round(ae_common::init_pop_size)/i))
      {
        if ( ((i<=ae_common::grid_x)&&(ae_common::init_pop_size/i<=ae_common::grid_y)) || ((i<=ae_common::grid_y)&&(ae_common::init_pop_size/i<=ae_common::grid_x)) )
        {
          b=true;
        }
        else
        {
          i=i-1;
        }
      }
      else
      {
        i=i-1;
      }
    }
    if (!b)
    {
      printf( "ERROR : can not find a matching size for the new grid.\n" );
      exit( EXIT_FAILURE );
    }
    
    // save the old size of the grid and write the new one
    int ansx = ae_common::grid_x;
    int ansy = ae_common::grid_y;

    if ((i<=ansx)&&(ae_common::init_pop_size/i<=ansy))
    {
      ae_common::grid_x = i;
      ae_common::grid_y = ae_common::init_pop_size/i;
    }
    else
    {
      ae_common::grid_y = i;
      ae_common::grid_x = ae_common::init_pop_size/i;
    }
    
    // Create a population with the new number of individuals
    for ( int x = 0 ; x < ae_common::grid_x ; x++ )
    {
      for ( int y = 0 ; y < ae_common::grid_y ; y++ )
      {
        indiv = (ae_individual*) _old_indivs->get_object( x*ansy + y );
        indiv->set_index_in_population( x*ae_common::grid_y + y );
        _indivs->add( indiv );
      }
    }
    
    // Create the grid and put tje individuals on it
    if ( ae_common::pop_structure == true )
    {
      _pop_grid = new ae_grid_cell** [ae_common::grid_x];
      _output_grid = new double*[ae_common::grid_x];
      for ( int16_t i = 0 ; i < ae_common::grid_x ; i++ )
      {
       _pop_grid[i] = new ae_grid_cell* [ae_common::grid_y];
       _output_grid[i] = new double[ae_common::grid_y];
      }

      ae_list_node*  indiv_node = _indivs->get_first();
      for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
      {
        for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
        {         
          _pop_grid[x][y] = new ae_grid_cell( x, y, (ae_individual*) indiv_node->get_obj() );
          _output_grid[x][y] = 0; 
          _pop_grid[x][y]->get_individual()->set_grid_cell( _pop_grid[x][y] );  
          _pop_grid[x][y]->get_individual()->set_placed_in_population(true);
          indiv_node = indiv_node->get_next();
        }
      }
    }
    
    // reload the secretion grid and only keep the choosen part of it
    if ( ae_common::use_secretion ) 
    {
      double tmp_comp;
      for ( int16_t x = 0 ; x < ansx ; x++ )
      {
        for ( int16_t y = 0 ; y < ansy ; y++ )
        {
          gzread( backup_file, &tmp_comp, sizeof(tmp_comp) );
          if ((x < ae_common::grid_x)&&(y < ae_common::grid_y))
          {
            _pop_grid[x][y]->set_compound_amount(tmp_comp); 
          }
        }
      }
    }
    
    // update the number of individuals
    _nb_indivs = ae_common::init_pop_size;
  }
  // Otherwise we keep all the individuals
  else
  {
    if ( ae_common::pop_structure == true )
    {
      _pop_grid = new ae_grid_cell** [ae_common::grid_x];
      _output_grid = new double*[ae_common::grid_x];
      for ( int16_t i = 0 ; i < ae_common::grid_x ; i++ )
      {
       _pop_grid[i] = new ae_grid_cell* [ae_common::grid_y];
       _output_grid[i] = new double[ae_common::grid_y];
      }


      ae_list_node*  indiv_node = _indivs->get_first();

      for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
      {
        for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
        {         
          _pop_grid[x][y] = new ae_grid_cell( x, y, (ae_individual*) indiv_node->get_obj() );
          _output_grid[x][y] = 0; 
          _pop_grid[x][y]->get_individual()->set_grid_cell( _pop_grid[x][y] );  
          _pop_grid[x][y]->get_individual()->set_placed_in_population(true);
          indiv_node = indiv_node->get_next();
        }
      }
    }
  
    if ( ae_common::use_secretion ) 
    {
      double tmp_comp;
      for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
      {
        for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
        {
          gzread( backup_file, &tmp_comp, sizeof(tmp_comp) );
          _pop_grid[x][y]->set_compound_amount(tmp_comp); 
        }
      }
    }
  
  }

  _prob_reprod = NULL;

#ifdef FIXED_POPULATION_SIZE
  // if population is spacially structured, then the probability of reproduction is computed at each step.
  // If the selection scheme is FITNESS_PROPORTIONATE, same thing.
  if ((ae_common::pop_structure==false)&&(ae_common::selection_scheme != FITNESS_PROPORTIONATE))
  {
    compute_prob_reprod();
  }
#endif
}



// =================================================================
//                             Destructors
// =================================================================
ae_population::~ae_population( void )
{
  _indivs->erase( DELETE_OBJ );
  delete _indivs;

  if ( ae_common::pop_structure == true )
  { 
    for ( int16_t i = 0 ; i < ae_common::grid_x ; i++ )
    {
      for ( int16_t j = 0 ; j < ae_common::grid_y ; j++ )
      {
        delete _pop_grid[i][j];
      }
      
      delete [] _pop_grid[i];
      delete [] _output_grid[i];
    }
    
    delete [] _pop_grid;
    delete [] _output_grid;
  }
  
  if ( _prob_reprod != NULL )
  {
    delete [] _prob_reprod;
  }
}

// =================================================================
//                            Public Methods
// =================================================================
void ae_population::step_to_next_generation( void )
{
  // To create the new generation, we must create _nb_indivs new individuals
  // (offsprings) and "kill" the existing ones.
  // The number of offspring on a given individual will be given by a stochastic
  // process biased on it's fitness value (the selection process).
  // There are 3 possible selection schemes :
  //    * Linear Ranking
  //    * Exponential Ranking
  //    * Fitness proportionate
  //
  // Whichever method is chosen, we will
  // 1) Compute the probability of reproduction of each individual in the population
  // 2) Simulate the stochastic process by a multinomial drawing (based upon the probabilities computed in 1)
  // 3) Make the selected individuals reproduce, thus creating the new generation
  // 4) Replace the current generation by the newly created one.
  // 5) Sort the newly created population*
  
  assert( ae_common::selection_scheme != FITTEST );


  // -------------------------------------------------------------------------------
  // 1) Compute the probability of reproduction of each individual in the population
  // -------------------------------------------------------------------------------
  #ifndef FIXED_POPULATION_SIZE
    compute_prob_reprod();
  #else
    if ( ae_common::selection_scheme == FITNESS_PROPORTIONATE )
    {
      compute_prob_reprod();
    }
  #endif

  // --------------------------------------------------------------------------------------------------------
  // 2) Simulate the stochastic process by a multinomial drawing (based upon the probabilities computed in 1)
  // --------------------------------------------------------------------------------------------------------
  int32_t* nb_offsprings = new int32_t[_nb_indivs];
  ae_common::sim->alea->multinomial_drawing( nb_offsprings, _prob_reprod, _nb_indivs, _nb_indivs );

  // ------------------------------------------------------------------------------
  // 3) Make the selected individuals "reproduce", thus creating the new generation
  // ------------------------------------------------------------------------------
  ae_list*        new_generation  = new ae_list();
  ae_list_node*   indiv_node      = _indivs->get_first();
  ae_list_node*   next_indiv_node = NULL;
  ae_individual*  indiv           = NULL;
  int32_t         index_new_indiv = 0;

  for ( int32_t i = 0 ; i < _nb_indivs ; i++ )
  {
    // Make indiv i reproduce (nb_offsprings[i] offsprings)
    indiv = (ae_individual*) indiv_node->get_obj();
    
    next_indiv_node = indiv_node->get_next();
    
    for ( int32_t j = 0 ; j < nb_offsprings[i] ; j++ )
    {
      // Create a new individual (evaluated at the end of do_replication)
      new_generation->add( do_replication( indiv, index_new_indiv++ ) );
    }
    
    // All the offsprings of this individual have been generated, if there is no transfer,
    // the indiv will not be used any more and can hence be deleted
    if ( ! ae_common::with_transfer )
    {
      _indivs->remove( indiv_node, DELETE_OBJ, DELETE_OBJ );
    }
        
    indiv_node = next_indiv_node;
  }
  
  if ( ae_common::with_transfer )
  {
    // The individuals have not yet been deleted, do it now.
    _indivs->erase( DELETE_OBJ );
  }
  

  delete [] nb_offsprings;



  // -------------------------------------------------------------
  //  4) Replace the current generation by the newly created one.
  // -------------------------------------------------------------
  assert( _indivs->is_empty() );
  delete _indivs;
  _indivs = new_generation;


  // --------------------------------------
  //  5) Sort the newly created population
  // --------------------------------------
  sort_individuals();
}

// This function creates the next generation in a spatially structured population
void ae_population::step_to_next_generation_grid( void )
{
  // create a  grid matrix to store new individuals and fitness matrix to store the fitness values
  ae_individual*** new_pop_grid = new ae_individual** [ae_common::grid_x];
  for ( int16_t i = 0 ; i < ae_common::grid_x ; i++ )
  {
    new_pop_grid[i] = new ae_individual* [ae_common::grid_y];
  }

  // diffusion and degradation of compound in the environment
  secretion_grid_update();

  // randomly migrate some organisms, if necessary 
  if ( ae_common::migration_number > 0 )
  {
    do_random_migrations();
  }    
  

  double *  probs = new double[9];
  for ( int8_t i = 0 ; i < 9 ; i++ ) { probs[i] = ((double) 1) / 9; } 
  double pick_one;
  int16_t found_org, x_offset, y_offset, new_x, new_y; 
        
  // go though the population and do local competitions
  for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
  {
    for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
    {   
      new_pop_grid[x][y] = calculate_local_competition(x,y);
      new_pop_grid[x][y]->set_grid_cell(_pop_grid[x][y]); 
      new_pop_grid[x][y]->set_placed_in_population(true);       
    }
  }  
  
  
  // Create the new generation
  ae_list * new_generation = new ae_list(); 
  double tmp_secretion; 
  for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
  {
    for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
    {
      
        tmp_secretion = _pop_grid[x][y]->get_compound_amount() + _pop_grid[x][y]->get_individual()->get_fitness_by_feature(SECRETION);        
        _pop_grid[x][y]->set_individual( do_replication( new_pop_grid[x][y], -1, x, y ) );       
        _pop_grid[x][y]->set_compound_amount( tmp_secretion );
        new_generation->add( _pop_grid[x][y]->get_individual() );
    
    }
  }

  if ( ae_common::with_transfer )
  {
	// int16_t ae_common::nb_horiz_trans=3;    //How much plasmids can a cell send in its neighboorhood per generation
	for (int16_t i=0;i<ae_common::nb_horiz_trans;i++) 
	{
      
	  //We will shuffle the grid:
	  int16_t total_size=((ae_common::grid_x)*(ae_common::grid_y));
	  int16_t** shuffled_table=new int16_t* [total_size];
	  for(int16_t z=0;z <total_size;z++)
	  {
	  	shuffled_table[z]=new int16_t[2];
	  	int16_t quotient=z/ae_common::grid_x;
	  	int16_t remainder=z%ae_common::grid_x;
	  	shuffled_table[z][0]=(int16_t) remainder;
	  	shuffled_table[z][1]=(int16_t) quotient;
	  	
	  }
	  
	  for(int16_t z=0;z <total_size-1;z++)
	  {
	  	int16_t rand_nb=ae_common::sim->alea->random((int16_t) (total_size-z));
	  	int16_t* tmp=shuffled_table[z+rand_nb];
	  	shuffled_table[z+rand_nb]=shuffled_table[z];
	  	shuffled_table[z]=tmp;		
	  }
	  
	  
      
	    
        // First transfer all the plasmids, but just add them at the end of the list of the GUs
        for ( int16_t z = 0 ; z < total_size ; z++ )
        {
	    // 1. Will there be the plasmid transfered this time
	    // The probability that at least one of 8 neighbours sends a plasmid is  
	    // 1 minus the probability that none send it, so (1-(1-pht)^8)
	    // where pht is the probability that an individual plasmid transfers
        
	      if (ae_common::sim->alea->random() < (1.0 - pow( (1.0 - ae_common::prob_horiz_trans), 9)) ) 
	      {
        
	        int16_t x=shuffled_table[z][0];
	        int16_t y=shuffled_table[z][1];
	        // printf('\ny: ');printf(y); 
	        // 2. pick which neighbor that will donate the plasmid
	        pick_one = 0.0;
	        while ( pick_one == 0 ) { pick_one = ae_common::sim->alea->random(); }
	        found_org = 0;
	        pick_one -= probs[0];
	        while ( pick_one > 0 ) { pick_one -= probs[++found_org]; }
  	      
	        // for simplicity of calculations, assume that plasmid may "transfer" to the same cell
	        x_offset = ( found_org / 3 ) - 1;
	        y_offset = ( found_org % 3 ) - 1;
  	      
	        //For now, assume that it is the second genetic unit that is being transfered
	        new_x = (x+x_offset+ae_common::grid_x) % ae_common::grid_x;
	        new_y = (y+y_offset+ae_common::grid_y) % ae_common::grid_y;
	        _pop_grid[x][y]->get_individual()->inject_GU(_pop_grid[new_x][new_y]->get_individual()); 
  	      
	      }	
	    }
	  delete [] shuffled_table;
	}


  	
	
	
	
    // // First transfer all the plasmids, but just add them at the end of the list of the GUs
    // for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
    // {
      // for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
      // { 
        // // 1. Will there be any plasmid transfer? 
        // // The probability that at least one of 8 neighbours sends a plasmid is  
        // // 1 minus the probability that none send it, so (1-(1-pht)^8)
        // // where pht is the probability that an individual plasmid transfers
        // if (ae_common::sim->alea->random() < (1.0 - pow( (1.0 - ae_common::prob_horiz_trans), 9)) ) 
        // {
          // // 2. pick which neighbor that will donate the plasmid
          // pick_one = 0.0;
          // while ( pick_one == 0 ) { pick_one = ae_common::sim->alea->random(); }
          // found_org = 0;
          // pick_one -= probs[0];
          // while ( pick_one > 0 ) { pick_one -= probs[++found_org]; }
          
          // // for simplicity of calculations, assume that plasmid may "transfer" to the same cell
          // x_offset = ( found_org / 3 ) - 1;
          // y_offset = ( found_org % 3 ) - 1;
          
          // //For now, assume that it is the second genetic unit that is being transfered
          // new_x = (x+x_offset+ae_common::grid_x) % ae_common::grid_x;
          // new_y = (y+y_offset+ae_common::grid_y) % ae_common::grid_y;
          // _pop_grid[x][y]->get_individual()->inject_GU(_pop_grid[new_x][new_y]->get_individual()); 
          
        // }
      // }      
    // }
    
	
	
    // If an individual has more than 2 GUs, it keeps only the last one
    // and re-evaluate the individual
    for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
    {
      for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
      { 
		bool reevaluate=false;
		while (_pop_grid[x][y]->get_individual()->get_genetic_unit_list()->get_nb_elts() > 2) 
        {
		  reevaluate=true;
          _pop_grid[x][y]->get_individual()->get_genetic_unit_list()->remove(_pop_grid[x][y]->get_individual()->get_genetic_unit_list()->get_first()->get_next(), DELETE_OBJ, DELETE_OBJ);
		}
		
		if (reevaluate)
		{
		  _pop_grid[x][y]->get_individual()->reevaluate( ae_common::sim->get_env() );
		}
        
      }
    }
    
  }
      
  delete [] probs;   
  
  _indivs->erase( DELETE_OBJ );
  delete _indivs;
  _indivs = new_generation;
  
  
  // Sort the newly created population
  sort_individuals();
  
  //printf( "new_gen : 0x%x nb_indivs : %ld\n", new_generation, new_generation->get_nb_elts() );
  //printf( "first node : 0x%x obj : 0x%x\n", new_generation->get_first(), new_generation->get_first()->get_obj() );
 
  // delete the new grid
  for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
  {
    delete [] new_pop_grid[x];    
  }
  
  delete [] new_pop_grid;
  
}

/*!
  \brief Replicate an individual, giving the new iniv the index i [and coordinates (x, y) if the population is structured]

  The new individual will:
    1) Inherit the genome of its parent.
    2) (if population is structured) Be located on the population grid at coordinate (x, y).
    3) (if transfer enabled) Potentially receive some DNA from a random donor
    4) Undergo rearrangements and mutations
    5) Be evaluated
    6) Have its statistics computed
*/
ae_individual* ae_population::do_replication( ae_individual* parent, int32_t index, int16_t x /*= 0*/, int16_t y /*= 0*/ )
{
  ae_individual* new_indiv = NULL;

  // ===========================================================================
  //  1) Copy parent
  // ===========================================================================
  #ifdef __NO_X
    #ifndef __REGUL
      new_indiv = new ae_individual( parent, index );
    #else
      new_indiv = new ae_individual_R( dynamic_cast<ae_individual_R*>(parent), index );
    #endif
  #elif defined __X11
    #ifndef __REGUL
      new_indiv = new ae_individual_X11( dynamic_cast<ae_individual_X11*>(parent), index );
    #else
      new_indiv = new ae_individual_R_X11( dynamic_cast<ae_individual_R_X11*>(parent), index );
    #endif
  #endif
  
  
  // ===========================================================================
  //  2) Set the new individual's location on the grid
  //     (needed if the population is structured)
  // ===========================================================================
  if ( ae_common::pop_structure == true )
  {
    new_indiv->set_grid_cell( ae_common::sim->get_pop()->get_pop_grid()[x][y] );
    new_indiv->set_placed_in_population( true ); // TODO : Shouldn't _place_in_pop be set to true in set_grid_cell()?
    ae_common::sim->get_pop()->get_pop_grid()[x][y]->set_individual( new_indiv );
  }
  
  
  // ===========================================================================
  //  3) Perform transfer
  // ===========================================================================
  // -----------------------------------
  //  a) Insertion transfer
  // -----------------------------------
  if ( ae_common::sim->alea->random() < ae_common::transfer_ins_rate )
  {
    // Insertion transfer
    // Requirements:
    //    * A circular exogenote => an alignment on the donor chromosome
    //    * An alignment between the exogenote and the endogenote
    
    // 1) Draw a random donor (uniform drawing).
    // We use the rank because indivs are sorted by rank (1 for the worst, POP_SIZE for the best).
    ae_individual * donor = NULL;
    do donor = get_indiv_by_rank( ae_common::sim->alea->random( _nb_indivs ) + 1 );
    while ( donor == parent );
    
    // 2) Look for an alignment within the donor genome
    ae_vis_a_vis* alignment_1   = NULL;
    ae_dna*       donor_dna     = donor->get_genetic_unit( 0 )->get_dna();
    ae_dna*       new_indiv_dna = new_indiv->get_genetic_unit( 0 )->get_dna();
    int32_t       nb_pairs_1    = (int32_t)( ceil( donor_dna->get_length() * ae_common::neighbourhood_rate ) );
    
    alignment_1 = ae_dna::search_alignment( donor_dna, donor_dna, nb_pairs_1, DIRECT );
    
    if ( alignment_1 != NULL )
    {
      // 3) Make a copy of the sequence to be transferred (the exogenote)
      ae_genetic_unit* exogenote = donor_dna->copy_into_new_GU( alignment_1->get_i_1(), alignment_1->get_i_2() );
      
      // 4) Look for an alignments between the exogenote and the endogenote
      ae_vis_a_vis* alignment_2 = NULL;
      int32_t       nb_pairs_2  = (int32_t)( ceil( new_indiv_dna->get_length() * ae_common::neighbourhood_rate ) );
      
      alignment_2 = ae_dna::search_alignment( exogenote->get_dna(), new_indiv_dna, nb_pairs_2, BOTH_SENSES );
      
      if ( alignment_2 != NULL )
      {
        //~ FILE * logfile = fopen( "transfer.out", "w" );
        //~ fprintf( logfile, "DONOR:\n%s\n\n\n", donor_dna->get_data() );
        //~ fprintf( logfile, "EXOGENOTE:\n%s\n\n\n", exogenote->get_dna()->get_data() );
        //~ fprintf( logfile, "RECIPIENT:\n%s\n\n\n", new_indiv_dna->get_data() );
        //~ #ifdef DEBUG
          //~ ae_common::sim->get_logs()->flush();
          //~ new_indiv->assert_promoters();
          //~ new_indiv->assert_promoters_order();
        //~ #endif
        
        //~ printf( "new indiv length: %"PRId32" parent: %"PRId32" donor: %"PRId32" exogenote: %"PRId32"\n",
                //~ new_indiv_dna->get_length(), parent->get_genetic_unit( 0 )->get_dna()->get_length(), donor_dna->get_length(), exogenote->get_dna()->get_length() );
        
        //~ printf( "************************************************************\n" );
        //~ donor->get_genetic_unit( 0 )->print_rnas();
        //~ printf( "************************************************************\n" );
        //~ exogenote->print_rnas();
        //~ printf( "************************************************************\n" );
        
        int32_t genome_length_before  = new_indiv_dna->get_length();
        int32_t genome_length_after   = new_indiv_dna->get_length() + exogenote->get_dna()->get_length();
        
        if ( genome_length_after > ae_common::max_genome_length )
        {
          if ( ae_common::sim->get_logs()->get_to_be_logged( LOG_BARRIER ) == true )
          {
            // Write an entry in the barrier log file
            fprintf(  ae_common::sim->get_logs()->get_log( LOG_BARRIER ), "%"PRId32" %"PRId32" INS_TRANSFER %"PRId32" %"PRId32" %"PRId32"\n",
                      ae_common::sim->get_num_gener(),
                      new_indiv->get_index_in_population(),
                      exogenote->get_dna()->get_length(),
                      0,
                      genome_length_before );
          }
        }
        else
        {
          new_indiv_dna->insert_GU( exogenote, alignment_2->get_i_2(), alignment_2->get_i_1(), (alignment_2->get_sense() == INDIRECT) );
          //~ fprintf( logfile, "RESULT:\n%s\n\n\n", new_indiv_dna->get_data() );
          //~ fflush( logfile );
          

          // Write a line in transfer logfile
          if ( ae_common::sim->get_logs()->get_to_be_logged( LOG_TRANSFER ) == true )
          {
            fprintf(  ae_common::sim->get_logs()->get_log( LOG_TRANSFER ),
                      "%"PRId32" %"PRId32" %"PRId32" %"PRId8" %"PRId32" %"PRId32" %"PRId32" %"PRId32" %"PRId32" %"PRId32" %"PRId16" %"PRId32" %"PRId32" %"PRId16"\n",
                      ae_common::sim->get_num_gener(),
                      new_indiv->get_index_in_population(),
                      donor->get_index_in_population(),
                      0, // Transfer type
                      exogenote->get_dna()->get_length(),
                      0,
                      genome_length_before,
                      new_indiv_dna->get_length(),
                      alignment_1->get_i_1(),
                      alignment_1->get_i_2(),
                      alignment_1->get_score(),
                      alignment_2->get_i_1(),
                      alignment_2->get_i_2(),
                      alignment_2->get_score() );
          }
          
          //~ printf( "INS  Transfer (%"PRId32", %"PRId32") => (%"PRId32", %"PRId32"). Length: %"PRId32"\n",
                  //~ alignment_1->get_i_1(), alignment_1->get_i_2(),
                  //~ alignment_2->get_i_1(), alignment_2->get_i_2(), new_indiv_dna->get_length() );
          //~ getchar();
    
          //~ #ifdef DEBUG
            //~ ae_common::sim->get_logs()->flush();
            //~ new_indiv->assert_promoters();
            //~ new_indiv->assert_promoters_order();
          //~ #endif
        }
        
        delete alignment_2;
      }
      
      delete exogenote;
      delete alignment_1;
    }
  }
  
  // -----------------------------------
  //  b) Replacement transfer
  // -----------------------------------
  if ( ae_common::sim->alea->random() < ae_common::transfer_repl_rate )
  {
    // Replacement transfer
    // Requirements:
    //    * 2 distinct alignments between the (linear) exogenote and the endogenote
    
    // 1) Draw a random donor (uniform drawing).
    // We use the rank because indivs are sorted by rank (1 for the worst, POP_SIZE for the best).
    ae_individual * donor = NULL;
    do donor = get_indiv_by_rank( ae_common::sim->alea->random( _nb_indivs ) + 1 );
    while ( donor == parent );
    
    // 2) Look for an alignment between the parent genome and the donor genome
    ae_vis_a_vis* alignment_1   = NULL;
    ae_vis_a_vis* alignment_2   = NULL;
    ae_dna*       parent_dna    = parent->get_genetic_unit( 0 )->get_dna();
    ae_dna*       donor_dna     = donor->get_genetic_unit( 0 )->get_dna();
    ae_dna*       new_indiv_dna = new_indiv->get_genetic_unit( 0 )->get_dna();
    ae_sense      sense         = (ae_common::sim->alea->random() < 0.5) ? DIRECT : INDIRECT;
    int32_t       nb_pairs_1    = (int32_t)( ceil( new_indiv_dna->get_length() * ae_common::neighbourhood_rate ) );
    int32_t       nb_pairs_2    = (int32_t)( ceil( new_indiv_dna->get_length() * ae_common::neighbourhood_rate ) );
    
    alignment_1 = ae_dna::search_alignment( parent_dna, donor_dna, nb_pairs_1, sense );
    
    if ( alignment_1 != NULL )
    {
      // Look for a second alignement between the parent and the donor (must be different from alignment_1)
      while ( alignment_2 == NULL && nb_pairs_2 > 0 )
      {
        alignment_2 = ae_dna::search_alignment( parent_dna, donor_dna, nb_pairs_2, sense );
        
        // Forbid the replacement of the whole genome of the parent
        if ( alignment_2 != NULL && alignment_2->get_i_1() == alignment_1->get_i_1() )
        {
          delete alignment_2;
          alignment_2 = NULL;
        }
      }
    
    
      // If both alignments were found, proceed to the transfer
      if ( alignment_2 != NULL )
      {
        int32_t genome_length_before  = new_indiv_dna->get_length();
        int32_t genome_length_after   = new_indiv_dna->get_length();
        int32_t exogenote_length      = ae_utils::mod( alignment_2->get_i_2() - alignment_1->get_i_2() - 1, donor_dna->get_length() ) + 1;
        int32_t replaced_seq_length   = ae_utils::mod( alignment_2->get_i_1() - alignment_1->get_i_1() - 1, genome_length_before ) + 1;
        
        if ( genome_length_after < ae_common::min_genome_length || genome_length_after > ae_common::max_genome_length )
        {
          if ( ae_common::sim->get_logs()->get_to_be_logged( LOG_BARRIER ) == true )
          {
            // Write an entry in the barrier log file
            fprintf(  ae_common::sim->get_logs()->get_log( LOG_BARRIER ), "%"PRId32" %"PRId32" REPL_TRANSFER %"PRId32" %"PRId32" %"PRId32"\n",
                      ae_common::sim->get_num_gener(),
                      new_indiv->get_index_in_population(),
                      exogenote_length,
                      replaced_seq_length,
                      genome_length_before );
          }
        }
        
        // 3) Make a copy of the sequence to be transferred (the exogenote)
        ae_genetic_unit* exogenote = NULL;
        if ( sense == DIRECT )
        {
          exogenote = donor_dna->copy_into_new_GU( alignment_1->get_i_2(), alignment_2->get_i_2() );
        }
        else
        {
          exogenote = donor_dna->copy_into_new_GU( alignment_2->get_i_2(), alignment_1->get_i_2() );
        }
        
        
        //~ FILE * logfile = fopen( "transfer.out", "w" );
        //~ fprintf( logfile, "DONOR:\n%s\n\n\n", donor_dna->get_data() );
        //~ fprintf( logfile, "EXOGENOTE:\n%s\n\n\n", exogenote->get_dna()->get_data() );
        //~ fprintf( logfile, "RECIPIENT:\n%s\n\n\n", new_indiv_dna->get_data() );
        //~ #ifdef DEBUG
          //~ ae_common::sim->get_logs()->flush();
          //~ new_indiv->assert_promoters();
          //~ new_indiv->assert_promoters_order();
        //~ #endif
        
        //~ printf( "new indiv length: %"PRId32" parent: %"PRId32" donor: %"PRId32" exogenote: %"PRId32"\n",
                //~ new_indiv_dna->get_length(), parent->get_genetic_unit( 0 )->get_dna()->get_length(), donor_dna->get_length(), exogenote->get_dna()->get_length() );
        
        //~ printf( "************************************************************\n" );
        //~ donor->get_genetic_unit( 0 )->print_rnas();
        //~ printf( "************************************************************\n" );
        //~ exogenote->print_rnas();
        //~ printf( "************************************************************\n" );
        // Delete the sequence to be replaced
        new_indiv_dna->do_deletion( alignment_1->get_i_1(), alignment_2->get_i_1() );
        
        //~ printf( "intermediate genome length : %"PRId32"\n", new_indiv_dna->get_length() );
        
        // Insert the transfered sequence
        if ( alignment_1->get_i_1() < alignment_2->get_i_1() )
        {
          new_indiv_dna->insert_GU( exogenote, alignment_1->get_i_1(), 0, sense == INDIRECT );
        }
        else
        {
          new_indiv_dna->insert_GU( exogenote, 0, 0, sense == INDIRECT );
        }
        

        // Write a line in transfer logfile
        if ( ae_common::sim->get_logs()->get_to_be_logged( LOG_TRANSFER ) == true )
        {
          fprintf(  ae_common::sim->get_logs()->get_log( LOG_TRANSFER ),
                    "%"PRId32" %"PRId32" %"PRId32" %"PRId8" %"PRId32" %"PRId32" %"PRId32" %"PRId32" %"PRId32" %"PRId32" %"PRId16" %"PRId32" %"PRId32" %"PRId16"\n",
                    ae_common::sim->get_num_gener(),
                    new_indiv->get_index_in_population(),
                    donor->get_index_in_population(),
                    1, // Transfer type
                    exogenote->get_dna()->get_length(),
                    replaced_seq_length,
                    genome_length_before,
                    new_indiv_dna->get_length(),
                    alignment_1->get_i_1(),
                    alignment_1->get_i_2(),
                    alignment_1->get_score(),
                    alignment_2->get_i_1(),
                    alignment_2->get_i_2(),
                    alignment_2->get_score() );
        }
        
        //~ fprintf( logfile, "RESULT:\n%s\n\n\n", new_indiv_dna->get_data() );
        //~ fflush( logfile );
        //~ printf( "REPL Transfer (%"PRId32", %"PRId32") replacing (%"PRId32", %"PRId32"). Length: %"PRId32"\n", alignment_1->get_i_2(), alignment_2->get_i_2(), alignment_1->get_i_1(), alignment_2->get_i_1(), new_indiv_dna->get_length() );
        //~ getchar();

        //~ #ifdef DEBUG
          //~ ae_common::sim->get_logs()->flush();
          //~ new_indiv->assert_promoters();
          //~ new_indiv->assert_promoters_order();
        //~ #endif
        
        delete exogenote;
        delete alignment_2;
      }
      
      delete alignment_1;
    }
  }
  
  
  // ===========================================================================
  //  4) Perform rearrangements and mutations for each genetic unit
  // ===========================================================================
  // Perform mutations for each genetic unit
  int32_t number_GU=ae_common::sim->alea->random((int32_t) 2);// It draws a number between 0 and 1.
  
  
  ae_list_node*     gen_unit_node = new_indiv->get_genetic_unit_list()->get_first();
  ae_genetic_unit*  gen_unit      = NULL;
  if(number_GU<1) // if the number is 0 : we begin with the chromosome
	{
		while ( gen_unit_node != NULL )
		{
			gen_unit = (ae_genetic_unit*) gen_unit_node->get_obj();
			
			gen_unit->get_dna()->perform_mutations();
			
			if ( new_indiv->get_replic_report() != NULL )
			{
				new_indiv->get_replic_report()->get_dna_replic_reports()->add( gen_unit->get_dna()->get_replic_report() );
			}

			gen_unit_node = gen_unit_node->get_next();
		}

	}  
  else  // if the number is 1 : we begin with the plasmid
	{
		gen_unit_node = gen_unit_node->get_next();  // we take the second GU
		while ( gen_unit_node != NULL )
		{
			gen_unit = (ae_genetic_unit*) gen_unit_node->get_obj();
			
			gen_unit->get_dna()->perform_mutations();
			
			if ( new_indiv->get_replic_report() != NULL )
			{
				new_indiv->get_replic_report()->get_dna_replic_reports()->add( gen_unit->get_dna()->get_replic_report() );
			}

			gen_unit_node = gen_unit_node->get_next();
		}
		
		gen_unit_node = new_indiv->get_genetic_unit_list()->get_first(); // We make the operation on the first GU
		gen_unit = (ae_genetic_unit*) gen_unit_node->get_obj();
		
		gen_unit->get_dna()->perform_mutations();
		
		if ( new_indiv->get_replic_report() != NULL )
		{
			new_indiv->get_replic_report()->get_dna_replic_reports()->add( gen_unit->get_dna()->get_replic_report() );
		}

		gen_unit_node = gen_unit_node->get_next();
		
	}


  
  // ===========================================================================
  //  5) Evaluate new individual
  // ===========================================================================
  new_indiv->evaluate( ae_common::sim->get_env() );
  
  
  // ===========================================================================
  //  6) Compute statistics
  // ===========================================================================
  new_indiv->compute_statistical_data();
  
  
  #ifdef DEBUG
    ae_common::sim->get_logs()->flush();
    new_indiv->assert_promoters();
  #endif
  
  
  return new_indiv;
}

ae_individual* ae_population::calculate_local_competition ( int16_t x, int16_t y )
{
  int16_t neighborhood_size = 9; 
  int16_t cur_x;
  int16_t cur_y;

  // construct a temporarry local array of fitness values
  double *  local_fit_array   = new double[neighborhood_size];
  double *  sort_fit_array    = new double[neighborhood_size];
  int16_t * initial_location  = new int16_t[neighborhood_size];
  int16_t * ranks             = new int16_t[neighborhood_size];
  double *  probs             = new double[neighborhood_size];
  int16_t   count             = 0;
  double    sum_local_fit     = 0.0;
  double    sum_local_probs   = 0.0;

  for ( int8_t i = -1 ; i < 2 ; i++ )
  {
    for ( int8_t j = -1 ; j < 2 ; j++ )
    {
      cur_x = ( x + i + ae_common::grid_x ) % ae_common::grid_x;
      cur_y = ( y + j + ae_common::grid_y ) % ae_common::grid_y;
      local_fit_array[count]  = _pop_grid[cur_x][cur_y]->get_individual()->get_fitness();
      sort_fit_array[count]   = local_fit_array[count];
      initial_location[count] = count;
      sum_local_fit = sum_local_fit + local_fit_array[count];
      count = count + 1;
    }
  }

  // sort the local fitness values using bubble sort :
  // we sort by increasing order, so the first element will have the worst fitness.
  bool swaped = true;
  int16_t loop_length = 8; 
  double  tmp_holder;
  int16_t tmp_holder2;
  while ( swaped == true )
  {   
    swaped = false;
    for ( int16_t i = 0 ; i < loop_length ; i++ )
    {
      //if the first is higher than the second,  exchange them
      if ( sort_fit_array[i] > sort_fit_array[i+1] )
      {
        tmp_holder = sort_fit_array[i];
        sort_fit_array[i] = sort_fit_array[i+1];
        sort_fit_array[i+1] = tmp_holder;

        tmp_holder2 = initial_location[i];
        initial_location[i] = initial_location[i+1];
        initial_location[i+1] = tmp_holder2;

        swaped = true;
      }
    }
    
    loop_length = loop_length - 1;
  }
  
  // Assign the ranks to each organism in the neighborhood. 
  for ( int16_t i = 0; i < neighborhood_size ; i++ )
  {
    ranks[initial_location[i]] = i+1 ;
  }
  
  // Do the competitions between the individuals, based on one of the 4 methods: 
  // 1. Rank linear
  // 2. Rank exponential
  // 3. Fitness proportionate
  // 4. Fittest individual
  
  // 1. Linear rank based selection
  if (ae_common::selection_scheme == RANK_LINEAR ) 
  { 
    double increment = (2 * (ae_common::selection_pressure-1)) / (neighborhood_size * (neighborhood_size-1));
    double init_prob = (2 - ae_common::selection_pressure) / neighborhood_size;

    for ( int16_t i = 0 ; i < neighborhood_size ; i++ )
    {
      // The loop is done on the sorted individuals. 
      // In order to know where they are, it uses the initial_location
      probs[initial_location[i]] = init_prob + increment * i;
    }   
  }   

  // 2. Exponential  rank based selection
  else if (ae_common::selection_scheme == RANK_EXPONENTIAL ) {
    double SP_N = pow(ae_common::selection_pressure, neighborhood_size); 
    probs[initial_location[0]] = ( (ae_common::selection_pressure - 1) * SP_N ) /
                                 ( (SP_N - 1) * ae_common::selection_pressure );

    for ( int16_t i = 1 ; i < neighborhood_size ; i++ )
    {
      probs[initial_location[i]] =  probs[initial_location[i-1]] /  ae_common::selection_pressure ; 
    }
    for ( int16_t i = 0 ; i < neighborhood_size ; i++ )
    {
      // TODO : ?
    } 
  }
  
  // 3. Fitness proportionate selection
  else if (ae_common::selection_scheme == FITNESS_PROPORTIONATE)
  {
    for( int16_t i = 0 ; i < neighborhood_size ; i++ )
    {
      probs[i] = local_fit_array[i]/sum_local_fit;
      sum_local_probs += probs[i];
    }
    
    //printf("sum local fitness probs   %f \n", sum_local_probs);
  }

  // 4. Fittest individual
  else if ( ae_common::selection_scheme == FITTEST )
  {
    for ( int16_t i = 0 ; i < neighborhood_size ; i++ )
    {
	
      if ( ranks[i] == neighborhood_size ) //if its rank is neighborhood_size, then it is the best individual
      {
        probs[i] = 1;
      }
      else
      {
        probs[i] = 0;
      }
    }
  }  
  else
  {
    printf( "ERROR, invalid selection scheme in file %s:%d\n", __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
  
  count = 0; 
  for ( int8_t i = -1 ; i < 2 ; i++ )
  {
    for ( int8_t j = -1 ; j < 2 ; j++ )
    {
      cur_x = ( x + i + ae_common::grid_x ) % ae_common::grid_x;
      cur_y = ( y + j + ae_common::grid_y ) % ae_common::grid_y;
      
      count = count + 1;
    } 
  }
  
  // pick one organism to reproduce, based on probs[] calculated above,  using roulette selection
  double pick_one = 0.0;
  while ( pick_one == 0 )
  {
    pick_one = ae_common::sim->alea->random();
  }

  int16_t found_org = 0;

  pick_one -= probs[0];
  while ( pick_one > 0 )
  {
    pick_one -= probs[++found_org];
  }

  int16_t x_offset = ( found_org / 3 ) - 1;
  int16_t y_offset = ( found_org % 3 ) - 1;
  
  delete [] local_fit_array; 
  delete [] sort_fit_array; 
  delete [] initial_location; 
  delete [] ranks; 
  delete [] probs; 
  
  return _pop_grid[(x+x_offset+ae_common::grid_x) % ae_common::grid_x]
                  [(y+y_offset+ae_common::grid_y) % ae_common::grid_y]->get_individual();
}

void ae_population::do_random_migrations ( void ) 
{
  ae_individual * tmp_swap;

  int16_t old_x; int16_t old_y; int16_t new_x; int16_t new_y;
  for ( int16_t i = 0 ; i < ae_common::migration_number ; i++ )
  {
    old_x = (int16_t) (ae_common::sim->alea->random() * ae_common::grid_x);
    old_y = (int16_t) (ae_common::sim->alea->random() * ae_common::grid_y);
    new_x = (int16_t) (ae_common::sim->alea->random() * ae_common::grid_x);
    new_y = (int16_t) (ae_common::sim->alea->random() * ae_common::grid_y);


    // swap the individuals in these grid cells...
    tmp_swap = _pop_grid[old_x][old_y]->get_individual();
    _pop_grid[old_x][old_y]->set_individual( _pop_grid[new_x][new_y]->get_individual() );
    _pop_grid[new_x][new_y]->set_individual( tmp_swap );

  }

}

// do diffusion + degradation of the compound
void ae_population::secretion_grid_update ( void )
{ 
  int16_t cur_x, cur_y; 
  
  double ** new_secretion = new double*[ae_common::grid_x];
  for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
  {
    new_secretion[x] = new double[ae_common::grid_y];
    for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
    {
      new_secretion[x][y] = _pop_grid[x][y]->get_compound_amount();
    }
  }

  for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
  {
    for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
    {
      // look at the entire neighborhood
      for ( int8_t i = -1 ; i < 2 ; i++ )
      {
        for ( int8_t j = -1 ; j < 2 ; j ++ )
        {
          cur_x = (x + i + ae_common::grid_x) % ae_common::grid_x;
          cur_y = (y + j + ae_common::grid_y) % ae_common::grid_y;
          
          // add the difusion from the neighboring cells
          new_secretion[x][y] += _pop_grid[cur_x][cur_y]->get_compound_amount() * ae_common::secretion_difusion_prop;
        }
      }
    }
  }
  
  // substract what has difused from each cell, and calculate the compound degradation
  for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
  {
    for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
    {
      _pop_grid[x][y]->set_compound_amount( new_secretion[x][y] - 9 * _pop_grid[x][y]->get_compound_amount()
                                                                    * ae_common::secretion_difusion_prop );
      _pop_grid[x][y]->set_compound_amount(   _pop_grid[x][y]->get_compound_amount()
                                            * (1 - ae_common::secretion_degradation_prop) );
    }
  }
  for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
  {
    delete [] new_secretion[x];
  }  
  delete [] new_secretion; 
}


void ae_population::write_to_backup( gzFile* backup_file )
{
  // Save population intrinsic data
  gzwrite( backup_file, &_nb_indivs,                  sizeof(_nb_indivs) );
  //~ printf( "write %d bytes (_nb_indivs : %ld)\n", sizeof(_nb_indivs), _nb_indivs );
  gzwrite( backup_file, &_prob_reprod_previous_best,  sizeof(_prob_reprod_previous_best) );
  //~ printf( "write %d bytes (_prob_reprod_previous_best : %f)\n", sizeof(_prob_reprod_previous_best), _prob_reprod_previous_best );
  
  // Save individuals
  if ( ae_common::pop_structure == true )
  {
    for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
    {
      for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
      {
        _pop_grid[x][y]->get_individual()->write_to_backup( backup_file );
      }  
    }
    
    if ( ae_common::use_secretion ) 
    {
      double tmp_comp;
      for ( int16_t x = 0 ; x < ae_common::grid_x ; x++ )
      {
        for ( int16_t y = 0 ; y < ae_common::grid_y ; y++ )
        {
          tmp_comp = _pop_grid[x][y]->get_compound_amount(); 
          gzwrite( backup_file, &tmp_comp, sizeof(tmp_comp) );
        }
      }
    }
  }   
  else 
  {
    ae_list_node*   indiv_node = _indivs->get_first();
    ae_individual*  indiv;
    for ( int32_t i = 0 ; i < _nb_indivs ; i++ )
    {
      indiv = ( ae_individual* ) indiv_node->get_obj();
      indiv->write_to_backup( backup_file );
      indiv_node = indiv_node->get_next();
    }
  }
}


// =================================================================
//                           Protected Methods
// =================================================================
void ae_population::sort_individuals( void )
{
  // Insertion sort
  ae_list_node* last_sorted  = _indivs->get_first();
  ae_list_node* comp         = NULL;
  ae_list_node* item_to_sort = NULL;
  double  fit_comp;
  double  fitness_to_sort;
  bool    is_sorted;

  // only "pop_size - 1" iterations since the first item is already "sorted"
  for ( int32_t nb_sorted = 1 ; nb_sorted < _nb_indivs ; nb_sorted++ )
  {
    item_to_sort    = last_sorted->get_next();
    fitness_to_sort = ((ae_individual*) item_to_sort->get_obj())->get_fitness();
    is_sorted       = false;

    comp = last_sorted;

    // looking for its place among the sorted items
    while ( !is_sorted )
    {
      fit_comp = ((ae_individual*)comp->get_obj())->get_fitness();
      
      if ( fitness_to_sort >= fit_comp ) // The right place of the item is after comp
      {
        if ( item_to_sort->get_prev() == comp )
        {
          // item_to_sort is already at the right place
          is_sorted = true;
          last_sorted = item_to_sort;
        }

        // item_to_sort has to be inserted just after comp
        _indivs->remove( item_to_sort, NO_DELETE, NO_DELETE );
        _indivs->insert_after( item_to_sort, comp );
        is_sorted = true;
      }
      else
      {
        // move on to compare with the next item
        comp = comp->get_prev();
      }
      
      if ( comp == NULL )
      {
        // item_to_sort has to be inserted at the beginning of the list
        _indivs->remove( item_to_sort, NO_DELETE, NO_DELETE );
        _indivs->add_front( item_to_sort );
        is_sorted = true;
      }
    }
  }
  
  // Update the rank of the individuals
  ae_list_node *  indiv_node  = _indivs->get_first();
  ae_individual * indiv       = NULL;
  
  for ( int32_t rank = 1 ; rank <= _nb_indivs ; rank++ )
  {
    indiv = (ae_individual *) indiv_node->get_obj();
    indiv->set_rank_in_population( rank );
    indiv_node = indiv_node->get_next();
  }
}

ae_individual* ae_population::create_random_individual( int32_t index )
{
  ae_individual* indiv;
  
  //~ printf( "create_random_individual\n" );
  
  #ifdef __NO_X
    #ifndef __REGUL
      indiv = new ae_individual();
    #else
      indiv = new ae_individual_R();
    #endif
  #elif defined __X11
    #ifndef __REGUL
      indiv = new ae_individual_X11();
    #else
      indiv = new ae_individual_R_X11();
    #endif
  #endif
  
  indiv->set_index_in_population( index );
  
  // <DEBUG>
  //~ int32_t pos1;
  //~ int32_t pos2;
  //~ int32_t junk;
  // </DEBUG>
  
  if ( ae_common::init_method & WITH_INS_SEQ )
  {
    // Create a random sequence
    int32_t seq_len = 50;
    char* ins_seq = new char[seq_len+1];
    int16_t nb_insert = 50;
    int16_t nb_invert = 50;
    
    for ( int32_t i = 0 ; i < seq_len ; i++ )
    {
      ins_seq[i] = '0' + ae_common::sim->alea->random( NB_BASE );
    }
    ins_seq[seq_len] = '\0';
    
    
    // Insert the sequence at random positions
    ae_mutation* mut1 = NULL;
    for ( int16_t i = 0 ; i < nb_insert ; i++ )
    {
      mut1 = indiv->get_genetic_unit(0)->get_dna()->do_insertion( ins_seq, seq_len );
      // <DEBUG>
      //~ mut1->get_infos_insertion( &pos1, &junk );
      // </DEBUG>
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
      // <DEBUG>
      //~ mut1->get_infos_insertion( &pos2, &junk );
      // </DEBUG>
      delete mut1;
    }
    
    // <DEBUG>
    //~ if ( pos2 <= pos1 ) pos1 += 50;
    
    //~ char* seqA1;
    //~ char* seqA2;
    //~ char* seqA3;
    //~ char* seqB1;
    //~ char* seqB2;
    //~ char* seqB3;
    //~ char* seqA1_inv;
    //~ char* seqA2_inv;
    //~ char* seqA3_inv;
    //~ char* seqB1_inv;
    //~ char* seqB2_inv;
    //~ char* seqB3_inv;
    //~ seqA1 = indiv->get_genetic_unit(0)->get_dna()->get_subsequence( pos1-10, pos1,    LEADING );
    //~ seqA2 = indiv->get_genetic_unit(0)->get_dna()->get_subsequence( pos1,    pos1+50, LEADING );
    //~ seqA3 = indiv->get_genetic_unit(0)->get_dna()->get_subsequence( pos1+50, pos1+60, LEADING );
    //~ seqB1 = indiv->get_genetic_unit(0)->get_dna()->get_subsequence( pos2-10, pos2,    LEADING );
    //~ seqB2 = indiv->get_genetic_unit(0)->get_dna()->get_subsequence( pos2,    pos2+50, LEADING );
    //~ seqB3 = indiv->get_genetic_unit(0)->get_dna()->get_subsequence( pos2+50, pos2+60, LEADING );
    //~ seqA1_inv = indiv->get_genetic_unit(0)->get_dna()->get_subsequence( pos1+60, pos1+50, LAGGING );
    //~ seqA2_inv = indiv->get_genetic_unit(0)->get_dna()->get_subsequence( pos1+50, pos1,    LAGGING );
    //~ seqA3_inv = indiv->get_genetic_unit(0)->get_dna()->get_subsequence( pos1,    pos1-10, LAGGING );
    //~ seqB1_inv = indiv->get_genetic_unit(0)->get_dna()->get_subsequence( pos2+60, pos2+50, LAGGING );
    //~ seqB2_inv = indiv->get_genetic_unit(0)->get_dna()->get_subsequence( pos2+50, pos2,    LAGGING );
    //~ seqB3_inv = indiv->get_genetic_unit(0)->get_dna()->get_subsequence( pos2,    pos2-10, LAGGING );
  
    //~ printf( "             %s\n", ins_seq );
    //~ printf( "             %s\n", inverted_seq );
    //~ printf( "\n" );
    //~ printf( "\n" );
  
    //~ printf( "  %s %s %s\n", seqA1, seqA2, seqA3 );
    //~ printf( "  %s %s %s\n", seqB1_inv, seqB2_inv, seqB3_inv );
    //~ printf( "\n" );
    //~ printf( "  %s %s %s\n", seqB1, seqB2, seqB3 );
    //~ printf( "  %s %s %s\n", seqA1_inv, seqA2_inv, seqA3_inv );
    //~ getchar();
    // </DEBUG>
    
    // <DEBUG>
    //~ ae_vis_a_vis* align = NULL;
    //~ align = ae_align::search_alignment_indirect( indiv->get_genetic_unit(0)->get_dna(), pos1, indiv->get_genetic_unit(0)->get_dna(), pos2+50, 40 );
    
    //~ if ( align == NULL )
    //~ {
      //~ printf( "NO ALIGNMENT!\n" );
      //~ getchar();
    //~ }
    //~ else
    //~ {
      //~ printf( "Alignment found at %"PRId32", %"PRId32" (%"PRId32", %"PRId32")\n", align->get_i_1(), align->get_i_2(), pos1, pos2+50 );
      //~ getchar();
    //~ }
    
    //~ printf( "%s\n%s\n", ins_seq, inverted_seq );
    //~ getchar();
    // </DEBUG>
    
    delete [] ins_seq;
    delete [] inverted_seq;
  }
  
  //~ printf( "random individual created\n" );
  
  return indiv;
}


ae_individual* ae_population::create_individual_from_file( char* organism_file_name, int32_t index )
{
  ae_individual* indiv;
  
  
  #ifdef __NO_X
    #ifndef __REGUL
      indiv = new ae_individual( organism_file_name );
    #else
      printf( "ERROR, not implemented %s:%d\n", __FILE__, __LINE__ );
    #endif
  #elif defined __X11
     printf( "ERROR, not implemented %s:%d\n", __FILE__, __LINE__ );
  #endif
  
  indiv->set_index_in_population( index );
  indiv->evaluate( ae_common::sim->get_env() );
  indiv->compute_statistical_data();

  printf ("Loaded an individual with metabolism : %f . Metabolic area: %f . \n", indiv->get_dist_to_target_by_feature( METABOLISM ), ae_common::sim->get_env()->get_area_by_feature( METABOLISM ));

  return indiv;
}

ae_individual* ae_population::create_random_individual_with_good_gene( int32_t index )
{
  // Create a random individual and evaluate it
  ae_individual* indiv = create_random_individual( index );
  indiv->evaluate( ae_common::sim->get_env() );
  
  // While the created individual is not better than the flat individual (indiv whith no metabolic gene),
  // we delete it and replace it by another random individual
  double env_metabolic_area;
  if ( ae_common::env_axis_is_segmented )
  {
    env_metabolic_area = ae_common::sim->get_env()->get_area_by_feature( METABOLISM );
  }
  else
  {
    env_metabolic_area = ae_common::sim->get_env()->get_total_area();
  }

  // If there are plasmids, make sure there is at least one metabolic gene on each genetic units
  if ( ae_common::allow_plasmids ) 
  {
    if ( ae_common::plasmid_initial_gene == 1)
    {
      while ( indiv->get_genetic_unit(0)->get_dist_to_target_by_feature( METABOLISM ) >= env_metabolic_area  ||
             indiv->get_genetic_unit(1)->get_dist_to_target_by_feature( METABOLISM ) >= env_metabolic_area  )
      {
        delete indiv;
        indiv = create_random_individual( index );
        indiv->evaluate( ae_common::sim->get_env() );
      }
    }
    else // if  ( ae_common::plasmid_initial_gene == 2 )
    {
      // here things work the same as before, but in the constructor of the individual, 
      // a single genetic unit is created and then copied from the chromosome to the plasmid
      while ( indiv->get_dist_to_target_by_feature( METABOLISM ) >= env_metabolic_area )
      {
        delete indiv;
        indiv = create_random_individual( index );
        indiv->evaluate( ae_common::sim->get_env() );
      }
    }
  }
  else
  {
    while ( indiv->get_dist_to_target_by_feature( METABOLISM ) >= env_metabolic_area )
    {
      delete indiv;
      indiv = create_random_individual( index );
      indiv->evaluate( ae_common::sim->get_env() );
    }
  }
  //printf("found an individual! counter = %ld\n ", counter);
  
  //~ printf( "\"good\" individual created\n" );
  
  // Compute the "good" individual's statistics
  indiv->compute_statistical_data();
  
  return indiv;
}

ae_individual* ae_population::create_clone( ae_individual* dolly, int32_t index )
{
  ae_individual* indiv;
  
  #ifdef __NO_X
    #ifndef __REGUL
      indiv = new ae_individual( *dolly );
    #else
      indiv = new ae_individual_R( *(dynamic_cast<ae_individual_R*>(dolly)) );
    #endif
  #elif defined __X11
    #ifndef __REGUL
      indiv = new ae_individual_X11( *(dynamic_cast<ae_individual_X11*>(dolly)) );
    #else
      indiv = new ae_individual_R_X11( *(dynamic_cast<ae_individual_R_X11*>(dolly)) );
    #endif
  #endif
  
  indiv->set_index_in_population( index );
  
  return indiv;
}

void ae_population::compute_prob_reprod( void )
{
  if ( _prob_reprod != NULL )
  {
    delete [] _prob_reprod;
  }
  
  _prob_reprod = new double[_nb_indivs];

  if ( ae_common::selection_scheme == RANK_LINEAR )
  {
    // The probability of reproduction for an individual is given by
    // ( 2-SP + 2 * (SP-1) * (R-1)/(N-1) ) / N
    // With :
    //      SP : selective pressure. Linear ranking allows values of SP in [1.0, 2.0].
    //      R  : the rank of the individual in the population (1 for the worst individual)
    //      N  : the number of individuals in the population
    //
    // We can transform this expression into (2-SP)/N + ((2*(SP-1)) / (N*(N-1))) * (R-1)
    // Furthermore, (R-1) is given directly by <i> (the index of our probability table)
    //
    // probs[0] will hence be given by (2-SP)/N
    // probs[i+1] can then be expressed by probs[i] + (2*(SP-1)) / (N*(N-1))

    double increment = (2 * (ae_common::selection_pressure-1)) / (_nb_indivs * (_nb_indivs-1));
    _prob_reprod[0] = (2 - ae_common::selection_pressure) / _nb_indivs;

    for ( int32_t i = 1 ; i < _nb_indivs ; i++ )
    {
      _prob_reprod[i] = _prob_reprod[i-1] + increment;
    }

    // No need to normalize: The sum is always 1 for linear ranking
  }
  else if ( ae_common::selection_scheme == RANK_EXPONENTIAL )
  {
    // The probability of reproduction for an individual is given by
    // ( (SP-1) * SP^(N-R) ) / ( SP^N - 1 )
    // Which is equivalent to
    // ( (SP-1) * SP^N ) / ( (SP^N - 1) * SP^R )
    // With :
    //      SP : selective pressure. Exponential ranking allows values of SP in ]0.0, 1.0[
    //      R  : the rank of the individual in the population (1 for the worst individual)
    //      N  : the number of individuals in the population
    //
    // NB : The only rank-dependent term is SP^R
    //
    // Because we don't allow ex-aequo,
    // probs[i+1] can hence be expressed as (probs[i] / SP)
    // We will hence compute probs[0] with the original formula and infer the remaining values

    double SP_N = pow(ae_common::selection_pressure, _nb_indivs); // SP^N
    _prob_reprod[0] = ( (ae_common::selection_pressure - 1) * SP_N ) /
                      ( (SP_N - 1) * ae_common::selection_pressure );

    for ( int32_t i = 1 ; i < _nb_indivs ; i++ )
    {
      _prob_reprod[i] = _prob_reprod[i-1] / ae_common::selection_pressure;
    }

    // No need to normalize: We don't allow ex-aequo
  }
  else if ( ae_common::selection_scheme == FITNESS_PROPORTIONATE ) // Fitness Proportionate
  {
    // The probability of reproduction for an individual is given by
    // exp( -SP * gap ) / sum of this measure on all individuals
    //    SP : selective pressure. Fitness proportionate allows values of SP in ]0, +inf[
    //                             The closer SP to 0, the closer the selection to being linear.

    double* fitnesses = new double[_nb_indivs];
    double  sum       = 0;
    
    ae_list_node* node = _indivs->get_first();
    ae_individual* indiv = NULL;
   
    for ( int32_t i = 0 ; i < _nb_indivs ; i++ )
    {
      indiv = (ae_individual *) node->get_obj();
      fitnesses[i] = indiv->get_fitness();
      sum += fitnesses[i];
      node = node->get_next();
    }

    for ( int32_t i = 0 ; i < _nb_indivs ; i++ )
    {
      _prob_reprod[i] = fitnesses[i] / sum;
    }

    delete [] fitnesses;

    // Remember the probability of reproduction of the best individual
    // so it can be printed in stat.out
    _prob_reprod_previous_best = _prob_reprod[_nb_indivs-1];
  }
  else if ( ae_common::selection_scheme == FITTEST) //  Fittest individual
  {
    if (ae_common::pop_structure == false) // If we are here, it should always be the case
    { 
      printf( "Warning: fittest selection scheme is meant to be used for spatially structure populations, using rank exponential \n");
    }
    double SP_N = pow(ae_common::selection_pressure, _nb_indivs); // SP^N
    _prob_reprod[0] = ( (ae_common::selection_pressure - 1) * SP_N ) /
                      ( (SP_N - 1) * ae_common::selection_pressure );

    for ( int32_t i = 1 ; i < _nb_indivs ; i++ )
    {
      _prob_reprod[i] = _prob_reprod[i-1] / ae_common::selection_pressure;
    }
  }
  else
  {
    printf( "ERROR, invalid selection scheme in file %s:%d\n", __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
}


// =================================================================
//                          Non inline accessors
// =================================================================
ae_individual * ae_population::get_indiv_by_index( int32_t index ) const
{
  ae_list_node*   indiv_node = _indivs->get_first();
  ae_individual*  indiv;
  while ( indiv_node != NULL )
  {
    indiv = ( ae_individual* ) indiv_node->get_obj();
    
    if ( indiv->get_index_in_population() == index )
    {
      return indiv;
    }
    
    indiv_node = indiv_node->get_next();
  }
  
  return NULL;
}
