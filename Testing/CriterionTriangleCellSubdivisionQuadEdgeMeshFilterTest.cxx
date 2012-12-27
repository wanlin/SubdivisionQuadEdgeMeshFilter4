/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#if defined( _MSC_VER )
#pragma warning ( disable : 4786 )
#endif

#include "itkModifiedButterflyTriangleCellSubdivisionQuadEdgeMeshFilter.h"
#include "itkLinearTriangleCellSubdivisionQuadEdgeMeshFilter.h"
#include "itkLoopTriangleCellSubdivisionQuadEdgeMeshFilter.h"
#include "itkSquareThreeTriangleCellSubdivisionQuadEdgeMeshFilter.h"
#include "itkConditionalTriangleCellSubdivisionQuadEdgeMeshFilter.h"
#include "itkCellAreaTriangleCellSubdivisionCriterion.h"
#include "itkQuadEdgeMeshParamMatrixCoefficients.h"
#if ( ITK_VERSION_MAJOR < 4 )
#include "itkQuadEdgeMeshSmoothing.h"
#else
#include "itkSmoothingQuadEdgeMeshFilter.h"
#endif
#include "itkMeshFileReader.h"
#include "itkMeshFileWriter.h"

template< typename TTriangleCellSubdivisionFilter >
int ConditionalTriangleCellSubdivisionFilterTest( int argc, char *argv[] )
{

  typedef TTriangleCellSubdivisionFilter                                TriangleCellSubdivisionFilterType;
  typedef typename TTriangleCellSubdivisionFilter::Pointer              TriangleCellSubdivisionFilterPointer;
  typedef typename TriangleCellSubdivisionFilterType::InputMeshType     InputMeshType;
  typedef typename TriangleCellSubdivisionFilterType::OutputMeshType    OutputMeshType;

  typedef itk::CellAreaTriangleCellSubdivisionCriterion< typename TriangleCellSubdivisionFilterType::CellSubdivisionFilterType > CriterionType;
  typedef typename CriterionType::Pointer                                                    CriterionPointer;
  typedef itk::MeshFileReader< InputMeshType >  ReaderType;
  typedef itk::MeshFileWriter< OutputMeshType > WriterType;

  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( argv[1] );
  try
    {
    reader->Update();
    }
  catch ( itk::ExceptionObject & exp )
    {
    std::cerr << "Exception thrown while reading the input file " << std::endl;
    std::cerr << exp << std::endl;
    return EXIT_FAILURE;
    }

  TriangleCellSubdivisionFilterPointer subdivision = TriangleCellSubdivisionFilterType::New();

  CriterionPointer criterion = CriterionType::New();
  criterion->SetMaximumArea(1.0);
  if ( argc >= 5 )
    {
    float area = std::atof(argv[4]);
    criterion->SetMaximumArea(area);
    }

  subdivision->SetSubdivisionCriterion( criterion.GetPointer() );
  subdivision->SetInput( reader->GetOutput() );
  subdivision->Update();
  typename OutputMeshType::Pointer output = subdivision->GetOutput();

  bool smoothing = false;
  if ( argc >= 6 )
  {
  smoothing = true;
  }

  if ( smoothing )
    {
#if ( ITK_VERSION_MAJOR < 4 )
    typedef itk::QuadEdgeMeshSmoothing< OutputMeshType, OutputMeshType > OutputMeshSmoothingFilterType;
#else
    typedef itk::SmoothingQuadEdgeMeshFilter< OutputMeshType, OutputMeshType > OutputMeshSmoothingFilterType;
#endif
    typedef itk::MatrixCoefficients< OutputMeshType >                          MatrixCoefficientsType;
    typedef itk::OnesMatrixCoefficients< OutputMeshType >                      OnesMatrixCoefficientsType;
    typedef itk::InverseEuclideanDistanceMatrixCoefficients< OutputMeshType >  InverseEuclideanDistanceMatrixCoefficientsType;
    typedef itk::ConformalMatrixCoefficients< OutputMeshType >                 ConformalMatrixCoefficientsType;
    typedef itk::AuthalicMatrixCoefficients< OutputMeshType >                  AuthalicMatrixCoefficientsType;
    typedef itk::IntrinsicMatrixCoefficients< OutputMeshType >                 IntrinsicMatrixCoefficientsType;
    typedef itk::HarmonicMatrixCoefficients< OutputMeshType >                  HarmonicMatrixCoefficientsType;

    OnesMatrixCoefficientsType             coef;
    typename OutputMeshSmoothingFilterType::Pointer meshSmoothingFilter = OutputMeshSmoothingFilterType::New();
    meshSmoothingFilter->SetInput( output );
    meshSmoothingFilter->SetCoefficientsMethod(&coef);
    meshSmoothingFilter->SetDelaunayConforming(1);
    meshSmoothingFilter->SetNumberOfIterations(1);
    meshSmoothingFilter->Update();

    output->Graft( meshSmoothingFilter->GetOutput() );
    }

  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(argv[2]);
  writer->SetInput( output );

  try
    {
    writer->Update();
    }
  catch ( itk::ExceptionObject & exp )
    {
    std::cerr << "Exception thrown while writting the output file " << std::endl;
    std::cerr << exp << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
  if ( argc < 3 )
    {
    std::cerr << "Missing Parameters " << std::endl;
    std::cerr << "Usage: " << argv[0];
    std::cerr << " inputMeshFile  outputMeshFile subdivisionType area" << std::endl;
    std::cerr << " 0 : ModifiedButterfly " << std::endl;
    std::cerr << " 1 : Linear " << std::endl;
    std::cerr << " 2 : Loop " << std::endl;
    std::cerr << " 3 : Squarethree " << std::endl;
    return EXIT_FAILURE;
    }

  typedef float MeshPixelType;
  const unsigned int Dimension = 3;

  typedef itk::QuadEdgeMesh< MeshPixelType, Dimension > InputMeshType;
  typedef itk::QuadEdgeMesh< MeshPixelType, Dimension > OutputMeshType;

  typedef itk::ModifiedButterflyTriangleCellSubdivisionQuadEdgeMeshFilter< OutputMeshType, OutputMeshType >   ModifiedButterflySubdivisionFilterType;
  typedef itk::LinearTriangleCellSubdivisionQuadEdgeMeshFilter< OutputMeshType, OutputMeshType >      LinearSubdivisionFilterType;
  typedef itk::LoopTriangleCellSubdivisionQuadEdgeMeshFilter< OutputMeshType, OutputMeshType >        LoopSubdivisionFilterType;
  typedef itk::SquareThreeTriangleCellSubdivisionQuadEdgeMeshFilter< OutputMeshType, OutputMeshType > SquareThreeSubdivisionFilterType;

  typedef itk::ConditionalTriangleCellSubdivisionQuadEdgeMeshFilter< InputMeshType, ModifiedButterflySubdivisionFilterType > ConditionalModifiedButterflySubdivisionFilterType;
  typedef itk::ConditionalTriangleCellSubdivisionQuadEdgeMeshFilter< InputMeshType, LinearSubdivisionFilterType > ConditionalLinearSubdivisionFilterType;
  typedef itk::ConditionalTriangleCellSubdivisionQuadEdgeMeshFilter< InputMeshType, LoopSubdivisionFilterType > ConditionalLoopSubdivisionFilterType;
  typedef itk::ConditionalTriangleCellSubdivisionQuadEdgeMeshFilter< InputMeshType, SquareThreeSubdivisionFilterType > ConditionalSquareThreeSubdivisionFilterType;

  if ( argc >= 4 )
    {
    int type = std::atoi(argv[3]);

    switch ( type )
      {
      case 0:
        return ConditionalTriangleCellSubdivisionFilterTest< ConditionalModifiedButterflySubdivisionFilterType >( argc, argv );
      case 1:
        return ConditionalTriangleCellSubdivisionFilterTest< ConditionalLinearSubdivisionFilterType >( argc, argv );
      case 2:
        return ConditionalTriangleCellSubdivisionFilterTest< ConditionalLoopSubdivisionFilterType >( argc, argv );
      case 3:
        return ConditionalTriangleCellSubdivisionFilterTest< ConditionalSquareThreeSubdivisionFilterType >( argc, argv );
      default:
        std::cerr << "Invalid subdivision type : " << type << std::endl;
        return EXIT_FAILURE;
      }
    }
  else
    {
    std::cerr << "You must have subdivision type " << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
