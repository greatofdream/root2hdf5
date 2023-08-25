#include "converter.h"
Converter::Converter(string filename){
	fapl_id = H5Pcreate (H5P_FILE_ACCESS);
	H5Pset_cache(fapl_id,0,500/*slots*/,1024*1024*128/*Bytes*/,1.0/*only write once*/);
	output = H5Fcreate(filename.data(), H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);
	if(output <0) { fprintf(stderr, "Couldn't create file.\n"); }
	dsp = H5Pcreate(H5P_DATASET_CREATE);
	err = H5Pset_deflate(dsp, 5); // compression level
	if(err < 0) fprintf(stderr, "Error setting compression level.");
}
void Converter::Init_HEADER_Type(){
	headtype = H5Tcreate(H5T_COMPOUND, sizeof(Head_t));
	// skhead_common
	H5Tinsert(headtype, "nrunsk", HOFFSET(Head_t, nrunsk), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "nsubsk", HOFFSET(Head_t, nsubsk), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "nevsk", HOFFSET(Head_t, nevsk), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "ndaysk", HOFFSET(Head_t, ndaysk), H5Tarray_create(H5T_NATIVE_INT32, 1, &dim[2]));
	H5Tinsert (headtype, "ntimsk", HOFFSET(Head_t, ntimsk), H5Tarray_create(H5T_NATIVE_INT32, 1, &dim[3]));
	H5Tinsert (headtype, "nt48sk", HOFFSET(Head_t, nt48sk), H5Tarray_create(H5T_NATIVE_INT32, 1, &dim[2]));
	H5Tinsert (headtype, "mdrnsk", HOFFSET(Head_t, mdrnsk), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "idtgsk", HOFFSET(Head_t, idtgsk), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "ifevsk", HOFFSET(Head_t, ifevsk), H5T_NATIVE_INT32);
	// skheada_common
	H5Tinsert (headtype, "ltcgps", HOFFSET(Head_t, ltcgps), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "nsgps", HOFFSET(Head_t, nsgps), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "nusgps", HOFFSET(Head_t, nusgps), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "ltctrg", HOFFSET(Head_t, ltctrg), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "ltcbip", HOFFSET(Head_t, ltcbip), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "ltdct0", HOFFSET(Head_t, itdct0), H5Tarray_create(H5T_NATIVE_INT32, 1, &dim[3]));
	H5Tinsert (headtype, "iffscc", HOFFSET(Head_t, iffscc), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "icalva", HOFFSET(Head_t, icalva), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "sk_geometry", HOFFSET(Head_t, sk_geometry), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "onldata_block_id", HOFFSET(Head_t, onldata_block_id), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "swtrg_id", HOFFSET(Head_t, swtrg_id), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "counter_32", HOFFSET(Head_t, counter_32), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "hostid_high", HOFFSET(Head_t, hostid_high), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "hostid_low", HOFFSET(Head_t, hostid_low), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "t0", HOFFSET(Head_t, t0), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "gate_width", HOFFSET(Head_t, gate_width), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "sw_trg_mask", HOFFSET(Head_t, sw_trg_mask), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "contents", HOFFSET(Head_t, contents), H5T_NATIVE_INT32);
	H5Tinsert (headtype, "ntrg", HOFFSET(Head_t, ntrg), H5T_NATIVE_INT32);
	
	header_d = H5PTcreate(output, "HEADER", headtype, 16, dsp);
	if(H5PTis_valid(header_d)<0) {
		fprintf(stderr, "Unable to create packet table header. err: " + H5PTis_valid(header_d));
		abort();
	}

}
void Converter::Init_TQREAL_Type(){
	tqrealtype = H5Tcreate (H5T_COMPOUND, sizeof(tqreal_t));
	H5Tinsert (tqrealtype, "nrunsk", HOFFSET(RunNo_t, nrunsk), H5T_NATIVE_INT32);
	H5Tinsert (tqrealtype, "nsubsk", HOFFSET(RunNo_t, nsubsk), H5T_NATIVE_INT32);
	H5Tinsert (tqrealtype, "nevsk", HOFFSET(RunNo_t, nevsk), H5T_NATIVE_INT32);
	H5Tinsert (tqrealtype, "ntrigsk", HOFFSET(RunNo_t, ntrigsk), H5T_NATIVE_INT32);
	H5Tinsert (tqrealtype, "iflag", HOFFSET(tqreal_t, iflag), H5T_NATIVE_INT32);
	H5Tinsert (tqrealtype, "cable", HOFFSET(tqreal_t, cable), H5T_NATIVE_INT32);
	H5Tinsert (tqrealtype, "T", HOFFSET(tqreal_t, T), H5T_NATIVE_FLOAT);
	H5Tinsert (tqrealtype, "Q", HOFFSET(tqreal_t, Q), H5T_NATIVE_FLOAT);
	tqreal_d = H5PTcreate(output, "TQReal", tqrealtype, 16, dsp);
	if(H5PTis_valid(tqreal_d)<0) {
		fprintf(stderr, "Unable to create packet table tqreal for ID. err: " + H5PTis_valid(tqreal_d));
		abort();
	}
	hid_t tqmetatype = H5Tcreate (H5T_COMPOUND, sizeof(tqmeta_t));
	H5Tinsert (tqmetatype, "nrunsk", HOFFSET(RunNo_t, nrunsk), H5T_NATIVE_INT32);
	H5Tinsert (tqmetatype, "nsubsk", HOFFSET(RunNo_t, nsubsk), H5T_NATIVE_INT32);
	H5Tinsert (tqmetatype, "nevsk", HOFFSET(RunNo_t, nevsk), H5T_NATIVE_INT32);
	H5Tinsert (tqmetatype, "ntrigsk", HOFFSET(RunNo_t, ntrigsk), H5T_NATIVE_INT32);

	H5Tinsert (tqmetatype, "nhits", HOFFSET(tqmeta_t, nhits), H5T_NATIVE_INT32);
	H5Tinsert (tqmetatype, "pc2pe", HOFFSET(tqmeta_t, pc2pe), H5T_NATIVE_FLOAT);
	H5Tinsert (tqmetatype, "tqreal_version", HOFFSET(tqmeta_t, tqreal_version), H5T_NATIVE_INT32);
	H5Tinsert (tqmetatype, "qbconst_version", HOFFSET(tqmeta_t, qbconst_version), H5T_NATIVE_INT32);
	H5Tinsert (tqmetatype, "tqmap_version", HOFFSET(tqmeta_t, tqmap_version), H5T_NATIVE_INT32);
	H5Tinsert (tqmetatype, "pgain_version", HOFFSET(tqmeta_t, pgain_version), H5T_NATIVE_INT32);
	H5Tinsert (tqmetatype, "it0xsk", HOFFSET(tqmeta_t, it0xsk), H5T_NATIVE_INT32);
	H5Tinsert (tqmetatype, "nIflagNot7", HOFFSET(tqmeta_t, nIflagNot7), H5T_NATIVE_INT32);
	tqmeta_d = H5PTcreate(output, "TQMetadata", tqmetatype, 16, dsp);
	if(H5PTis_valid(tqmeta_d)<0) {
		fprintf(stderr, "Unable to create packet table tqmeta for ID. err: " + H5PTis_valid(tqmeta_d));
		abort();
	}

	// tqareal_d = H5PTcreate(output, "TQAReal", tqrealtype, 16, dsp);
	// if(! H5PTis_valid(tqareal_d)) {
	// 	fprintf(stderr, "Unable to create packet table tqreal for OD.");
	// 	abort();
	// }
}
void Converter::Init_LOWE_Type(){
	lowetype = H5Tcreate(H5T_COMPOUND, sizeof(lowe_t));
	const char* member_names[] = {
		"nrunsk", "nsubsk", "nevsk", "ntrigsk",
		"bsvertex", "bsresult", "bsdir", "bsgood", "bsdirks", "bseffhit", "bsenergy", "bsn50", "bscossun",
		"clvertex", "clresult", "cldir", "clgoodness", "cldirks", "cleffhit", "clenergy", "cln50", "clcossun",
		"latmnum", "latmh", "lmx24", "ltimediff", "lnsratio", "lsdir",
		"spaevnum", "spaloglike", "sparesq", "spadt", "spadll", "spadlt", "spamuyn", "spamugdn",
		"posmc", "dirmc", "pabsmc", "energymc"
		};
	size_t member_offsets[] = {
		HOFFSET(RunNo_t, nrunsk), HOFFSET(RunNo_t, nsubsk), HOFFSET(RunNo_t, nevsk), HOFFSET(RunNo_t, ntrigsk),
		HOFFSET(lowe_t, bsvertex), HOFFSET(lowe_t, bsresult), HOFFSET(lowe_t, bsdir), HOFFSET(lowe_t, bsgood), HOFFSET(lowe_t, bsdirks), HOFFSET(lowe_t, bseffhit), HOFFSET(lowe_t, bsenergy), HOFFSET(lowe_t, bsn50), HOFFSET(lowe_t, bscossun),
		HOFFSET(lowe_t, clvertex), HOFFSET(lowe_t, clresult), HOFFSET(lowe_t, cldir), HOFFSET(lowe_t, clgoodness), HOFFSET(lowe_t, cldirks), HOFFSET(lowe_t, cleffhit), HOFFSET(lowe_t, clenergy), HOFFSET(lowe_t, cln50), HOFFSET(lowe_t, clcossun),
		HOFFSET(lowe_t, latmnum), HOFFSET(lowe_t, latmh), HOFFSET(lowe_t, lmx24), HOFFSET(lowe_t, ltimediff), HOFFSET(lowe_t, lnsratio), HOFFSET(lowe_t, lsdir),
		HOFFSET(lowe_t, spaevnum), HOFFSET(lowe_t, spaloglike), HOFFSET(lowe_t, sparesq), HOFFSET(lowe_t, spadt), HOFFSET(lowe_t, spadll), HOFFSET(lowe_t, spadlt), HOFFSET(lowe_t, spamuyn), HOFFSET(lowe_t, spamugdn),
		HOFFSET(lowe_t, posmc), HOFFSET(lowe_t, dirmc), HOFFSET(lowe_t, pabsmc), HOFFSET(lowe_t, energymc)
		};
    hid_t member_types[] = {
		H5T_NATIVE_INT, H5T_NATIVE_INT, H5T_NATIVE_INT, H5T_NATIVE_INT,
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[3]),
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[5]),
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[2]),
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[2]),
		H5T_NATIVE_FLOAT,
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[11]),
		H5T_NATIVE_FLOAT,
		H5T_NATIVE_INT,
		H5T_NATIVE_FLOAT,
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[3]),
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[3]),
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[2]),
		H5T_NATIVE_FLOAT,
		H5T_NATIVE_FLOAT,
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[11]),
		H5T_NATIVE_FLOAT,
		H5T_NATIVE_INT,
		H5T_NATIVE_FLOAT,
		H5T_NATIVE_INT,
		H5T_NATIVE_INT,
		H5T_NATIVE_INT,
		H5T_NATIVE_DOUBLE,
		H5T_NATIVE_FLOAT,
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[2]),
		H5T_NATIVE_INT,
		H5T_NATIVE_FLOAT,
		H5T_NATIVE_FLOAT,
		H5T_NATIVE_FLOAT,
		H5T_NATIVE_FLOAT,
		H5T_NATIVE_FLOAT,
		H5T_NATIVE_INT,
		H5T_NATIVE_FLOAT,
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[2]),
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[5]),
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[1]),
		H5Tarray_create(H5T_NATIVE_FLOAT, 1, &dim[1]),
		};
    int num_members = 40;
	for (int i = 0; i < num_members; ++i) {
        H5Tinsert(lowetype, member_names[i], member_offsets[i], member_types[i]);
    }
	lowe_d = H5PTcreate(output, "LOWE", lowetype, 16, dsp);
	if(H5PTis_valid(lowe_d)<0) {
		fprintf(stderr, "Unable to create packet table tqmeta for ID. err: " + H5PTis_valid(lowe_d));
		abort();
	}
}
void Converter::Convert_HEADER(Header* HEAD){
	// runno
	h5_runno.nrunsk = HEAD->nrunsk;
	h5_runno.nsubsk = HEAD->nsubsk;
	h5_runno.nevsk = HEAD->nevsk;
	h5_runno.ntrigsk = HEAD->t0;
	// header
	h5_head.nrunsk = HEAD->nrunsk;
	h5_head.nsubsk = HEAD->nsubsk;
	h5_head.nevsk = HEAD->nevsk;
	h5_head.mdrnsk = HEAD->mdrnsk;
	h5_head.idtgsk = HEAD->idtgsk;
	h5_head.ifevsk = HEAD->ifevsk;
	h5_head.ltcgps = HEAD->ltcgps;
	h5_head.nsgps = HEAD->nsgps;
	h5_head.mdrnsk = HEAD->mdrnsk;
	h5_head.nusgps = HEAD->nusgps;
	h5_head.ltctrg = HEAD->ltctrg;
	h5_head.ltcbip = HEAD->ltcbip;
	h5_head.iffscc = HEAD->iffscc;
	h5_head.icalva = HEAD->icalva;
	h5_head.sk_geometry = HEAD->sk_geometry;
	h5_head.onldata_block_id = HEAD->onldata_block_id;
	h5_head.swtrg_id = HEAD->swtrg_id;
	h5_head.counter_32 = HEAD->counter_32;
	h5_head.hostid_high = HEAD->hostid_high;
	h5_head.hostid_low = HEAD->hostid_low;
	h5_head.t0 = HEAD->t0;
	h5_head.gate_width = HEAD->gate_width;
	h5_head.sw_trg_mask = HEAD->sw_trg_mask;
	h5_head.sw_trg_mask = HEAD->sw_trg_mask;
	h5_head.ntrg = HEAD->ntrg;
	H5PTappend(header_d, 1, &h5_head);
}
void Converter::Convert_TQREAL(TQReal* TQREAL){
	// runno
	int ntrigsk = (TQREAL->it0xsk - h5_runno.ntrigsk) / 1.92 / 10;
	h5_runno.ntrigsk = ntrigsk;
	// tqmeta and tqreal
	h5_tqreal.runno = h5_runno;

	const int icabbit=0xffff;
	int nIflagNot7 = 0;
	for (int j = 0; j < TQREAL->nhits; j++) {
		int icab = TQREAL->cables[j] & icabbit;
		int iflag = TQREAL->cables[j] >> 16;
		if (iflag != 7) ++nIflagNot7;
		h5_tqreal.cable = icab;
		h5_tqreal.iflag = iflag;
		h5_tqreal.T = TQREAL->T[j];
		h5_tqreal.Q = TQREAL->Q[j];
		H5PTappend(tqreal_d, 1, &h5_tqreal);
	}

	h5_tqmeta.runno = h5_runno;
	h5_tqmeta.nhits = TQREAL->nhits;
	h5_tqmeta.tqreal_version = TQREAL->tqreal_version;
	h5_tqmeta.qbconst_version = TQREAL->qbconst_version;
	h5_tqmeta.tqmap_version = TQREAL->tqmap_version;
	h5_tqmeta.pgain_version = TQREAL->pgain_version;
	h5_tqmeta.nIflagNot7 = nIflagNot7;
	h5_tqmeta.it0xsk = TQREAL->it0xsk;
	H5PTappend(tqmeta_d, 1, &h5_tqmeta);
}
// void Converter::Convert_TQREAL(TQAReal* TQAREAL){
	
// }
void Converter::Convert_LOWE(LoweInfo* LOWE){
	h5_lowe.runno = h5_runno;
	for(int i=0; i<4; i++) h5_lowe.bsvertex[i] = LOWE->bsvertex[i];
	for(int i=0; i<6; i++) h5_lowe.bsresult[i] = LOWE->bsresult[i];
	for(int i=0; i<3; i++) h5_lowe.bsdir[i] = LOWE->bsdir[i];
	for(int i=0; i<3; i++) h5_lowe.bsgood[i] = LOWE->bsgood[i];
	h5_lowe.bsdirks = LOWE->bsdirks;
	for(int i=0; i<12; i++) h5_lowe.bseffhit[i] = LOWE->bseffhit[i];
	h5_lowe.bsenergy = LOWE->bsenergy;
	h5_lowe.bsn50 = LOWE->bsn50;
	h5_lowe.bscossun = LOWE->bscossun;
	for(int i=0; i<4; i++) h5_lowe.clvertex[i] = LOWE->clvertex[i];
	for(int i=0; i<4; i++) h5_lowe.clresult[i] = LOWE->clresult[i];
	for(int i=0; i<3; i++) h5_lowe.cldir[i] = LOWE->cldir[i];
	h5_lowe.clgoodness = LOWE->clgoodness;
	h5_lowe.cldirks = LOWE->cldirks;
	for(int i=0; i<12; i++) h5_lowe.cleffhit[i] = LOWE->cleffhit[i];
	h5_lowe.clenergy = LOWE->clenergy;
	h5_lowe.cln50 = LOWE->cln50;
	h5_lowe.clcossun = LOWE->clcossun;
	h5_lowe.latmnum = LOWE->latmnum;
	h5_lowe.latmh = LOWE->latmh;
	h5_lowe.lmx24 = LOWE->lmx24;
	h5_lowe.ltimediff = LOWE->ltimediff;
	h5_lowe.lnsratio = LOWE->lnsratio;
	for(int i=0; i<3; i++) h5_lowe.lsdir[i] = LOWE->lsdir[i];
	h5_lowe.spaevnum = LOWE->spaevnum;
	h5_lowe.spaloglike = LOWE->spaloglike;
	h5_lowe.sparesq = LOWE->sparesq;
	h5_lowe.spadt = LOWE->spadt;
	h5_lowe.spadll = LOWE->spadll;
	h5_lowe.spadlt = LOWE->spadlt;
	h5_lowe.spamuyn = LOWE->spamuyn;
	h5_lowe.spamugdn = LOWE->spamugdn;
	for(int i=0; i<3; i++) h5_lowe.posmc[i] = LOWE->posmc[i];
	for(int i=0; i<2; i++) for(int j=0; j<3; j++) h5_lowe.dirmc[i*3+j] = LOWE->dirmc[i][j];
	for(int i=0; i<2; i++) h5_lowe.pabsmc[i] = LOWE->pabsmc[i];
	for(int i=0; i<2; i++) h5_lowe.energymc[i] = LOWE->energymc[i];		
	H5PTappend(lowe_d, 1, &h5_lowe);
}