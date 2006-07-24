/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSeedWidget - place multiple seed points
// .SECTION Description
// The vtkSeedWidget is used to placed multiple seed points in the scene.
// The seed points can be used for operations like connectivity, segmentation,
// and region growing. 
// 
// To use this widget, specify an instance of vtkSeedWidget and a
// representation (a subclass of vtkSeedRepresentation). The widget is
// implemented using multiple instances of vtkHandleWidget which can be used
// to position the seed points (after they are initially placed). The
// representations for these handle widgets are provided by the
// vtkSeedRepresentation.
//
// .SECTION Event Bindings
// By default, the widget responds to the following VTK events (i.e., it
// watches the vtkRenderWindowInteractor for these events):
// <pre>
//   LeftButtonPressEvent - add a point or select a handle (i.e., seed)
//   RightButtonPressEvent - finish adding the seeds
//   MouseMoveEvent - move a handle (i.e., seed)
//   LeftButtonReleaseEvent - release the selected handle (seed)
// </pre>
//
// Note that the event bindings described above can be changed using this
// class's vtkWidgetEventTranslator. This class translates VTK events 
// into the vtkSeedWidget's widget events:
// <pre>
//   vtkWidgetEvent::AddPoint -- add one point; depending on the state
//                               it may the first or second point added. Or,
//                               if near handle, select handle.
//   vtkWidgetEvent::Completed -- finished adding seeds.
//   vtkWidgetEvent::Move -- move the second point or handle depending on the state.
//   vtkWidgetEvent::EndSelect -- the handle manipulation process has completed.
// </pre>
//
// This widget invokes the following VTK events on itself (which observers
// can listen for):
// <pre>
//   vtkCommand::StartInteractionEvent (beginning to interact)
//   vtkCommand::EndInteractionEvent (completing interaction)
//   vtkCommand::InteractionEvent (moving after selecting something)
//   vtkCommand::PlacePointEvent (after point is positioned; 
//                                call data includes handle id (0,1))
// </pre>

// .SECTION See Also
// vtkHandleWidget vtkSeedReoresentation


#ifndef __vtkSeedWidget_h
#define __vtkSeedWidget_h

#include "vtkAbstractWidget.h"

class vtkSeedRepresentation;
class vtkSeedList;
class vtkHandleWidget;
class vtkHandleRepresentation;


class VTK_WIDGETS_EXPORT vtkSeedWidget : public vtkAbstractWidget
{
public:
  // Description:
  // Instantiate this class.
  static vtkSeedWidget *New();

  // Description:
  // Standard methods for a VTK class.
  vtkTypeRevisionMacro(vtkSeedWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The method for activiating and deactiviating this widget. This method
  // must be overridden because it is a composite widget and does more than
  // its superclasses' vtkAbstractWidget::SetEnabled() method.
  virtual void SetEnabled(int);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of vtkProp
  // so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkSeedRepresentation *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}
  
  // Description:
  // Create the default widget representation if one is not set. 
  void CreateDefaultRepresentation();

protected:
  vtkSeedWidget();
  ~vtkSeedWidget();

  // The state of the widget
//BTX
  enum {Start=0,PlacingSeeds,PlacedSeeds,MovingSeed};
//ETX
  int WidgetState;

  // Callback interface to capture events when
  // placing the widget.
  static void AddPointAction(vtkAbstractWidget*);
  static void CompletedAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  static void DeleteAction(vtkAbstractWidget*);
  
  // The positioning handle widgets
  vtkSeedList *Seeds;
  void DeleteSeed(int i);
  
  // Helper method for creating widgets
  static vtkHandleWidget *CreateHandleWidget(vtkSeedWidget *self, vtkSeedRepresentation *rep);

private:
  vtkSeedWidget(const vtkSeedWidget&);  //Not implemented
  void operator=(const vtkSeedWidget&);  //Not implemented
};

#endif
