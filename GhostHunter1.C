/*================================================================
 *   Filename   : main.C
 *   Author     : Yiyang Wu
 *   Created on  Wed 28 Oct 2020 03:38:40 PM CST
 *   Description:
 *
 ================================================================*/


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
#include "H5Ppublic.h"
#include "H5Zpublic.h"
#include "argparse.hpp"

#define ZSTD_FILTER 32015

using namespace std;


// A program to convert raw data from a root file of JP1t to hdf5

void Convert_Readout_Tree(TTree* ReadoutTree, hid_t outputfile, hid_t dsp, vector<int> readout_chunksize);
void Convert_SimTriggerInfo_Tree(TTree* SimTriggerInfoTree, hid_t outputfile, hid_t dsp, vector<int> simtriggerinfo_chunksize);

int main(int argc, char** argv)
{
	// Parsing Arguments
	argparse::ArgumentParser program("ConvertSimData");
	program.add_argument("InputROOTfile");
	program.add_argument("OutputH5File");
	program.add_argument("-co","--compress").help("compression level").default_value(4).action([](const std::string& value) { return std::stoi(value); });;
	program.add_argument("-zstd","--zstandard").help("use zstandard as compression filter").default_value(false).implicit_value(true);
	program.add_argument("-hch","--runheader-chunksize").help("chunksize of Runeader table").default_value(1).action([](const std::string& value) { return std::stoi(value); });;
	program.add_argument("-rch","--readout-chunksize").help("chunksize of {TriggerInfo, Waveform} table").nargs(2).default_value(vector<int>{16,16}).action([](const std::string& value) { return std::stoi(value); });;
	program.add_argument("-ich","--simtriggerinfo-chunksize").help("chunksize of {TruthList, PEList} table").nargs(2).default_value(vector<int>{16, 32}).action([](const std::string& value) { return std::stoi(value); });;

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
	vector<int> readout_chunksize = program.get<vector<int>>("--readout-chunksize");
	vector<int> simtriggerinfo_chunksize = program.get<vector<int>>("--simtriggerinfo-chunksize");
	bool use_zstd = program.get<bool>("-zstd");


	// Load Dictionary
	gSystem->Load("libJPSIMOUTPUT_rdict.pcm");
	gSystem->Load("libJPSIMOUTPUT.so");


	// Read Input file
	TFile* ipt = new TFile(TString(inputfilename), "read");
	TTree* ReadoutTree = nullptr, *RunHeaderTree = nullptr, *SimTriggerInfoTree = nullptr, *SimTruthTree = nullptr;
	ipt->GetObject("Readout",ReadoutTree);
	ipt->GetObject("RunHeader",RunHeaderTree);
	ipt->GetObject("SimTriggerInfo",SimTriggerInfoTree);
	ipt->GetObject("SimTruth",SimTruthTree);
	// Create output file
	hid_t fapl_id = H5Pcreate (H5P_FILE_ACCESS);
	H5Pset_cache(fapl_id,0,500/*slots*/,1024*1024*128/*Bytes*/,1.0/*only write once*/);
	hid_t output = H5Fcreate(outputfilename.data(), H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);
	if(output <0) { fprintf(stderr, "Couldn't create file.\n"); return 1; }
	// Set H5 compression level
	herr_t err;
	hid_t dsp = H5Pcreate(H5P_DATASET_CREATE);
	if(use_zstd)
	{
		if(H5Zfilter_avail(ZSTD_FILTER))
		{
			unsigned int compress[1];
			compress[0] = compression_level;
			err = H5Pset_filter(dsp,32015/*zstd*/,0b00,1,compress);
		}
		else
		{
			cerr<<"No zstd support! Change to gzip."<<endl;
			use_zstd=false;
		}
	}
	if(!use_zstd) err = H5Pset_deflate(dsp, compression_level);
	if(err < 0) fprintf(stderr, "Error setting compression level.");

#ifdef DEBUG
	cout<<"has_verbose_track="<<has_verbose_track<<", has_trigger_track="<<has_trigger_track<<endl;
	cout<<"SaveTruthTrack="<<SaveTruthTrack<<", SaveTriggerTrack="<<SaveTriggerTrack<<endl;
#endif


	// Converting
	Convert_Readout_Tree(ReadoutTree, output, dsp, readout_chunksize);
	Convert_SimTriggerInfo_Tree(SimTriggerInfoTree, output, dsp, simtriggerinfo_chunksize);

	err = H5Fclose(output);
	if( err < 0 )
		fprintf(stderr, "Failed to close file.\n");
	cout<<"done!"<<endl;

	ipt->Close();

	return 0;
}
