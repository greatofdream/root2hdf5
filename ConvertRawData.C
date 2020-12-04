/*================================================================
 *   Filename   : main.C
 *   Author     : Yiyang Wu
 *   Created on  Wed 28 Oct 2020 03:38:40 PM CST
 *   Description:
 *
 ================================================================*/


#include <cstddef>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "H5PacketTable.h"
#include "H5Ppublic.h"
#include "H5Zpublic.h"
#include "argparse.hpp"

#define ZSTD_FILTER 32015

using namespace std;


// A program to convert raw data from a root file of JP1t to hdf5

void Convert_Readout_Tree(TTree* ReadoutTree, hid_t outputfile, hid_t dsp, vector<int> readout_chunksize);

int main(int argc, char** argv)
{
	// Parsing Arguments
	argparse::ArgumentParser program("ConvertRawData");
	program.add_argument("InputROOTfile");
	program.add_argument("OutputH5File");
	program.add_argument("-co","--compress").help("compression level").default_value(4).action([](const std::string& value) { return std::stoi(value); });;
	program.add_argument("-zstd","--zstandard").help("use zstandard as compression filter").default_value(false).implicit_value(true);
	program.add_argument("-rch","--readout-chunksize").help("chunksize of {TriggerInfo, Waveform} table").nargs(2).default_value(vector<int>{16,16}).action([](const std::string& value) { return std::stoi(value); });;

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
	bool use_zstd = program.get<bool>("-zstd");
	vector<int> readout_chunksize = program.get<vector<int>>("--readout-chunksize");

	// Read Input file
	TFile* ipt = new TFile(TString(inputfilename), "read");
	TTree* ReadoutTree = nullptr;
	ipt->GetObject("Readout",ReadoutTree);
	// Create output file
	hid_t fapl_id = H5Pcreate (H5P_FILE_ACCESS);
	H5Pset_cache(fapl_id,0,500/*slots*/,(1029*2)*readout_chunksize[1]*2/*Bytes*/,1.0/*only write once*/);
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

	Convert_Readout_Tree(ReadoutTree, output, dsp, readout_chunksize);

	err = H5Fclose(output);
	if( err < 0 )
		fprintf(stderr, "Failed to close file.\n");
	cout<<"done!"<<endl;

	ipt->Close();

	return 0;
}
