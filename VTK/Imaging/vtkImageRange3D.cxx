/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkImageData.h"

#include "vtkImageRange3D.h"
#include "vtkImageEllipsoidSource.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkImageRange3D* vtkImageRange3D::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageRange3D");
  if(ret)
    {
    return (vtkImageRange3D*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageRange3D;
}





//----------------------------------------------------------------------------
// Construct an instance of vtkImageRange3D fitler.
// By default zero values are dilated.
vtkImageRange3D::vtkImageRange3D()
{
  this->HandleBoundaries = 1;
  this->KernelSize[0] = 1;
  this->KernelSize[1] = 1;
  this->KernelSize[2] = 1;

  this->Ellipse = vtkImageEllipsoidSource::New();
  // Setup the Ellipse to default size
  this->SetKernelSize(1, 1, 1);
}


//----------------------------------------------------------------------------
vtkImageRange3D::~vtkImageRange3D()
{
  if (this->Ellipse)
    {
    this->Ellipse->Delete();
    this->Ellipse = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkImageRange3D::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageSpatialFilter::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
// This method sets the size of the neighborhood.  It also sets the 
// default middle of the neighborhood and computes the eliptical foot print.
void vtkImageRange3D::SetKernelSize(int size0, int size1, int size2)
{
  int modified = 0;
  
  if (this->KernelSize[0] != size0)
    {
    modified = 1;
    this->KernelSize[0] = size0;
    this->KernelMiddle[0] = size0 / 2;
    }
  if (this->KernelSize[1] != size1)
    {
    modified = 1;
    this->KernelSize[1] = size1;
    this->KernelMiddle[1] = size1 / 2;
    }
  if (this->KernelSize[2] != size2)
    {
    modified = 1;
    this->KernelSize[2] = size2;
    this->KernelMiddle[2] = size2 / 2;
    }

  if (modified)
    {
    this->Modified();
    this->Ellipse->SetWholeExtent(0, this->KernelSize[0]-1, 
				  0, this->KernelSize[1]-1, 
				  0, this->KernelSize[2]-1);
    this->Ellipse->SetCenter((float)(this->KernelSize[0]-1)*0.5,
			     (float)(this->KernelSize[1]-1)*0.5,
			     (float)(this->KernelSize[2]-1)*0.5);
    this->Ellipse->SetRadius((float)(this->KernelSize[0])*0.5,
			     (float)(this->KernelSize[1])*0.5,
			     (float)(this->KernelSize[2])*0.5);
    // make sure scalars have been allocated (needed if multithreaded is used)
    this->Ellipse->GetOutput()->SetUpdateExtent(0, this->KernelSize[0]-1, 
						0, this->KernelSize[1]-1, 
						0, this->KernelSize[2]-1);
    this->Ellipse->GetOutput()->Update();
    }
}

//----------------------------------------------------------------------------
// Output is always float
void vtkImageRange3D::ExecuteInformation(vtkImageData *vtkNotUsed(inData), 
					 vtkImageData *outData)
{
  outData->SetScalarType(VTK_FLOAT);
}


//----------------------------------------------------------------------------
// This templated function executes the filter on any region,
// whether it needs boundary checking or not.
// If the filter needs to be faster, the function could be duplicated
// for strictly center (no boundary ) processing.
template <class T>
static void vtkImageRange3DExecute(vtkImageRange3D *self,
				   vtkImageData *mask,
				   vtkImageData *inData, T *inPtr, 
				   vtkImageData *outData, int *outExt, 
				   float *outPtr, int id)
{
  int *kernelMiddle, *kernelSize;
  // For looping though output (and input) pixels.
  int outMin0, outMax0, outMin1, outMax1, outMin2, outMax2;
  int outIdx0, outIdx1, outIdx2;
  int inInc0, inInc1, inInc2;
  int outInc0, outInc1, outInc2;
  T *inPtr0, *inPtr1, *inPtr2;
  float *outPtr0, *outPtr1, *outPtr2;
  int numComps, outIdxC;
  // For looping through hood pixels
  int hoodMin0, hoodMax0, hoodMin1, hoodMax1, hoodMin2, hoodMax2;
  int hoodIdx0, hoodIdx1, hoodIdx2;
  T *hoodPtr0, *hoodPtr1, *hoodPtr2;
  // For looping through the mask.
  unsigned char *maskPtr, *maskPtr0, *maskPtr1, *maskPtr2;
  int maskInc0, maskInc1, maskInc2;
  // The extent of the whole input image
  int inImageMin0, inImageMin1, inImageMin2;
  int inImageMax0, inImageMax1, inImageMax2;
  // to compute the range
  T pixelMin, pixelMax;
  unsigned long count = 0;
  unsigned long target;

  // Get information to march through data
  inData->GetIncrements(inInc0, inInc1, inInc2); 
  self->GetInput()->GetWholeExtent(inImageMin0, inImageMax0, inImageMin1,
				   inImageMax1, inImageMin2, inImageMax2);
  outData->GetIncrements(outInc0, outInc1, outInc2); 
  outMin0 = outExt[0];   outMax0 = outExt[1];
  outMin1 = outExt[2];   outMax1 = outExt[3];
  outMin2 = outExt[4];   outMax2 = outExt[5];
  numComps = outData->GetNumberOfScalarComponents();
  
  
  // Get ivars of this object (easier than making friends)
  kernelSize = self->GetKernelSize();
  kernelMiddle = self->GetKernelMiddle();
  hoodMin0 = - kernelMiddle[0];
  hoodMin1 = - kernelMiddle[1];
  hoodMin2 = - kernelMiddle[2];
  hoodMax0 = hoodMin0 + kernelSize[0] - 1;
  hoodMax1 = hoodMin1 + kernelSize[1] - 1;
  hoodMax2 = hoodMin2 + kernelSize[2] - 1;

  // Setup mask info
  maskPtr = (unsigned char *)(mask->GetScalarPointer());
  mask->GetIncrements(maskInc0, maskInc1, maskInc2);
  
  // in and out should be marching through corresponding pixels.
  inPtr = (T *)(inData->GetScalarPointer(outMin0, outMin1, outMin2));

  target = (unsigned long)(numComps*(outMax2-outMin2+1)*
			   (outMax1-outMin1+1)/50.0);
  target++;

  // loop through components
  for (outIdxC = 0; outIdxC < numComps; ++outIdxC)
    {
    // loop through pixels of output
    outPtr2 = outPtr;
    inPtr2 = inPtr;
    for (outIdx2 = outMin2; outIdx2 <= outMax2; ++outIdx2)
      {
      outPtr1 = outPtr2;
      inPtr1 = inPtr2;
      for (outIdx1 = outMin1; 
	   !self->AbortExecute && outIdx1 <= outMax1; ++outIdx1)
	{
	if (!id) 
	  {
	  if (!(count%target))
	    {
	    self->UpdateProgress(count/(50.0*target));
	    }
	  count++;
	  }

	outPtr0 = outPtr1;
	inPtr0 = inPtr1;
	for (outIdx0 = outMin0; outIdx0 <= outMax0; ++outIdx0)
	  {
	  
	  // Find min and max
	  pixelMin = pixelMax = *inPtr0;
	  // loop through neighborhood pixels
	  // as sort of a hack to handle boundaries, 
	  // input pointer will be marching through data that does not exist.
	  hoodPtr2 = inPtr0 - kernelMiddle[0] * inInc0 
	    - kernelMiddle[1] * inInc1 - kernelMiddle[2] * inInc2;
	  maskPtr2 = maskPtr;
	  for (hoodIdx2 = hoodMin2; hoodIdx2 <= hoodMax2; ++hoodIdx2)
	    {
	    hoodPtr1 = hoodPtr2;
	    maskPtr1 = maskPtr2;
	    for (hoodIdx1 = hoodMin1; hoodIdx1 <= hoodMax1; ++hoodIdx1)
	      {
	      hoodPtr0 = hoodPtr1;
	      maskPtr0 = maskPtr1;
	      for (hoodIdx0 = hoodMin0; hoodIdx0 <= hoodMax0; ++hoodIdx0)
		{
		// A quick but rather expensive way to handle boundaries
		if ( outIdx0 + hoodIdx0 >= inImageMin0 &&
		     outIdx0 + hoodIdx0 <= inImageMax0 &&
		     outIdx1 + hoodIdx1 >= inImageMin1 &&
		     outIdx1 + hoodIdx1 <= inImageMax1 &&
		     outIdx2 + hoodIdx2 >= inImageMin2 &&
		     outIdx2 + hoodIdx2 <= inImageMax2)
		  {
		  if (*maskPtr0)
		    {
		    if (*hoodPtr0 < pixelMin)
		      {
		      pixelMin = *hoodPtr0;
		      }
		    if (*hoodPtr0 > pixelMax)
		      {
		      pixelMax = *hoodPtr0;
		      }
		    }
		  }
		
		hoodPtr0 += inInc0;
		maskPtr0 += maskInc0;
		}
	      hoodPtr1 += inInc1;
	      maskPtr1 += maskInc1;
	      }
	    hoodPtr2 += inInc2;
	    maskPtr2 += maskInc2;
	    }
	  *outPtr0 = (float)(pixelMax - pixelMin);
	  
	  inPtr0 += inInc0;
	  outPtr0 += outInc0;
	  }
	inPtr1 += inInc1;
	outPtr1 += outInc1;
	}
      inPtr2 += inInc2;
      outPtr2 += outInc2;
      }
    ++inPtr;
    ++outPtr;
    }
}


		

//----------------------------------------------------------------------------
// This method contains the first switch statement that calls the correct
// templated function for the input and output Data types.
// It hanldes image boundaries, so the image does not shrink.
void vtkImageRange3D::ThreadedExecute(vtkImageData *inData, 
				      vtkImageData *outData, 
				      int outExt[6], int id)
{
  int inExt[6];
  this->ComputeInputUpdateExtent(inExt,outExt);
  void *inPtr = inData->GetScalarPointerForExtent(inExt);
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  vtkImageData *mask;

  // Error checking on mask
  this->Ellipse->GetOutput()->Update();
  mask = this->Ellipse->GetOutput();
  if (mask->GetScalarType() != VTK_UNSIGNED_CHAR)
    {
    vtkErrorMacro(<< "Execute: mask has wrong scalar type");
    return;
    }

  // this filter expects the output to be float
  if (outData->GetScalarType() != VTK_FLOAT)
    {
    vtkErrorMacro(<< "Execute: output ScalarType, " 
      << vtkImageScalarTypeNameMacro(outData->GetScalarType())
      << " must be float");
    return;
    }

  switch (inData->GetScalarType())
    {
    case VTK_DOUBLE:
      vtkImageRange3DExecute(this, mask, inData, (double *)(inPtr), 
				outData, outExt, (float *)(outPtr), id);
      break;
    case VTK_FLOAT:
      vtkImageRange3DExecute(this, mask, inData, (float *)(inPtr), 
				outData, outExt, (float *)(outPtr), id);
      break;
    case VTK_LONG:
      vtkImageRange3DExecute(this, mask, inData, (long *)(inPtr), 
				outData, outExt, (float *)(outPtr), id);
      break;
    case VTK_UNSIGNED_LONG:
      vtkImageRange3DExecute(this, mask, inData, (unsigned long *)(inPtr), 
				outData, outExt, (float *)(outPtr), id);
      break;
    case VTK_INT:
      vtkImageRange3DExecute(this, mask, inData, (int *)(inPtr), 
				outData, outExt, (float *)(outPtr), id);
      break;
    case VTK_UNSIGNED_INT:
      vtkImageRange3DExecute(this, mask, inData, (unsigned int *)(inPtr), 
				outData, outExt, (float *)(outPtr), id);
      break;
    case VTK_SHORT:
      vtkImageRange3DExecute(this, mask, inData, (short *)(inPtr), 
				outData, outExt, (float *)(outPtr), id);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageRange3DExecute(this, mask, inData, (unsigned short *)(inPtr), 
				outData, outExt, (float *)(outPtr), id);
      break;
    case VTK_CHAR:
      vtkImageRange3DExecute(this, mask, inData, (char *)(inPtr), 
				outData, outExt, (float *)(outPtr), id);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageRange3DExecute(this, mask, inData, (unsigned char *)(inPtr), 
				outData, outExt, (float *)(outPtr), id);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}


