import pickle
import shutil
import os

with open('mapdump.bin', 'rb') as f:
    mapresult = pickle.load(f)

RESULT_PATH = "result"

try:
    os.mkdir(RESULT_PATH)
except:
    pass

for i in mapresult:
    src = i[0]
    dst = i[1][0]
    srcfn = os.path.basename(src)
    dstfn = os.path.basename(dst)
    
    print(srcfn, "=>", dstfn)
    shutil.copy2(dst, RESULT_PATH + '/' + srcfn)

