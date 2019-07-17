/*******************************************************************************
 * tlx/thread_barrier_mutex.hpp
 *
 * Part of tlx - http://panthema.net/tlx
 *
 * Copyright (C) 2015-2019 Timo Bingmann <tb@panthema.net>
 *
 * All rights reserved. Published under the Boost Software License, Version 1.0
 ******************************************************************************/

#ifndef TLX_THREAD_BARRIER_MUTEX_HEADER
#define TLX_THREAD_BARRIER_MUTEX_HEADER

#include <tlx/meta/no_operation.hpp>

#include <condition_variable>
#include <mutex>

namespace tlx {

/*!
 * Implements a thread barrier using mutex locking and condition variables that
 * can be used to synchronize threads.
 */
class ThreadBarrierMutex
{
public:
    /*!
     * Creates a new barrier that waits for n threads.
     */
    explicit ThreadBarrierMutex(size_t thread_count)
        : thread_count_(thread_count) { }

    /*!
     * Waits for n threads to arrive.
     *
     * This method blocks and returns as soon as n threads are waiting inside
     * the method. Prior to continuing work, the lambda functor is called by the
     * last thread entering the barrier.
     */
    template <typename Lambda = NoOperation<void> >
    void wait(Lambda lambda = Lambda()) {
        std::unique_lock<std::mutex> lock(mutex_);

        size_t local_ = step_;
        counts_[local_]++;

        if (counts_[local_] < thread_count_) {
            while (counts_[local_] < thread_count_) {
                cv_.wait(lock);
            }
        }
        else {
            // last thread has reached the barrier
            step_ = step_ ? 0 : 1;
            counts_[step_] = 0;
            lambda();
            cv_.notify_all();
        }
    }

    //! return generation step bit: 0 or 1
    size_t step() const {
        return step_;
    }

    //! return next generation step bit: 0 or 1
    size_t next_step() const {
        return step_ ? 0 : 1;
    }

protected:
    //! number of threads
    const size_t thread_count_;

    //! mutex to synchronize access to the counters
    std::mutex mutex_;

    //! condition variable everyone waits on for the last thread to signal
    std::condition_variable cv_;

    //! two counters: switch between them every run.
    size_t counts_[2] = { 0, 0 };

    //! current counter used.
    size_t step_ = 0;
};

} // namespace tlx

#endif // !TLX_THREAD_BARRIER_MUTEX_HEADER

/******************************************************************************/
