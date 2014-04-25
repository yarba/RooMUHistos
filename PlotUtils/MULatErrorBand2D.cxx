#ifndef MNV_MULatErrorBand2D_cxx
#define MNV_MULatErrorBand2D_cxx 1

#include "MULatErrorBand2D.h"
#include "HistogramUtils.h"
#include <algorithm>

using namespace PlotUtils;

ClassImp(MULatErrorBand2D);

MULatErrorBand2D::MULatErrorBand2D( const std::string& name, const TH2D* base, const unsigned int nHists /* = 1000 */ ) :
		TH2D( *base )
{
	SetName( name.c_str() );
	SetTitle( name.c_str() );

	fNHists = nHists; 
	char tmpName[256];

	//set the good colors
	if( fGoodColors.size() == 0 )
	{
		fGoodColors.push_back( 2 );
		fGoodColors.push_back( 4 );
		fGoodColors.push_back( 6 );
		fGoodColors.push_back( 8 );
		fGoodColors.push_back( 9 );
		for( int i = 20; i < 50; i++ )
			fGoodColors.push_back( i );
	}

	for( unsigned int i = 0; i < fNHists; i++ )
	{
		sprintf(tmpName, "%s_universe%d", name.c_str(), i );
		TH2D *tmp = new TH2D( *base );
		tmp->Reset();
		tmp->SetName(tmpName);

		//give the universe histos a style and color
		tmp->SetLineColor( fGoodColors[ i % fGoodColors.size() ] );
		tmp->SetLineStyle( i % 10 + 1 );

		fHists.push_back( tmp );
	}

	if( nHists < 10 )
		fUseSpreadError = true;
	else
		fUseSpreadError = false;

}

MULatErrorBand2D::MULatErrorBand2D( const std::string& name, const TH2D* base, const std::vector<TH2D*>& hists ) :
	TH2D (*base)
{

	SetName( name.c_str() );
	SetTitle( name.c_str() );

	fNHists = hists.size();
	char tmpName[256];

	//set the good colors
	if( fGoodColors.size() == 0 )
	{
		fGoodColors.push_back( 2 );
		fGoodColors.push_back( 4 );
		fGoodColors.push_back( 6 );
		fGoodColors.push_back( 8 );
		fGoodColors.push_back( 9 );
		for( int i = 20; i < 50; i++ )
			fGoodColors.push_back( i );
	}

	std::vector<TH2D*>::const_iterator it = hists.begin();
	int it_pos = 0;
	for( ; it != hists.end(); ++it, ++it_pos )
	{
		sprintf(tmpName, "%s_universe%d", name.c_str(), it_pos );
		TH2D *tmp = new TH2D( **it );
		tmp->SetName(tmpName);

		//give the universe histos a style and color
		tmp->SetLineColor( fGoodColors[ it_pos % fGoodColors.size() ] );
		tmp->SetLineStyle( it_pos % 10 + 1 );

		fHists.push_back( tmp );
	}

	if( fNHists < 10 )
		fUseSpreadError = true;
	else
		fUseSpreadError = false;

}

MULatErrorBand2D::MULatErrorBand2D( const MULatErrorBand2D& h ) :
	TH2D( h )
{
	//!Deep copy the variables
	DeepCopy( h );
}

MULatErrorBand2D& MULatErrorBand2D::operator=( const MULatErrorBand2D& h ) 
{
	//! If this is me, no copy is needed
	if( this == &h )
		return *this;

	//! Call the base class's assignment
	TH2D::operator=(h);

	//! Delete and clear the hists vector
	for( unsigned int i = 0; i < h.fNHists; ++i )
		delete fHists[i];
	fHists.clear();
	fGoodColors.clear();

	DeepCopy( h );

	return *this;
}

void MULatErrorBand2D::DeepCopy( const MULatErrorBand2D& h )
{
	fUseSpreadError = h.GetUseSpreadError();
	fNHists = h.fNHists;
	for( unsigned int i = 0; i < fNHists; ++i )
		fHists.push_back( new TH2D(*h.GetHist(i)) );

	//set the good colors
	if( fGoodColors.size() == 0 )
	{
		fGoodColors.push_back( 2 );
		fGoodColors.push_back( 4 );
		fGoodColors.push_back( 6 );
		fGoodColors.push_back( 8 );
		fGoodColors.push_back( 9 );
		for( int i = 20; i < 50; i++ )
			fGoodColors.push_back( i );
	}
}

bool MULatErrorBand2D::Fill( const double xval, const double yval, const double *xshifts, const double *yshifts, const double cvweight /*= 1.0*/, const bool fillcv /*= true*/, const double* weights /*= 0*/ )
{
	//! Fill the CV hist with the CV weight and value
	if( fillcv ) 
		this->TH2D::Fill( xval, yval, cvweight );

	//! Use FindBin to get the CV bin 
	const int cvbin = FindBin( xval, yval );
	int x_cvbin, y_cvbin, z_cvbin;
	GetBinXYZ(cvbin, x_cvbin, y_cvbin, z_cvbin);

	//! Add to the bin content of all the universes
	//! Assume that the bin we will fill is close to the bin at the central value
	//!@todo find a clever way to find the bin based on the cvbin information
	/*
		 const int x_nbins = GetNbinsX(), y_nbins = GetNbinsY();
		 const double x_cvBinLowEdge  = GetBinLowEdge( x_cvbin );
		 const double y_cvBinLowEdge  = GetBinLowEdge( y_cvbin );
		 const double x_cvBinHighEdge = x_cvBinLowEdge + GetBinWidth( x_cvbin );
		 const double y_cvBinHighEdge = y_cvBinLowEdge + GetBinWidth( y_cvbin );
		 */
	for( unsigned int i = 0; i != fNHists; ++i )
	{
		//!@todo Check consistency of this condition
		if( MUHist::IsNotPhysicalShift( xshifts[i] ) || MUHist::IsNotPhysicalShift( yshifts[i] ) )
			continue;

		const double x_shiftVal = xval + xshifts[i];
		const double y_shiftVal = yval + yshifts[i];
		//!@todo find a clever way to find the bin based on the cvbin information
		int bin = FindBin( x_shiftVal, y_shiftVal );
		/*
			 int x_bin = x_cvbin, y_bin = y_cvbin;

			 if( x_shiftVal < x_cvBinLowEdge )
			 {
		//! If the shift puts the value below the low edge of the CV bin, start looking down.  If not found, then go to underflow at bin=0.
		for( ; x_bin != 0; --x_bin )
		if( GetBinLowEdge( x_bin ) < x_shiftVal )
		break;
		}
		else if( x_cvBinHighEdge < x_shiftVal )
		{
		//! If the shift puts the value above the high edge of the CV bin, start looking up.  If not found, then go to underflow at bin=nbins+1
		for( ; x_bin != x_nbins+1; ++x_bin )
		if( x_shiftVal <  (GetBinLowEdge( x_bin ) + GetBinWidth( x_bin ) ) )
		break;
		}
		if( y_shiftVal < y_cvBinLowEdge )
		{
		//! If the shift puts the value below the low edge of the CV bin, start looking down.  If not found, then go to underflow at bin=0.
		for( ; y_bin != 0; --y_bin )
		if( GetBinLowEdge( y_bin ) < y_shiftVal )
		break;
		}
		else if( y_cvBinHighEdge < y_shiftVal )
		{
		//! If the shift puts the value above the high edge of the CV bin, start looking up.  If not found, then go to underflow at bin=nbins+1
		for( ; y_bin != y_nbins+1; ++y_bin )
		if( y_shiftVal <  (GetBinLowEdge( y_bin ) + GetBinWidth( y_bin ) ) )
		break;
		}

		//! Now that we know the bin, add the weight to its content
		int bin = GetBin(x_bin, y_bin);
		*/
		if( 0==weights )
		{
			fHists[i]->AddBinContent( bin, cvweight );
		}
		else
		{
			fHists[i]->AddBinContent( bin, cvweight*weights[i] );
		}

	}

	return true;
}

bool MULatErrorBand2D::Fill( const double xval, const double yval, const double xshiftDown, const double xshiftUp, const double yshiftDown, const double yshiftUp, const double cvweight /*= 1.0*/, const bool fillcv /*= true*/ )
{
	//! Throw an exception if nUniverses is not 2
	if( fNHists != 2 )
	{
		std::cout << "ERROR [MULatErrorBand2D::Fill] - You used the specialized 2 shifts version, but you do not have 2 universes." << std::endl;
		throw 1;
	}

	//! put the weights in an array and use the standard Fill
	double xshifts[2] = {xshiftDown, xshiftUp};
	double yshifts[2] = {yshiftDown, yshiftUp};

	return Fill( xval, yval, xshifts, yshifts, cvweight, fillcv);
}

TH2D MULatErrorBand2D::GetErrorBand(bool asFrac /*= false*/ , bool cov_area_normalize /*= false*/) const
{
	TH2D errBand( *this );
	errBand.Reset();
	const int lowBin = 0;
	const int highBinX = GetNbinsX() + 1;
	const int highBinY = GetNbinsY() + 1;
	const int highBin  = GetBin( highBinX, highBinY );

	TMatrixD covmx(highBin+1,highBin+1);
	covmx = CalcCovMx(cov_area_normalize,asFrac);


	for( int i = lowBin; i <= highBin; ++i )
	{
		double err = (covmx[i][i]>0.)? sqrt( covmx[i][i] ): 0.; //Protect against odd sqrt(0) = NaN errors.

		errBand.SetBinContent( i, err );
		errBand.SetBinError(i, 0.);
	}

	return errBand;
}

const TH2D *MULatErrorBand2D::GetHist( unsigned int i ) const
{
	if( i >= fNHists )
	{
		Error("GetHist", "Cannot return histogram of universe %d because this object has %d universes.", i, fNHists);
		return NULL;
	}
	return fHists[i];
}

TH2D *MULatErrorBand2D::GetHist( unsigned int i )
{
	if( i >= fNHists )
	{
		Error("GetHist", "Cannot return histogram of universe %d because this object has %d universes.", i, fNHists);
		return NULL;
	}
	//! Temporary Fix to a problem when Drawing fHists[] (histogram has content but plot is blank)
	TH2D *hist = dynamic_cast<TH2D*>(fHists[i]->Clone());
	const int Nbins = fHists[i]->GetBin( fHists[i]->GetNbinsX()+1, fHists[i]->GetNbinsY()+1);
	for (int i=0; i<=Nbins ; ++i)
	{
		hist->SetBinContent( i , hist->GetBinContent(i) );
		hist->SetBinError( i , hist->GetBinError(i) );
	}

	return hist;
	//return fHists[i];
}

TMatrixD MULatErrorBand2D::CalcCovMx(bool area_normalize, bool asFrac) const
{
	//Calculating the Mean
	TH2D hmean = TH2D(*this);

	// if there's more than one varied universe, calculate the mean; if not, use the CV
	// histo as the 'mean'
	if(fNHists > 1)
		hmean.Reset();

	//!Storing Area Normalization Factors for the many universes
	//! @todo Need to Check this! 
	std::vector<double> normFactors;

	for( unsigned int j = 0; j < fNHists; ++j ) {
		if (area_normalize)
		{
			double area_scale = fHists[j]->Integral();
			normFactors.push_back(area_scale/Integral());

			if (fHists[j]->Integral()!=0) //just in case
			{
				fHists[j]->Scale(Integral()/area_scale);
			}
		}
		if(fNHists>1)
			hmean.Add(fHists[j]);
	}

	if(fNHists>1)
		hmean.Scale(1.0/(double) fNHists);

	// Calculating Covariance Matrix
	//! @todo what to do with underflow( bin=0 ) and overlow ( bin=nbins+1 )?
	const int lowBin = 0; // considering underflow bin
	const int highBin = hmean.GetBin( hmean.GetNbinsX()+1, hmean.GetNbinsY()+1 ); // considering under/overflow

	TMatrixD covmx(highBin+1, highBin+1);

	if (fUseSpreadError)
	{
		std::vector< std::vector<double> > binValsVec(highBin+1);
		//calculate maximum and minum values for every bin
		for( int i = lowBin; i <= highBin; ++i )
		{
			std::vector<double> binVals;
			for( unsigned int j = 0; j < fNHists; ++j )
			{
				const double val = fHists[j]->GetBinContent(i);
				binVals.push_back( val );
			}
			//get the CV value for this bin
			const double cv = GetBinContent(i);
			binVals.push_back( cv );
			sort( binVals.begin(), binVals.end() );

			binValsVec.at(i) =  binVals ;
		}

		//! For spread errors in only 1 universe take the full max spread
		//! For spread errors in less than 10 universes take 1/2 the max spread
		//! For spread errors with more than 10 universes, use the interquartile spread
		for( int i = lowBin; i <= highBin; ++i )
		{
			for( int k = i; k <= highBin; ++k )
			{
				if ( fNHists == 1 )
					covmx[i][k] = ( (binValsVec.at(i)).back()- (binValsVec.at(i)).front() ) * ( (binValsVec.at(k)).back()- (binValsVec.at(k)).front() );
				else if ( fNHists < 10 )
					covmx[i][k] = ( (binValsVec.at(i)).back() - (binValsVec.at(i)).front() ) * ( (binValsVec.at(k)).back() - (binValsVec.at(k)).front() ) / 4. ;
				else
					covmx[i][k] = MUHist::GetInterquartileRange( binValsVec.at(i) ) * MUHist::GetInterquartileRange( binValsVec.at(k) ) * pow(MUHist::InterquartileRangeToSigma,2);

				covmx[k][i] = covmx[i][k];
			}
		}
	}
	else
	{
		//Calculating Covariance
		for( unsigned int j = 0; j < fNHists; ++j ) 
		{
			for( int i = lowBin; i <= highBin; ++i )
			{
				double xi=fHists[j]->GetBinContent(i);
				double ximean=hmean.GetBinContent(i);
				for( int k = i; k <= highBin; ++k )
				{
					double xk=fHists[j]->GetBinContent(k);
					double xkmean=hmean.GetBinContent(k);
					covmx[i][k] +=(xi-ximean)*(xk-xkmean);
				}
			}
		}

		for( int i = lowBin; i <= highBin; ++i )
		{
			for( int k = i; k <= highBin; ++k )
			{
				covmx[i][k]/=((double)fNHists);
				covmx[k][i]=covmx[i][k];
			}
		}
	}
	if (asFrac)
	{
		for( int i = lowBin; i <= highBin; ++i )
		{
			for( int k = i; k <= highBin; ++k )
			{
				//double sign = covmx[i][k]>0 ? 1. : -1. ;
				//Gettting the the CV value for bin i
				const double cv_i = GetBinContent(i);
				const double cv_k = GetBinContent(k);

				//covmx[i][k]=sign * sqrt(TMath::Abs(covmx[i][k])/(cv_i * cv_k));
				covmx[i][k]= ( cv_i != 0. && cv_k != 0. ) ? covmx[i][k]/(cv_i * cv_k) : 0.;
				covmx[k][i]=covmx[i][k];
			}
		}
	}

	//! Getting fHists back to normal
	//! @todo Need to Check this
	if (area_normalize)
	{
		for( unsigned int j = 0; j < fNHists; ++j ) {
			if (fHists[j]->Integral()!=0)
			{
				fHists[j]->Scale(normFactors[j]);
			}
		}
	}
	return covmx;
}

Bool_t MULatErrorBand2D::Add( const MULatErrorBand2D* h1, const Double_t c1 /*= 1.*/ )
{
	//! Check that we all have the same number of universes.
	if( h1->GetNHists() != this->GetNHists() )
	{
		Error("Add", "Attempt to Add with different numbers of universes" );
		return kFALSE;
	}

	//! Call Add on the CVHists
	this->TH2D::Add( h1, c1 );

	//! Call Add for all universes
	for( unsigned int iHist = 0; iHist != fNHists; ++iHist )
		fHists[iHist]->Add( h1->GetHist(iHist), c1 );

	return true;
}

Bool_t MULatErrorBand2D::Multiply( const MULatErrorBand2D* h1, const MULatErrorBand2D* h2, Double_t c1 /*= 1*/, Double_t c2 /*= 1*/ )
{
	//! Check that we all have the same number of universes.
	if( h1->GetNHists() != h2->GetNHists() || h1->GetNHists() != this->GetNHists() )
	{
		Error("Multiply", "Attempt to divide by different numbers of universes" );
		return kFALSE;
	}

	//! Call Multiply on the CVHists
	this->TH2D::Multiply( h1, h2, c1, c2 );

	//! Call Multiply for all universes
	for( unsigned int iHist = 0; iHist != fNHists; ++iHist )
		fHists[iHist]->Multiply( h1->GetHist(iHist), h2->GetHist(iHist), c1, c2 );

	return true;
}

Bool_t MULatErrorBand2D::DivideSingle( const MULatErrorBand2D* h1, const TH2* h2, Double_t c1 /*= 1*/, Double_t c2 /*= 1*/, Option_t* option /*=""*/ )
{
	//! Check that we all have the same number of universes.
	if( h1->GetNHists() != this->GetNHists() )
	{
		Error("DivideSingle", "Attempt to divide by different numbers of universes" );
		return kFALSE;
	}

	//! Call Divide on the CVHists
	this->TH2D::Divide( (TH2D*)h1, h2, c1, c2, option);

	//! Call Divide for all universes
	for( unsigned int iHist = 0; iHist != fNHists; ++iHist )
		fHists[iHist]->Divide( h1->GetHist(iHist), h2, c1, c2, option );

	return true;
}

Bool_t MULatErrorBand2D::Divide( const MULatErrorBand2D* h1, const MULatErrorBand2D* h2, Double_t c1 /*= 1*/, Double_t c2 /*= 1*/, Option_t* option /*=""*/ )
{
	//! Check that we all have the same number of universes.
	if( h1->GetNHists() != h2->GetNHists() || h1->GetNHists() != this->GetNHists() )
	{
		Error("Divide", "Attempt to divide by different numbers of universes" );
		return kFALSE;
	}

	//! Call Divide on the CVHists
	this->TH2D::Divide( h1, h2, c1, c2, option);

	//! Call Divide for all universes
	for( unsigned int iHist = 0; iHist != fNHists; ++iHist )
		fHists[iHist]->Divide( h1->GetHist(iHist), h2->GetHist(iHist), c1, c2, option );

	return true;
}

void MULatErrorBand2D::Scale( Double_t c1 /*= 1.*/, Option_t *option /*= ""*/ )
{ 
	//! Scale the CVHist
	this->TH2D::Scale( c1, option );

	//! Scale all universes
	for( unsigned int iHist = 0; iHist != fNHists; ++iHist )
		fHists[iHist]->Scale( c1, option );
}

#endif
