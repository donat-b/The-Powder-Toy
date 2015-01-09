#include "DownloadManager.h"
#include "Download.h"
#include "http.h"
#include "defines.h"

int DownloadManager::nextDownloadID = 0;

DownloadManager::DownloadManager():
	lastUsed(time(NULL)),
	managerRunning(false),
	managerShutdown(false),
	downloads(NULL)
{
	pthread_mutex_init(&downloadLock, NULL);
}

DownloadManager::~DownloadManager()
{

}

void DownloadManager::Shutdown()
{
	pthread_mutex_lock(&downloadLock);
	for (std::vector<Download*>::iterator iter = downloads.begin(); iter != downloads.end(); ++iter)
	{
		Download *download = (*iter);
		if (download->http)
			http_force_close(download->http);
		download->Cancel();
		delete download;
	}
	downloads.clear();
	managerShutdown = true;
	pthread_mutex_unlock(&downloadLock);
	pthread_join(downloadThread, NULL);
}

//helper function for download
TH_ENTRY_POINT void* DownloadManagerHelper(void* obj)
{
	DownloadManager *temp = (DownloadManager*)obj;
	temp->Update();
}

void DownloadManager::Start()
{
	managerRunning = true;
	lastUsed = time(NULL);
	pthread_create(&downloadThread, NULL, &DownloadManagerHelper, this);
}

void DownloadManager::Update()
{
	while (!managerShutdown)
	{
		if (downloads.size())
		{
			pthread_mutex_lock(&downloadLock);
			for (std::vector<Download*>::iterator iter = downloads.begin(); iter != downloads.end(); ++iter)
			{
				Download *download = (*iter);
				if (download->CheckCanceled())
				{
					if (download->http && download->keepAlive && download->CheckStarted())
						http_force_close(download->http);
					delete download;
					downloads.erase(iter);
					iter--;
				}
				else if (download->CheckStarted() && !download->CheckDone())
				{
					if (http_async_req_status(download->http) != 0)
					{
						download->downloadData = http_async_req_stop(download->http, &download->downloadStatus, &download->downloadSize);
						download->downloadFinished = true;
						if (!download->keepAlive)
							download->http = NULL;
					}
					lastUsed = time(NULL);
				}
			}
			pthread_mutex_unlock(&downloadLock);
		}
		if (time(NULL) > lastUsed+HTTP_TIMEOUT*2)
		{
			pthread_mutex_lock(&downloadLock);
			managerRunning = false;
			pthread_mutex_unlock(&downloadLock);
			return;
		}
		millisleep(10);
	}
}

int DownloadManager::AddDownload(Download *download)
{
	pthread_mutex_lock(&downloadLock);
	int id = nextDownloadID++;
	downloads.push_back(download);
	if (!managerRunning)
	{
		if (nextDownloadID)
			pthread_join(downloadThread, NULL);
		Start();
	}
	pthread_mutex_unlock(&downloadLock);
}
