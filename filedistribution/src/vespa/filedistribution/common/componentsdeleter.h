// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <map>
#include <typeinfo>
#include <string>
#include <mutex>
#include <thread>

#include <boost/function.hpp>
#include <boost/checked_delete.hpp>
#include <boost/smart_ptr.hpp>

#include "concurrentqueue.h"

namespace filedistribution {
/**
 * Ensures that components are deleted in a separate thread,
 * and that their lifetime is tracked.
 *
 * This prevents situations as e.g. deleting ZKFacade from a zookeeper watcher thread.
 */
class ComponentsDeleter {
    class Worker;
    typedef std::lock_guard<std::mutex> LockGuard;

    std::mutex _trackedComponentsMutex;
    typedef std::map<void*, std::string> TrackedComponentsMap;
    TrackedComponentsMap _trackedComponents;

    typedef boost::function<void (void)> CallDeleteFun;
    ConcurrentQueue<CallDeleteFun> _deleteRequests;
    bool _closed;
    std::thread _deleterThread;

    void removeFromTrackedComponents(void* component);

    template <class T>
    void deleteComponent(T* component) {
        removeFromTrackedComponents(component);
        delete component;
    }

    template <class T>
    void requestDelete(T* component) {
        _deleteRequests.push(std::bind(&ComponentsDeleter::deleteComponent<T>, this, component));
    }

    void waitForAllComponentsDeleted();
    bool allComponentsDeleted();
    void logNotDeletedComponents();
    void close();
  public:
    ComponentsDeleter(const ComponentsDeleter &) = delete;
    ComponentsDeleter & operator = (const ComponentsDeleter &) = delete;
    ComponentsDeleter();

    /*
     *  Waits blocking for up to 60 seconds until all components are deleted.
     *  If it fails, the application is killed.
     */
    ~ComponentsDeleter();

    template <class T>
    boost::shared_ptr<T> track_boost(T* t) {
        LockGuard guard(_trackedComponentsMutex);
        if (_closed) {
            return boost::shared_ptr<T>(t);
        }

        _trackedComponents[t] = typeid(t).name();
        return boost::shared_ptr<T>(t, std::bind(&ComponentsDeleter::requestDelete<T>, this, t));
    }

    template <class T>
    std::shared_ptr<T> track(T* t) {
        LockGuard guard(_trackedComponentsMutex);
        if (_closed) {
            return std::shared_ptr<T>(t);
        }

        _trackedComponents[t] = typeid(t).name();
        return std::shared_ptr<T>(t, std::bind(&ComponentsDeleter::requestDelete<T>, this, t));
    }
};
}

