/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK USRP_UHD.
 *
 * REDHAWK USRP_UHD is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK USRP_UHD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef __RH_BLOCKINGREADFIFO_H__
#define __RH_BLOCKINGREADFIFO_H__

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/call_traits.hpp>
#include <boost/bind.hpp>
#include <deque>

template<typename VAL_T>
class BlockingReadFifo {
public:

    BlockingReadFifo() : m_interrupted(false) {
    }

    void push(VAL_T& _data) {
        boost::mutex::scoped_lock lock(m_mutex);
        m_buf.push_back(_data);
        lock.unlock();
        m_not_empty.notify_one();
    }

    //blocking read - blocks when empty
    VAL_T& front() {
        boost::mutex::scoped_lock lock(m_mutex);
        m_not_empty.wait(lock,
                boost::bind(&BlockingReadFifo<VAL_T>::is_not_empty, this));
        if(m_buf.empty())
            throw std::runtime_error("Error: called front() on empty sequence.");
        return m_buf.front();
    }

    //blocking read - blocks when empty
    VAL_T& back() {
        boost::mutex::scoped_lock lock(m_mutex);
        m_not_empty.wait(lock,
                boost::bind(&BlockingReadFifo<VAL_T>::is_not_empty, this));
        if(m_buf.empty())
            throw std::runtime_error("Error: called back() on empty sequence.");
        return m_buf.back();
    }

    //blocking remove - blocks when empty
    void pop() {
        boost::mutex::scoped_lock lock(m_mutex);
        m_not_empty.wait(lock,
                boost::bind(&BlockingReadFifo<VAL_T>::is_not_empty, this));
        if(!m_buf.empty()) // need to check because could have been m_interrupted
            m_buf.pop_front();
    }

    //blocking remove - blocks when empty
    bool pop(VAL_T& _val) {
        boost::mutex::scoped_lock lock(m_mutex);
        m_not_empty.wait(lock,
                boost::bind(&BlockingReadFifo<VAL_T>::is_not_empty, this));
        if(!m_buf.empty()) { // need to check because could have been m_interrupted
            _val = m_buf.front();
            m_buf.pop_front();
            return true;
        }
        return false;
    }

    //non-blocking remove
    bool trypop(VAL_T& _val) {
        boost::mutex::scoped_lock lock(m_mutex);
        if(!m_buf.empty()) {
            _val = m_buf.front();
            m_buf.pop_front();
            return true;
        }
        return false;
    }

    //non-blocking clear
    void clear() {
        boost::mutex::scoped_lock lock(m_mutex);
        m_buf.clear();
    }

    size_t size() {
        boost::mutex::scoped_lock lock(m_mutex);
        return m_buf.size();
    }

    bool empty() {
        boost::mutex::scoped_lock lock(m_mutex);
        return m_buf.empty();
    }

    void interrupt() {
        m_interrupted = true;
        m_not_empty.notify_all();
    }

    void resetinterrupt() {
        m_interrupted = false;
    }

    /*
    // should call lock() prior to calling this
    // and of course unlock() when done
    bool unsafe_empty() {
        return m_buf.empty();
    }

    void lock() {
        m_mutex.lock();
    }

    void unlock() {
        m_mutex.unlock();
    }
    */

private:
    BlockingReadFifo(const BlockingReadFifo&);             // Disabled copy constructor.
    BlockingReadFifo& operator =(const BlockingReadFifo&); // Disabled assign operator.

    bool is_not_empty() const {
        return !m_buf.empty() || m_interrupted;
    }

    std::deque<VAL_T> m_buf;
    bool m_interrupted;

    boost::mutex m_mutex;
    boost::condition m_not_empty;

};

#endif /* __RH_BLOCKINGREADFIFO_H__ */

