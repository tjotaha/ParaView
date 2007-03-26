/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.1. 

   See License_v1.1.txt for the full ParaView license.
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

#ifndef _pqAutoGeneratedObjectPanel_h
#define _pqAutoGeneratedObjectPanel_h

#include "pqNamedObjectPanel.h"
#include <QVariant> // needed for QVariant::Type

class QGridLayout;
class QLineEdit;
class vtkSMProxy;


/// Auto-generated Widget which provides an editor for editing properties of a proxy
/// Widgets are created based on the types of the proxy properties
class PQCOMPONENTS_EXPORT pqAutoGeneratedObjectPanel : public pqNamedObjectPanel
{
  Q_OBJECT
public:
  /// constructor
  pqAutoGeneratedObjectPanel(pqProxy* proxy, QWidget* p = NULL);
  /// destructor
  ~pqAutoGeneratedObjectPanel();
  
  /// populate a layout with widgets to represent the properties
  static void createWidgets(QGridLayout* l, pqProxy* refproxy, vtkSMProxy* pxy);

protected:
  
  void linkServerManagerProperties();

  /// sets up a QValidator for the line edit based on the type.
  static void setupValidator(QLineEdit* lineEdit, QVariant::Type propertyType);

  static void processHints(QGridLayout* panelLayout, pqProxy* refProxy,
                           vtkSMProxy* pxy, QStringList& propertiesToHide,
                           QStringList& properiesToShow);

  QGridLayout* PanelLayout;
};

#endif
