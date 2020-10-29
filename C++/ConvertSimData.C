/*================================================================
 *   Filename   : main.C
 *   Author     : Yiyang Wu
 *   Created on  Wed 28 Oct 2020 03:38:40 PM CST
 *   Description:
 *
 ================================================================*/


#include "argparse.hpp"
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <cstdlib>
#include <vector>
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include "JPSimOutput.hh"
#include "H5PacketTable.h"

using namespace std;


// A program to convert raw data from a root file of JP1t to hdf5

void Convert_Readout_Tree(TTree* ReadoutTree, hid_t outputfile, hid_t dsp, int chunksize);
void Convert_RunHeader_Tree(TTree* RunHeaderTree, hid_t outputfile, hid_t dsp, int chunksize);
void Convert_SimTriggerInfo_Tree(TTree* SimTriggerInfoTree, hid_t outputfile, hid_t dsp, int chunksize);

int main(int argc, char** argv)
{
	// Parsing Arguments
	argparse::ArgumentParser program("H5Test");
	program.add_argument("InputROOTfile");
	program.add_argument("OutputH5File");
	program.add_argument("-co","--compress").help("compression level").default_value(4).action([](const std::string& value) { return std::stoi(value); });;
	program.add_argument("-ch","--chunksize").help("chunksize of h5 file").default_value(16).action([](const std::string& value) { return std::stoi(value); });;

	try {
		program.parse_args(argc, argv);
	}
	catch (const runtime_error& err) {
		cout << program;
		if(string(err.what())=="help called") return 0;
		cout << err.what() << endl;
		return 2;
	}
	auto inputfilename = program.get<string>("InputROOTfile");
	auto outputfilename = program.get<string>("OutputH5File");
	int compression_level = program.get<int>("--compress");
	int chunksize = program.get<int>("--chunksize");

	// Load Dictionary
	gSystem->Load("libJPSIMOUTPUT_rdict.pcm");

	// Read Input file
	TFile* ipt = new TFile(TString(inputfilename), "read");
	TTree* ReadoutTree = nullptr, *RunHeaderTree = nullptr, *SimTriggerInfoTree = nullptr;
	ipt->GetObject("Readout",ReadoutTree);
	ipt->GetObject("RunHeader",RunHeaderTree);
	ipt->GetObject("SimTriggerInfo",SimTriggerInfoTree);
	// Create output file
	hid_t output = H5Fcreate(outputfilename.data(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	if(output <0) { fprintf(stderr, "Couldn't create file.\n"); return 1; }
	// Set H5 compression level
	herr_t err;
	hid_t dsp = H5Pcreate(H5P_DATASET_CREATE);
	err = H5Pset_deflate(dsp, compression_level);
	if(err < 0) fprintf(stderr, "Error setting compression level.");

	// Convert_Readout_Tree(ReadoutTree, output, dsp, chunksize);
	// Convert_RunHeader_Tree(RunHeaderTree, output, dsp, chunksize);
	Convert_SimTriggerInfo_Tree(SimTriggerInfoTree, output, dsp, chunksize);

	err = H5Fclose(output);
	if( err < 0 )
		fprintf(stderr, "Failed to close file.\n");
	cout<<"done!"<<endl;

	ipt->Close();

	return 0;
}
