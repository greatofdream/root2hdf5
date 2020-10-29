/*================================================================
 *   Filename   : main.C
 *   Author     : Yiyang Wu
 *   Created on  Wed 28 Oct 2020 03:38:40 PM CST
 *   Description:
 *
 ================================================================*/


#include "argparse.hpp"
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

// A program to dump waveforms from a binary file of XMASS 800kg FADC
// to hdf5.
int main(int argc, char** argv)
{
	argparse::ArgumentParser program("H5Test");
	program.add_argument("InputROOTfile");
	program.add_argument("OutputH5File");
	program.add_argument("-c","--compress").help("compression level").default_value(4).action([](const std::string& value) { return std::stoi(value); });;

	try {
		program.parse_args(argc, argv);
	}
	catch (const runtime_error& err) {
		cout << err.what() << endl;
		cout << program;
		abort();
	}
	auto inputfilename = program.get<string>("InputROOTfile");
	auto outputfilename = program.get<string>("OutputH5File");
	int compression_level = program.get<int>("--compress");

	// Read Input file
	TFile* ipt = new TFile(TString(inputfilename), "read");
	TTree* ReaoutTree = nullptr;
	ipt->GetObject("Readout",ReaoutTree);
	int32_t TriggerNo;
	vector<uint32_t>* Waveform = nullptr;
	vector<uint32_t>* ChannelId = nullptr;
	ReaoutTree->SetBranchAddress("TriggerNo",&TriggerNo);
	ReaoutTree->SetBranchAddress("ChannelId",&ChannelId);
	ReaoutTree->SetBranchAddress("Waveform",&Waveform);

	// determine WindowSize
	ReaoutTree->GetEntry(0);
	int WindowSize = Waveform->size()/ChannelId->size();

	// caution: struct alignment. sizeof(Readout_t) != HOFFSET(Readout_t, waveform)
	struct Readout_t {
		uint64_t eventid;
		uint16_t channelid;
		uint16_t waveform[];
	};
	size_t readout_size = sizeof(Readout_t)+sizeof(*Readout_t::waveform)*WindowSize;
	Readout_t* Readout= (Readout_t*) malloc(readout_size);

	herr_t err;
	// Create output file
	hid_t output = H5Fcreate(outputfilename.data(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	if(output <0) {
		fprintf(stderr, "Couldn't create file.\n");
		return 1;
	}
	hid_t ev_c = H5Tcreate (H5T_COMPOUND, readout_size);
	H5Tinsert (ev_c, "EventID", HOFFSET(Readout_t, eventid), H5T_NATIVE_UINT);
	H5Tinsert (ev_c, "ChannelID", HOFFSET(Readout_t, channelid), H5T_NATIVE_USHORT);
	hsize_t dim[] = {0};
	dim[0] = WindowSize;
	hid_t waveform_t = H5Tarray_create(H5T_NATIVE_USHORT, 1, dim);
	H5Tinsert (ev_c, "Waveform", HOFFSET(Readout_t, waveform), waveform_t);

	hid_t dsp = H5Pcreate(H5P_DATASET_CREATE);
	err = H5Pset_deflate(dsp, compression_level);
	if(err < 0)
		fprintf(stderr, "Error setting compression level.");

	/*
chunksize:
32: 27.12s, 7.2M @kmlb
16: 26.88, 7.2M @kmlb
*/
	FL_PacketTable waveform_d(output, "/Waveform", ev_c, 16, dsp);
	if(! waveform_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table.");
		return 1;
	}

	// converting loop
	cout<<"Converting "<<argv[1]<<" to "<<argv[2]<<endl;
	uint64_t nevt= ReaoutTree->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;

	for (unsigned int ievt=0; ievt<nevt; ievt++) {
		ReaoutTree->GetEntry(ievt);
		Readout->eventid = TriggerNo;
		for(int i=0;i<ChannelId->size();i++)
		{
			Readout->channelid = (*ChannelId)[i];
			memcpy(Readout->waveform, Waveform->data()+i*WindowSize, sizeof(*Readout_t::waveform)*WindowSize);
			waveform_d.AppendPacket( Readout );
		}

		if (ievt==0) cout<<"start processing ..."<<endl;
		else if (ievt%1000==0) cout<<ievt<<" events converted"<<endl;
	}

	err = H5Fclose(output);
	if( err < 0 )
		fprintf(stderr, "Failed to close file.\n");
	cout<<"done!"<<endl;

	free(Readout);
	ipt->Close();

	return 0;
}


