#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H
#include <pthread.h>
#include <time.h>
#include <vector>
#include "Singleton.h"

class Download;
class DownloadManager : public Singleton<DownloadManager>
{
private:
	pthread_t downloadThread;
	pthread_mutex_t downloadLock;
	bool threadStarted;

	int lastUsed;
	volatile bool managerRunning;
	volatile bool managerShutdown;
	std::vector<Download*> downloads;

	void Start();
public:
	DownloadManager();
	~DownloadManager();

	void Shutdown();
	void Update();
	void EnsureRunning();

	void AddDownload(Download *download);
	void RemoveDownload(int id);
};

#endif // DOWNLOADMANAGER_H
