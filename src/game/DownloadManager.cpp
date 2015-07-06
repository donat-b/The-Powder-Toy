#include "DownloadManager.h"
#include "Download.h"
#include "http.h"
#include "defines.h"

DownloadManager::DownloadManager():
	threadStarted(false),
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
	return NULL;
}

void DownloadManager::Start()
{
	managerRunning = true;
	lastUsed = time(NULL);
	pthread_create(&downloadThread, NULL, &DownloadManagerHelper, this);
}

void DownloadManager::Update()
{
	unsigned int numActiveDownloads;
	while (!managerShutdown)
	{
		if (downloads.size())
		{
			numActiveDownloads = 0;
			pthread_mutex_lock(&downloadLock);
			for (size_t i = 0; i < downloads.size(); i++)
			{
				Download *download = downloads[i];
				if (download->CheckCanceled())
				{
					if (download->http && download->keepAlive && download->CheckStarted())
						http_force_close(download->http);
					delete download;
					downloads.erase(downloads.begin()+i);
					i--;
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
					numActiveDownloads++;
				}
			}
			pthread_mutex_unlock(&downloadLock);
		}
		if (time(NULL) > lastUsed+HTTP_TIMEOUT*2 && !numActiveDownloads)
		{
			pthread_mutex_lock(&downloadLock);
			managerRunning = false;
			pthread_mutex_unlock(&downloadLock);
			return;
		}
		millisleep(3);
	}
}

void DownloadManager::EnsureRunning()
{
	pthread_mutex_lock(&downloadLock);
	if (!managerRunning)
	{
		if (threadStarted)
			pthread_join(downloadThread, NULL);
		else
			threadStarted = true;
		Start();
	}
	pthread_mutex_unlock(&downloadLock);
}

void DownloadManager::AddDownload(Download *download)
{
	pthread_mutex_lock(&downloadLock);
	downloads.push_back(download);
	pthread_mutex_unlock(&downloadLock);
	EnsureRunning();
}
