/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCollection - create and manipulate unsorted lists of objects
// .SECTION Description
// vtkCollection is a general object for creating and manipulating lists
// of objects. The lists are unsorted and allow duplicate entries. 
// vtkCollection also serves as a base class for lists of specific types 
// of objects.

// .SECTION See Also
// vtkActorCollection vtkAssemblyPaths vtkDataSetCollection
// vtkImplicitFunctionCollection vtkLightCollection vtkPolyDataCollection
// vtkRenderWindowCollection vtkRendererCollection
// vtkStructuredPointsCollection vtkTransformCollection vtkVolumeCollection

#ifndef __vtkCollection_h
#define __vtkCollection_h

#include "vtkObject.h"

//BTX - begin tcl exclude
class vtkCollectionElement //;prevents pick-up by man page generator
{
 public:
  vtkCollectionElement():Item(NULL),Next(NULL) {};
  vtkObject *Item;
  vtkCollectionElement *Next;
};
//ETX end tcl exclude

class vtkCollectionIterator;

class VTK_COMMON_EXPORT vtkCollection : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkCollection,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct with empty list.
  static vtkCollection *New();

  // Description:
  // Add an object to the list. Does not prevent duplicate entries.
  void AddItem(vtkObject *);

  // Description:
  // Replace the i'th item in the collection with a
  void ReplaceItem(int i, vtkObject *);

  // Description:
  // Remove the i'th item in the list.
  // Be careful if using this function during traversal of the list using 
  // GetNextItemAsObject (or GetNextItem in derived class).  The list WILL
  // be shortened if a valid index is given!  If this->Current is equal to the
  // element being removed, have it point to then next element in the list.
  void RemoveItem(int i);  

  // Description:
  // Remove an object from the list. Removes the first object found, not
  // all occurrences. If no object found, list is unaffected.  See warning
  // in description of RemoveItem(int).
  void RemoveItem(vtkObject *);

  // Description:
  // Remove all objects from the list.
  void RemoveAllItems();

  // Description:
  // Search for an object and return location in list. If location == 0,
  // object was not found.
  int  IsItemPresent(vtkObject *);

  // Description:
  // Return the number of objects in the list.
  int  GetNumberOfItems();

  // Description:
  // Initialize the traversal of the collection. This means the data pointer
  // is set at the beginning of the list.
  void InitTraversal() { this->Current = this->Top;};

  //BTX
  // Description:
  // Less bloated way to iterate through a collection. 
  // Just pass the same cookie value around each time
  void InitTraversal(void *&cookie) {cookie = static_cast<void *>(this->Top);};
  //ETX

  // Description:
  // Get the next item in the collection. NULL is returned if the collection
  // is exhausted.
  vtkObject *GetNextItemAsObject();  

  // Description:
  // Get the i'th item in the collection. NULL is returned if i is out
  // of range
  vtkObject *GetItemAsObject(int i);

  //BTX
  // Description: 
  // Less bloated way to get the next object as a collection. Just pass the
  // same cookie back and forth. 
  vtkObject *GetNextItemAsObject(void *&cookie);
  
  // Description:
  // Get an iterator to traverse the objects in this collection.
  vtkCollectionIterator* NewIterator();
  
protected:
  vtkCollection();
  ~vtkCollection();

  virtual void DeleteElement(vtkCollectionElement *); 
  int NumberOfItems;
  vtkCollectionElement *Top;
  vtkCollectionElement *Bottom;
  vtkCollectionElement *Current;

  //BTX
  friend class vtkCollectionIterator;
  //ETX
  
private:
  vtkCollection(const vtkCollection&); // Not implemented
  void operator=(const vtkCollection&); // Not implemented
};


inline vtkObject *vtkCollection::GetNextItemAsObject()
{
  vtkCollectionElement *elem=this->Current;

  if ( elem != NULL )
    {
    this->Current = elem->Next;
    return elem->Item;
    }
  else
    {
    return NULL;
    }
}

inline vtkObject *vtkCollection::GetNextItemAsObject(void *&cookie)
{
  vtkCollectionElement *elem=static_cast<vtkCollectionElement *>(cookie);

  if ( elem != NULL )
    {
    cookie = static_cast<void *>(elem->Next);
    return elem->Item;
    }
  else
    {
    return NULL;
    }
}

#endif





