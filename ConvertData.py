#!/opt/gentoo/usr/bin/python3.6

import uproot
import sys
import tables
import numpy as np
from time import time

if len(sys.argv)!=3 : print("Usage: python3 ConvertData.py inputfile outputfile")

fip = uproot.open(sys.argv[1])
fout = sys.argv[2]

start=time()
ReadoutTree = fip["Readout"]
WaveformArray = np.array(ReadoutTree["Waveform"].array())


