/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#ifndef __DisplayPolicy_h
#define __DisplayPolicy_h

#include "OverViewCoreExport.h"

#include "pqDisplayPolicy.h"

class pqDataRepresentation;
class pqOutputPort;
class pqPipelineSource;
class pqView;
class vtkSMProxy;

/// Display policy defines the application specific policy
/// for creating display proxies. Given a pair of a proxy to be displayed 
/// and a view proxy in which to display, this class must tell the type 
/// of display to create, if any. Custom applications can subclass 
/// this to define their own policy. The pqApplicationCore maintains
/// an instance of the policy used by the application. Custom applications
/// should set their own policy instance on the global application core 
/// instance.
class OVERVIEW_CORE_EXPORT DisplayPolicy : public pqDisplayPolicy
{
  Q_OBJECT
  //typedef pqDisplayPolicy Superclass;
public:
  DisplayPolicy(QObject* p);
  virtual ~DisplayPolicy();

  /// Returns a new display for the given (source,view) pair, or NULL 
  /// on failure. If the \c view is not a preferred view to display the source
  /// this method will attempt to create a new view of the preferred type
  /// and create a display for that view. In other words, the argument \c view
  /// is merely a suggestion, it's not necessary that the new display will
  /// be added to the same view. 
  /// In the current implementation the rules as as under:
  /// <ul>
  /// <li>If \c dont_create_view is true and \c view is non-null, then a display
  ///     is created for the given view and returned. If \c view is null, then
  ///     NULL is returned, since we can't create a new view. </li>
  /// <li> If \c dont_create_view is false and \c view is non-null:
  ///     <ul>
  ///     <li>If \c view is of preferred type, we added display to \c view itself.
  ///     </li>
  ///     <li>If \c view is not of preferred type, 
  ///         we create a a new view of the preferred type.
  ///     </li>
  ///     </ul>
  /// </li>
  /// <li> If \c dont_create_view is false and view is null, then a new view of 
  ///      the preferred type is created and the display added to that view.
  /// </li>
  /// </ul>
  /// or not of the type preferred by the source, it may create a new view and 
  /// add the displayto new view. \c dont_create_view can be used to 
  /// override this behavior.
  virtual pqDataRepresentation* createPreferredRepresentation(
    pqOutputPort* port, pqView* view, bool dont_create_view) const;

  /// Set the visibility of the source in the given view. 
  /// Current implementation creates a new display for the source, if possible, 
  /// if none exists. If view is NULL, then a new view of "suitable" type will 
  /// be created for the source. Since custom applications may not necessarily
  /// create new views, we provide this as part of display policy which can 
  /// be easily overridden by creating a new subclass.
  virtual pqDataRepresentation* setRepresentationVisibility(
    pqOutputPort* opPort, pqView* view, bool visible) const;

  // Description
  // Apps can choose whether new filter outputs are hidden upon creation by
  // overriding this method. The default behavior is to create a representation
  // such that it obeys to the Visibility from rendering.xml.
  virtual bool getHideByDefault() const {
    return false;
  }

  /// Returns the visibility state for port in the given view (view may be null
  /// for an empty view).
  virtual VisibilityState getVisibility(pqView* view, pqOutputPort* port) const;

};

#endif

