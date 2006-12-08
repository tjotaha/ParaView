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

#ifndef __pqTreeWidgetCheckHelper_h
#define __pqTreeWidgetCheckHelper_h

#include "QtWidgetsExport.h"
#include <QObject>

class QTreeWidget;
class QTreeWidgetItem;

/// If a QTreeWidget has checkable elements in any column, on needs
/// to explicitly click on the checkbox to change the check state.
/// However, sometimes we simply want the checkbox to be updated
/// when the user clicks on the entire row. For that
/// purpose, we use pqTreeWidgetCheckHelper. Simply create
/// and instance of pqTreeWidgetCheckHelper, and 
/// set a tree to use.
class QTWIDGETS_EXPORT pqTreeWidgetCheckHelper : public QObject
{
  Q_OBJECT
public:
  // treeWidget :- the tree widget managed by this helper.
  // checkableColumn :- column index for the checkable item.
  // parent :- QObject parent.
  pqTreeWidgetCheckHelper(QTreeWidget* treeWidget, 
    int checkableColumn, QObject* parent);

  enum CheckMode
    {
    CLICK_IN_COLUMN, // toggle check state when clicked in the column
                     // with the checkable item.
    CLICK_IN_ROW,    // toggle check state when clicked in the row
                     // with the checkable item (default).
    };
 
  // Check Mode controls whether the user must click in the column
  // with the checkable item or any column in the same row.
  void setCheckMode(CheckMode mode) {this->Mode = mode;}
  CheckMode checkMode() const { return this->Mode; }

protected slots:
  void onItemClicked (QTreeWidgetItem *item, int column);
  void onItemPressed (QTreeWidgetItem *item, int column); 
private:
  pqTreeWidgetCheckHelper(const pqTreeWidgetCheckHelper&); // Not implemented.
  void operator=(const pqTreeWidgetCheckHelper&); // Not implemented.

  QTreeWidget* Tree;
  int CheckableColumn;
  int PressState;
  CheckMode Mode;
};

#endif

