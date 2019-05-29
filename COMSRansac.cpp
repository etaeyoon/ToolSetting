// OMSRansac.cpp: implementation of the COMSRansac class.
//
//////////////////////////////////////////////////////////////////////

#include "COMSRansac.h"
#include "COMSModel.h"
#include <iostream.h>
#include <fstream.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COMSRansac::COMSRansac()
{
	InputData = NULL; 
	Inlier = NULL;
}

COMSRansac::~COMSRansac()
{
	if(InputData != NULL) delete InputData;
	if(Inlier != NULL ) delete Inlier;
}

void COMSRansac::setInput( double* Col, double* Row, double* XX, double* YY, double* ZZ, double* Lat, double* Lon, int numData ) {

	NumData = numData;
	InputData = new struct _InputData[NumData];

	for(int i = 0; i < NumData; i++) {
		InputData[i].Col = Col[i];
		InputData[i].Row = Row[i];
		InputData[i].XX = XX[i];
		InputData[i].YY = YY[i];
		InputData[i].ZZ = ZZ[i];
		InputData[i].Lat = Lat[i];
		InputData[i].Lon = Lon[i];
	}

} // end of member funciton -- COMSRansac::setInput

int COMSRansac::RansacDLT(int IterationNo, double _error_threshold, bool* inflag, fstream & log) {

	COMSModel	DLTModel;

	int i,j,k,iter;
	int Total_Iteration = IterationNo; // number of total Iteration
	double error_bound =_error_threshold;  // criterion for correct matching
	int MinimumGCP = 6;   // Perspective DLT requires minimum 6 GCPs
 
	bool *check = new bool[NumData];

	double *uv, *XYZ;
	double *temp_best_uv;
	double *temp_best_XYZ, *temp_best_LatLon;
	double *best_uv;
	double *best_XYZ;
	double *best_LatLon;
 
	double errorx, errory, error, errorsum, minerror = 10000;
	int count, max_count=0;
 
	uv = 		 new double[2*MinimumGCP];
	XYZ = 		 new double[3*MinimumGCP]; 
	temp_best_uv =  new double[2*NumData]; 
	temp_best_XYZ = new double[3*NumData];
	temp_best_LatLon = new double[2*NumData];
	best_uv = 	 new double[2*NumData];
	best_XYZ = 	 new double[3*NumData];
	best_LatLon = new double[2*NumData];
 
	int DLTReturn;
	double InverseCol, InverseRow;

///////////////////////////////////////////////////////////////////
	for ( iter = 0 ; iter < Total_Iteration ; iter++)
	{
		// Random sampling of the data - 6 points
 		for ( i = 0 ; i < NumData ; i++) 
			check[i] = false;
	
		j=0;
		for ( i = 0 ; i < MinimumGCP ; i++) 
		{
			k = (int)( rand() / (float)(RAND_MAX) * NumData );
			j = ( k + j ) % NumData;
		
			while (check[j]==true) 
				j = (j + 1) % NumData;

			uv [ i*2 + 0 ] = InputData[j].Col;
			uv [ i*2 + 1 ] = InputData[j].Row;  // uv: col, row
			XYZ[ i*3 + 0 ] = InputData[j].XX;
			XYZ[ i*3 + 1 ] = InputData[j].YY;
			XYZ[ i*3 + 2 ] = InputData[j].ZZ;
			check[j] = true;
		}  // end of the sampling loop

		//*** setup perspective DLT with random sampled data
		DLTReturn = DLTModel.DLT_normalized(uv, XYZ, MinimumGCP);
		//DLTReturn = DLTModel.LPDLT_normalized(uv, XYZ, MinimumGCP);

		//*** count number of supporting points for the established model
		count = 0; errorsum = 0;
		
		if( DLTReturn != -1 ) 		// check the support only if DLT worked
		for ( i = 0 ; i < NumData ; i++)
		{
			DLTModel.DLTInverseMapping( InputData[i].XX, InputData[i].YY, InputData[i].ZZ, InverseCol, InverseRow);
			//DLTModel.LPDLTInverseMapping( InputData[i].XX, InputData[i].YY, InputData[i].ZZ, InverseCol, InverseRow);

			errorx = InputData[i].Col - InverseCol;
			errory = InputData[i].Row - InverseRow;
			error = sqrt( errorx*errorx + errory*errory );
		
			if ( error < error_bound )
			{
				temp_best_uv[count*2 + 0] =  InputData[i].Col;
				temp_best_uv[count*2 + 1] =  InputData[i].Row;
				temp_best_XYZ[count*3 + 0] = InputData[i].XX;
				temp_best_XYZ[count*3 + 1] = InputData[i].YY;
				temp_best_XYZ[count*3 + 2] = InputData[i].ZZ;
				temp_best_LatLon[count*2 +0] = InputData[i].Lat;
				temp_best_LatLon[count*2 +1] = InputData[i].Lon;
				count++;
				errorsum += error;
			}
		} // end of "if / for i"

		//*** if no. of supporting points exceeds the current max, take them as best inliers
		if ( count > max_count || (count == max_count && errorsum < minerror) )
		{
			for ( i = 0 ; i < count ; i++)
			{
				best_uv[i*2 + 0] = temp_best_uv[i*2 + 0];
				best_uv[i*2 + 1] = temp_best_uv[i*2 + 1];
				best_XYZ[i*3 + 0] = temp_best_XYZ[i*3 + 0];
				best_XYZ[i*3 + 1] = temp_best_XYZ[i*3 + 1];
				best_XYZ[i*3 + 2] = temp_best_XYZ[i*3 + 2];
				best_LatLon[i*2+0] = temp_best_LatLon[i*2 + 0];
				best_LatLon[i*2+1] = temp_best_LatLon[i*2 + 1];
		    }

			max_count = count;
			minerror = errorsum;
		}
	}  // end of iteration iter
 
	//*** refine LP modeling matrix with the best supporting inliers
	if( DLTModel.DLT_normalized(best_uv, best_XYZ, max_count) == -1)
	//if( DLTModel.LPDLT_normalized(best_uv, best_XYZ, max_count) == -1)
		return -1;

	log << "Best Dataset\n";
	
	for (i=0;i<max_count;i++){
		log << i+1 << " " << best_uv[i*2+0] << " " << best_uv[i*2+1] << " " <<
			best_XYZ[i*3 + 0] << " " << best_XYZ[i*3 + 1] << " " << best_XYZ[i*3 + 2] << " " <<
			best_LatLon[i*2+0] << " " << best_LatLon[i*2+1] << "\n";
	}

	/*printf("\n\n");
	for (i=0;i<max_count;i++){
		//printf("%i %lf %lf %lf %lf %lf %lf %lf\n", i, best_pt[i].col, best_pt[i].row,
		//	best_pt[i].u1, best_pt[i].u2, best_pt[i].u3, best_pt[i].lat, best_pt[i].lon );
		double u1, u2, u3;
		DLTModel.image2scraft( best_uv[i*2+0], best_uv[i*2+1], u1, u2, u3);
		printf("%lf %lf %lf %lf %lf %lf %i %i\n", u1, u2, u3,
			best_XYZ[i*3+0], best_XYZ[i*3+1], best_XYZ[i*3+2], (int) (best_uv[i*2+0]+0.5), (int)(best_uv[i*2+1]+0.5) );
	}

	printf("\n\n");

	for (i=0;i<max_count;i++){
		double u1, u2, u3;
		DLTModel.image2scraft( best_uv[i*2+0], best_uv[i*2+1], u1, u2, u3);
		printf("%lf %lf %lf %lf %lf %lf %i %i\n", u1, u2, u3,
			best_XYZ[i*3+0], best_XYZ[i*3+1], best_XYZ[i*3+2], 
			(int)((best_uv[i*2+0]-3997)/4./1.7+0.5), (int)((best_uv[i*2+1]-2397)/4+0.5) );
	}
	*/

	NumInlier = max_count;
	Inlier = new _InputData[NumInlier];
	for(i = 0; i < NumInlier; i++ ) {
		Inlier[i].Col = best_uv[i*2+0];
		Inlier[i].Row = best_uv[i*2+1];
		Inlier[i].XX = best_XYZ[i*3+0];
		Inlier[i].YY = best_XYZ[i*3+1];
		Inlier[i].ZZ = best_XYZ[i*3+2];
	}

	double error_t = 0.;
	int cnt = 0;

	log << "\n\n";
	
	for ( i = 0 ; i < NumData ; i++)
	{
		DLTModel.DLTInverseMapping(InputData[i].XX, InputData[i].YY, InputData[i].ZZ, InverseCol, InverseRow );
		//DLTModel.LPDLTInverseMapping(InputData[i].XX, InputData[i].YY, InputData[i].ZZ, InverseCol, InverseRow );
		
		errorx = InputData[i].Col - InverseCol;
		errory = InputData[i].Row - InverseRow; 
		error_t = sqrt( errorx*errorx + errory*errory );
  
		if(error_t >= _error_threshold ) inflag[i] = false;
		else inflag[i] = true;

		log << ++cnt << " " << InputData[i].Col << " " << InputData[i].Row << " " << InputData[i].XX
			<< " " << InputData[i].YY << " " << InputData[i].ZZ << " " << InputData[i].Lat << " " 
			<< InputData[i].Lon << " " << error_t << " " << ( (error_t >= _error_threshold) ? "outlier" : "inlier") << "\n";
	}

	if(uv != NULL) delete(uv);
	if(XYZ != NULL) delete(XYZ);
	if(temp_best_uv != NULL) delete(temp_best_uv); 
	if(temp_best_XYZ != NULL) delete(temp_best_XYZ);
	if(temp_best_LatLon != NULL) delete(temp_best_LatLon);
	if(best_uv != NULL) delete(best_uv);
	if(best_XYZ != NULL) delete(best_XYZ); 
	if(best_LatLon != NULL) delete(best_LatLon);
	
	delete check;

	cnt = 0;

	return max_count;
}

int COMSRansac::RansacOrbit(int IterationNo, double _error_threshold) {

	double bestPara[9];
	COMSModel	OrbitModel;

	int numData = this->NumData;
	double *u1 = new double[numData];
	double *u2 = new double[numData];
	double *u3 = new double[numData];
	
	double* scale = new double[numData];

	int i,j,k,iter;

	//*** recalculate u1, u2, u3 since the current input u1, u2, u3 is completely wrong
	for(i = 0; i < numData; i++)
		OrbitModel.image2scraft(InputData[i].Col, InputData[i].Row, u1[i], u2[i], u3[i]);

	int Total_Iteration = IterationNo; // number of total Iteration
	double error_bound =_error_threshold;  // criterion for correct matching
	int MinimumGCP = 3;   // Perspective Orbitbased Modeling requires minimum 3 GCPs
 
	bool *check = new bool[numData];

	knpoint* pt = new knpoint[MinimumGCP];
	knpoint* ref = new knpoint[numData];
	knpoint* temp_best_pt = new knpoint[numData];
	knpoint* best_pt = new knpoint[numData];

	double InverseCol, InverseRow;
	double errorx,errory,error,errorsum,minerror = 100;
 
	int count, max_count=0;
	bool OrbitReturn;
	//int OrbitError;
	//int DLTReturn;
	
	for (iter=0;iter<Total_Iteration;iter++)
	{
		for (i=0;i<numData;i++) 
			check[i] = false;

		j=0;
		for (i=0;i<MinimumGCP;i++) 
		{
			k = (int)(rand()/(float)(RAND_MAX)*numData);
			j=(k+j)%numData;
			
			while (check[j]==true) 
			j=(j+1)%numData;
			
			pt[i].u1 = u1[j];	pt[i].u2 = u2[j];	pt[i].u3 = u3[j];
			pt[i].x = InputData[j].XX;		pt[i].y = InputData[j].YY;		pt[i].z = InputData[j].ZZ;
			
		 }  // end of the sampling loop
		
		OrbitReturn = OrbitModel.OrbitModelingBatch(pt, MinimumGCP, bestPara);
 
		//rec.OrbitErCal(numData, bestPara, X, Y, Z, col, row);
		
		// check the validity
		count=0; errorsum =0;

		if(OrbitReturn != true) continue;
		
		for (i=0;i<numData;i++)
		{
			OrbitModel.OrbitInverseMapping(bestPara, InputData[i].XX, InputData[i].YY,
				InputData[i].ZZ, InverseCol, InverseRow );
			//_model.scraft2image( _u1[i],_u2[i], _u3[i], pixel, line);

			errorx = InputData[i].Col - InverseCol;	
			errory = InputData[i].Row - InverseRow;
			error = sqrt(errorx*errorx+errory*errory);	

			//cout << "Error for " << i << " " << error << "\n";

			if (error < error_bound)
			{
				temp_best_pt[count].col = InputData[i].Col;
				temp_best_pt[count].row = InputData[i].Row;
				temp_best_pt[count].u1 = u1[i];
				temp_best_pt[count].u2 = u2[i];
				temp_best_pt[count].u3 = u3[i];
				temp_best_pt[count].x = InputData[i].XX;
				temp_best_pt[count].y = InputData[i].YY;
				temp_best_pt[count].z = InputData[i].ZZ;
				temp_best_pt[count].lat = InputData[i].Lat;
				temp_best_pt[count].lon = InputData[i].Lon;
				count++;
				errorsum += error;
			}
		}// end of "if / for i"

		if (count>max_count || (count==max_count && errorsum < minerror) )
	   {
			for (i=0;i<count;i++)
			{
				best_pt[i].col = temp_best_pt[i].col;
				best_pt[i].row = temp_best_pt[i].row;
				best_pt[i].u1 = temp_best_pt[i].u1;
				best_pt[i].u2 = temp_best_pt[i].u2;
				best_pt[i].u3 = temp_best_pt[i].u3;
				best_pt[i].x  = temp_best_pt[i].x;
				best_pt[i].y  = temp_best_pt[i].y;
				best_pt[i].z  = temp_best_pt[i].z;
				best_pt[i].lat = temp_best_pt[i].lat;
				best_pt[i].lon = temp_best_pt[i].lon;
			}
    
			max_count = count;
   			minerror = errorsum;
	   }
		
		//printf("Supporting Number %i\n", count );

	}  // end of iteration iter
 
	printf("Best Dataset\n");
	
	for (i=0;i<max_count;i++){
		printf("%i %lf %lf %lf %lf %lf %lf %lf\n", i+1, best_pt[i].col, best_pt[i].row,
			best_pt[i].u1, best_pt[i].u2, best_pt[i].u3, best_pt[i].lat, best_pt[i].lon );
		//printf("%lf %lf %lf %lf %lf %lf %i %i\n", best_pt[i].u1, best_pt[i].u2, best_pt[i].u3,
		//	best_pt[i].x, best_pt[i].y, best_pt[i].z, (int) (best_pt[i].col+0.5), (int)(best_pt[i].row+0.5) );
	}

	//printf("\n\n");
	//for (i=0;i<max_count;i++){
	//	printf("%lf %lf %lf %lf %lf %lf %i %i\n", best_pt[i].u1, best_pt[i].u2, best_pt[i].u3,
	//		best_pt[i].x, best_pt[i].y, best_pt[i].z, (int)((best_pt[i].col-3997)/4./1.7+0.5), (int)((best_pt[i].row-2397)/4+0.5) );
	//}

	double error_t = 0.;
	static int cnt = 0;
	
	printf("\n\n");

	OrbitReturn = OrbitModel.OrbitModelingBatch(best_pt, max_count, bestPara);

	printf("refine best para\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n\n\n",
	  bestPara[0], bestPara[1], bestPara[2], bestPara[3], bestPara[4], bestPara[5],
	  bestPara[6], bestPara[7], bestPara[8]);
	
	for (i=0;i<numData;i++)
	{
		OrbitModel.OrbitInverseMapping(bestPara, InputData[i].XX, InputData[i].YY, InputData[i].ZZ, 
			InverseCol, InverseRow ); 
		//_model.scraft2image( _u1[i],_u2[i], _u3[i], pixel, line);
		errorx = InputData[i].Col - InverseCol;
		errory = InputData[i].Row - InverseRow;
		error_t = sqrt( errorx*errorx + errory*errory );
  
		printf("%i %lf %lf %lf %lf %lf %lf %lf %lf\t\t\t%s\n", i+1,
				InputData[i].Col, InputData[i].Row, InputData[i].XX, InputData[i].YY, InputData[i].ZZ, 
			InputData[i].Lat, InputData[i].Lon, 
				error_t, (error_t >= error_bound) ? "outlier" : "inlier");
	}
 
	delete u1, u2, u3;
	delete check;

	delete  pt,temp_best_pt,best_pt;
 delete ref;
 
 return max_count;
} // end of member function -- RansacOrbit

int COMSRansac::RansacColli(int IterationNo, double _error_threshold) {

	double bestPara[9];
	COMSModel	ColliModel;

	int numData = this->NumData;
	double *u1 = new double[numData];
	double *u2 = new double[numData];
	double *u3 = new double[numData];
	
	double* scale = new double[numData];

	int i,j,k,iter;

	//*** recalculate u1, u2, u3 since the current input u1, u2, u3 is completely wrong
	for(i = 0; i < numData; i++) {
		ColliModel.image2scraft(InputData[i].Col, InputData[i].Row, u1[i], u2[i], u3[i]);
		//cout << InputData[i].Col << " " << InputData[i].Row << " " << u1[i] << " " << u2[i] << " " << u3[i] << "\n";
	}

	int Total_Iteration = IterationNo; // number of total Iteration
	double error_bound =_error_threshold;  // criterion for correct matching
	int MinimumGCP = 3;   // Perspective Orbitbased Modeling requires minimum 3 GCPs
 
	bool *check = new bool[numData];

	knpoint* pt = new knpoint[MinimumGCP];
	knpoint* ref = new knpoint[numData];
	knpoint* temp_best_pt = new knpoint[numData];
	knpoint* best_pt = new knpoint[numData];

	double InverseCol, InverseRow;
	double errorx,errory,error,errorsum,minerror = 100;
 
	int count, max_count=0;
	bool OrbitReturn;
	//int OrbitError;
	//int DLTReturn;
	
	for (iter=0;iter<Total_Iteration;iter++)
	{
		for (i=0;i<numData;i++) 
			check[i] = false;

		j=0;
		for (i=0;i<MinimumGCP;i++) 
		{
			k = (int)(rand()/(float)(RAND_MAX)*numData);
			j=(k+j)%numData;
			
			while (check[j]==true) 
			j=(j+1)%numData;
			
			pt[i].u1 = u1[j];	pt[i].u2 = u2[j];	pt[i].u3 = u3[j];
			pt[i].x = InputData[j].XX;		pt[i].y = InputData[j].YY;		pt[i].z = InputData[j].ZZ;
			
		 }  // end of the sampling loop
		
		OrbitReturn = ColliModel.CollinearModeling(pt, MinimumGCP);
 
		// check the validity
		count=0; errorsum =0;

		if(OrbitReturn != true) continue;
		
		for (i=0;i<numData;i++)
		{
			ColliModel.CollinearInverseMapping( InputData[i].XX, InputData[i].YY,
				InputData[i].ZZ, InverseCol, InverseRow );
			//_model.scraft2image( _u1[i],_u2[i], _u3[i], pixel, line);

			errorx = InputData[i].Col - InverseCol;	
			errory = InputData[i].Row - InverseRow;
			error = sqrt(errorx*errorx+errory*errory);	

			//cout << "Error for " << i << " " << error << "\n";

			if (error < error_bound)
			{
				temp_best_pt[count].col = InputData[i].Col;
				temp_best_pt[count].row = InputData[i].Row;
				temp_best_pt[count].u1 = u1[i];
				temp_best_pt[count].u2 = u2[i];
				temp_best_pt[count].u3 = u3[i];
				temp_best_pt[count].x = InputData[i].XX;
				temp_best_pt[count].y = InputData[i].YY;
				temp_best_pt[count].z = InputData[i].ZZ;
				temp_best_pt[count].lat = InputData[i].Lat;
				temp_best_pt[count].lon = InputData[i].Lon;
				count++;
				errorsum += error;
			}
		}// end of "if / for i"

		if (count>max_count || (count==max_count && errorsum < minerror) )
	   {
			for (i=0;i<count;i++)
			{
				best_pt[i].col = temp_best_pt[i].col;
				best_pt[i].row = temp_best_pt[i].row;
				best_pt[i].u1 = temp_best_pt[i].u1;
				best_pt[i].u2 = temp_best_pt[i].u2;
				best_pt[i].u3 = temp_best_pt[i].u3;
				best_pt[i].x  = temp_best_pt[i].x;
				best_pt[i].y  = temp_best_pt[i].y;
				best_pt[i].z  = temp_best_pt[i].z;
				best_pt[i].lat = temp_best_pt[i].lat;
				best_pt[i].lon = temp_best_pt[i].lon;
			}
    
			max_count = count;
   			minerror = errorsum;
	   }
		
		//printf("Supporting Number %i\n", count );

	}  // end of iteration iter
 
	printf("Best Dataset\n");
	
	for (i=0;i<max_count;i++){
		printf("%i %lf %lf %lf %lf %lf %lf %lf\n", i, best_pt[i].col, best_pt[i].row,
			best_pt[i].u1, best_pt[i].u2, best_pt[i].u3, best_pt[i].lat, best_pt[i].lon );
		//printf("%lf %lf %lf %lf %lf %lf %i %i\n", best_pt[i].u1, best_pt[i].u2, best_pt[i].u3,
		//	best_pt[i].x, best_pt[i].y, best_pt[i].z, (int) (best_pt[i].col+0.5), (int)(best_pt[i].row+0.5) );
	}

	//printf("\n\n");
	//for (i=0;i<max_count;i++){
	//	printf("%lf %lf %lf %lf %lf %lf %i %i\n", best_pt[i].u1, best_pt[i].u2, best_pt[i].u3,
	//		best_pt[i].x, best_pt[i].y, best_pt[i].z, (int)((best_pt[i].col-3997)/4./1.7+0.5), (int)((best_pt[i].row-2397)/4+0.5) );
	//}

	double error_t = 0.;
	static int cnt = 0;
	
	printf("\n\n");

	OrbitReturn = ColliModel.CollinearModeling(best_pt, max_count);

	for (i=0;i<numData;i++)
	{
		ColliModel.CollinearInverseMapping( InputData[i].XX, InputData[i].YY, InputData[i].ZZ, 
			InverseCol, InverseRow ); 
		//_model.scraft2image( _u1[i],_u2[i], _u3[i], pixel, line);
		errorx = InputData[i].Col - InverseCol;
		errory = InputData[i].Row - InverseRow;
		error_t = sqrt( errorx*errorx + errory*errory );
  
		printf("%i %lf %lf %lf %lf %lf %lf %lf %lf\t\t\t%s\n", i,
				InputData[i].Col, InputData[i].Row, InputData[i].XX, InputData[i].YY, InputData[i].ZZ, 
			InputData[i].Lat, InputData[i].Lon, 
				error_t, (error_t >= error_bound) ? "outlier" : "inlier");
	}
 
	delete u1, u2, u3;
	delete check;

	delete  pt,temp_best_pt,best_pt;
	delete ref;
 
 return max_count;
} // end of member function -- RansacColli
