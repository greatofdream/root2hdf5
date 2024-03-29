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
#include "TChain.h"
#include "TTree.h"
#include "TSystem.h"
#include "JPSimOutput.hh"
#include "H5PacketTable.h"
#include "H5Ppublic.h"
#include "H5Zpublic.h"
#include "argparse.hpp"
// #include <boost/program_options.hpp>
// #include <boost/program_options/options_description.hpp>

#define ZSTD_FILTER 32015

using namespace std;
// namespace po = boost::program_options;

// A program to convert raw data from a root file of JP1t to hdf5


void Convert_RunHeader_Tree(TTree* RunHeaderTree, hid_t outputfile, hid_t dsp, int chunksize);
void Convert_Readout_Tree(TTree* ReadoutTree, hid_t outputfile, hid_t dsp, vector<int> readout_chunksize);
void Convert_SimTriggerInfo_Tree(TTree* SimTriggerInfoTree, hid_t outputfile, hid_t dsp, vector<int> simtriggerinfo_chunksize, bool SaveTrack);
void Convert_SimTruth_Tree(TTree* SimTruthTree, hid_t outputfile, hid_t dsp, vector<int> simtruth_chunksize, bool SaveTrack);
void Convert_RunHeader_Tree(TChain* RunHeaderTree, hid_t outputfile, hid_t dsp, int chunksize);
void Convert_Readout_Tree(TChain* ReadoutTree, hid_t outputfile, hid_t dsp, vector<int> readout_chunksize);
void Convert_SimTriggerInfo_Tree(TChain* SimTriggerInfoTree, hid_t outputfile, hid_t dsp, vector<int> simtriggerinfo_chunksize, bool SaveTrack);
void Convert_SimTruth_Tree(TChain* SimTruthTree, hid_t outputfile, hid_t dsp, vector<int> simtruth_chunksize, bool SaveTrack);

int main(int argc, char** argv)
{
	// Parsing Arguments
	argparse::ArgumentParser program("ConvertSimData");
	//po::options_description program("jinping_cvt")
	program.add_argument( "InputROOTfile");//.nargs('+');
	program.add_argument( "OutputH5File");
	program.add_argument("-co","--compress").help("compression level").default_value(4).action([](const std::string& value) { return std::stoi(value); });;
	program.add_argument("-zstd","--zstandard").help("use zstandard as compression filter").default_value(false).implicit_value(true);
	program.add_argument("-hch","--runheader-chunksize").help("chunksize of Runeader table").default_value(1).action([](const std::string& value) { return std::stoi(value); });;
	program.add_argument("-rch","--readout-chunksize").help("chunksize of {TriggerInfo, Waveform} table").nargs(2).default_value(vector<int>{16,16}).action([](const std::string& value) { return std::stoi(value); });;
	program.add_argument("-ich","--simtriggerinfo-chunksize").help("chunksize of {TruthList, PEList} table").nargs(2).default_value(vector<int>{16, 32}).action([](const std::string& value) { return std::stoi(value); });;
	program.add_argument("-uch","--simtruth-chunksize").help("chunksize of {SimTruth, PrimaryPartcle, DepositEergy, TrackList, StepPoint, SecondaryParticle} table").nargs(6).default_value(vector<int>{16,32,32,32,32,32}).action([](const std::string& value) { return std::stoi(value); });;

	try {
		program.parse_args(argc, argv);
	}
	catch (const runtime_error& err) {
		cout << program;
		if(string(err.what())=="help called") return 0;
		cout << err.what() << endl;
		return 2;
	}
	cout<< "read args"<<endl;
	auto inputfilename = program.get<string>("InputROOTfile");
	auto outputfilename = program.get<string>("OutputH5File");
	int compression_level = program.get<int>("--compress");
	int runheader_chunksize = program.get<int>("--runheader-chunksize");
	vector<int> readout_chunksize = program.get<vector<int>>("--readout-chunksize");
	vector<int> simtriggerinfo_chunksize = program.get<vector<int>>("--simtriggerinfo-chunksize");
	vector<int> simtruth_chunksize = program.get<vector<int>>("--simtruth-chunksize");
	bool use_zstd = program.get<bool>("-zstd");


	// Load Dictionary
	gSystem->Load("libJPSIMOUTPUT_rdict.pcm");
	gSystem->Load("libJPSIMOUTPUT.so");


	// Read Input file
	// TFile* ipt = new TFile(TString(inputfilename), "read");
	// TTree* ReadoutTree = nullptr, *RunHeaderTree = nullptr, *SimTriggerInfoTree = nullptr, *SimTruthTree = nullptr;
	TChain* ReadoutTree = new TChain(TString("Readout"));
	TChain* RunHeaderTree = new TChain(TString("RunHeader"));
	TChain* SimTriggerInfoTree = new TChain(TString("SimTriggerInfo"));
	TChain* SimTruthTree = new TChain(TString("SimTruth"));
	cout<<"construct TChain"<<endl;
	// ipt->GetObject("Readout",ReadoutTree);
	// ipt->GetObject("RunHeader",RunHeaderTree);
	// ipt->GetObject("SimTriggerInfo",SimTriggerInfoTree);
	// ipt->GetObject("SimTruth",SimTruthTree);
	// for(int i=0;i<inputfilename.size();i++){
	// 	ReadoutTree->Add(TString(inputfilename[i]));
	// 	RunHeaderTree->Add(TString(inputfilename[i]));
	// 	SimTriggerInfoTree->Add(TString(inputfilename[i]));
	// 	SimTruthTree->Add(TString(inputfilename[i]));
	// }
	ReadoutTree->Add(TString(inputfilename));
	RunHeaderTree->Add(TString(inputfilename));
	SimTriggerInfoTree->Add(TString(inputfilename));
	SimTruthTree->Add(TString(inputfilename));
	cout<<"add files to TChain"<<endl;
	cout<<"SimTruth entries:"<<SimTruthTree->GetEntries()<<endl;
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

	// Decide if and where the track should be saved
	SimTriggerInfoTree->SetBranchStatus("*",0);
	SimTriggerInfoTree->SetBranchStatus("truthList", 1);
	SimTriggerInfoTree->SetBranchStatus("truthList.trackList", 1);
	vector<JPSimTruthTree_t>* TruthList_test = nullptr;
	bool has_trigger_track=false;
	SimTriggerInfoTree->SetBranchAddress("truthList",&TruthList_test);
	for(int entry=0;entry<SimTriggerInfoTree->GetEntries();entry++)
	{
		SimTriggerInfoTree->GetEntry(entry);
		if(TruthList_test->size()==0) continue;
		auto truth_0 = TruthList_test->data();
		auto track_0 = truth_0->trackList;
		has_trigger_track = track_0.size()!=0;
		break;
	}

	vector<JPSimTrack_t>* TrackList_test = nullptr;
	SimTruthTree->SetBranchStatus("*",0);
	SimTruthTree->SetBranchStatus("trackList*", 1);
	SimTruthTree->SetBranchAddress("trackList",&TrackList_test);
	SimTruthTree->GetEntry(0);
	bool has_verbose_track = TrackList_test->size()!=0;

	bool SaveTriggerTrack, SaveTruthTrack;
	if(has_trigger_track && has_verbose_track) {SaveTriggerTrack=false; SaveTruthTrack=true;}
	else if(SimTriggerInfoTree->GetEntries()==0 && has_verbose_track) {SaveTriggerTrack=false; SaveTruthTrack=true;} // in case No Trigger, has_trigger_track=false
	else if((!has_trigger_track) && (!has_verbose_track))  {SaveTriggerTrack=false; SaveTruthTrack=false;}
	else if(( has_trigger_track) && (!has_verbose_track))  {SaveTriggerTrack=true; SaveTruthTrack=false;}
	else {cerr<<"Unknown Track Structure in root file!"<<endl; abort();}
#ifdef DEBUG
	cout<<"has_verbose_track="<<has_verbose_track<<", has_trigger_track="<<has_trigger_track<<endl;
	cout<<"SaveTruthTrack="<<SaveTruthTrack<<", SaveTriggerTrack="<<SaveTriggerTrack<<endl;
#endif


	// Converting
	Convert_RunHeader_Tree(RunHeaderTree, output, dsp, runheader_chunksize);
	Convert_Readout_Tree(ReadoutTree, output, dsp, readout_chunksize);
	Convert_SimTriggerInfo_Tree(SimTriggerInfoTree, output, dsp, simtruth_chunksize, SaveTriggerTrack);
	Convert_SimTruth_Tree(SimTruthTree, output, dsp, simtruth_chunksize, SaveTruthTrack);

	err = H5Fclose(output);
	if( err < 0 )
		fprintf(stderr, "Failed to close file.\n");
	cout<<"done!"<<endl;

	//ipt->Close();

	return 0;
}
