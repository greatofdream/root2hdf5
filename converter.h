#include "skroot.h"
#include "H5Ppublic.h"
#include "H5Zpublic.h"
#include "H5PacketTable.h"
struct Head_t {//tqrealroot.h
	// skhead_common h5head_;
	// skheada_common h5head_a;
	int32_t nrunsk;
    int32_t nsubsk;
    int32_t nevsk;
    int32_t ndaysk[3];
    int32_t ntimsk[4];
    int32_t nt48sk[3];
    int32_t mdrnsk;
    int32_t idtgsk;
    int32_t ifevsk;
    int32_t ltcgps;
    int32_t nsgps;
    int32_t nusgps;
    int32_t ltctrg;
    int32_t ltcbip;
    int32_t iffscc;
    int32_t itdct0[4];
    int32_t icalva;
	int32_t sk_geometry;
    // for SK-IV
    int32_t onldata_block_id;  // block id in online data format
    int32_t swtrg_id;          // software trigger ID
    int32_t counter_32;        //
    int32_t hostid_high;       //
    int32_t hostid_low;        //
    int32_t t0;                // t0 from software trigger
    int32_t gate_width;        // gate width from software trigger
    int32_t sw_trg_mask;       //
    int32_t contents;          //
    int32_t ntrg;
};
struct RunNo_t {
	int32_t nrunsk;
	int32_t nsubsk;
	int32_t nevsk;
	int32_t ntrigsk;
};
struct tqmeta_t {//tqrealroot.h
		RunNo_t runno;
		int32_t nhits;
		float pc2pe;
		int32_t tqreal_version; 
		int32_t qbconst_version;
		int32_t tqmap_version;  
		int32_t pgain_version;  
		int32_t it0xsk;
		int32_t nIflagNot7;  
	};
struct tqreal_t {
		RunNo_t runno;
		int iflag;
		int cable;
		float T;
		float Q;
	};
class Converter{
	public:
		hid_t fapl_id, output, dsp;
		herr_t err;
		hid_t headtype, tqmetatype, tqrealtype;
		hid_t header_d, tqmeta_d, tqreal_d;
		Head_t h5_head;
		RunNo_t h5_runno;
		tqmeta_t h5_tqmeta;
		tqreal_t h5_tqreal;
		hsize_t dim[2] = { 3, 4 };

		Converter(string filename);
		void Init_HEADER_Type();
		void Init_TQREAL_Type();
		void Convert_HEADER(Header* HEADER);
		void Convert_TQREAL(TQReal* TQREAL);
		void Close(){
			H5Tclose(headtype);H5Tclose(tqmetatype);H5Tclose(tqrealtype);
			H5Dclose(header_d);H5Dclose(tqmeta_d);H5Dclose(tqreal_d);
			err = H5Fclose(output);
			if( err < 0 )
				fprintf(stderr, "Failed to close file.\n");
		}
};
