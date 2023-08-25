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
struct lowe_t {// loweroot.h
	RunNo_t runno;
	float bsvertex[4];
	float bsresult[6];
	float  bsdir[3];     // bonsai direction(x,y,z) based on bonsai vertex
    float  bsgood[3];    // bonsai goodness: likelihood, goodness, ?
    float  bsdirks;      // bonsai direction KS
    float  bseffhit[12]; // bonsai effective hits at fixed water transparencies
    float  bsenergy;     // bonsai energy
    int32_t    bsn50;        // bonsai # of hits in 50 nsec after TOF subtraction
    float  bscossun;     // bonsai cossun
    float  clvertex[4];  // clusfit vertex: x, y, z, t
    float  clresult[4];  // clusfit results: theta, phi, cos_theta, good_t
    float  cldir[3];     // clusfit direction(x,y,z) based on clusfit vertex
    float  clgoodness;   // clusfit goodness
    float  cldirks;      // clusfit direction KS
    float  cleffhit[12]; // clusfit effective hits at fixed water transparencies
    float  clenergy;     // clusfit energy
    int32_t    cln50;        // clusfit # of hits in 50 nsec after TOF subtraction
    float  clcossun;     // clusfit cossun
    int32_t    latmnum;      // ATM number
    int32_t    latmh;        // ATM hit
    int32_t    lmx24;        // max 24
    double ltimediff;    // time to the previous LE event (in raw data)
    float  lnsratio;     // Noise-Signal ratio
    float  lsdir[3];     // solar direction at the time (x,y,z)
    int32_t    spaevnum;     // event number of the parent muon
    float  spaloglike;   // spallation log likelihood
    float  sparesq;      // spallation residual charge
    float  spadt;        // spallation delta-T between parent muon
    float  spadll;       // longitudinal distance
    float  spadlt;       // traversal distance
    int32_t    spamuyn;      // spallation muyn
    float  spamugdn;     // spallation muon goodness
    float  posmc[3];     // MC true vertex position
    float  dirmc[6];  // MC true direction (1st and 2nd particles)
    float  pabsmc[2];    // MC absolute momentum (1st & 2nd)
    float  energymc[2];  // MC generated energy(s) (1st & 2nd)
};
class Converter{
	public:
		hid_t fapl_id, output, dsp;
		herr_t err;
		hid_t headtype, tqmetatype, tqrealtype, lowetype;
		hid_t header_d, tqmeta_d, tqreal_d, lowe_d;
		Head_t h5_head;
		RunNo_t h5_runno;
		tqmeta_t h5_tqmeta;
		tqreal_t h5_tqreal;
		lowe_t h5_lowe;
		hsize_t dim[12] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

		Converter(string filename);
		void Init_HEADER_Type();
		void Init_TQREAL_Type();
		void Init_LOWE_Type();
		void Convert_HEADER(Header* HEADER);
		void Convert_TQREAL(TQReal* TQREAL);
		void Convert_LOWE(LoweInfo* LOWE);
		void Close(){
			H5Tclose(headtype); H5Tclose(tqmetatype); H5Tclose(tqrealtype);; H5Tclose(lowetype);
			H5Dclose(header_d); H5Dclose(tqmeta_d); H5Dclose(tqreal_d); H5Dclose(lowe_d);
			err = H5Fclose(output);
			if( err < 0 )
				fprintf(stderr, "Failed to close file.\n");
		}
};
