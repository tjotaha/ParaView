/*******************************************************************/
/*                               XDMF                              */
/*                   eXtensible Data Model and Format              */
/*                                                                 */
/*  Id : $Id$  */
/*  Date : $Date$ */
/*  Version : $Revision$ */
/*                                                                 */
/*  Author:                                                        */
/*     Jerry A. Clarke                                             */
/*     clarke@arl.army.mil                                         */
/*     US Army Research Laboratory                                 */
/*     Aberdeen Proving Ground, MD                                 */
/*                                                                 */
/*     Copyright @ 2002 US Army Research Laboratory                */
/*     All Rights Reserved                                         */
/*     See Copyright.txt or http://www.arl.hpc.mil/ice for details */
/*                                                                 */
/*     This software is distributed WITHOUT ANY WARRANTY; without  */
/*     even the implied warranty of MERCHANTABILITY or FITNESS     */
/*     FOR A PARTICULAR PURPOSE.  See the above copyright notice   */
/*     for more information.                                       */
/*                                                                 */
/*******************************************************************/
#include "XdmfAttribute.h"

#include "XdmfDataItem.h"
#include "XdmfDataDesc.h"
#include "XdmfArray.h"
#include "XdmfDOM.h"

XdmfAttribute::XdmfAttribute() {
  this->AttributeType = XDMF_ATTRIBUTE_TYPE_NONE;
  this->ValuesAreMine = 1;
  this->Values = NULL;
  this->ShapeDesc = new XdmfDataDesc();
  this->Active = 0;
  }

XdmfAttribute::~XdmfAttribute() {
  if( this->ValuesAreMine && this->Values )  delete this->Values;
  delete this->ShapeDesc;
  }


XdmfInt32
XdmfAttribute::Adopt( XdmfElement *Child){
    if(Child && (
        XDMF_WORD_CMP(Child->GetElementName(), "DataItem") ||
        XDMF_WORD_CMP(Child->GetElementName(), "Information")
        )){
        return(XdmfElement::Adopt(Child));
    }else{
        XdmfErrorMessage("Attribute can only Adopt DataItem or Information elements");
    }
    return(XDMF_FAIL);
}


XdmfConstString
XdmfAttribute::GetAttributeTypeAsString( void ){
  switch ( this->AttributeType ){
    case XDMF_ATTRIBUTE_TYPE_SCALAR :
      return("Scalar");
    case XDMF_ATTRIBUTE_TYPE_VECTOR :
      return("Vector");
    case XDMF_ATTRIBUTE_TYPE_TENSOR :
      return("Tensor");
    case XDMF_ATTRIBUTE_TYPE_MATRIX :
      return("Matrix");
    default :
      break;
    }
return("None");
}

XdmfInt32
XdmfAttribute::SetAttributeTypeFromString( XdmfConstString attributeType ){
XdmfInt64  Dimensions[3];

XdmfDebug("Setting Type to " << attributeType );
if( XDMF_WORD_CMP( attributeType, "Scalar" ) ) {
  this->AttributeType = XDMF_ATTRIBUTE_TYPE_SCALAR;
  Dimensions[0] = 1;
  this->ShapeDesc->SetShape( 1, Dimensions );
} else if( XDMF_WORD_CMP( attributeType, "Vector" ) ) {
  this->AttributeType = XDMF_ATTRIBUTE_TYPE_VECTOR;
  Dimensions[0] = 3;
  this->ShapeDesc->SetShape( 1, Dimensions );
} else if( XDMF_WORD_CMP( attributeType, "Tensor" ) ) {
  this->AttributeType = XDMF_ATTRIBUTE_TYPE_TENSOR;
  Dimensions[0] = 3;
  Dimensions[1] = 3;
  this->ShapeDesc->SetShape( 2, Dimensions );
} else if( XDMF_WORD_CMP( attributeType, "Matrix" ) ) {
  this->AttributeType = XDMF_ATTRIBUTE_TYPE_MATRIX;
} else {
  XdmfErrorMessage("Unknown Attribute Type " << attributeType );
  return( XDMF_FAIL );
  }
return( XDMF_SUCCESS );
}

XdmfConstString
XdmfAttribute::GetAttributeCenterAsString( void ){
  switch ( this->AttributeCenter ){
    case XDMF_ATTRIBUTE_CENTER_GRID :
      return( "Grid" );
    case XDMF_ATTRIBUTE_CENTER_CELL :
      return( "Cell" );
    case XDMF_ATTRIBUTE_CENTER_FACE :
      return( "Face" );
    case XDMF_ATTRIBUTE_CENTER_EDGE :
      return( "Edge" );
    case XDMF_ATTRIBUTE_CENTER_NODE :
      return( "Node" );
    default :
      break;
    }
return( "Node" );
}

XdmfInt32
XdmfAttribute::SetAttributeCenterFromString( XdmfConstString attributeCenter ){

if( XDMF_WORD_CMP( attributeCenter, "Grid" ) ) {
  this->AttributeCenter = XDMF_ATTRIBUTE_CENTER_GRID;
} else if( XDMF_WORD_CMP( attributeCenter, "Cell" ) ) {
  this->AttributeCenter = XDMF_ATTRIBUTE_CENTER_CELL;
} else if( XDMF_WORD_CMP( attributeCenter, "Face" ) ) {
  this->AttributeCenter = XDMF_ATTRIBUTE_CENTER_FACE;
} else if( XDMF_WORD_CMP( attributeCenter, "Edge" ) ) {
  this->AttributeCenter = XDMF_ATTRIBUTE_CENTER_EDGE;
} else if( XDMF_WORD_CMP( attributeCenter, "Node" ) ) {
  this->AttributeCenter = XDMF_ATTRIBUTE_CENTER_NODE;
} else {
  XdmfErrorMessage("Unknown Attribute Center " << attributeCenter );
  return( XDMF_FAIL );
  }
return( XDMF_SUCCESS );
}

XdmfInt32
XdmfAttribute::UpdateInformation() {

XdmfConstString  Attribute;

if(XdmfElement::UpdateInformation() != XDMF_SUCCESS) return(XDMF_FAIL);
if( XDMF_WORD_CMP(this->GetElementType(), "Attribute") == 0){
    XdmfErrorMessage("Element type" << this->GetElementType() << " is not of type 'Attribute'");
    return(XDMF_FAIL);
}

Attribute = this->Get( "Type" );
if( Attribute ){
  this->SetAttributeTypeFromString( Attribute );
} else {
  this->AttributeType = XDMF_ATTRIBUTE_TYPE_SCALAR;
}

Attribute = this->Get( "Active" );
this->Active = 0;
if ( Attribute ){
  if( XDMF_WORD_CMP( Attribute, "1" ) ) {
    this->Active = 1;
  }
}

Attribute = this->Get( "Center" );
if( Attribute ){
  this->SetAttributeCenterFromString( Attribute );
} else {
  this->AttributeCenter = XDMF_ATTRIBUTE_CENTER_NODE;
}
Attribute = this->Get( "Dimensions" );
if( Attribute ){
  this->ShapeDesc->SetShapeFromString( Attribute );
}
if(!this->Name) this->SetName(GetUnique("Attribute_"));
return( XDMF_SUCCESS );
}

XdmfInt32
XdmfAttribute::Update() {

XdmfInt32  Status;
XdmfXmlNode  ValuesNode;
XdmfDataItem ValueReader;

XdmfConstString  Attribute;

if(XdmfElement::Update() != XDMF_SUCCESS) return(XDMF_FAIL);
if( this->AttributeType == XDMF_ATTRIBUTE_TYPE_NONE ){
  Status = this->UpdateInformation();
  if( Status == XDMF_FAIL ) {
    XdmfErrorMessage("Can't Initialize");
    return( XDMF_FAIL );
    }
  }

ValuesNode = this->DOM->FindElement( NULL, 0, Element );
if( ValuesNode ){
  ValueReader.SetDOM( this->DOM );
  if( this->ValuesAreMine && this->Values ){
    delete this->Values;
    this->Values = NULL;
  } else {
  }
  if(ValueReader.SetElement(ValuesNode) == XDMF_FAIL) return(XDMF_FAIL);
  if(ValueReader.UpdateInformation() == XDMF_FAIL) return(XDMF_FAIL);
  if(ValueReader.Update() == XDMF_FAIL) return(XDMF_FAIL);
  this->Values = ValueReader.GetArray();
  if( !this->Values ) {
    XdmfErrorMessage("Error Retriving Data Values");
    return( XDMF_FAIL );
    }
} else {
  XdmfErrorMessage("Element has no Data");
  return( XDMF_FAIL );
  }
return( XDMF_SUCCESS );
}
