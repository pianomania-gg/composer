/*
MIT License

Copyright (c) 2025 Igor Korsukov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>

#include <gtest/gtest.h>

#include "../async/internal/queuepool.h"

using namespace kors::async;

struct Store {
    std::vector<Queue*> queues;
    std::vector<std::thread::id> threadIds;
    std::mutex mutex;
};

TEST(QueuePool_Tests, Force)
{
    QueuePool* qp = QueuePool::instance();
    Store store;
    std::vector<std::thread> threads;
    std::atomic<bool> running = true;
    for (size_t i = 0; i < 50; ++i) {
        auto t = std::thread([qp, &store, i, &running]() {
            {
                std::scoped_lock<std::mutex> lock(store.mutex);

                store.threadIds.push_back(std::this_thread::get_id());
                if (store.threadIds.size() == 1) {
                    return;
                }
                Queue* q = new Queue();
                store.queues.push_back(q);

                qp->regPort(store.threadIds[i-1], q->port1());
                qp->regPort(store.threadIds[i], q->port2());
            }

            while (running.load()) {
                qp->processMessages(store.threadIds[i]);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        threads.push_back(std::move(t));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    running.store(false);

    for (auto& t : threads) {
        t.join();
    }
}

TEST(QueuePool_Tests, UnregisterWaitsForActiveMessageProcessing)
{
    QueuePool* pool = QueuePool::instance();
    Queue queue;
    std::mutex mutex;
    std::condition_variable changed;
    std::thread::id receiverId;
    bool receiverReady = false;
    bool startProcessing = false;
    bool handlerEntered = false;
    bool releaseHandler = false;
    bool unregisterStarted = false;
    bool unregisterFinished = false;

    queue.port1()->onMessage([&](const CallMsg&) {
        std::unique_lock<std::mutex> lock(mutex);
        handlerEntered = true;
        changed.notify_all();
        changed.wait(lock, [&]() { return releaseHandler; });
    });

    std::thread receiver([&]() {
        const auto threadId = std::this_thread::get_id();
        pool->regPort(threadId, queue.port1());
        bool shouldProcess = false;
        {
            std::unique_lock<std::mutex> lock(mutex);
            receiverId = threadId;
            receiverReady = true;
            changed.notify_all();
            changed.wait(lock, [&]() { return startProcessing || releaseHandler; });
            shouldProcess = startProcessing && !releaseHandler;
        }

        if (shouldProcess) {
            // tryLock may fail spuriously; retry without creating another message.
            for (int attempt = 0; attempt < 128; ++attempt) {
                pool->processMessages(threadId);
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    if (handlerEntered || releaseHandler) {
                        break;
                    }
                }
                std::this_thread::yield();
            }
        }

        pool->unregPort(threadId, queue.port1());
    });

    bool receiverObserved = false;
    bool handlerObserved = false;
    {
        std::unique_lock<std::mutex> lock(mutex);
        receiverObserved = changed.wait_for(lock, std::chrono::seconds(2), [&]() { return receiverReady; });
        if (receiverObserved) {
            queue.port2()->send(CallMsg {});
            startProcessing = true;
            changed.notify_all();
            handlerObserved = changed.wait_for(lock, std::chrono::seconds(2), [&]() { return handlerEntered; });
        }
    }

    std::thread remover;
    bool unregisterObserved = false;
    bool completedBeforeRelease = false;
    if (handlerObserved) {
        remover = std::thread([&]() {
            {
                std::lock_guard<std::mutex> lock(mutex);
                unregisterStarted = true;
                changed.notify_all();
            }
            pool->unregPort(receiverId, queue.port1());
            {
                std::lock_guard<std::mutex> lock(mutex);
                unregisterFinished = true;
                changed.notify_all();
            }
        });

        std::unique_lock<std::mutex> lock(mutex);
        unregisterObserved = changed.wait_for(lock, std::chrono::seconds(2), [&]() { return unregisterStarted; });
        if (unregisterObserved) {
            completedBeforeRelease = changed.wait_for(lock, std::chrono::milliseconds(250), [&]() { return unregisterFinished; });
        }
    }

    // Release and join both threads before any assertion, including timeout failures.
    {
        std::lock_guard<std::mutex> lock(mutex);
        releaseHandler = true;
        changed.notify_all();
    }
    receiver.join();
    if (remover.joinable()) {
        remover.join();
    }

    EXPECT_TRUE(receiverObserved);
    EXPECT_TRUE(handlerObserved);
    EXPECT_TRUE(unregisterObserved);
    EXPECT_FALSE(completedBeforeRelease);
    EXPECT_TRUE(unregisterFinished);
}

namespace {
// Runs body on its own thread and reports whether it returned before the deadline.
// A body that never returns is detached, so it must own everything it touches.
bool returnsWithin(std::chrono::seconds deadline, std::function<void()> body)
{
    auto finished = std::make_shared<std::promise<void> >();
    std::future<void> done = finished->get_future();
    std::thread worker([body = std::move(body), finished]() {
        body();
        finished->set_value();
    });

    if (done.wait_for(deadline) != std::future_status::ready) {
        worker.detach();
        return false;
    }

    worker.join();
    return true;
}

struct HandlerPorts {
    Queue trigger;
    Queue other;
    std::atomic<bool> handled = false;
};
}

// A handler's first cross-thread Channel send or Async::call registers ports for the thread
// that is processing it.
TEST(QueuePool_Tests, HandlerRegistersPortForProcessingThread)
{
    auto ports = std::make_shared<HandlerPorts>();
    const bool returned = returnsWithin(std::chrono::seconds(5), [ports]() {
        QueuePool* pool = QueuePool::instance();
        const std::thread::id self = std::this_thread::get_id();
        HandlerPorts* p = ports.get();

        p->trigger.port1()->onMessage([pool, self, p](const CallMsg&) {
            pool->regPort(self, p->other.port1());
            p->handled = true;
        });

        pool->regPort(self, p->trigger.port1());
        p->trigger.port2()->send(CallMsg {});
        pool->processMessages(self);

        pool->unregPort(self, p->other.port1());
        pool->unregPort(self, p->trigger.port1());
    });

    EXPECT_TRUE(returned) << "regPort for the processing thread did not return inside a message handler";
    EXPECT_TRUE(ports->handled);
}

// A Channel destroyed inside a handler unregisters its ports for the thread that is processing it.
TEST(QueuePool_Tests, HandlerUnregistersPortForProcessingThread)
{
    auto ports = std::make_shared<HandlerPorts>();
    const bool returned = returnsWithin(std::chrono::seconds(5), [ports]() {
        QueuePool* pool = QueuePool::instance();
        const std::thread::id self = std::this_thread::get_id();
        HandlerPorts* p = ports.get();

        p->trigger.port1()->onMessage([pool, self, p](const CallMsg&) {
            pool->unregPort(self, p->other.port1());
            p->handled = true;
        });

        pool->regPort(self, p->trigger.port1());
        pool->regPort(self, p->other.port1());
        p->trigger.port2()->send(CallMsg {});
        pool->processMessages(self);

        pool->unregPort(self, p->trigger.port1());
    });

    EXPECT_TRUE(returned) << "unregPort for the processing thread did not return inside a message handler";
    EXPECT_TRUE(ports->handled);
}
