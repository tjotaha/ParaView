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
// .NAME vtkGreedyTerrainDecimation - reduce height field (represented as image) to reduced TIN
// .SECTION Description
// vtkGreedyTerrainDecimation approximates a height field with a triangle
// mesh (TIM) using a greedy insertion algorithm similar to that
// described by Garland and Heckbert (Technical Report CMU-CS-95-181).
// The input to the filter is a height field (represented by a 
// image whose scalar values are height) and the output of the filter
// is polygonal data consisting of triangles. The number of triangles in
// the output is reduced in number as compared to a naive tessellation of
// the input height field.
//
// An brief description of the algorithm is as follows. It is a
// top-down approach that initially represents the height field with
// two triangles (whose vertices are at the four corners of the
// image). These two triangles form a Delaunay triangulation. In an
// iterative fashion, those points in the image with the greatest
// error (as compared to the original height field) are injected into
// the triangulation. This modifies the triangulation using the
// standard incremental Delaunay point insertion (see vtkDelaunay2D)
// algorithm. Points are repeatedly inserted until the appropriate
// (user-specified) error criterion is met.
//
// To use this filter, set the input and specify the error measure to be used.
// The error measure options are 1) the absolute number of triangles to be
// produced; 2) a fractional reduction of the mesh (numTris/maxTris);
// 3) an absolute measure on error (maximum difference in height field to reduced
// TIN); and 4) relative error (the absolute error is set to a fraction of the
// diagonal of the bounding box of the height field).
// 
// .SECTION Caveats
// This algorithm requires the entire input dataset to be in memory, hence it 
// may not work for extremely large images. Invoking BoundaryVertexDeletionOff
// will allow you to stitch together images with matching boundaries.
//
// The input height image is assumed to be positioned in the x-y plane so the
// z coordinate is the height value.
//
// .SECTION See Also
// vtkDecimatePro vtkQuadricDecimation vtkQuadricClustering

#ifndef __vtkGreedyTerrainDecimation_h
#define __vtkGreedyTerrainDecimation_h

#include "vtkStructuredPointsToPolyDataFilter.h"

class vtkPriorityQueue;
class vtkDataArray;
class vtkPointData;
class vtkIdList;
class vtkDoubleArray;

//PIMPL Encapsulation for STL containers
class vtkGreedyTerrainDecimationTerrainInfoType;
class vtkGreedyTerrainDecimationPointInfoType;

#define VTK_ERROR_NUMBER_OF_TRIANGLES 0
#define VTK_ERROR_SPECIFIED_REDUCTION 1
#define VTK_ERROR_ABSOLUTE            2
#define VTK_ERROR_RELATIVE            3

class VTK_HYBRID_EXPORT vtkGreedyTerrainDecimation : public vtkStructuredPointsToPolyDataFilter
{
public:
  vtkTypeRevisionMacro(vtkGreedyTerrainDecimation,vtkStructuredPointsToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate the class.
  static vtkGreedyTerrainDecimation* New();

  // Description:
  // Specify how to terminate the algorithm: either as an absolute number of
  // triangles, a relative number of triangles (normalized by the full
  // resolution mesh), an absolute error (in the height field), or relative
  // error (normalized by the length of the diagonal of the image).
  vtkSetClampMacro(ErrorMeasure,int,VTK_ERROR_NUMBER_OF_TRIANGLES,VTK_ERROR_RELATIVE);
  vtkGetMacro(ErrorMeasure,int);
  void SetErrorMeasureToNumberOfTriangles() 
    {this->SetErrorMeasure(VTK_ERROR_NUMBER_OF_TRIANGLES);}
  void SetErrorMeasureToSpecifiedReduction() 
    {this->SetErrorMeasure(VTK_ERROR_SPECIFIED_REDUCTION);}
  void SetErrorMeasureToAbsoluteError() 
    {this->SetErrorMeasure(VTK_ERROR_ABSOLUTE);}
  void SetErrorMeasureToRelativeError() 
    {this->SetErrorMeasure(VTK_ERROR_RELATIVE);}

  // Description:
  // Specify the number of triangles to produce on output. (It is a
  // good idea to make sure this is less than a tessellated mesh
  // at full resolution.) You need to set this value only when
  // the error measure is set to NumberOfTriangles.
  vtkSetClampMacro(NumberOfTriangles,vtkIdType,2,VTK_LONG_MAX);
  vtkGetMacro(NumberOfTriangles,vtkIdType);

  // Description:
  // Specify the reduction of the mesh (represented as a fraction).  Note
  // that a value of 0.10 means a 10% reduction.  You need to set this value
  // only when the error measure is set to SpecifiedReduction.
  vtkSetClampMacro(Reduction,float,0.0,1.0);
  vtkGetMacro(Reduction,float);

  // Description:
  // Specify the absolute error of the mesh; that is, the error in height
  // between the decimated mesh and the original height field.  You need to
  // set this value only when the error measure is set to AbsoluteError.
  vtkSetClampMacro(AbsoluteError,float,0.0,VTK_LARGE_FLOAT);
  vtkGetMacro(AbsoluteError,float);

  // Description:
  // Specify the relative error of the mesh; that is, the error in height
  // between the decimated mesh and the original height field normalized by
  // the diagonal of the image.  You need to set this value only when the
  // error measure is set to RelativeError.
  vtkSetClampMacro(RelativeError,float,0.0,VTK_LARGE_FLOAT);
  vtkGetMacro(RelativeError,float);

  // Description:
  // Turn on/off the deletion of vertices on the boundary of a mesh. This
  // may limit the maximum reduction that may be achieved.
  vtkSetMacro(BoundaryVertexDeletion,int);
  vtkGetMacro(BoundaryVertexDeletion,int);
  vtkBooleanMacro(BoundaryVertexDeletion,int);

protected:
  vtkGreedyTerrainDecimation();
  ~vtkGreedyTerrainDecimation();

  void Execute();

  //ivars that the API addresses
  int       ErrorMeasure;
  vtkIdType NumberOfTriangles;
  float     Reduction;
  float     AbsoluteError;
  float     RelativeError;
  int       BoundaryVertexDeletion; //Can we delete boundary vertices?

  //Used for convenience
  vtkPolyData    *Mesh;
  vtkPointData   *InputPD;
  vtkPointData   *OutputPD;
  vtkDoubleArray *Points;
  vtkDataArray   *Heights;
  vtkIdType      CurrentPointId;
  double         Tolerance;
  vtkIdList      *Neighbors;
  int            Dimensions[3];
  double         Origin[3];
  double         Spacing[3];
  vtkIdType      MaximumNumberOfTriangles;
  float          Length;

  //Bookeeping arrays
  vtkPriorityQueue                           *TerrainError; //errors for each pt in height field
  vtkGreedyTerrainDecimationTerrainInfoType  *TerrainInfo;  //owning triangle for each pt
  vtkGreedyTerrainDecimationPointInfoType    *PointInfo;    //map mesh pt id to input pt id
  
  //Make a guess at initial allocation
  void EstimateOutputSize(const vtkIdType numInputPts, vtkIdType &numPts, vtkIdType &numTris);
  
  //Returns non-zero if the error measure is satisfied.
  virtual int SatisfiesErrorMeasure(double error);

  //Insert all the boundary vertices into the TIN
  void InsertBoundaryVertices();
    
  //Insert a point into the triangulation; get a point from the triangulation
  vtkIdType AddPointToTriangulation(vtkIdType inputPtId);
  vtkIdType InsertNextPoint(vtkIdType inputPtId, double x[3]);
  double *GetPoint(vtkIdType id);
  void GetPoint(vtkIdType id, double x[3]);

  //Helper functions
  void GetTerrainPoint(int i, int j, double x[3]);
  void ComputeImageCoordinates(vtkIdType inputPtId, int ij[2]);
  int InCircle (double x[3], double x1[3], double x2[3], double x3[3]);
  int FindTriangle(double x[3], vtkIdType ptIds[3], vtkIdType tri,
                         double tol, vtkIdType nei[3], vtkIdList *neighbors, int& status);
  void CheckEdge(vtkIdType ptId, double x[3], vtkIdType p1, vtkIdType p2,
                 vtkIdType tri);

  void UpdateTriangles(vtkIdType meshPtId); //update all points connected to this point
  void UpdateTriangle(vtkIdType triId, vtkIdType p1, vtkIdType p2, vtkIdType p3);
  void UpdateTriangle(vtkIdType triId, int ij1[2], int ij2[2], int ij3[2], float h[4]);

  int CharacterizeTriangle(int ij1[2], int ij2[2], int ij[3],
                           int* &min, int* &max, int* &midL, int* &midR,
                           int* &mid, int mid2[2], float h[3], float &hMin, float &hMax, 
                           float &hL, float &hR);

private:
  vtkGreedyTerrainDecimation(const vtkGreedyTerrainDecimation&);  // Not implemented.
  void operator=(const vtkGreedyTerrainDecimation&);  // Not implemented.

};

#endif

