#ifndef DOWNLOAD_H
#define DOWNLOAD_H
#include <pthread.h>
#include <string>
#include <time.h>

#define HTTP_TIMEOUT 10 //normally in defines.h

class Download
{
	std::string uri;
	bool keepAlive;
	void *http;
	time_t lastUse;

	char *downloadData;
	int downloadSize;
	int downloadStatus;

	volatile bool downloadFinished;
	volatile bool downloadCanceled;
	pthread_t downloadThread;
	pthread_mutex_t downloadLock;

public:
	Download(std::string uri, bool keepAlive = false);
	~Download();

	void AuthHeaders(const char *userID, const char *session);
	void Start();
	char* Finish(int *length, int *status);
	void Cancel();

	void CheckProgress(int *total, int *done);
	bool CheckDone();
	bool CheckCanceled();

	void DoDownload();
};

#endif
