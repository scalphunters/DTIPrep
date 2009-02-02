#include "IntensityMotionCheckPanel.h"
#include "IntensityMotionCheck.h"
#include "ThreadIntensityMotionCheck.h"
#include <QtGui>

#include "itkMetaDataDictionary.h"
#include "itkNrrdImageIO.h"
//#include "itkGDCMImageIO.h"
//#include "itkGDCMSeriesFileNames.h"
//#include "itkImageSeriesReader.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

//#include "itkQtAdaptor.h"
//#include "itkQtAdaptor.h"
//#include "itkQtLightIndicator.h"
//#include "itkQtProgressBar.h"

#include "XmlStreamReader.h"
#include "XmlStreamWriter.h"

IntensityMotionCheckPanel::IntensityMotionCheckPanel(QMainWindow *parent):QDockWidget(parent)
{
	setupUi(this);
	verticalLayout->setContentsMargins(0,0,0,0);
	progressBar->setValue(0);

	DwiImage= NULL ;
	protocal.clear();
	bDwiLoaded = false;

	QStringList labels;
	labels << tr("Parameter") << tr("Value");
	//treeWidget->header()->setResizeMode(QHeaderView::Stretch);
	treeWidget->setHeaderLabels(labels);
			
	ThreadIntensityMotionCheck = new CThreadIntensityMotionCheck;
	connect(ThreadIntensityMotionCheck, SIGNAL(kkk(int )), this, SLOT(UpdateProgressBar(const int )) );
}

void IntensityMotionCheckPanel::on_treeWidget_itemDoubleClicked(QTreeWidgetItem* item,int col)
{
	if(col==1 && pushButton_Editable->isChecked())
		treeWidget->openPersistentEditor(item,col);
}

void IntensityMotionCheckPanel::on_treeWidget_currentItemChanged( QTreeWidgetItem *current,  QTreeWidgetItem *previous)
{
	treeWidget->closePersistentEditor(previous,1); // does nothing if none open
}

  
IntensityMotionCheckPanel::~IntensityMotionCheckPanel(void)
{
}

void IntensityMotionCheckPanel::on_pushButton_RunPipeline_clicked( )
{
	//CIntensityMotionCheck IntensityMotionCheck(lineEdit_DWIFileName->text().toStdString());//,"report_54.txt");//
	//IntensityMotionCheck.SetProtocal(&protocal);
	//IntensityMotionCheck.SetQCResult(&qcResult);
	//IntensityMotionCheck.GetImagesInformation();
	//IntensityMotionCheck.CheckByProtocal();

	ThreadIntensityMotionCheck->SetFileName(lineEdit_DWIFileName->text().toStdString());
	ThreadIntensityMotionCheck->SetProtocal( &protocal);
	ThreadIntensityMotionCheck->SetQCResult(&qcResult);
	ThreadIntensityMotionCheck->start();
}


void IntensityMotionCheckPanel::UpdateProgressBar(int pos )
{
	 progressBar->setValue(pos);
}
void IntensityMotionCheckPanel::SetFileName(QString nrrd )
{
	lineEdit_DWIFileName->setText(nrrd);
}

void IntensityMotionCheckPanel::on_DwiBrowseButton_clicked()
{
	// QString DWINrrdFile = QFileDialog::getOpenFileName ( this, tr("Open nrrd DWI"), QDir::currentPath(), tr("Nrrd Files (*.nhdr *.nrrd)") );
	 QString DWINrrdFile = QFileDialog::getOpenFileName ( this, tr("Open nrrd DWI"), lineEdit_DWIFileName->text(), tr("Nrrd Files (*.nhdr *.nrrd)") );

	 if(DWINrrdFile.length()>0)
	 {
		SetFileName(DWINrrdFile);
		//LoadDwiImage();
	 }
}

void IntensityMotionCheckPanel::on_toolButton_ProtocalFileOpen_clicked( )
{
	 QString xmlFile = QFileDialog::getOpenFileName ( this, tr("Select Protocal"), lineEdit_DWIFileName->text(), tr("xml Files (*.xml)") );
	 if(xmlFile.length()>0)
	 {
		lineEdit_Protocal->setText(xmlFile);
	 }

	treeWidget->clear();
	protocal.clear();

	XmlStreamReader XmlReader(treeWidget);
	XmlReader.setProtocal( &protocal);
	XmlReader.readFile(xmlFile, XmlStreamReader::TreeWise);
	XmlReader.readFile(xmlFile, XmlStreamReader::ProtocalWise);

/*
	lineEdit_ReportFile->setText( QString::fromStdString(protocal.GetReportFileName()));
	lineEdit_QCOutputNrrd->setText( QString::fromStdString(protocal.GetIntensityMotionCheckProtocal().OutputFileName));
	lineEdit_EddyMotionCommand->setText( QString::fromStdString(protocal.GetEddyMotionCorrectionProtocal().EddyMotionCommand));
	lineEdit_EddyMotionInput->setText( QString::fromStdString(protocal.GetEddyMotionCorrectionProtocal().InputFileName));
	lineEdit_EddyMotionOutput->setText( QString::fromStdString(protocal.GetEddyMotionCorrectionProtocal().OutputFileName));
	lineEdit_dtiestim->setText( QString::fromStdString(protocal.GetDTIProtocal().dtiestimCommand));
	lineEdit_dtiprocess->setText( QString::fromStdString(protocal.GetDTIProtocal().dtiprocessCommand));
	lineEdit_MaskFile->setText( QString::fromStdString(protocal.GetDTIProtocal().mask));
	lineEdit_TensorFile->setText( QString::fromStdString(protocal.GetDTIProtocal().tensor));
	
	spinBox_Baseline_Threshold->setRange ( 0, 100000 );
	spinBox_Baseline_Threshold->setSingleStep ( 10 );
	spinBox_Baseline_Threshold->setValue( protocal.GetDTIProtocal().baselineThreshold );
*/
	protocal.print();
}

bool IntensityMotionCheckPanel::LoadDwiImage()
{
	DwiFileName = lineEdit_DWIFileName->text().toStdString();

// use with windows
	//std::string str;
	//str=DwiFileName.substr(0,DwiFileName.find_last_of('\\')+1);
	//std::cout<< str<<std::endl;
	//::SetCurrentDirectory(str.c_str());
	
	itk::NrrdImageIO::Pointer  NrrdImageIO = itk::NrrdImageIO::New();

	if(DwiFileName.length()!=0)
	{
		try
		{
			DwiReader = DwiReaderType::New();
			DwiReader->SetImageIO(NrrdImageIO);
			DwiReader->SetFileName(DwiFileName);
			std::cout<< "Loading"<<DwiFileName<<" ... ";
			DwiReader->Update();
		}
		catch(itk::ExceptionObject & e)
		{
			std::cout<< e.GetDescription()<<std::endl;
			bDwiLoaded=false;
			return false;
		}
	}
	else
	{
		std::cout<< "Dwi file name not set"<<std::endl;
		bDwiLoaded=false;
		return false;
	}
	std::cout<< "Done "<<std::endl;

	DwiImage = DwiReader->GetOutput();
	bDwiLoaded = true;  
	
	std::cout<<"Image Dimension"<< DwiImage->GetLargestPossibleRegion().GetSize().GetSizeDimension()<<": ";
	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[0]<<" ";
	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[1]<<" ";
	std::cout<<DwiImage->GetLargestPossibleRegion().GetSize()[2]<<std::endl;

	std::cout<<"Pixel Vector Length: "<<DwiImage->GetVectorLength()<<std::endl;

	return bDwiLoaded;
}


bool IntensityMotionCheckPanel::GetGridentDirections()
{
	if(!bDwiLoaded) LoadDwiImage();
	if(!bDwiLoaded)
	{
		std::cout<<"DWI load error, no Gradient Direction Loaded"<<std::endl;
		bGetGridentDirections=false;
		return false;
	}		

	itk::MetaDataDictionary imgMetaDictionary = DwiImage->GetMetaDataDictionary();    //
	std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
	std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
	std::string metaString;

	//int numberOfImages=0;
	TensorReconstructionImageFilterType::GradientDirectionType vect3d;
	
	GradientDirectionContainer = GradientDirectionContainerType::New();
	GradientDirectionContainer->clear();
	
	for ( ; itKey != imgMetaKeys.end(); itKey ++)
	{
		//double x,y,z;
		itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
		if (itKey->find("DWMRI_gradient") != std::string::npos)
		{ 
			std::istringstream iss(metaString);
			iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
			//sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
			//vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
			GradientDirectionContainer->push_back(vect3d);

		}
		else if (itKey->find("DWMRI_b-value") != std::string::npos)
		{
			readb0 = true;
			b0 = atof(metaString.c_str());
			//std::cout<<"b Value: "<<b0<<std::endl;
		}
	}

	if(!readb0)
	{
		std::cout<<"BValue not specified in header file" <<std::endl;
		return false;
	}
	if(GradientDirectionContainer->Size()<=6) 
	{
		std::cout<<"Gradient Images Less than 7" <<std::endl;
		bGetGridentDirections=false;
		return false;
	}

	std::cout<<"b Value: "<<b0<<std::endl;
	std::cout<<"DWI image gradient count: "<<DwiImage->GetVectorLength()<<std::endl;

	for( unsigned int i = 0; i<DwiImage->GetVectorLength();i++ )//GradientDirectionContainer->Size()
	{
		std::cout<<"Gradient Direction "<<i<<": \t[";
		std::cout<<GradientDirectionContainer->at(i)[0]<<",\t";
		std::cout<<GradientDirectionContainer->at(i)[1]<<",\t";
		std::cout<<GradientDirectionContainer->at(i)[2]<<" ]"<<std::endl;
	}

	bGetGridentDirections=true;
	return true;
}

/*
void IntensityMotionCheckPanel::on_comboBox_Protocal_currentIndexChanged(QString protocalName)
{
	//std::cout<<"current protocal: "<<protocal<<std::endl;
	treeWidget->clear();
	protocal.clear();

	if(protocalName!=tr("none"))
	{
		QString str;
		str.append( QCoreApplication::applicationDirPath() );
		str.append(tr("/"));
		str.append(protocalName);
		str.append(tr(".xml"));
		XmlStreamReader XmlReader(treeWidget);
		XmlReader.setProtocal( &protocal);
		XmlReader.readFile(str, XmlStreamReader::TreeWise);
		XmlReader.readFile(str, XmlStreamReader::ProtocalWise);
		
		protocal.print();
	}
}
*/

void IntensityMotionCheckPanel::on_treeWidget_itemChanged(QTreeWidgetItem * item, int column)
{
	pushButton_Save->setEnabled(pushButton_Editable->isCheckable());
}

void IntensityMotionCheckPanel::on_pushButton_Editable_toggled( bool)
{	
	treeWidget->closePersistentEditor(treeWidget->currentItem(),1); // does nothing if none open

}

void IntensityMotionCheckPanel::on_pushButton_Save_clicked( )
{
	if(lineEdit_Protocal->text().length()>0)
	{
		XmlStreamWriter XmlWriter(treeWidget);
		XmlWriter.writeXml(lineEdit_Protocal->text());
	}
	else
	{
		 QString xmlFile = QFileDialog::getSaveFileName( this, tr("Save Protocal As"),lineEdit_Protocal->text(),  tr("xml Files (*.xml)") );
		 if(xmlFile.length()>0)
		 {
			lineEdit_Protocal->setText(xmlFile);
			XmlStreamWriter XmlWriter(treeWidget);
			XmlWriter.writeXml(xmlFile);
		 }
	}
/*	if(comboBox_Protocal->currentText()!=tr("none"))
	{
		QString str;
		str.append( QCoreApplication::applicationDirPath() );
		str.append(tr("/"));
		str.append(comboBox_Protocal->currentText());
		str.append(tr(".xml"));
		XmlStreamWriter XmlWriter(treeWidget);
		XmlWriter.writeXml(str);
	}*/
}
/*
void IntensityMotionCheckPanel::on_pushButton_Identity_clicked( )
{
	lineEdit_MeasurementFrame11->setText(tr("1.0000"));
	lineEdit_MeasurementFrame12->setText(tr("0.0000"));
	lineEdit_MeasurementFrame13->setText(tr("0.0000"));

	lineEdit_MeasurementFrame21->setText(tr("0.0000"));
	lineEdit_MeasurementFrame22->setText(tr("1.0000"));
	lineEdit_MeasurementFrame23->setText(tr("0.0000"));

	lineEdit_MeasurementFrame31->setText(tr("0.0000"));
	lineEdit_MeasurementFrame32->setText(tr("0.0000"));
	lineEdit_MeasurementFrame33->setText(tr("1.0000"));
}

void IntensityMotionCheckPanel::on_pushButton_ExchangeXY_clicked( )
{
	double temp;

	temp = lineEdit_MeasurementFrame11->text().toDouble();
	lineEdit_MeasurementFrame11->setText( QString::number( lineEdit_MeasurementFrame12->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame12->setText( QString::number(temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame21->text().toDouble();
	lineEdit_MeasurementFrame21->setText( QString::number( lineEdit_MeasurementFrame22->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame22->setText( QString::number(temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame31->text().toDouble();
	lineEdit_MeasurementFrame31->setText( QString::number( lineEdit_MeasurementFrame32->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame32->setText( QString::number(temp,'f',4 ) );
}


void IntensityMotionCheckPanel::on_pushButton_ExchangeXZ_clicked( )
{
	double temp;

	temp = lineEdit_MeasurementFrame11->text().toDouble();
	lineEdit_MeasurementFrame11->setText( QString::number( lineEdit_MeasurementFrame13->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame13->setText( QString::number(temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame21->text().toDouble();
	lineEdit_MeasurementFrame21->setText( QString::number( lineEdit_MeasurementFrame23->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame23->setText( QString::number(temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame31->text().toDouble();
	lineEdit_MeasurementFrame31->setText( QString::number( lineEdit_MeasurementFrame33->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame33->setText( QString::number(temp,'f',4 ) );

}


void IntensityMotionCheckPanel::on_pushButton_ExchangeYZ_clicked( )
{
	double temp;

	temp = lineEdit_MeasurementFrame12->text().toDouble();
	lineEdit_MeasurementFrame12->setText( QString::number( lineEdit_MeasurementFrame13->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame13->setText( QString::number(temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame22->text().toDouble();
	lineEdit_MeasurementFrame22->setText( QString::number( lineEdit_MeasurementFrame23->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame23->setText( QString::number(temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame32->text().toDouble();
	lineEdit_MeasurementFrame32->setText( QString::number( lineEdit_MeasurementFrame33->text().toDouble(),'f',4 ) );
	lineEdit_MeasurementFrame33->setText( QString::number(temp,'f',4 ) );
}


void IntensityMotionCheckPanel::on_pushButton_FlipX_clicked( )
{
	double temp;

	temp = lineEdit_MeasurementFrame11->text().toDouble();
	lineEdit_MeasurementFrame11->setText( QString::number( -temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame21->text().toDouble();
	lineEdit_MeasurementFrame21->setText( QString::number( -temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame31->text().toDouble();
	lineEdit_MeasurementFrame31->setText( QString::number( -temp,'f',4 ) );
}


void IntensityMotionCheckPanel::on_pushButton_FlipY_clicked( )
{
	double temp;

	temp = lineEdit_MeasurementFrame12->text().toDouble();
	lineEdit_MeasurementFrame12->setText( QString::number( -temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame22->text().toDouble();
	lineEdit_MeasurementFrame22->setText( QString::number( -temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame32->text().toDouble();
	lineEdit_MeasurementFrame32->setText( QString::number( -temp,'f',4 ) );
}


void IntensityMotionCheckPanel::on_pushButton_FlipZ_clicked( )
{
	double temp;

	temp = lineEdit_MeasurementFrame13->text().toDouble();
	lineEdit_MeasurementFrame13->setText( QString::number( -temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame23->text().toDouble();
	lineEdit_MeasurementFrame23->setText( QString::number( -temp,'f',4 ) );

	temp = lineEdit_MeasurementFrame33->text().toDouble();
	lineEdit_MeasurementFrame33->setText( QString::number( -temp,'f',4 ) );
}
*/
void IntensityMotionCheckPanel::UpdatePanelDWI( DwiImageType::Pointer DwiImage )
{
	lineEdit_SizeX->setText( QString::number(DwiImage->GetLargestPossibleRegion().GetSize()[0]) );
	lineEdit_SizeY->setText( QString::number(DwiImage->GetLargestPossibleRegion().GetSize()[1]));
	lineEdit_SizeZ->setText( QString::number(DwiImage->GetLargestPossibleRegion().GetSize()[2]));

	lineEdit_OriginX->setText( QString::number(DwiImage->GetOrigin()[0], 'f'));
	lineEdit_OriginY->setText( QString::number(DwiImage->GetOrigin()[1], 'f'));
	lineEdit_OriginZ->setText( QString::number(DwiImage->GetOrigin()[2], 'f'));

	lineEdit_SpacingX->setText( QString::number(DwiImage->GetSpacing()[0], 'f'));
	lineEdit_SpacingY->setText( QString::number(DwiImage->GetSpacing()[1], 'f'));
	lineEdit_SpacingZ->setText( QString::number(DwiImage->GetSpacing()[2], 'f'));

	GetGridentDirections();
	
	QTreeWidgetItem *bValue = new QTreeWidgetItem(treeWidget_DiffusionInformation);
	bValue->setText(0, tr("DWMRI_b-value"));
	bValue->setText(1, QString::number(b0, 'f', 0));

	for(int i=0; i< GradientDirectionContainer->size();i++)
	{
		QString str;
		
		str.append(QString::number(GradientDirectionContainer->ElementAt(i)[0], 'f'));
		str.append(tr(" "));
		str.append(QString::number(GradientDirectionContainer->ElementAt(i)[1], 'f'));
		str.append(tr(" "));
		str.append(QString::number(GradientDirectionContainer->ElementAt(i)[2], 'f'));


		QTreeWidgetItem *gradient = new QTreeWidgetItem(treeWidget_DiffusionInformation);
		gradient->setText(0, tr("DWMRI_gradient_")+ QString::number(i));
		gradient->setText(1, str);
	}

/////////////////////////////////////////////////

	itk::MetaDataDictionary imgMetaDictionary = DwiImage->GetMetaDataDictionary(); 
	std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
	std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
	std::string metaString;

  //  measurement frame 
  if(imgMetaDictionary.HasKey("NRRD_measurement frame"))
  {
    // measurement frame
    vnl_matrix<double> mf(3,3);
    // imaging frame
    vnl_matrix<double> imgf(3,3);
    std::vector<std::vector<double> > nrrdmf;
    itk::ExposeMetaData<std::vector<std::vector<double> > >(imgMetaDictionary,"NRRD_measurement frame",nrrdmf);

    imgf = DwiImage->GetDirection().GetVnlMatrix();
    
	//Image frame
    std::cout << "Image frame: " << std::endl;
    std::cout << imgf << std::endl;

	lineEdit_SpaceDir11->setText( QString::number(imgf(0,0), 'f'));
	lineEdit_SpaceDir12->setText( QString::number(imgf(0,1), 'f'));
	lineEdit_SpaceDir13->setText( QString::number(imgf(0,2), 'f'));
	lineEdit_SpaceDir21->setText( QString::number(imgf(1,0), 'f'));
	lineEdit_SpaceDir22->setText( QString::number(imgf(1,1), 'f'));
	lineEdit_SpaceDir23->setText( QString::number(imgf(1,2), 'f'));
	lineEdit_SpaceDir31->setText( QString::number(imgf(2,0), 'f'));
	lineEdit_SpaceDir32->setText( QString::number(imgf(2,1), 'f'));
	lineEdit_SpaceDir33->setText( QString::number(imgf(2,2), 'f'));

    for(unsigned int i = 0; i < 3; ++i)
    {
      for(unsigned int j = 0; j < 3; ++j)
      {
        mf(i,j) = nrrdmf[j][i];
        nrrdmf[j][i] = imgf(i,j);
      }
    }
  
	// Meausurement frame
	std::cout << "Meausurement frame: " << std::endl;
	std::cout << mf << std::endl;

	lineEdit_MeasurementFrame11->setText( QString::number(mf(0,0), 'f'));
	lineEdit_MeasurementFrame12->setText( QString::number(mf(0,1), 'f'));
	lineEdit_MeasurementFrame13->setText( QString::number(mf(0,2), 'f'));
	lineEdit_MeasurementFrame21->setText( QString::number(mf(1,0), 'f'));
	lineEdit_MeasurementFrame22->setText( QString::number(mf(1,1), 'f'));
	lineEdit_MeasurementFrame23->setText( QString::number(mf(1,2), 'f'));
	lineEdit_MeasurementFrame31->setText( QString::number(mf(2,0), 'f'));
	lineEdit_MeasurementFrame32->setText( QString::number(mf(2,1), 'f'));
	lineEdit_MeasurementFrame33->setText( QString::number(mf(2,2), 'f'));

 
    //itk::EncapsulateMetaData<std::vector<std::vector<double> > >(dict,NRRD_MEASUREMENT_KEY,nrrdmf);
  }

  // space
  itk::ExposeMetaData<std::string> (imgMetaDictionary, "NRRD_space", metaString);
  std::cout<<"space: "<<metaString.c_str()<<std::endl;
  comboBox_Space->setCurrentIndex(  comboBox_Space->findText ( QString::fromStdString( metaString), Qt::MatchExactly));
}

///////////////////////////////////
/*
void IntensityMotionCheckPanel::on_pushButton_InformationCheck_clicked( )
{

}

void IntensityMotionCheckPanel::on_pushButton_DWIInfoUpdateProtocal_clicked( )
{

}

*/
/*
void IntensityMotionCheckPanel::on_toolButton_ReportFile_clicked( )
{
	 QString ReportFile = QFileDialog::getSaveFileName ( this, tr("Save Report File As"), lineEdit_ReportFile->text() , tr("text Files (*.txt)") );
	 if (!ReportFile.isEmpty()) 
	 {
		 std::cout<<"Report File: "<<ReportFile.toStdString()<<std::endl;
		 this->lineEdit_ReportFile->setText(ReportFile);
	 }
	 return;
}

void IntensityMotionCheckPanel::on_toolButton_QCOutputNrrd_clicked( )
{
	 QString QCOutputNrrd = QFileDialog::getSaveFileName ( this, tr("Save EddyMotion Output As"), lineEdit_QCOutputNrrd->text() , tr("nrrd Files (*.nhdr)") );
	 if (!QCOutputNrrd.isEmpty()) 
	 {
		 std::cout<<"QCOutput Nrrd File: "<<QCOutputNrrd.toStdString()<<std::endl;
		 this->lineEdit_QCOutputNrrd->setText(QCOutputNrrd);
		 this->lineEdit_EddyMotionInput->setText(QCOutputNrrd);
	 }
	 return;
}

void IntensityMotionCheckPanel::on_pushButton_QCUpdateProtocol_clicked( )
{

}

void IntensityMotionCheckPanel::on_pushButton_QCCheck_clicked( )
{

}
*/
/*
void IntensityMotionCheckPanel::on_toolButton_EddyMotionCommand_clicked( )
{
	QString EddyMotionCommand= QFileDialog::getOpenFileName(this, tr("Set the dtiestim command"),lineEdit_EddyMotionCommand->text());
	
	 if (!EddyMotionCommand.isEmpty()) 
	 {
		 std::cout<<"EddyMotion  Command: "<<EddyMotionCommand.toStdString()<<std::endl;
		 this->lineEdit_EddyMotionCommand->setText(EddyMotionCommand);
	 }
}

void IntensityMotionCheckPanel::on_toolButton_EddyMotionInput_clicked( )
{
	QString EddyMotionInput= QFileDialog::getOpenFileName(this, tr("Set tEddyMotion Input"),lineEdit_EddyMotionInput->text(), tr("nrrd Files (*.nhdr)") );
	
	 if (!EddyMotionInput.isEmpty()) 
	 {
		 std::cout<<"EddyMotion Input: "<<EddyMotionInput.toStdString()<<std::endl;
		 this->lineEdit_EddyMotionInput->setText(EddyMotionInput);
	 }
}

void IntensityMotionCheckPanel::on_toolButton_EddyMotionOutput_clicked( )
{
	 QString EddyMotionOutput = QFileDialog::getSaveFileName ( this, tr("Save EddyMotion Output As"), lineEdit_EddyMotionOutput->text() , tr("nrrd Files (*.nhdr)") );
	 if (!EddyMotionOutput.isEmpty()) 
	 {
		 std::cout<<"EddyMotion Output File: "<<EddyMotionOutput.toStdString()<<std::endl;
		 this->lineEdit_EddyMotionOutput->setText(EddyMotionOutput);
	 }
	 return;
}

void IntensityMotionCheckPanel::on_pushButton_CorrectUpdateProtocol_clicked( )
{

}

void IntensityMotionCheckPanel::on_pushButton_EddyMotionCorrect_clicked( )
{

}


void IntensityMotionCheckPanel::on_toolButton_dtiestim_clicked( )
{
	QString dtiestimCommand= QFileDialog::getOpenFileName(this, tr("Set the dtiestim command"),lineEdit_dtiestim->text());
	
	 if (!dtiestimCommand.isEmpty()) 
	 {
		 std::cout<<"dtiestim command: "<<dtiestimCommand.toStdString()<<std::endl;
		 this->lineEdit_dtiestim->setText(dtiestimCommand);
	 }
}

void IntensityMotionCheckPanel::on_toolButton_dtiprocess_clicked( )
{
	QString dtiprocessCommand= QFileDialog::getOpenFileName(this, tr("Set the dtiprocess command"),lineEdit_dtiprocess->text());
	
	 if (!dtiprocessCommand.isEmpty()) 
	 {
		 std::cout<<"dtiprocess command: "<<dtiprocessCommand.toStdString()<<std::endl;
		 this->lineEdit_dtiprocess->setText(dtiprocessCommand);
	 }
}

void IntensityMotionCheckPanel::on_toolButton_MaskFile_clicked( )
{
	QString MaskFile= QFileDialog::getOpenFileName(this, tr("Set the Mask File"),lineEdit_MaskFile->text());
	
	 if (!MaskFile.isEmpty()) 
	 {
		 std::cout<<"Mask File: "<<MaskFile.toStdString()<<std::endl;
		 this->lineEdit_MaskFile->setText(MaskFile);
	 }

}

void IntensityMotionCheckPanel::on_toolButton_TensorFile_clicked( )
{
	 QString TensorFile = QFileDialog::getSaveFileName ( this, tr("Save Tensor Files As"), lineEdit_TensorFile->text() , tr("Tensor Files (*.nhdr)") );
	 if (!TensorFile.isEmpty()) 
	 {
		 std::cout<<"Tensor File: "<<TensorFile.toStdString()<<std::endl;
		 //nrrdPath=nrrFile;
	     this->lineEdit_TensorFile->setText(TensorFile);
	 }
	 return;
}


void IntensityMotionCheckPanel::on_pushButton_DTIUpdateProtocol_clicked( )
{

}

void IntensityMotionCheckPanel::on_pushButton_DTICompute_clicked( )
{

}
*/