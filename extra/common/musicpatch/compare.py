import librosa
from numpy.linalg import norm
#import dtw
import fastdtw
import numpy
import pickle

SAMPLE_OFFSET = 10.0
SAMPLE_LEN = 10.0
SAMPLE_RATE = 44100



# use these shell command to generate file list
#   find candidate/ -type f | sort > candidate.txt
#   find original/ -type f | sort > original.txt




def load_list(fn):
    f = open(fn)
    l = list(filter(lambda x: len(x) > 0 and x[0] != ';' and x[0] != '#', f.read().splitlines()))
    f.close()
    return l

def calc_mfcc(l):
    r = []
    for f in l:
        print("loading", f, "... ", end='', flush=True)
        y, sr = librosa.load(f, sr=None, offset=SAMPLE_OFFSET, duration=SAMPLE_LEN)
        if len(y) > 0 and (not (sr is None)) and numpy.isclose(float(len(y)) / sr, SAMPLE_LEN) and sr == SAMPLE_RATE:
            print("mfcc ... ", end='', flush=True)
            mfcc = librosa.feature.mfcc(y, sr)
            r.append((f, mfcc))
            print("done")
        else:
            print("WARNING: ignored file", f)
    return r

def calc_dist(a, b):
    #return dtw.dtw(a[1].T, b[1].T, dist=lambda x, y: norm(x - y, ord=1))[0]
    return fastdtw.fastdtw(a[1].T, b[1].T, dist=lambda x, y: norm(x - y, ord=1))[0] / (len(a[1].T) + len(b[1].T))


# load list
ofl = load_list("original.txt")
print("ORIGINAL:", ofl)
cfl = load_list("candidate.txt")
print("CANDIDATE:", cfl)


# calc mfcc
o = calc_mfcc(ofl)
c = calc_mfcc(cfl)

# do compare
result = []
for oitem in o:
    dl = []
    for citem in c:
        print("compare", oitem[0], "with", citem[0], "... ", end='', flush=True)
        d = calc_dist(oitem, citem)
        print("distance =", d)
        dl.append((d, citem[0]))
    
    print(dl)
    result.append((oitem[0], dl))

# write result
with open('resultdump.bin', 'wb') as f:
    pickle.dump(result, f)

