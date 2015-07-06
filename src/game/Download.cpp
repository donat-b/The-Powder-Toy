#include <stdlib.h>
#include "defines.h"
#include "Download.h"
#include "DownloadManager.h"
#include "http.h"

Download::Download(std::string uri_, bool keepAlive):
	http(NULL),
	keepAlive(keepAlive),
	downloadData(NULL),
	downloadSize(0),
	downloadStatus(0),
	downloadFinished(false),
	downloadCanceled(false),
	downloadStarted(false),
	postData(""),
	postDataBoundary(""),
	userID(NULL),
	userSession(NULL)
{
	uri = std::string(uri_);
	DownloadManager::Ref().AddDownload(this);
}

// called by download thread itself if download was canceled
Download::~Download()
{
	if (http && (keepAlive || downloadCanceled))
		http_async_req_close(http);
	if (downloadData)
		free(downloadData);
}

// add post data to a request
void Download::AddPostData(std::map<std::string, std::string> data)
{
	postDataBoundary = FindBoundary(data, "");
	postData = GetMultipartMessage(data, postDataBoundary);
}
void Download::AddPostData(std::pair<std::string, std::string> data)
{
	std::map<std::string, std::string> postData;
	postData.insert(data);
	AddPostData(postData);
}

// add userID and sessionID headers to the download. Must be done after download starts for some reason
void Download::AuthHeaders(const char *ID, const char *session)
{
	userID = ID;
	userSession = session;
}

// start the download thread
void Download::Start()
{
	if (CheckStarted() || CheckDone())
		return;
	http = http_async_req_start(http, uri.c_str(), postData.c_str(), postData.length(), keepAlive ? 1 : 0);
	// add the necessary headers
	if (userID || userSession)
		http_auth_headers(http, userID, NULL, userSession);
	if (postDataBoundary.length())
		http_add_multipart_header(http, postDataBoundary);
	downloadStarted = true;
}

// for persistent connections (keepAlive = true), reuse the open connection to make another request
bool Download::Reuse(std::string newuri)
{
	if (!keepAlive || !CheckDone() || CheckCanceled())
	{
		return false;
	}
	uri = std::string(newuri);
	downloadFinished = false;
	Start();
	DownloadManager::Ref().EnsureRunning();
	return true;
}

// finish the download (if called before the download is done, this will block)
char* Download::Finish(int *length, int *status)
{
	if (CheckCanceled())
		return NULL; // shouldn't happen but just in case
	while (!CheckDone()); // block
	downloadStarted = false;
	if (length)
		*length = downloadSize;
	if (status)
		*status = downloadStatus;
	char *ret = downloadData;
	downloadData = NULL;
	if (!keepAlive)
		Cancel();
	return ret;
}

// returns the download size and progress (if the download has the correct length headers)
void Download::CheckProgress(int *total, int *done)
{
	if (!CheckDone() && http)
		http_async_get_length(http, total, done);
	else
		*total = *done = 0;
}

// returns true if the download has finished
bool Download::CheckDone()
{
	return downloadFinished;
}

// returns true if the download was canceled
bool Download::CheckCanceled()
{
	return downloadCanceled;
}

// returns true if the download is running
bool Download::CheckStarted()
{
	return downloadStarted;
}

// cancels the download, the download thread will delete the Download* when it finishes (do not use Download in any way after canceling)
void Download::Cancel()
{
	downloadCanceled = true;
}
