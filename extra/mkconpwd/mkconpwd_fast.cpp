#include <vector>
#include <queue>
#include <cassert>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

// gbCrc32 definition
class gbCrc32 {
public:
	static unsigned CrcTbl[0x100];
	static void gbCrc32Init();
	static unsigned gbCrc32Compute(const char *str, int size);
	static unsigned gbCrc32Compute(const char *str);
};


// gbCrc32 implementation
unsigned gbCrc32::CrcTbl[0x100];
void gbCrc32::gbCrc32Init()
{
	unsigned v0 = 0;
	unsigned *v1 = CrcTbl;
	unsigned v2;
	unsigned v3;
	do {
		v2 = 8;
		v3 = v0 << 24;
		do {
			if (v3 & 0x80000000) {
				v3 = 2 * v3 ^ 0x04C11DB7;
			} else {
				v3 *= 2;
			}
			--v2;
		} while (v2);
		*v1 = v3;
		++v1;
		++v0;
	} while (v1 < CrcTbl + 0x100);
}
unsigned gbCrc32::gbCrc32Compute(const char *str, int size)
{
	assert(CrcTbl[0x100 - 1]);
	unsigned char *s = (unsigned char *) str;
	unsigned x = 0;
	int i;
	for (i = 0; i < 4; i++) {
		x = (x << 8);
		if (i < size) x |= *s++;
	}
	x = ~x;
	size -= 4;
	while (size-- > 0) {
		x = CrcTbl[x >> 24] ^ (*s++ | (x << 8));
	}
	return ~x;
}
unsigned gbCrc32::gbCrc32Compute(const char *str)
{
	assert(CrcTbl[0x100 - 1]);
	unsigned char *s = (unsigned char *) str;
	unsigned x = 0;
	int i;
	for (i = 0; i < 4; i++) {
		x = (x << 8);
		if (*s) x |= *s++;
	}
	x = ~x;
	while (*s) {
		x = CrcTbl[x >> 24] ^ (*s++ | (x << 8));
	}
	return ~x;
}




// ArrayBlockingQueue
template<class T>
class BlockingQueue {
	std::queue<T> q;
	std::mutex mtx;
	std::condition_variable cv_full, cv_empty;
	size_t max_size;

public:
	BlockingQueue(size_t max_size) : max_size(max_size)
	{
		assert(max_size > 0);
	}
	void push(const T &in_val)
	{
		{
			std::unique_lock<std::mutex> lck(mtx);
			while (q.size() >= max_size) {
				cv_empty.wait(lck);
			}
			q.push(in_val);
		}
		cv_full.notify_one();
	}
	void pop(T &out_val)
	{
		{
			std::unique_lock<std::mutex> lck(mtx);
			while (q.empty()) {
				cv_full.wait(lck);
			}
			out_val = q.front(); q.pop();
		}
		cv_empty.notify_one();
	}
};




class MkConPwd {
public:
	static const int MAXLEN = 50;
	static const int MAXSIGMA = 50;
	static const char LOGFILE[];

private:
	unsigned target;
	const char *sigma;
	int maxlen;
	int sz;
	
	struct Job {
		int len;
		std::string base;
		Job() : len(0) {}
		Job(int len, std::string base) : len(len), base(base) {}
	};

	BlockingQueue<Job> *q;
	std::mutex fmtx;

private:
	void reportfound(const char *str, int wrkid);
	void searchkrnl(const char *base, int len, int wrkid);
	void worker(int wrkid);
public:
	void run(unsigned target, const char *sigma, int maxlen, int workerlen, int maxworkers);
};

const char MkConPwd::LOGFILE[] = "conpwd.txt";

void MkConPwd::reportfound(const char *str, int wrkid)
{
	printf("FOUND: worker %d found %s\n", wrkid, str);
	std::unique_lock<std::mutex> lck(fmtx);
	FILE *fp = fopen(LOGFILE, "a");
	fprintf(fp, "%s\n", str);
	fclose(fp);
}
void MkConPwd::searchkrnl(const char *base, int len, int wrkid)
{
	//printf("krnl(%s, %d, %d)\n", base, len, wrkid);
	unsigned char sigma[MAXSIGMA + 1];
	strcpy((char *) sigma, this->sigma);
	int a[MAXLEN];
	unsigned s[MAXLEN + 1];
	int bl = strlen(base);
	assert(bl >= 4);
	int l = len;
	int z = 0, f = -1;
	
	s[0] = ~gbCrc32::gbCrc32Compute(base);
	unsigned ntarget = ~target;

	do if (++f >= sz || z >= l)	{
		f = a[--z];
	} else {
		s[z + 1] = gbCrc32::CrcTbl[s[z] >> 24] ^ (sigma[f] | (s[z] << 8));
		a[z++] = f;
		f = -1;
		if (z >= l) {
			if (s[z] == ntarget) {
				char b[MAXLEN + 1];
				strcpy(b, base);
				for (int i = 0; i < z; i++) b[bl + i] = sigma[a[i]];
				b[bl + z] = 0;
				reportfound(b, wrkid);
			}
		}
	} while (z >= 0);
}
void MkConPwd::worker(int wrkid)
{
	printf("INFO: worker %d initialized.\n", wrkid);
	while (1) {
		Job j; q->pop(j); 
		if (j.len <= 0) break;

		printf("INFO: worker %d got job (len=%d, base='%s')\n", wrkid, j.len, j.base.c_str());
		
		int bl = j.base.length();
		int l = j.len;
		int z = 0, f = -1;
		int a[4];
		char s[5];
		if (bl + l <= 4) {
			strcpy(s, j.base.c_str());
			s[bl + l] = 0;
			do if (++f >= sz || z >= l)	{
				f = a[--z];
			} else {
				s[bl + z] = sigma[f];
				a[z++] = f;
				f = -1;
				if (z >= l) {
					//printf("%s\n", s);
					if (gbCrc32::gbCrc32Compute(s) == target) {
						reportfound(s, wrkid);
					}
				}
			} while (z >= 0);
		} else {
			if (bl >= 4) {
				searchkrnl(j.base.c_str(), j.len, wrkid);
			} else {
				l = 4 - bl;
				strcpy(s, j.base.c_str());
				s[4] = 0;
				do if (++f >= sz || z >= l)	{
					f = a[--z];
				} else {
					s[bl + z] = sigma[f];
					a[z++] = f;
					f = -1;
					if (z >= l) {
						searchkrnl(s, j.len - l, wrkid);
					}
				} while (z >= 0);
			}
		}
	}
	printf("INFO: worker %d exited.\n", wrkid);
}

void MkConPwd::run(unsigned target, const char *sigma, int maxlen, int workerlen, int workers)
{
	this->target = target;
	this->sigma = sigma;
	this->maxlen = maxlen;
	sz = strlen(sigma);
	
	assert(maxlen <= MAXLEN);
	assert(sz <= MAXSIGMA);

	printf("INFO: MkConPwd::MAXLEN = %d\n", MkConPwd::MAXLEN);
	printf("INFO: MkConPwd::MAXSIGMA = %d\n", MkConPwd::MAXLEN);
	printf("INFO: MkConPwd::LOGFILE = %s\n", MkConPwd::LOGFILE);

	printf("INFO: target=%08x, sigma='%s'.\n", target, sigma);
	printf("INFO: maxlen=%d, workerlen=%d, workers=%d\n", maxlen, workerlen, workers);
	printf("INFO: search started ...\n");

	FILE *fp = fopen(LOGFILE, "a");
	time_t rawtime;
	time(&rawtime);
	fprintf(fp, "\n; %s\n\n", ctime(&rawtime));
	fclose(fp);

	clock_t st = clock();

	std::vector<std::thread> wrk;
	q = new BlockingQueue<Job>(workers);
	for (int i = 0; i < workers; i++) wrk.push_back(std::thread(&MkConPwd::worker, this, i));
	for (int curlen = 1; curlen <= maxlen; curlen++) {
		if (curlen <= workerlen) {
			q->push(Job(curlen, ""));
		} else {
			int l = curlen - workerlen;
			int z = 0, f = -1;
			int a[MAXLEN];
			char s[MAXLEN + 1];
			s[l] = 0;
			do if (++f >= sz || z >= l)	{
				f = a[--z];
			} else {
				s[z] = sigma[f];
				a[z++] = f;
				f = -1;
				if (z >= l) {
					q->push(Job(workerlen, s));
				}
			} while (z >= 0);
		}
	}
	for (int i = 0; i < workers; i++) q->push(Job());
	for (int i = 0; i < workers; i++) wrk[i].join();
	delete q;

	clock_t ed = clock();
	printf("INFO: search exhausted, time = %.3fs\n", ((double) (ed - st) / CLOCKS_PER_SEC));
}

int main(int argc, char *argv[])
{
	unsigned target = 0xD5E4C8F8;
	const char *sigma = "0123456789";
	int maxlen = 10;
	int workerlen = 8;
	unsigned workers = std::thread::hardware_concurrency();
	if (argc > 1) target = strtoul(argv[1], NULL, 16); else printf("WARNING: target defaulted to %08x\n", target);
	if (argc > 2) sigma = argv[2]; else printf("WARNING: sigma defaulted to '%s'\n", sigma);
	if (argc > 3) maxlen = atoi(argv[3]); else printf("WARNING: maxlen defaulted to %d\n", maxlen);
	if (argc > 4) workerlen = atoi(argv[4]); else printf("WARNING: workerlen defaulted to %d\n", workerlen);
	if (argc > 5) workers = atoi(argv[5]); else printf("WARNING: workers defaulted to %d\n", workers);

	gbCrc32::gbCrc32Init();
	MkConPwd mkconpwd;
	mkconpwd.run(target, sigma, maxlen, workerlen, workers);
	
	return 0;
}
