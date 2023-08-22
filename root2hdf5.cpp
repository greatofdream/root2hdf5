//
// root2hdf5.cpp 
//
// read skroot ofl file, then dump variables
//
#include "skheadC.h"
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <string>
#include "tqrealroot.h"
#include "loweroot.h"
#include "converter.h"

int main(int argc, char* argv[])
{
    // check arguments
    if (argc < 2) {
	cout << endl;
	cout << "    usage: " << argv[0] << " [input file_name] [output file_name]" << endl;
	cout << endl;
	return 1;
    }

    // open skroot file (read only)
    int id = 10;
    string fname_in = argv[1];
    string outputfilename = argv[2];
    TFile* rfin = new TFile(TString(fname_in));
    TTree* skdata = (TTree *) rfin->Get("data");
    Header    *HEAD    = new Header;
    TQReal    *TQREAL  = new TQReal;
    TQReal    *TQAREAL = new TQReal;
    skdata->SetBranchAddress("HEADER", &HEAD);
    skdata->SetBranchAddress("TQREAL", &TQREAL);
    int ntotal = skdata->GetEntries();

	// Create Output
	Converter cv(outputfilename);
    if(cv.output<0) return 1;
	cv.Init_HEADER_Type();
	cv.Init_TQREAL_Type();
    // main loop
    for (int i = 0; i < ntotal; i++) {
        // read all branches
        skdata->GetEntry(i);
		cv.Convert_HEADER(HEAD);
		cv.Convert_TQREAL(TQREAL);
	}
	cv.Close();
	cout<<"done!"<<endl;
}


