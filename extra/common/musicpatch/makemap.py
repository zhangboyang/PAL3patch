import pickle
import audioread
import simpleaudio

def audiolen(filename):
    with audioread.audio_open(filename) as f:
        return f.duration
def playfile(filename):
    with audioread.audio_open(filename) as f:
        print("  chan =", f.channels, ", samplerate =", f.samplerate, ", length =", f.duration, end='', flush=True)
        p = simpleaudio.play_buffer(b''.join([b for b in f]), f.channels, 2, f.samplerate)
        input()
        p.stop()


INF = 1e100

# what the program do
#   1. filter out dist >= DIST_LIMIT
#   2. if dist < DIST_SURE_LIMIT or dist > DIST_ERROR_LIMIT
#         no audio play, directly output to result
#      otherwise
#         play audio and output to result

# howto
#   1. set DIST_LIMIT to some reasonable value, for example, 100.0
#        set DIST_SURE_LIMIT to -INF
#        set DIST_ERROR_LIMIT to INF
#      listen to audio, make sure they matched exactly
#      if not, lower the value of DIST_LIMIT
#      (however, at this time, there may be good files doesn't matched)
#
#   2. set DIST_LIMIT to INF
#        set DIST_SURE_LIMIT to the DIST_LIMIT in last step
#        set DIST_ERROR_LIMIT to some reasonable value, for example, 110.0
#      this will let the program play the audio track
#        which distance is in [DIST_SURE_LIMIT, DIST_ERROR_LIMIT]
#      listen to audio, make sure they matched exactly
#        then you can let the new DIST_SURE_LIMIT be DIST_ERROR_LIMIT
#      repeat this step, until you cannot increase DIST_SURE_LIMIT
#
#   3. set DIST_LIMIT to INF
#        set DIST_SURE_LIMIT to the DIST_LIMIT in last step
#        set DIST_ERROR_LIMIT to INF
#      listen to audio, make sure there is NO match
#
#   4. set DIST_LIMIT to the final value of DIST_SURE_LIMIT in last step
#        set DIST_SURE_LIMIT to INF
#        set DIST_ERROR_LIMIT to INF
#      this will be the final result

DIST_LIMIT = 110
DIST_SURE_LIMIT = -INF
DIST_ERROR_LIMIT = INF



FIRST_EXT = ["m4a"]
SECOND_EXT = ["mp3"]

with open('resultdump.bin', 'rb') as f:
    cmpresult = pickle.load(f)

result = []
for o, dl in cmpresult:
    print("  COMPARE:")
    for x in sorted(dl)[0:5]:
        print("   ", x)

    dl = list(filter(lambda x: x[0] < DIST_LIMIT, dl))
    
    l1 = list(filter(lambda x: x[1].split('.')[-1].lower() in FIRST_EXT, dl))
    l2 = list(filter(lambda x: x[1].split('.')[-1].lower() in SECOND_EXT, dl))
    l = l1 + l2
    assert(len(dl) == len(l))
    print("  COMPARE WITH PRIORITY:")
    for x in l[0:5]:
        print("   ", x)
    
    assert(len(l) > 0)
    assert(len(l2) > 0)
    
    comments = []
    
    if len(l1) == 0:
        print("  WARNING: no good result for '" + o + "'")
        comments.append("no-primary")
    
    if len(l1) > 1:
        print("  WARNING: multiple good result for '" + o + "', using first match")
        comments.append("multi-primary")
    
    if len(l2) > 1:
        print("  WARNING: multiple secondary result for '" + o + "'")
        comments.append("multi-secondary")

    mind = sorted(dl)[0][0]
    d, c = l[0]
    c2 = l2[0][1]
    print("  O:", o)
    print("  C:", c)
    print("  D   :", d)
    print("  MIND:", mind)
    ol = audiolen(o)
    cl = audiolen(c)

    if d > DIST_SURE_LIMIT:
        if d < DIST_ERROR_LIMIT:
            playfile(o)
            playfile(c)
        
            print("  COMMENTS: ", end='', flush=True)
            comments.append("user: " + input())
        else:
            comments.append("error")
    
    result.append((o, (c, c2), (ol, cl), (d, mind), d/mind, (len(l1), len(l2)), l1, l2, ", ".join(comments)))
    
    

for x in result:
    print(x)
for x in result:
    print("===")
    for y in x:
        print("    ", y)


with open('mapdump.bin', 'wb') as f:
    pickle.dump(result, f)

