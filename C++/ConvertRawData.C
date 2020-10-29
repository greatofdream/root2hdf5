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

using namespace std;

#include "H5PacketTable.h"

// A program to convert raw data from a root file of JP1t to hdf5

void Convert_Readout_Tree(TTree* ReadoutTree, hid_t outputfile, hid_t dsp, int chunksize);

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

	// Read Input file
	TFile* ipt = new TFile(TString(inputfilename), "read");
	TTree* ReadoutTree = nullptr;
	ipt->GetObject("Readout",ReadoutTree);
	// Create output file
	hid_t output = H5Fcreate(outputfilename.data(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	if(output <0) { fprintf(stderr, "Couldn't create file.\n"); return 1; }
	// Set H5 compression level
	herr_t err;
	hid_t dsp = H5Pcreate(H5P_DATASET_CREATE);
	err = H5Pset_deflate(dsp, compression_level);
	if(err < 0) fprintf(stderr, "Error setting compression level.");

	Convert_Readout_Tree(ReadoutTree, output, dsp, chunksize);

	err = H5Fclose(output);
	if( err < 0 )
		fprintf(stderr, "Failed to close file.\n");
	cout<<"done!"<<endl;

	ipt->Close();

	return 0;
}

void Convert_Readout_Tree(TTree* ReadoutTree, hid_t outputfile, hid_t dsp, int chunksize)
{
	// Read Waveform and ChannelId
	vector<uint32_t>* Waveform = nullptr;
	vector<uint32_t>* ChannelId = nullptr;
	ReadoutTree->SetBranchAddress("ChannelId",&ChannelId);
	ReadoutTree->SetBranchAddress("Waveform",&Waveform);

	// determine WindowSize
	ReadoutTree->GetEntry(0);
	int WindowSize = Waveform->size()/ChannelId->size();

	// define outputfile data structure
	struct TriggerInfo_t {
		int32_t runno;
		int32_t triggerno;
		int32_t detectorid;
		int32_t triggertype;
		int32_t sec;
		int32_t nanosec;
	};
	TriggerInfo_t* TriggerInfo= (TriggerInfo_t*) malloc(sizeof(TriggerInfo_t));

	// directly read root data into struct.
	ReadoutTree->SetBranchAddress("RunNo",&TriggerInfo->runno);
	ReadoutTree->SetBranchAddress("TriggerNo",&TriggerInfo->triggerno);
	ReadoutTree->SetBranchAddress("TriggerType",&TriggerInfo->triggertype);
	ReadoutTree->SetBranchAddress("DetectorID",&TriggerInfo->triggerno);
	ReadoutTree->SetBranchAddress("Sec",&TriggerInfo->sec);
	ReadoutTree->SetBranchAddress("NanoSec",&TriggerInfo->nanosec);
	
	// define outputfile data structure
	// caution: struct alignment. sizeof(Readout_t) != HOFFSET(Readout_t, waveform)
	struct Readout_t {
		int32_t triggerno;
		int32_t channelid;
		int16_t waveform[];
	};
	size_t readout_size = sizeof(Readout_t)+sizeof(*Readout_t::waveform)*WindowSize;
	Readout_t* Readout= (Readout_t*) malloc(readout_size);

	// Create Readout Group
	hid_t ReadoutGroup = H5Gcreate1(outputfile, "/Readout",100);
	// Create outputfile Table
	hid_t waveformtable = H5Tcreate (H5T_COMPOUND, readout_size);
	H5Tinsert (waveformtable, "TriggerNo", HOFFSET(Readout_t, triggerno), H5T_NATIVE_INT32);
	H5Tinsert (waveformtable, "ChannelID", HOFFSET(Readout_t, channelid), H5T_NATIVE_INT32);
	hsize_t dim[1]; dim[0] = WindowSize;
	hid_t waveform_t = H5Tarray_create(H5T_NATIVE_INT16, 1, dim);
	H5Tinsert (waveformtable, "Waveform", HOFFSET(Readout_t, waveform), waveform_t);
	FL_PacketTable waveform_d(ReadoutGroup, "Waveform", waveformtable, chunksize, dsp);
	if(! waveform_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table Waveform.");
		abort();
	}
	
	hid_t triggerinfotable = H5Tcreate (H5T_COMPOUND, sizeof(TriggerInfo_t));
	H5Tinsert (triggerinfotable, "RunNo", HOFFSET(TriggerInfo_t, runno), H5T_NATIVE_INT32);
	H5Tinsert (triggerinfotable, "TriggerNo", HOFFSET(TriggerInfo_t, triggerno), H5T_NATIVE_INT32);
	H5Tinsert (triggerinfotable, "TriggerType", HOFFSET(TriggerInfo_t, triggertype), H5T_NATIVE_INT32);
	H5Tinsert (triggerinfotable, "DetectorID", HOFFSET(TriggerInfo_t, detectorid), H5T_NATIVE_INT32);
	H5Tinsert (triggerinfotable, "Sec", HOFFSET(TriggerInfo_t, sec), H5T_NATIVE_INT32);
	H5Tinsert (triggerinfotable, "NanoSec", HOFFSET(TriggerInfo_t, nanosec), H5T_NATIVE_INT32);
	FL_PacketTable triggerinfo_d(ReadoutGroup, "TriggerInfo", triggerinfotable, chunksize, dsp);
	if(! triggerinfo_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table TriggerInfo.");
		abort();
	}

	// converting loop
	char outputfilename[100];
	H5Fget_name(outputfile,outputfilename,100);
	cout<<"Converting Readout Tree of "<<ReadoutTree->GetCurrentFile()->GetName()<<" to "<<outputfilename<<endl;
	uint64_t nevt= ReadoutTree->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;

	for (unsigned int ievt=0; ievt<nevt; ievt++) {
		ReadoutTree->GetEntry(ievt);
		Readout->triggerno = TriggerInfo->triggerno;
		for(int i=0;i<ChannelId->size();i++)
		{
			Readout->channelid = (*ChannelId)[i];
			memcpy(Readout->waveform, Waveform->data()+i*WindowSize, sizeof(*Readout_t::waveform)*WindowSize);
			waveform_d.AppendPacket( Readout );
		}
		triggerinfo_d.AppendPacket( TriggerInfo );

		if (ievt==0) cout<<"start processing ..."<<endl;
		else if (ievt%1000==0) cout<<ievt<<" events converted"<<endl;
	}
	free(Readout);
}
