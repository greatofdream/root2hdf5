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

print(time()-start)
embed()
