// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_BROWSING_DATA_BROWSING_DATA_SERVICE_WORKER_HELPER_H_
#define CHROME_BROWSER_BROWSING_DATA_BROWSING_DATA_SERVICE_WORKER_HELPER_H_

#include <list>
#include <set>
#include <string>
#include <vector>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include "base/time/time.h"
#include "content/public/browser/service_worker_context.h"
#include "content/public/browser/service_worker_usage_info.h"
#include "url/gurl.h"

class Profile;

// BrowsingDataServiceWorkerHelper is an interface for classes dealing with
// aggregating and deleting browsing data stored for Service Workers -
// registrations, scripts, and caches.
// A client of this class need to call StartFetching from the UI thread to
// initiate the flow, and it'll be notified by the callback in its UI thread at
// some later point.
class BrowsingDataServiceWorkerHelper
    : public base::RefCountedThreadSafe<BrowsingDataServiceWorkerHelper> {
 public:
  using FetchCallback =
      base::Callback<void(const std::list<content::ServiceWorkerUsageInfo>&)>;

  // Create a BrowsingDataServiceWorkerHelper instance for the Service Workers
  // stored in |context|'s associated profile's user data directory.
  explicit BrowsingDataServiceWorkerHelper(
      content::ServiceWorkerContext* context);

  // Starts the fetching process, which will notify its completion via
  // |callback|. This must be called only in the UI thread.
  virtual void StartFetching(const FetchCallback& callback);
  // Requests the Service Worker data for an origin be deleted.
  virtual void DeleteServiceWorkers(const GURL& origin);

 protected:
  virtual ~BrowsingDataServiceWorkerHelper();

  // Owned by the profile.
  content::ServiceWorkerContext* service_worker_context_;

 private:
  friend class base::RefCountedThreadSafe<BrowsingDataServiceWorkerHelper>;

  // Enumerates all Service Worker instances on the IO thread.
  void FetchServiceWorkerUsageInfoOnIOThread(const FetchCallback& callback);

  // Deletes Service Workers for an origin the IO thread.
  void DeleteServiceWorkersOnIOThread(const GURL& origin);

  // Callback from ServiceWorkerContext::GetAllOriginsInfo()
  void GetAllOriginsInfoCallback(
      const FetchCallback& callback,
      const std::vector<content::ServiceWorkerUsageInfo>& origins);

  DISALLOW_COPY_AND_ASSIGN(BrowsingDataServiceWorkerHelper);
};

// This class is an implementation of BrowsingDataServiceWorkerHelper that does
// not fetch its information from the Service Worker context, but is passed the
// info as a parameter.
class CannedBrowsingDataServiceWorkerHelper
    : public BrowsingDataServiceWorkerHelper {
 public:
  // Contains information about a Service Worker.
  struct PendingServiceWorkerUsageInfo {
    PendingServiceWorkerUsageInfo(const GURL& origin,
                                  const std::vector<GURL>& scopes);
    ~PendingServiceWorkerUsageInfo();

    bool operator<(const PendingServiceWorkerUsageInfo& other) const;

    GURL origin;
    std::vector<GURL> scopes;
  };

  explicit CannedBrowsingDataServiceWorkerHelper(
      content::ServiceWorkerContext* context);

  // Add a Service Worker to the set of canned Service Workers that is
  // returned by this helper.
  void AddServiceWorker(const GURL& origin, const std::vector<GURL>& scopes);

  // Clear the list of canned Service Workers.
  void Reset();

  // True if no Service Workers are currently stored.
  bool empty() const;

  // Returns the number of currently stored Service Workers.
  size_t GetServiceWorkerCount() const;

  // Returns the current list of Service Workers.
  const std::set<
      CannedBrowsingDataServiceWorkerHelper::PendingServiceWorkerUsageInfo>&
      GetServiceWorkerUsageInfo() const;

  // BrowsingDataServiceWorkerHelper methods.
  void StartFetching(const FetchCallback& callback) override;
  void DeleteServiceWorkers(const GURL& origin) override;

 private:
  ~CannedBrowsingDataServiceWorkerHelper() override;

  std::set<PendingServiceWorkerUsageInfo> pending_service_worker_info_;

  DISALLOW_COPY_AND_ASSIGN(CannedBrowsingDataServiceWorkerHelper);
};

#endif  // CHROME_BROWSER_BROWSING_DATA_BROWSING_DATA_SERVICE_WORKER_HELPER_H_
