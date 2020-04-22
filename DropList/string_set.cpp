/* A 'string set' is defined as a set of strings stored
 * in sorted order in a drop list.  See the class video
 * for details.
 *
 * For lists that do not exceed 2^(max_next_width elements+1)
 * elements, the add, remove, and contains functions are 
 * O(lg size) on average.  The operator= and get_elements 
 * functions are O(size).   
 * 
 * Peter Jensen
 * January 28, 2020
 *
 * Made modifications for the resubmit version of the string_set.
 * (See header file for details.)
 * 
 * Jonathan Vidal-Contreras
 * March 25, 2020
 */

#include "string_set.h"
#include "node.h"
#include <iostream>  // For debugging, if needed.
#include <stdlib.h>


namespace cs3505
{
  /*******************************************************
   * string_set member function definitions
   ***************************************************** */

  /** Constructor:  The parameters indicate the maximum
    * width of the next pointers in the drop list nodes
    * as well as the sorting order of the set.
    */
  string_set::string_set(int max_next_width, bool ascending)
  {
    // std::cout<< "starting constructor" << std::endl; // for debugging
    
    this->max_next_width = max_next_width; // set the maximium height possible for any node
    this->ascending = ascending; // determines if this string_set is sorted in ascending or descending order

    // Create head node
    head = new node("", max_next_width);
    size = 0; // The head node doesn't count in the list

    // std::cout<< "ending constructor" << std::endl; // for debugging
  }

  
  /** Copy constructor:  Initialize this set
    *   to contain exactly the same elements as
    *   another set.
    */
  string_set::string_set (const string_set & other)
  {
    //std::cout << "starting copy constructor" << std::endl; // for debugging

    head = new node("", other.max_next_width);
    this->ascending = other.ascending;
    this->max_next_width = other.max_next_width;

    size = 0;
    node* other_current = other.head; // start from the head of the string_set to be copied
    while (other_current != NULL) // add the elements of the other string_set to this
      {
	if (other_current->next[0] != NULL)
	  add(other_current->next[0]->data); // this will increment the size
	      
	other_current = other_current->next[0];
      }

    // std::cout << "ending copy constructor" << std::endl; // for debugging
  }


  /*
   * Destructor:  release any memory allocated
   *   for this object.
   */
  string_set::~string_set()
  {
    // std::cout << "starting destructor" << std::endl; // for debugging
    node* current = head; // start at the head to traverse the list

    while(current->next[0] != NULL) // walk through the string set and free up memory
      {
	node* to_delete = current;
	current = current->next[0];
	delete to_delete; 
      }
    delete current;
    
    // std::cout<< "ending destructor" << std::endl; // For debugging
  }


  /*
   * Adds a string to the string_set if the string does not already exist in the set
   *  while maintaining the elements in sorted order.
   */
  void string_set::add(const std::string & target) 
  {
    std::vector<node*> prev;
    traverse(prev, target); // after traversal, prev[0]->next[0] is the desired node location for the operation

    int height = get_height_of_next();
    node* to_add = new node(target, height);

    if (prev[0]->next[0] != NULL && prev[0]->next[0]->data == target)
      return;  // don't do anything if the element already exists in the set

    for (int i = 0; i < height; i++)
      {
	if (prev[i] != NULL)
	  {
	    // the node to_add will now point to what prev at i was
	    to_add->next[i] = prev[i]->next[i];
	    prev[i]->next[i] = to_add; 
	  }
      }

    size++;
  }

  /*
   * Removes an element from the string_set, given that it exists within the data structure
   */
  void string_set::remove(const std::string & target) 
  {
    std::vector<node*> prev;
    traverse(prev, target);

    if (prev[0]->next[0] != NULL)
      {
	if (prev[0]->next[0]->data != target) // don't do anything if the element does not exist in the set
	  return;

	node* to_delete = prev[0]->next[0];
	for(int i = 0; i < to_delete->next.size(); i++) // make the prev pointers "skip" the node to be deleted
	  {
	    if (to_delete->next[i] != NULL)
	      prev[i]->next[i] = prev[i]->next[i]->next[i]; //prev.next = prev.next.next
	    else
	      prev[i]->next[i] = NULL;
	  }

	size--; 
	delete to_delete; // delete the node, free up memory
      } 
  }


  /*
   * Returns true if the string_set contains the string target, and false if the target cannot be found.
   */ 
  bool string_set::contains(const std::string & target) const 
  {
    std::vector<node*> prev;
    traverse(prev, target);

    if (prev[0] != NULL)
      {
	if (prev[0]->next[0] != NULL && prev[0]->next[0]->data == target) // if we find it
	  return true;
      }

    else
      {
	// std::cout<< prev[0]->data << std::endl; // for debugging-- print what was incorrectly found
	return false;
      }
  }

  /*
   * Returns the number of elements in the string_set
   */
  int string_set::get_size() const
  {
    return this->size;
  }

  /*
   * Returns a flag indicating whether this string_set is sorted in ascending or descending order.
   */
  const bool string_set::is_ascending() const
  {
    return this->ascending;
  }

  /*
   * Takes the elements of this string_set and organizes them in reverse sorted order.
   */
  void string_set::reverse()
  {
    std::vector<std::string> to_be_reversed = get_elements();

    for (int i = 0; i < to_be_reversed.size(); i++) // Clear out the list
      {
	remove(to_be_reversed[i]); 
      }

    // Make sure everything is cleared out and head points to NULL
    for (int i = 0; i < max_next_width; i++) 
      {
	head->next[i] = NULL;
      }
	
    ascending = !ascending; // switch the sorting order

    for (int i = 0; i < to_be_reversed.size(); i++) // Add the elements again
      {
	add(to_be_reversed[i]); 
      }    
  }

  /*
   * Modifies this string_set and changes the elements of this string_set
   * to be the same elements of the other.
   *
   * NOTE: Since the get_elements method does not return a const vector, it should not be used to retrieve
   * the elements of the string_set to be assigned. 
   * It is not used in this implementation of the = operator.
   */
  string_set & string_set::operator= (const string_set & rhs)
  {
    node* current = head->next[0]; // Delete the elements in this string_set
    while (current != NULL)
      {
	node* to_delete = current;
	current = current->next[0];
	
	delete to_delete;
      }

    head->next.resize(rhs.max_next_width); // resize this head's next vector to match rhs
    this->max_next_width = rhs.max_next_width;
    this->ascending = rhs.ascending; // make sure the sorting order matches rhs
    
    // Make sure everything is cleared out and head points to NULL
    for (int i = 0; i < max_next_width; i++) 
      head->next[i] = NULL;
   
    size = 0; // reset the size to 0, since we didn't call remove

    // start going through and adding all the elements of rhs string_set
    node* rhs_current = rhs.head; 
    while (rhs_current != NULL)
      {
	if (rhs_current->next[0] != NULL)
	  add(rhs_current->next[0]->data); // this increments the size
	      
	rhs_current = rhs_current->next[0];
      }
   
    return *this;
  }

  /*
   * Returns a vector of all the elements/entries in the set
   */
  std::vector<std::string> string_set::get_elements()
  {
    std::vector<std::string> elements;

    node* current = head;
    while (current != NULL)
      {
	if(current->next[0] != NULL)
	  {
	    std::string elem = current->next[0]->data;
	    elements.push_back(elem);
	  }
	current = current->next[0];
      }
 
    // for debugging purposes
    // for (int i = 0; i < elements.size(); i++)
    //  {
    // 	std::cout<< elements[i] << " ";
    //  }
    // std::cout<< "" << std::endl;
   
    return elements;
  }


  // Additional public and private helper function definitions needed
  
  /*
   * Randomly determines the height of each node's next pointers.
   * Vectors are guaranteed to have at least one, so this will determine 
   * the number of subsequent pointers.
   */
 const int cs3505::string_set::get_height_of_next()
  {
    int total_height = 1; // ALL node* vectors are guaranteed height of at least 1.

    while(rand() % 2 == 1 && total_height < max_next_width)
      {
       	total_height++;
      }

    // std::cout << "HEIGHT: " << total_height << std::endl; // for debugging
    return total_height;
  }
}

/*
 * Traverses the list to find desired location for various functions (add, remove, etc.).
 * If the "ascending" boolean flag is set to false for the string set, then the traverse function
 * will find the proper location for creating a string_set in 'reverse' sorted order.
 */
void cs3505::string_set::traverse(std::vector<node*> & prev, const std::string & target) const
{ 
   // prev should start with its entries POINTING TO head
  prev.resize(head->next.size());
   for (int i = 0; i < prev.size(); i++)
     prev[i] = head;

   // start from the highest level, move down to level zero in the drop list
   for (int i = prev.size() - 1; i > -1; i--)
     {
       node* current = prev[i];
       while(current != NULL) // drop down the list until you go too far
	 {
	   if (ascending)
	     {
	       if (current->next[i] == NULL || current->next[i]->data > target)
		 {
		   break; // move back one in the prev vector
		 }
	       else if (current->next[i]->data < target)
		 {
		   // found something preceding the target, modify the prev vector
		   prev[i] = current->next[i];
		   adjust_prev(prev, prev[i], i);
		   current = current->next[i]; // keep moving
		 }
	       else if (current->next[i]->data == target)
		 {
		   break;
		 }
	     }
	   else // find the place to put in the element in descending order
	     {
	       if (current->next[i] == NULL || current->next[i]->data < target)
		 {
		   break; // move back one in the prev vector
		 }
	       else if (current->next[i]->data > target)
		 {
		   // found something preceding the target, modify the prev vector
		   prev[i] = current->next[i];
		   adjust_prev(prev, prev[i], i);
		   current = current->next[i]; // keep moving
		 }
	       else if (current->next[i]->data == target)
		 {
		   break;
		 }
	     }
	 }
     }
 }

/*
 * When the traverse method finds a node that "precedes" a specified target in the drop list,
 * this method will set all entries of prev from 0 to i in prev (i excluded)
 */
void cs3505::string_set::adjust_prev(std::vector<node*> & prev, node* & new_level, const int i) const
{
  for (int j = 0; j < i; j++)
    prev[j] = new_level;
}
