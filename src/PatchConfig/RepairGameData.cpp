#include "stdafx.h"

RepairCommitter::~RepairCommitter()
{
}

static int detect_game_locale()
{
	DWORD key_CRC = 0xCB283888; /* datascript\lang.txt */
	const char *CHS_magic = "\x41\x70\x70\x4E\x61\x6D\x65\x09\x09\x24\xCF\xC9\xBD\xA3\xC6\xE6\xCF\xC0\xB4\xAB\x33\x26";
	const char *CHT_magic = "\x41\x70\x70\x4E\x61\x6D\x65\x09\x09\x24\xA5\x50\xBC\x43\xA9\x5F\xAB\x4C\xB6\xC7\x33\x26";
	size_t CHS_magic_len = strlen(CHS_magic);
	size_t CHT_magic_len = strlen(CHT_magic);

	int result = -1;

	FILE *fp = NULL;
	tagCPKHeader hdr;
	tagCPKTable *tbl = NULL;

	tagCPKTable *item = NULL;
	void *data = NULL;
	unsigned sz;
	unsigned i;

	fp = fopen("basedata\\basedata.cpk", "rb");
	if (!fp) goto done;

	if (fread(&hdr, sizeof(hdr), 1, fp) != 1) goto done;
	if (hdr.dwValidTableNum > 0x8000) goto done;

	tbl = (tagCPKTable *) malloc(hdr.dwValidTableNum * sizeof(tagCPKTable));
	if (!tbl) goto done;
	if (fread(tbl, sizeof(tagCPKTable), hdr.dwValidTableNum, fp) != hdr.dwValidTableNum) goto done;

    for (i = 0; i < hdr.dwValidTableNum; i++) {
        if (tbl[i].dwCRC == key_CRC && (tbl[i].dwFlag & 0x1) && !(tbl[i].dwFlag & 0x10)) {
			item = &tbl[i];
            break;
        }
    }
    if (!item) goto done;
    
	if (item->dwStartPos >= 0x40000000) goto done;
	if (item->dwPackedSize >= 0x100000) goto done;
    sz = item->dwPackedSize;
    data = malloc(sz);
    if (!data) goto done;
    if (fseek(fp, item->dwStartPos, SEEK_SET) != 0) goto done;
    if (fread(data, 1, sz, fp) != sz) goto done;
    
    for (i = 0; i < sz; i++) {
        if (i + CHS_magic_len <= sz && memcmp(PTRADD(data, i), CHS_magic, CHS_magic_len) == 0) {
            result = 0;
            break;
        }
        if (i + CHT_magic_len <= sz && memcmp(PTRADD(data, i), CHT_magic, CHT_magic_len) == 0) {
            result = 1;
            break;
        }
    }
    
done:
    free(data);
    free(tbl);
    if (fp) fclose(fp);
    
    return result;
}
static bool detect_music_patch()
{
	FileRW *fp = new FileRW("music\\music.cpk", sizeof(tagCPKHeader));
	fp->inc();
	unsigned sz = fp->realsize();
	fp->dec();
	return sz > 128 * 1048576;
}

class RepairProgress : public ProgressObject {
	unsigned curv;
	unsigned maxv;
	bool cancel;
	CWnd *fawnd;
	static RepairProgress *instance;
	static bool cancelfunc()
	{
		if (!instance->cancel) {
			if (GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_ASKCANCELREPAIR), STRTABLE(IDS_REPAIRGAMEDATA), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
				instance->cancel = true;
			}
		}
		return instance->cancel;
	}
public:
	RepairProgress(CWnd *fa) : curv(0), maxv(0), cancel(false), fawnd(fa)
	{
		assert(instance == NULL);
		instance = this;
	}
	~RepairProgress()
	{
		assert(instance == this);
		instance = NULL;
	}
	void update(LPCTSTR msg = NULL)
	{
		assert(curv <= maxv);
		float progress = maxv ? (double)curv / maxv : 0;
		ShowPleaseWaitDlg(fawnd, msg, false, progress, cancelfunc, msg != NULL);
	}
	void reset(unsigned maxvalue)
	{
		maxv = maxvalue;
		curv = 0;
		update();
	}
	void progress(unsigned value)
	{
		curv = value;
		update();
	}
	bool cancelled()
	{
		return cancel;
	}
};
RepairProgress *RepairProgress::instance = NULL;

static std::vector<std::pair<CString, std::vector<RepairCommitter *> > > transactions;
#define staged (transactions.back().second)
static void initialize_staged(const CString &component)
{
	transactions.push_back(std::make_pair(component, std::vector<RepairCommitter *>()));
}
static bool confirm_staged()
{
	if (staged.empty()) transactions.pop_back();
	return true;
}
static bool discard_staged()
{
	std::vector<RepairCommitter *>::iterator it;
	for (it = staged.begin(); it != staged.end(); it++) {
		delete *it;
	}
	staged.clear();
	return false;
}

static bool repair_cpk(RepairProgress *rp, BufferReader &r, LPCTSTR msg, bool grp)
{
	CString s;
	int num = grp ? r.u32() : 1;
	for (int i = 1; i <= num; i++) {
		if (rp->cancelled()) return false;
		rp->reset(0);
		const char *cpkpath = r.str();
		const char *cpkfile = strrchr(cpkpath, '\\') ? strrchr(cpkpath, '\\') + 1 : cpkpath;
		if (grp) {
			s.Format(msg, cpkfile, i, num);
		} else {
			s = msg;
		}
		rp->update(s);
		CPKFixer *cf = new CPKFixer(cpkpath, r);
		int state = cf->repair(rp);
		if (state > 0) {
			staged.push_back(cf);
			continue;
		}
		delete cf;
		if (state < 0) {
			return false;
		}
	}
	return true;
}
static bool repair_file(RepairProgress *rp, BufferReader &r, LPCTSTR msg, bool raw)
{
	if (rp->cancelled()) return false;
	rp->reset(0);
	rp->update(msg);
	FileFixer *ff;
	if (raw) {
		ff = new FileRawFixer(r);
	} else {
		ff = new FileXorFixer(r);
	}
	int state = ff->repair(rp);
	if (state > 0) {
		staged.push_back(ff);
		return true;
	} else {
		delete ff;
		return state == 0;
	}
}

static void branch_buffer(BufferReader &r, BufferReader *br, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		br[i] = r.adv(r.u32());
	}
}
static bool repair_core(RepairProgress *rp, BufferReader &r, int gl)
{
	if (rp->cancelled()) return false;
	initialize_staged(STRTABLE(IDS_REPAIRCORE));
	if (!repair_file(rp, r, STRTABLE(IDS_REPAIRCORE_FILE), true)) return discard_staged();
	BufferReader br[2];
	branch_buffer(r, br, 2);
	if (!repair_cpk(rp, br[gl], STRTABLE(IDS_REPAIRCORE_CPK), true)) return discard_staged();
	return confirm_staged();
}
static bool repair_music(RepairProgress *rp, BufferReader &r, int gl)
{
	if (rp->cancelled()) return false;
	initialize_staged(STRTABLE(IDS_REPAIRMUSIC));
	BufferReader br[3];
	branch_buffer(r, br, 3);
	BufferReader &music = br[detect_music_patch() ? 2 : gl];
	if (!repair_cpk(rp, music, STRTABLE(IDS_REPAIRMUSIC_MUSIC), false)) return discard_staged();
	if (!repair_file(rp, music, STRTABLE(IDS_REPAIRMUSIC_SOUND), false)) return discard_staged();
	return confirm_staged();
}
static bool repair_movie(RepairProgress *rp, BufferReader &r, int gl)
{
	if (rp->cancelled()) return false;
	initialize_staged(STRTABLE(IDS_REPAIRMOVIE));
	BufferReader br[2];
	branch_buffer(r, br, 2);
	if (!repair_cpk(rp, br[gl], STRTABLE(IDS_REPAIRMOVIE_CPK), true)) return discard_staged();
	return confirm_staged();
}
static bool repair_misc(RepairProgress *rp, BufferReader &r, int gl)
{
	if (rp->cancelled()) return false;
	initialize_staged(STRTABLE(IDS_REPAIRMISC));
	BufferReader br[2];
	branch_buffer(r, br, 2);
	if (!repair_file(rp, br[gl], STRTABLE(IDS_REPAIRMISC_FILE), true)) return discard_staged();
	return confirm_staged();
}
static void run_repair(RepairProgress *rp, BufferReader &r, int gl)
{
	assert(transactions.empty());
	r.str();
	repair_core(rp, r, gl);
	repair_music(rp, r, gl);
	repair_movie(rp, r, gl);
	repair_misc(rp, r, gl);
	std::vector<std::pair<CString, std::vector<RepairCommitter *> > >::iterator i;
	std::vector<RepairCommitter *>::iterator j;
	if (!rp->cancelled()) {
		ShowPleaseWaitDlg(NULL, STRTABLE(IDS_REPAIR_CHECKDONE), false);
		CString fix, bad, msg;
		for (i = transactions.begin(); i != transactions.end(); i++) {
			if (i->second.empty()) {
				bad += _T("\n    ") + i->first;
			} else {
				fix += _T("\n    ") + i->first;
			}
		}
		if (!fix.IsEmpty()) {
			if (!bad.IsEmpty()) {
				msg = STRTABLE(IDS_REPAIR_NEEDFIX) + fix + _T("\n\n") + STRTABLE(IDS_REPAIR_CANTFIX) + bad + _T("\n\n") + STRTABLE(IDS_REPAIR_ASKFIX);
			} else {
				msg = STRTABLE(IDS_REPAIR_NEEDFIX) + _T("\n") + fix + _T("\n\n") + STRTABLE(IDS_REPAIR_ASKFIX);
			}
			if (GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_REPAIR_REPORT), MB_YESNO | MB_ICONWARNING) == IDYES) {
				ShowPleaseWaitDlg(NULL, STRTABLE(IDS_REPAIR_COMMIT));
				bad.Empty();
				bool success = true;
				for (i = transactions.begin(); i != transactions.end(); i++) {
					if (i->second.empty()) {
						bad += _T("\n    ") + i->first;
						continue;
					}
					bool badcommit = false;
					for (j = i->second.begin(); j != i->second.end(); j++) {
						if (!(*j)->commit()) badcommit = true;
					}
					if (badcommit) {
						msg.Format(STRTABLE(IDS_REPAIR_BADCOMMIT), (LPCTSTR) i->first);
						GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_REPAIR_BADCOMMIT_TITLE), MB_ICONERROR);
						bad += _T("\n    ") + i->first;
						success = false;
					}
				}
				if (!bad.IsEmpty()) {
					if (!success) {
						msg = STRTABLE(IDS_REPAIR_COMMITERROR) + bad;
					} else {
						msg = STRTABLE(IDS_REPAIR_COMMITPART) + bad;
					}
					GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_REPAIR_COMMITREPORT), MB_ICONWARNING);
				} else {
					GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_REPAIR_COMMITOK), STRTABLE(IDS_REPAIR_COMMITREPORT), MB_ICONINFORMATION);
				}
			} else {
				bad.Empty();
				for (i = transactions.begin(); i != transactions.end(); i++) {
					bad += _T("\n    ") + i->first;
				}
				msg = STRTABLE(IDS_REPAIR_COMMITABORT) + bad;
				GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_REPAIR_COMMITREPORT), MB_ICONWARNING);
			}
		} else if (!bad.IsEmpty()) {
			msg = STRTABLE(IDS_REPAIR_CANTFIX) + bad;
			GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_REPAIR_REPORT), MB_ICONWARNING);
		} else {
			GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_REPAIR_GOOD), STRTABLE(IDS_REPAIR_REPORT), MB_ICONINFORMATION);
		}
	}
	for (i = transactions.begin(); i != transactions.end(); i++) {
		for (j = i->second.begin(); j != i->second.end(); j++) {
			delete *j;
		}
	}
	transactions.clear();
}

static const struct {
	const char *filename;
	unsigned filesize;
	const char *filehash;
} RepairPack = {
#ifdef BUILD_FOR_PAL3
    "PAL3repair.bin", 16521650, "bb6b959efa567ee286e19639367103e1c61804fd"
#endif
#ifdef BUILD_FOR_PAL3A
    "PAL3Arepair.bin", 12870839, "b074fbc903fb6f119872816e3232003ec3f30138"
#endif
};
static void *load_repair(RepairProgress *rp)
{
	void *buf = Malloc(RepairPack.filesize);
	void *ret = NULL;
	FileRW *fp = new FileRW(RepairPack.filename, RepairPack.filesize);
	fp->inc();
	rp->reset(RepairPack.filesize);
	if (fp->realsize() == RepairPack.filesize) {
		SHA1_CTX ctx;
		SHA1Init(&ctx);
		unsigned offset = 0;
		ProgressBinder<unsigned> pb(rp, &offset, RepairPack.filesize, 1);
		while (offset < RepairPack.filesize) {
			if (!pb.update()) break;
			size_t count = 4096;
			count = count < RepairPack.filesize - offset ? count : RepairPack.filesize - offset;
			if (!fp->read(PTRADD(buf, offset), offset, count)) break;
			SHA1Update(&ctx, (const unsigned char *) PTRADD(buf, offset), count);
			offset += count;
		}
		if (offset == RepairPack.filesize && pb.update()) {
			SHA1Hash hash;
			SHA1Final(hash.digest, &ctx);
			if (hash == SHA1Hash::fromhex(RepairPack.filehash)) {
				ret = buf;
			}
		}
	}
	if (!ret) free(buf);
	fp->dec();
	return ret;
}
void RepairGameData(CPatchConfigDlg *dlg)
{
	RepairProgress *rp = new RepairProgress(dlg);
	rp->update(STRTABLE(IDS_LOADREPAIRPACK));
	void *buf = load_repair(rp);
	if (buf) {
		int gl = detect_game_locale();
		if (gl < 0) {
			switch (GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_SELECTLOCALE), STRTABLE(IDS_SELECTLOCALE_TITLE), MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON3)) {
			case IDYES: gl = 0; break;
			case IDNO: gl = 1; break;
			default: gl = -1; break;
			}
		}
		if (gl >= 0) {
			BufferReader r(buf);
			run_repair(rp, r, gl);
		}
	} else {
		if (!rp->cancelled()) GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_BADREPAIRPACK), STRTABLE(IDS_REPAIRGAMEDATA), MB_ICONERROR);
	}
	free(buf);
	DestroyPleaseWaitDlg(dlg);
	delete rp;
}
