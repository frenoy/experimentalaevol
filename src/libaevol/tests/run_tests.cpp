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
#include <cppunit/TestCase.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>



// =================================================================
//                            Project Files
// =================================================================
#include "Test_ae_jumping_mt.h"
#include "Test_ae_individual.h"
#include "Test_ae_dna.h"


// ===========================================================================
//                             Declare Used Namespaces
// ===========================================================================
using namespace CppUnit;



// ===========================================================================
//                         Declare Miscellaneous Functions
// ===========================================================================



int main( int argc, char* argv[] )
{
  // Print message
  cout << "Running regression tests";

  // Get the top level suite from the registry
  Test *suite = TestFactoryRegistry::getRegistry().makeTest();

  // Adds the test to the list of test to run
  TextUi::TestRunner runner;
  runner.addTest( suite );

  // Change the default outputter to a compiler error format outputter
  runner.setOutputter( new CompilerOutputter( &runner.result(), cerr ) );
  
  // Run the tests.
  bool wasSucessful = runner.run();

  // Return error code 1 if the one of test failed.
  return wasSucessful ? 0 : 1;
}



// ===========================================================================
//                         Define Miscellaneous Functions
// ===========================================================================
