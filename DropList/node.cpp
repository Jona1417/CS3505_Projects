/* This node class is used to build linked lists for the
 * string_set class.
 *
 * Peter Jensen
 * January 28, 2020
 *
 */

#include "node.h"
#include <vector>
#include <string>
#include <iostream> // for print/debugging statements

// We're not in a namespace.  We are not in any class.  Symbols defined
//   here are globally available.  We need to qualify our function names
//   so that we are definining our cs3505::node class functions.
//
// Note that we could also use the namespace cs3505 { } block.  This would
//   eliminate one level of name qualification.  The 'using' statement will
//   not help in this situation.  
// 
// Qualify it as shown here for functions: cs3505::node::functionname, etc.

/*******************************************************
 * node member function definitions
 ***************************************************** */

// Students will decide how to implement the constructor, destructor, and
//   any helper methods.

/*
 * Creates a node with data as the string parameter, and 
 * a vector of next pointers for traversing the list.
 * 
 * Parameters:
 * --string s: the data in the node
 * --int width: the width of the node* vector, no greater than the max width 
 *   specified by the string_set's constructor.
 */

//cs3505::node::new_node_count = 0;
//cs3505::node::delete_node_count = 0

cs3505::node::node(const std:: string & s, int width)
{
  this->data = s; 
 
  for (int i = 0; i < width; i++)
    {
      next.push_back(NULL);
    }
  //new_node_count++;
}

cs3505::node::~node()
{
  for (int i = 0; i < next.size(); i++)
    {
      next[i] = NULL;
    }
  // delete_node_count++;
}
