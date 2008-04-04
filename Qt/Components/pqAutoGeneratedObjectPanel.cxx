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

// this include
#include "pqAutoGeneratedObjectPanel.h"

// Qt includes
#include <QTreeWidget>
#include <QGridLayout>

// VTK includes

// ParaView Server Manager includes

// ParaView includes
#include "pqSMAdaptor.h"
#include "pqProxy.h"
#include "pqNamedWidgets.h"
#include "pqTreeWidgetItemObject.h"
#include "pqPropertyManager.h"


//-----------------------------------------------------------------------------
/// constructor
pqAutoGeneratedObjectPanel::pqAutoGeneratedObjectPanel(pqProxy* object_proxy, QWidget* p) :
  pqNamedObjectPanel(object_proxy, p)
{
  this->PanelLayout = new QGridLayout(this);
  pqNamedWidgets::createWidgets(this->PanelLayout, object_proxy->getProxy());
  this->linkServerManagerProperties();
}

//-----------------------------------------------------------------------------
/// destructor
pqAutoGeneratedObjectPanel::~pqAutoGeneratedObjectPanel()
{
}


//-----------------------------------------------------------------------------
void pqAutoGeneratedObjectPanel::linkServerManagerProperties()
{
  pqNamedObjectPanel::linkServerManagerProperties();
  
  
  // TODO move this code into pqNamedWidgets?
  
  QPixmap cellPixmap(":/pqWidgets/Icons/pqCellData16.png");
  QPixmap pointPixmap(":/pqWidgets/Icons/pqPointData16.png");

  // link point/cell array status widget
  QTreeWidget* treeWidget;
  vtkSMProperty* cellArrayStatus;
  vtkSMProperty* pointArrayStatus;
  treeWidget = this->findChild<QTreeWidget*>("CellAndPointArrayStatus");
  cellArrayStatus = this->proxy()->GetProperty("CellArrayStatus");
  pointArrayStatus = this->proxy()->GetProperty("PointArrayStatus");

  if(treeWidget && cellArrayStatus && pointArrayStatus)
    {
    QList<QVariant> sel_domain;
    sel_domain = pqSMAdaptor::getSelectionPropertyDomain(cellArrayStatus);
    for(int j=0; j<sel_domain.size(); j++)
      {
      QList<QString> str;
      str.append(sel_domain[j].toString());
      pqTreeWidgetItemObject* item;
      item = new pqTreeWidgetItemObject(treeWidget, str);
      item->setData(0, Qt::DecorationRole, cellPixmap);
      this->propertyManager()->registerLink(item, 
                                        "checked", 
                                        SIGNAL(checkedStateChanged(bool)),
                                        this->proxy(), 
                                        cellArrayStatus, j);
      }
    sel_domain = pqSMAdaptor::getSelectionPropertyDomain(pointArrayStatus);
    for(int j=0; j<sel_domain.size(); j++)
      {
      QList<QString> str;
      str.append(sel_domain[j].toString());
      pqTreeWidgetItemObject* item;
      item = new pqTreeWidgetItemObject(treeWidget, str);
      item->setData(0, Qt::DecorationRole, pointPixmap);
      this->propertyManager()->registerLink(item,
                                        "checked", 
                                        SIGNAL(checkedStateChanged(bool)),
                                        this->proxy(), 
                                        pointArrayStatus, j);
      }
    }
}

