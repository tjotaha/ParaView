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
#include "vtkTransferFunctionEditorWidget.h"

#include "vtkCellData.h"
#include "vtkColorTransferFunction.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPointData.h"
#include "vtkRectilinearGrid.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkTransferFunctionEditorRepresentation.h"

vtkCxxRevisionMacro(vtkTransferFunctionEditorWidget, "$Revision$");

//----------------------------------------------------------------------------
vtkTransferFunctionEditorWidget::vtkTransferFunctionEditorWidget()
{
  this->NumberOfScalarBins = 10000;
  this->WholeScalarRange[0] = this->VisibleScalarRange[0] = 1;
  this->WholeScalarRange[1] = this->VisibleScalarRange[1] = 0;
  this->ModificationType = OPACITY;
  this->OpacityFunction = NULL;
  vtkPiecewiseFunction *oFunc = vtkPiecewiseFunction::New();
  this->SetOpacityFunction(oFunc);
  oFunc->Delete();
  this->ColorFunction = NULL;
  vtkColorTransferFunction *cFunc = vtkColorTransferFunction::New();
  this->SetColorFunction(cFunc);
  cFunc->Delete();
  this->Histogram = NULL;
  this->OpacityMTime = 0;
  this->ColorMTime = 0;
}

//----------------------------------------------------------------------------
vtkTransferFunctionEditorWidget::~vtkTransferFunctionEditorWidget()
{
  this->SetOpacityFunction(NULL);
  this->SetColorFunction(NULL);
  this->SetHistogram(NULL);
}

//----------------------------------------------------------------------------
int vtkTransferFunctionEditorWidget::TransferFunctionsInitialized()
{
  int cSize = this->ColorFunction->GetSize();
  int oSize = this->OpacityFunction->GetSize();
  return (cSize || oSize);
}

//----------------------------------------------------------------------------
void vtkTransferFunctionEditorWidget::SetVisibleScalarRange(double min,
                                                            double max)
{
  if (min == this->VisibleScalarRange[0] && max == this->VisibleScalarRange[1])
    {
    return;
    }

  this->VisibleScalarRange[0] = min;
  this->VisibleScalarRange[1] = max;
  this->Modified();

  vtkTransferFunctionEditorRepresentation *rep =
    vtkTransferFunctionEditorRepresentation::SafeDownCast(this->WidgetRep);
  if (this->Histogram)
    {
    double histogramRange[2];
    int numBins;
    vtkDataArray *dataArray = this->Histogram->GetXCoordinates();
    if (dataArray)
      {
      dataArray->GetRange(histogramRange);
      numBins = dataArray->GetNumberOfTuples() - 1;

      if (rep)
        {
        rep->SetScalarBinRange(
          static_cast<int>((min - histogramRange[0]) * numBins /
                           (histogramRange[1] - histogramRange[0])),
          static_cast<int>((max - histogramRange[0]) * numBins /
                           (histogramRange[1] - histogramRange[0])));
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkTransferFunctionEditorWidget::ShowWholeScalarRange()
{
  if (this->Histogram)
    {
    double range[2];
    vtkDataArray *dataArray = this->Histogram->GetXCoordinates();
    if (dataArray)
      {
      dataArray->GetRange(range);
      this->SetVisibleScalarRange(range);
      }
    }
  else
    {
    this->SetVisibleScalarRange(this->WholeScalarRange);
    }
}

//----------------------------------------------------------------------------
void vtkTransferFunctionEditorWidget::Configure(int size[2])
{
  vtkTransferFunctionEditorRepresentation *rep =
    vtkTransferFunctionEditorRepresentation::SafeDownCast(this->WidgetRep);
  if (rep)
    {
    rep->SetDisplaySize(size);
    }
}

//----------------------------------------------------------------------------
void vtkTransferFunctionEditorWidget::SetHistogram(
  vtkRectilinearGrid *histogram)
{
  if (this->Histogram != histogram)
    {
    vtkRectilinearGrid *tempHist = this->Histogram;
    this->Histogram = histogram;
    if (this->Histogram != NULL)
      {
      this->Histogram->Register(this);
      vtkDataArray *hist = this->Histogram->GetXCoordinates();
      if (hist)
        {
        double range[2];
        hist->GetRange(range);
        if (this->VisibleScalarRange[0] == 1 &&
            this->VisibleScalarRange[1] == 0)
          {
          this->SetVisibleScalarRange(range);
          }
        this->SetWholeScalarRange(range);
        }
      }
    if (tempHist != NULL)
      {
      tempHist->UnRegister(this);
      }
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkTransferFunctionEditorWidget::SetColorFunction(
  vtkColorTransferFunction *function)
{
  if (this->ColorFunction != function)
    {
    vtkColorTransferFunction *tempFunc = this->ColorFunction;
    this->ColorFunction = function;
    if (this->ColorFunction != NULL)
      {
      this->ColorFunction->Register(this);
      this->ColorMTime = this->ColorFunction->GetMTime();
      }
    if (tempFunc != NULL)
      {
      tempFunc->UnRegister(this);
      }
    this->Modified();
    }
  vtkTransferFunctionEditorRepresentation *rep =
    vtkTransferFunctionEditorRepresentation::SafeDownCast(this->WidgetRep);
  if (rep)
    {
    rep->SetColorFunction(this->ColorFunction);
    }
}

//----------------------------------------------------------------------------
void vtkTransferFunctionEditorWidget::SetOpacityFunction(
  vtkPiecewiseFunction *function)
{
  if (this->OpacityFunction != function)
    {
    vtkPiecewiseFunction *tempFunc = this->OpacityFunction;
    this->OpacityFunction = function;
    if (this->OpacityFunction != NULL)
      {
      this->OpacityFunction->Register(this);
      this->OpacityMTime = this->OpacityFunction->GetMTime();
      }
    if (tempFunc != NULL)
      {
      tempFunc->UnRegister(this);
      }
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkTransferFunctionEditorWidget::UpdateFromTransferFunctions()
{
  if (this->OpacityFunction)
    {
    this->OpacityMTime = this->OpacityFunction->GetMTime();
    }
  if (this->ColorFunction)
    {
    this->ColorMTime = this->ColorFunction->GetMTime();
    }
}

//----------------------------------------------------------------------------
void vtkTransferFunctionEditorWidget::MoveToPreviousElement()
{
  vtkTransferFunctionEditorRepresentation *rep =
    vtkTransferFunctionEditorRepresentation::SafeDownCast(this->WidgetRep);
  if (rep)
    {
    rep->SetActiveHandle(rep->GetActiveHandle()-1);
    }
}

//----------------------------------------------------------------------------
void vtkTransferFunctionEditorWidget::MoveToNextElement()
{
  vtkTransferFunctionEditorRepresentation *rep =
    vtkTransferFunctionEditorRepresentation::SafeDownCast(this->WidgetRep);
  if (rep)
    {
    rep->SetActiveHandle(rep->GetActiveHandle()+1);
    }
}

//----------------------------------------------------------------------------
void vtkTransferFunctionEditorWidget::OnChar()
{
  if (!this->Interactor)
    {
    return;
    }

  char *keySym = this->Interactor->GetKeySym();

  if (!strcmp(keySym, "Left"))
    {
    this->MoveToPreviousElement();
    }
  else if (!strcmp(keySym, "Right"))
    {
    this->MoveToNextElement();
    }
}

//----------------------------------------------------------------------------
void vtkTransferFunctionEditorWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "VisibleScalarRange: " << this->VisibleScalarRange[0] << " "
     << this->VisibleScalarRange[1] << endl;
  os << indent << "WholeScalarRange: " << this->WholeScalarRange[0] << " "
     << this->WholeScalarRange[1] << endl;
}
