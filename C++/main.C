/*================================================================
*   Filename   : main.C
*   Author     : Yiyang Wu
*   Created on  Wed 28 Oct 2020 03:38:40 PM CST
*   Description:
*
================================================================*/


#include <cstdint>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

#include "H5PacketTable.h"

// A program to dump waveforms from a binary file of XMASS 800kg FADC
// to hdf5.
int main(int argc, char** argv)
{
   if (argc!=3) {
      cout<<"Usage: "<<endl;
      cout<<"  Test input outputHDF5"<<endl;
      return 1;
   }
   const int wavel = 1029;

   typedef struct Readout_t {
     uint64_t eid;
     uint16_t ch;
     uint16_t waveform[wavel];
   } readout;

   herr_t err;
   hid_t output = H5Fcreate(argv[2], H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
   if(output <0) {
     fprintf(stderr, "Couldn't create file.\n");
     return 1;
   }
   hid_t ev_c = H5Tcreate (H5T_COMPOUND, sizeof(Readout_t));
   H5Tinsert (ev_c, "EventID", HOFFSET(Readout_t, eid), H5T_NATIVE_UINT);
   H5Tinsert (ev_c, "ChannelID", HOFFSET(Readout_t, ch), H5T_NATIVE_USHORT);
   hsize_t dim[] = {wavel};
   hid_t waveform_t = H5Tarray_create(H5T_NATIVE_USHORT, 1, dim);
   H5Tinsert (ev_c, "Waveform", HOFFSET(Readout_t, waveform), waveform_t);

   hid_t dsp = H5Pcreate(H5P_DATASET_CREATE);
   err = H5Pset_deflate(dsp, 9);
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
   uint64_t nevt=10;
   Readout_t Readout;
   Readout.eid=0;
   Readout.ch=0;
   cout<<nevt<<" events to be processed"<<endl;
   for (unsigned int ievt=0; ievt<nevt; ievt++) {
#ifdef DEBUG
     cout << ievt << " EventID:" << ev_r.eid << " | " << eid << "|";
#endif
	 Readout.eid++;
	 Readout.ch+=4;

#ifdef DEBUG
       cout << i << " ";
#endif
         waveform_d.AppendPacket( &Readout );

#ifdef DEBUG
     cout << endl;
#endif

     if (ievt==0) cout<<"start processing ..."<<endl;
     else if (ievt%100==0) cout<<ievt<<" events converted"<<endl;
   }

   err = H5Fclose(output);
   if( err < 0 )
     fprintf(stderr, "Failed to close file.\n");
   cout<<"done!"<<endl;

   return 0;
}


