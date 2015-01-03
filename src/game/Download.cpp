#include <pthread.h>
#include <stdlib.h>
#include "defines.h"
#include "Download.h"
#include "http.h"

Download::Download(std::string uri_, bool keepAlive):
	http(NULL),
	keepAlive(keepAlive),
	lastUse(time(NULL)),
	downloadFinished(false),
	downloadCanceled(false)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&downloadLock, &attr);

	uri = std::string(uri_);
}

//called by download thread itself if download was canceled
Download::~Download()
{
	if (downloadCanceled && downloadData)
		free(downloadData);
}

//helper function for download
TH_ENTRY_POINT void* DownloadHelper(void* obj)
{
	Download *temp = (Download*)obj;
	temp->DoDownload();
	//if download was canceled while running, delete the Download* and data
	temp->CheckCanceled(true);
}

//internal function used for actual download (don't use)
void Download::DoDownload()
{
	while (true)
	{
		pthread_mutex_lock(&downloadLock);
		if (downloadCanceled) //set http to HTS_DONE
			http_force_close(http);
		if (http_async_req_status(http) != 0)
		{
			lastUse = time(NULL);
			downloadData = http_async_req_stop(http, &downloadStatus, &downloadSize);
			if (keepAlive && downloadCanceled)
				http_async_req_close(http);
			downloadFinished = true;
			pthread_mutex_unlock(&downloadLock);
			return;
		}
		pthread_mutex_unlock(&downloadLock);
		millisleep(10);
	}
}

//add userID and sessionID headers to the download. Must be done after download starts for some reason
void Download::AuthHeaders(const char *userID, const char *session)
{
	http_auth_headers(http, userID, NULL, session);
}

//start the download thread
void Download::Start()
{
	http = http_async_req_start(http, uri.c_str(), NULL, 0, keepAlive ? 1 : 0);
	lastUse = time(NULL);
	pthread_create(&downloadThread, NULL, &DownloadHelper, this);
}

//for persistent connections (keepAlive = true), reuse the open connection to make another request
bool Download::Reuse(std::string newuri)
{
	pthread_mutex_lock(&downloadLock);
	if (!keepAlive || !CheckDone() || CheckCanceled())
	{
		pthread_mutex_unlock(&downloadLock);
		return false;
	}
	//timeout, start a new request
	if (time(NULL) > lastUse+HTTP_TIMEOUT)
	{
		http_async_req_close(http);
		http = NULL;
	}
	uri = std::string(newuri);
	downloadFinished = false;
	Start();
	pthread_mutex_unlock(&downloadLock);
	return true;
}

//finish the download (only call after CheckDone() returns true, or it will block)
char* Download::Finish(int *length, int *status)
{
	if (CheckCanceled())
		return NULL; //shouldn't happen but just in case
	pthread_join(downloadThread, NULL);
	if (length)
		*length = downloadSize;
	if (status)
		*status = downloadStatus;
	char *ret = downloadData;
	downloadData = NULL;
	return ret;
}

//returns the download size and progress (if the download has the correct length headers)
void Download::CheckProgress(int *total, int *done)
{
	pthread_mutex_lock(&downloadLock);
	if (!CheckDone() && http)
		http_async_get_length(http, total, done);
	else
		*total = *done = 0;
	pthread_mutex_unlock(&downloadLock);
}

//returns true if the download has finished
bool Download::CheckDone()
{
	/*bool ret;
	pthread_mutex_lock(&downloadLock);
	ret = downloadFinished;
	pthread_mutex_unlock(&downloadLock);
	return ret;*/
	return downloadFinished; //mutex lock probably not needed here ...
}

//returns true if the download was canceled
bool Download::CheckCanceled(bool del)
{
	bool ret;
	pthread_mutex_lock(&downloadLock);
	ret = downloadCanceled;
	if (del && ret)
	{
		delete this;
		return ret;
	}
	pthread_mutex_unlock(&downloadLock);
	return ret;
}

//calcels the download, the download thread will delete the Download* when it finishes (do not use Download in any way after canceling)
void Download::Cancel()
{
	pthread_mutex_lock(&downloadLock);
	//Download thread will delete the download object when it finishes
	pthread_detach(downloadThread);
	downloadCanceled = true;
	if (keepAlive && CheckDone())
	{
		http_async_req_close(http);
		delete this;
		return;
	}
	pthread_mutex_unlock(&downloadLock);
}
