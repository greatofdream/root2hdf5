#!/opt/gentoo/usr/bin/python3.6

import argparse
psr = argparse.ArgumentParser()
psr.add_argument("--limit", type=int, default=999, help="limit of number of files to process")
psr.add_argument('ipt', type=str, help="input filename prefix")
psr.add_argument("opt", help="output")
args = psr.parse_args()
file_prefix = args.ipt
maxfiles = args.limit
opt = args.opt

from time import time
start = time()
global_start = time()

import uproot
import h5py
import multiprocessing
import numpy as np

Len = np.vectorize(len)

filename = file_prefix + ".root"
fip = uproot.open(filename)
WindowSize = int(len(fip["Readout"]["Waveform"].array(entrystart=0, entrystop=1)[0]) / len(fip["Readout"]["ChannelId"].array(entrystart=0, entrystop=1)[0]))

h5file = h5py.File(opt, 'w')


def ReadFile(fip) :
    EventID = fip["Readout"]["TriggerNo"].array().astype(np.int64)
    # Sec = fip["Readout"]["Sec"].array().astype(np.int32)
    # NanoSec = fip["Readout"]["NanoSec"].array().astype(np.int32)

    Waveform = fip["Readout"]["Waveform"].array(flatten=True).astype(np.int16)
    ChannelID = fip["Readout"]["ChannelId"].array()
    nChannels = Len(ChannelID)
    EventID2 = EventID.repeat(nChannels)
    ChannelID2 = ChannelID.flatten().astype(np.int16)
    WindowSize = int(len(Waveform) / len(ChannelID2))
    nWave = len(ChannelID2)
    Waveform = Waveform.reshape(nWave, WindowSize)
    # WaveFrame = pd.DataFrame({"EventID":EventID2,"ChannelID":ChannelID2,"Waveform":list(Waveform)}):

    EventID = fip["SimTriggerInfo"]["TriggerNo"].array().astype(np.int64)
    nphotons = fip["SimTriggerInfo"]["PEList"].array(entrystop=len(EventID))
    EventID3 = EventID.repeat(nphotons)
    ChannelID3 = fip["SimTriggerInfo"]["PEList.PMTId"].array(flatten=True, entrystop=len(EventID))
    HitPos = fip["SimTriggerInfo"]["PEList.HitPosInWindow"].array(flatten=True, entrystop=len(EventID))
    Charge = fip["SimTriggerInfo"]["PEList.Charge"].array(flatten=True, entrystop=len(EventID))
    # TruthFrame = pd.DataFrame({"EventID":EventID3,"ChannelID":ChannelID3,"HitTime":HitTime,"PulseTime":PulseTime,"HitPos":HitPos})
    return {"EventID3": EventID3, "ChannelID3": ChannelID3, "Charge": Charge, "HitPos": HitPos, "EventID2": EventID2, "ChannelID2": ChannelID2, "Waveform": Waveform}


file_handles = [fip]
for fileNo in range(1, maxfiles) :
    filename = file_prefix + "_{}.root".format(fileNo)
    try : fip = uproot.open(filename)
    except FileNotFoundError :
        print("only {} files found.".format(fileNo))
        break
    file_handles.append(fip)
print("Initialized, consuming {:.2f}s.".format(time() - start))

start = time()
Results = [ReadFile(fip) for fip in file_handles]
keys = Results[-1].keys()
Result = {key: np.concatenate([result[key] for result in Results]) for key in keys}
print("root file read, consuming {:.2f}s.".format(time() - start))

waveform_dtype = [("EventID", np.int64), ("ChannelID", np.int16), ("Waveform", np.int16, 1029)]
truth_dtype = [("EventID", np.int64), ("ChannelID", np.int16), ("RiseTime", np.int16), ("Charge", np.double)]
start = time()
WaveformTable = np.empty(len(Result["Waveform"]), dtype=waveform_dtype)
WaveformTable["EventID"] = Result["EventID2"]
WaveformTable["ChannelID"] = Result["ChannelID2"]
WaveformTable["Waveform"] = Result["Waveform"]

GroundTruthTable = np.empty(len(Result["HitPos"]), dtype=truth_dtype)
GroundTruthTable["EventID"] = Result["EventID3"]
GroundTruthTable["ChannelID"] = Result["ChannelID3"]
GroundTruthTable["RiseTime"] = Result["HitPos"]
GroundTruthTable["Charge"] = Result["Charge"]
print("data copied, consuming {:.2f}s.".format(time() - start))

num_processors = 8
waveform_len = len(Result["Waveform"])
truth_len = len(Result["HitPos"])
dwf = h5file.create_dataset("Waveform", (waveform_len,), dtype=waveform_dtype, compression="gzip", compression_opts=9)


def WriteFile(rank) :
    slices = np.linspace(0, waveform_len, num_processors + 1, dtype=np.int64)
    print("{0}: slices[{0}]={1}".format(rank, slices[rank]))
    # with dwf.collective:
    dwf[slices[rank]:slices[rank + 1]] = WaveformTable[slices[rank]:slices[rank + 1]]


start = time()
with multiprocessing.Pool(num_processors) as pool :
    pool.map(WriteFile, list(range(num_processors)))
# for rank in range(num_processors) :
#     WriteFile(rank)
# dtr = h5file.create_dataset("GroundTruth", (truth_len,), dtype=truth_dtype, compression="gzip", compression_opts=9)
# with dtr.collective:
#     dtr[:] = GroundTruthTable
print("h5 file written, consuming {:.2f}s.".format(time() - start))

h5file.close()
print("Finished, consuming {:.2f}s.".format(time() - global_start))
