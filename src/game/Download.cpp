#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include "defines.h"
#include "Download.h"
#include "http.h"

Download::Download(std::string uri_, bool keepAlive):
	keepAlive(keepAlive),
	http(NULL),
	lastUse(HTTP_TIMEOUT),
	downloadFinished(false),
	downloadCanceled(false)
{
	uri = std::string(uri_);
	pthread_mutex_init(&downloadLock, NULL);
}

//called by download thread itself if download was canceled
Download::~Download()
{
	if (!downloadFinished)
		std::cout << "Warning: download deleted but not finished";
	if (downloadCanceled && downloadData)
		free(downloadData);
}

//helper function for download
TH_ENTRY_POINT void* DownloadHelper(void* obj)
{
	Download *temp = (Download*)obj;
	temp->DoDownload();
	//download was canceled while running, delete the Download* and data
	if (temp->CheckCanceled())
		delete temp;
}

//internal function used for actual download (don't use)
void Download::DoDownload()
{
	while (true)
	{
		pthread_mutex_lock(&downloadLock);
		if (http_async_req_status(http) != 0)
		{
			downloadFinished = true;
			lastUse = time(NULL);
			downloadData = http_async_req_stop(http, &downloadStatus, &downloadSize);
			http_async_req_close(http);
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

//finish the download (only call after CheckDone() returns true, or it will block)
char* Download::Finish(int *length, int *status)
{
	if (downloadCanceled)
		return NULL; //shouldn't happen but just in case
	int ret = pthread_join(downloadThread, NULL);
	if (ret)
		std::cout << "Warning: joining download thread failed with error code " << ret;
	if (length)
		*length = downloadSize;
	if (status)
		*status = downloadStatus;
	return downloadData;
}

//returns the download size and progress (if the download has the correct length headers)
void Download::CheckProgress(int *total, int *done)
{
	pthread_mutex_lock(&downloadLock);
	if (!downloadFinished && http)
		http_async_get_length(http, total, done);
	else
		*total = *done = 0;
	pthread_mutex_unlock(&downloadLock);
}

//returns true if the download has finished
bool Download::CheckDone()
{
	bool ret;
	pthread_mutex_lock(&downloadLock);
	ret = downloadFinished;
	pthread_mutex_unlock(&downloadLock);
	return ret;
}

//returns true if the download was canceled
bool Download::CheckCanceled()
{
	bool ret;
	pthread_mutex_lock(&downloadLock);
	ret = downloadCanceled;
	pthread_mutex_unlock(&downloadLock);
	return ret;
}

//calcels the download, the download thread will delete the Download* when it finishes
void Download::Cancel()
{
	pthread_mutex_lock(&downloadLock);
	//Download thread will delete the download object when it finishes
	pthread_detach(downloadThread);
	lastUse = time(NULL);
	downloadCanceled = true;
	pthread_mutex_unlock(&downloadLock);
}
