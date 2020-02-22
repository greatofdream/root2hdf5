#!/opt/gentoo/usr/bin/python3.6

import uproot
import sys
import tables
import numpy as np
from time import time

from IPython import embed

Len = np.vectorize(len)

if len(sys.argv)!=3 : print("Usage: python3 ConvertData.py inputfile outputfile")

fip = uproot.open(sys.argv[1])
fout = sys.argv[2]

start=time()
ReadoutTree = fip["Readout"]
Waveform = np.array(ReadoutTree["Waveform"].array(flatten=True),dtype=np.int16)
ChannelID = np.array(ReadoutTree["ChannelId"].array(flatten=True),dtype=np.int16)
EventID = np.array(ReadoutTree["TriggerNo"].array(),dtype=np.int64)
Sec= ReadoutTree["Sec"].array()
NanoSec = ReadoutTree["NanoSec"].array()

nWave = len(ChannelID)
WindowSize = int(len(Waveform)/nWave)
nChannels = Len(np.array(ReadoutTree["ChannelId"].array()))

EventID_2 = EventID.repeat(nChannels)
Waveform = Waveform.reshape(nWave,WindowSize)


class WaveformData(tables.IsDescription):
    EventID    = tables.Int64Col(pos=0)
    ChannelID  = tables.Int16Col(pos=1)
    Waveform   = tables.Col.from_type('int16', shape=WindowSize, pos=2)

class TriggerInfoData(tables.IsDescription):
    EventID    = tables.Int64Col(pos=0)
    Sec        = tables.Int32Col(pos=1)
    NanoSec    = tables.Int32Col(pos=2)

h5file = tables.open_file(sys.argv[2], mode="w", title="OneTonDetector",filters = tables.Filters(complevel=9))
TriggerInfoTable = h5file.create_table("/", "TriggerInfo", TriggerInfoData, "Trigger info")
triggerinfo = TriggerInfoTable.row
WaveformTable = h5file.create_table("/", "Waveform", WaveformData, "Waveform")
waveform = WaveformTable.row

print(time()-start)
mid=time()

for i in range(nWave) :
    waveform['EventID'] = EventID_2[i]
    waveform['ChannelID'] = ChannelID[i]
    waveform['Waveform'] = Waveform[i]
    waveform.append()
    
for i in range(len(EventID)) :
    triggerinfo['EventID'] = EventID[i]
    triggerinfo['Sec'] = Sec[i]
    triggerinfo['NanoSec'] = NanoSec[i]
    triggerinfo.append()

print(time()-mid)
mid=time()

WaveformTable.flush()
TriggerInfoTable.flush()
print(time()-mid)
mid=time()
h5file.close()

print(time()-start)
