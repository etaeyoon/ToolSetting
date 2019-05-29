// OMSRansac.h: interface for the COMSRansac class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OMSRANSAC_H__D84F0626_86AA_4F74_A966_9AD58D35E206__INCLUDED_)
#define AFX_OMSRANSAC_H__D84F0626_86AA_4F74_A966_9AD58D35E206__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <fstream.h>

struct _InputData {
		double Col;
		double Row;
		double XX;
		double YY;
		double ZZ;
		double Lat;
		double Lon;
};

class COMSRansac  
{
public:
	COMSRansac();
	virtual ~COMSRansac();

	void setInput( double* Col, double* Row, double* XX, double* YY, double* ZZ, double* Lat, double *Lon, int numData );

	int RansacDLT(int IterationNo, double _error_threshold, bool* inflag, fstream& log);
	//int COMSRansac::DLT_normalized(double* ColRow, double* XYZ, int DataNum);
	//int COMSRansac::LSE( double *LSE_A, double *LSE_x_hat, double *LSE_l, 
	//	   int ParameterNo, int ObservationNo);

	_InputData* getInlier() { return Inlier; }
	int getInlierNum() { return NumInlier; }

	int COMSRansac::RansacOrbit(int IterationNo, double _error_threshold);
	int COMSRansac::RansacColli(int IterationNo, double _error_threshold);

private:

	int NumData;

	struct _InputData *InputData;
	struct _InputData *Inlier;
	int NumInlier;

	// DLT camera matrix
	//double MParameter[12];

};

#endif // !defined(AFX_OMSRANSAC_H__D84F0626_86AA_4F74_A966_9AD58D35E206__INCLUDED_)
