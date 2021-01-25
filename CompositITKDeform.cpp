#include "itkVector.h"
#include "itkImage.h"
#include "itkWarpVectorImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkWarpImageFilter.h"
#include "itkNearestNeighborInterpolateImageFunction.h"



typedef float InternalPixelType; // for internal processing usage
const    unsigned int   ImageDimension = 3;
typedef itk::Vector< InternalPixelType, ImageDimension >    VectorPixelType;
typedef itk::Image<  VectorPixelType, ImageDimension >        DeformationFieldType;
typedef itk::Image< unsigned char, ImageDimension> InputImageType;


void ReadDeformationField(char* filename, DeformationFieldType::Pointer &deformationfield)
{
	typedef itk::ImageFileReader<DeformationFieldType> DeformationFieldReaderType;
    DeformationFieldReaderType::Pointer deformationFieldReader = DeformationFieldReaderType::New();
    deformationFieldReader->SetFileName( filename );
    try
    {
        deformationFieldReader->Update();
    }
    catch( itk::ExceptionObject & err )
    {
        std::cerr << err << std::endl;
        return;
    }
    deformationfield = deformationFieldReader->GetOutput();
    return;
}

void WriteDeformationField(char* filename, DeformationFieldType::Pointer deformationfield)
{
	typedef itk::ImageFileWriter<DeformationFieldType> DeformationFieldWriterType;
    DeformationFieldWriterType::Pointer deformationFieldWriter = DeformationFieldWriterType::New();
    deformationFieldWriter->SetFileName( filename );
    deformationFieldWriter->SetInput (deformationfield);
    deformationFieldWriter->SetUseCompression( false );
    try
    {
        deformationFieldWriter->Update();
    }
    catch( itk::ExceptionObject & err )
    {
        std::cerr << err << std::endl;
        return;
    }
    return;
}


void ComposeDeformationFields(DeformationFieldType::Pointer input,
                              DeformationFieldType::Pointer deformationField,
                              DeformationFieldType::Pointer &composedDeformationField)
{
	typedef itk::WarpVectorImageFilter< DeformationFieldType, DeformationFieldType, DeformationFieldType>  WarpVectorFilterType;
	typedef itk::AddImageFilter< DeformationFieldType, DeformationFieldType, DeformationFieldType > AddImageFilterType;
    WarpVectorFilterType::Pointer  vectorWarper = WarpVectorFilterType::New();
    vectorWarper->SetInput( input );
    //vectorWarper->SetDeformationField( input );
	vectorWarper->SetDisplacementField( deformationField );
    vectorWarper->SetOutputOrigin(input->GetOrigin());
    vectorWarper->SetOutputSpacing(input->GetSpacing());
    vectorWarper->SetOutputDirection(input->GetDirection());

    AddImageFilterType::Pointer addImageSum = AddImageFilterType::New();
    addImageSum->SetInput1(vectorWarper->GetOutput());
    addImageSum->SetInput2(deformationField);
    try
    {
        addImageSum->Update();
    }
    catch(itk::ExceptionObject & err)
    {
        std::cerr << err << std::endl;
        return;
    }
    composedDeformationField = addImageSum->GetOutput();
    composedDeformationField->DisconnectPipeline();
    return;
}

void TransformIntensityImage(DeformationFieldType::Pointer deformationField, char *InputImageFileName, char *OutputImageFileName)
{
	InputImageType::Pointer input_image = InputImageType::New();
	typedef itk::ImageFileReader<InputImageType> InputImageReader;
	typedef itk::ImageFileWriter<InputImageType> InputImageWriter;
	typedef itk::WarpImageFilter<InputImageType, InputImageType, DeformationFieldType >  WarperType;

	InputImageReader::Pointer reader = InputImageReader::New();
	InputImageWriter::Pointer writer = InputImageWriter::New();
	WarperType::Pointer warper = WarperType::New();

	reader->SetFileName(InputImageFileName);

	warper->SetInput( reader->GetOutput() );
	warper->SetOutputSpacing( deformationField->GetSpacing() );
	warper->SetOutputOrigin( deformationField->GetOrigin() );
	warper->SetOutputDirection( deformationField->GetDirection() );
	warper->SetDisplacementField( deformationField );
	writer->SetFileName(OutputImageFileName);
	//writer->SetDirection(reader->GetDirection());
	writer->SetInput(warper->GetOutput());	
	
	try{
		writer->Update();
	}
	catch(itk::ExceptionObject &err)
	{
		std::cout<<"Unexpected error."<<std::endl;
		std::cout<<err<<std::endl;
		exit(EXIT_FAILURE);
	}
}


void TransformSegmentImage(DeformationFieldType::Pointer deformationField, char *InputImageFileName, char *OutputImageFileName)
{
	InputImageType::Pointer input_image = InputImageType::New();
	typedef itk::ImageFileReader<InputImageType> InputImageReader;
	typedef itk::ImageFileWriter<InputImageType> InputImageWriter;
	typedef itk::WarpImageFilter<InputImageType, InputImageType, DeformationFieldType >  WarperType;
	typedef itk::NearestNeighborInterpolateImageFunction<InputImageType, WarperType::CoordRepType> InterpolateType;

	InputImageReader::Pointer reader = InputImageReader::New();
	InputImageWriter::Pointer writer = InputImageWriter::New();
	WarperType::Pointer warper = WarperType::New();
	InterpolateType::Pointer interp = InterpolateType::New(); 

	reader->SetFileName(InputImageFileName);

	warper->SetInput( reader->GetOutput() );
	warper->SetOutputSpacing( deformationField->GetSpacing() );
	warper->SetOutputOrigin( deformationField->GetOrigin() );
	warper->SetOutputDirection( deformationField->GetDirection() );
	warper->SetInterpolator(interp);
	warper->SetDisplacementField( deformationField );
	writer->SetFileName(OutputImageFileName);
	writer->SetInput(warper->GetOutput());
	try{
		writer->Update();
	}
	catch(itk::ExceptionObject &err)
	{
		std::cout<<"Unexpected error."<<std::endl;
		std::cout<<err<<std::endl;
		exit(EXIT_FAILURE);
	}
}

int main( int argc, char *argv[] )
{
    if ( argc <= 2 )
    {
        std::cerr << "Missing Parameters of ComposeDeformationField: " << std::endl;
        std::cerr << "Usage: ComposeDeformationField.exe" << std::endl;
        std::cerr << "       1st_DeformationFieldFileName[.mha] 2nd_DeformationFieldFileName[.mha]" << std::endl;
        std::cerr << "       [ComposedDeformationFieldFileName:output.mha]   " << std::endl;
		std::cerr << "		 [OriginalIntensityImage WarpedIntensityImageAfterComposition]"<<std::endl;
		std::cerr << "       [OriginalSegmentImage WarpedSegmentImageAfterComposition]"<<std::endl;
        return EXIT_FAILURE;
    }
    // argv[1]: 1st deformation field file name
    // argv[2]: 2nd deformation field file name
    char composedDeformationFieldFileName[] = "output.nii.gz";
    bool isDefaultComposedDeformationFieldFileName = true;
    if ( argc > 3)
		isDefaultComposedDeformationFieldFileName = false;
    // start ......
    ///////////////////////////////////////
    DeformationFieldType::Pointer inputDeformationField = DeformationFieldType::New();
    ReadDeformationField(argv[1], inputDeformationField);
    DeformationFieldType::Pointer deformationField = DeformationFieldType::New();
    ReadDeformationField(argv[2], deformationField);

    DeformationFieldType::Pointer composedDeformationField = DeformationFieldType::New();
    ComposeDeformationFields(inputDeformationField, deformationField, composedDeformationField);

    if (isDefaultComposedDeformationFieldFileName)
        WriteDeformationField(composedDeformationFieldFileName, composedDeformationField);
    else
        WriteDeformationField(argv[3], composedDeformationField);
	if(argc>4)
	{
		TransformIntensityImage(composedDeformationField, argv[4], argv[5]);
	}
	if(argc>6)
	{
		TransformSegmentImage(composedDeformationField, argv[6], argv[7]);
	}

	std::cerr << " done!" << std::endl;
    return 1;
}
