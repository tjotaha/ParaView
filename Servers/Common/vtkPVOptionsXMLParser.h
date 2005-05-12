/*=========================================================================
  
  Program:   ParaView
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVOptionsXMLParser - ParaView options storage
// .SECTION Description
// An object of this class represents a storage for ParaView options
// 
// These options can be retrieved during run-time, set using configuration file
// or using Command Line Arguments.
// 
// .SECTION See Also
// kwsys::CommandLineArguments

#ifndef __vtkPVOptionsXMLParser_h
#define __vtkPVOptionsXMLParser_h

#include "vtkCommandOptionsXMLParser.h"
class vtkCommandOptions;

class VTK_EXPORT vtkPVOptionsXMLParser : public vtkCommandOptionsXMLParser
{
public:
  static vtkPVOptionsXMLParser* New();
  vtkTypeRevisionMacro(vtkPVOptionsXMLParser,vtkCommandOptionsXMLParser);

protected:
  vtkPVOptionsXMLParser() {}
  ~vtkPVOptionsXMLParser() {}

  virtual void SetProcessType(const char* ptype);

private:
  vtkPVOptionsXMLParser(const vtkPVOptionsXMLParser&); // Not implemented
  void operator=(const vtkPVOptionsXMLParser&); // Not implemented
};

#endif // #ifndef __vtkPVOptionsXMLParser_h

