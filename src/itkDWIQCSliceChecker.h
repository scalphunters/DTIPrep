/*=========================================================================

Program:   NeuroLib
Module:    $file: itkDWIQCSliceChecker.h $
Language:  C++
Date:      $Date: 2009-08-27 01:39:40 $
Version:   $Revision: 1.2 $
Author:    Zhexing Liu (liuzhexing@gmail.com)

Copyright (c) NIRAL, UNC. All rights reserved.
See http://www.niral.unc.edu for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkDWIQCSliceChecker_h
#define _itkDWIQCSliceChecker_h

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"
#include "itkVectorContainer.h"

namespace itk
{
	/** \class DWIQCSliceChecker
	* \brief DWI QC by Slice-wise Check.
	*
	* WIQCSliceChecker DWI QC by Slice-wise Check.
	*
	* \ingroup Multithreaded
	* \ingroup Streamed
	*/

	template<class TImageType>
	class ITK_EXPORT DWIQCSliceChecker : 
		public ImageToImageFilter< TImageType, TImageType>
	{

	public:

		typedef enum
		{
			Report_New = 0,
			Report_Append,
		} ReportFileMode;

		struct struDiffusionDir
		{
			std::vector< double > gradientDir;
			int repetitionNumber;
		};

		/** Standard class typedefs. */
		typedef DWIQCSliceChecker							 Self;
		typedef ImageToImageFilter< TImageType, TImageType>  Superclass;
		typedef SmartPointer<Self>							Pointer;
		typedef SmartPointer<const Self>					ConstPointer;

		itkNewMacro(Self);

		/** Run-time type information (and related methods). */
		itkTypeMacro(DWIQCSliceChecker, ImageToImageFilter);

		/** Typedef to images */
		typedef TImageType									OutputImageType;
		typedef TImageType									InputImageType;
		typedef typename OutputImageType::Pointer           OutputImagePointer;
		typedef typename InputImageType::ConstPointer       InputImageConstPointer;
		typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;

		typedef unsigned short						DwiPixelType;
		typedef itk::Image<DwiPixelType, 2>			SliceImageType;
		typedef itk::Image<DwiPixelType, 3>			GradientImageType;

		typedef vnl_vector_fixed< double, 3 >       GradientDirectionType;

		/** Container to hold gradient directions of the 'n' DW measurements */
		typedef VectorContainer< unsigned int, GradientDirectionType >   GradientDirectionContainerType;
		
		/** ImageDimension enumeration. */
		itkStaticConstMacro(ImageDimension, unsigned int, TImageType::ImageDimension );

		/** Get & Set the HeadSkipRatio. */
		itkGetConstMacro( HeadSkipRatio, float );
		itkSetMacro( HeadSkipRatio, float );

		/** Get & Set the TailSkipRatio */
		itkGetConstMacro( TailSkipRatio, float );
		itkSetMacro( TailSkipRatio, float );

		/** Get & Set the StdevTimes */
		itkGetConstMacro( BaselineStdevTimes, float );
		itkSetMacro( BaselineStdevTimes, float );

		/** Get & Set the StdevTimes */
		itkGetConstMacro( GradientStdevTimes, float );
		itkSetMacro( GradientStdevTimes, float );

		/** Get & Set the CheckTimes */
		itkGetConstMacro( CheckTimes, int );
		itkSetMacro( CheckTimes, int );

		/** Get & Set the check status */
		itkBooleanMacro(CheckDone);
		itkGetConstMacro(CheckDone, bool);
		itkSetMacro(CheckDone, bool);

		/** Get & Set the report file mode */
		itkGetConstMacro( ReportFileMode, int );
		itkSetMacro( ReportFileMode, int  );

		/** Get & Set the ReportFilename */
		itkGetConstMacro( ReportFileName, std::string );
		itkSetMacro( ReportFileName, std::string  );


		/** DWIQCSliceChecker produces an image which is a different vector length
			* than its input image. As such, DWIQCSliceChecker needs to provide
			* an implementation for GenerateOutputInformation() in order to inform
			* the pipeline execution model.The original documentation of this
			* method is below.
			* \sa ProcessObject::GenerateOutputInformaton() */
		virtual void GenerateOutputInformation();

		
	protected:
		DWIQCSliceChecker();
		~DWIQCSliceChecker();

		void PrintSelf(std::ostream& os, Indent indent) const;
		void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,int threadId );  

	private:
		DWIQCSliceChecker(const Self&); //purposely not implemented
		void operator=(const Self&);    //purposely not implemented

		/** check parameters */
		float m_HeadSkipRatio ;
		float m_TailSkipRatio ;
		float m_BaselineStdevTimes ;
		float m_GradientStdevTimes ;
		
		/** check times, ineratively while <=0  */
		int m_CheckTimes;

		/** indicate whether chech is done */
		bool m_CheckDone;

		/** report filename */
		std::string m_ReportFileName ;

		/** report file mode */
		int m_ReportFileMode ;


		/** excluded gradients filename */
		OutputImagePointer      excludedDwiImage;

		/** input info */
		int baselineNumber;
		int bValueNumber;
		int gradientDirNumber;
		int repetitionNumber;
		int gradientNumber;
		
		/** output info */
		int baselineLeftNumber;
		int bValueLeftNumber;
		int gradientDirLeftNumber;
		int gradientLeftNumber;
		std::vector<int> repetitionLeftNumber;

		/** b value */
		double b0 ;
		
		/** container to hold gradient directions */
		GradientDirectionContainerType::Pointer  m_GradientDirectionContainer;

		/** container to hold input gradient directions histogram */
		std::vector<struDiffusionDir> DiffusionDirHistInput;

		/** container to hold input b values */
		std::vector<double> bValues;

		/** container to hold output gradient directions histogram */
		std::vector<struDiffusionDir> DiffusionDirHistOutput;

		/** for all gradients  slice wise correlation */
		std::vector<double> gradientMeans;
		std::vector<double> gradientDeviations;

		/** for all baseline slice wise correlation */
		std::vector<double> baselineMeans;
		std::vector<double> baselineDeviations;

		/** for all multi-bValued gradient slice wise correlation(after quardatic fitting) */
		std::vector<double> quardraticFittedMeans;
		std::vector<double> quardraticFittedDeviations;


		/** initialize qcResullts */
		std::vector< std::vector<double> >	ResultsContainer;// starts from #1 slice, "correlation<=0" means a "bad slice"
		std::vector<bool> qcResults;							

		void parseGridentDirections();
		void collectDiffusionStatistics();
		void initializeQCResullts();
		void calculateCorrelations();
		void report();
		void check();
		void iterativeCheck();
		void collectLeftDiffusionStatistics();
		void writeReport();

	public:
		OutputImagePointer GetExcludedGradiennts();
		inline std::vector<bool> getQCResults() { return qcResults; };
		inline GradientDirectionContainerType::Pointer  GetGradientDirectionContainer()
					{ return m_GradientDirectionContainer; };

		inline int getBaselineNumber()		{   return baselineNumber;};
		inline int getBValueNumber()		{   return bValueNumber;};
		inline int getGradientDirNumber()	{   return gradientDirNumber;};
		inline int getRepetitionNumber()	{   return repetitionNumber;};
		inline int getGradientNumber()		{   return gradientNumber;};

		inline int getBaselineLeftNumber()		{   return baselineLeftNumber;};
		inline int getBValueLeftNumber()		{   return bValueLeftNumber;};
		inline int getGradientDirLeftNumber()	{   return gradientDirLeftNumber;};
		inline int getGradientLeftNumber()		{   return gradientLeftNumber;};
		inline std::vector<int> getRepetitionLeftNumber()	{   return repetitionLeftNumber;};

	};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDWIQCSliceChecker.cpp"
#endif

#endif