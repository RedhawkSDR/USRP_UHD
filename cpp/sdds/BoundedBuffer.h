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
#ifndef __RH_BOUNDEDBUFFER_H__
#define __RH_BOUNDEDBUFFER_H__

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/call_traits.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <cstring>
#include <iostream>

// Modified from: http://www.boost.org/doc/libs/1_60_0/libs/circular_buffer/test/bounded_buffer_comparison.cpp

template<class T>
class BoundedBuffer {
public:

    // This circular buffer can guarantee access to elements in contiguous memory for the maximum read size specified.
    // To disable this feature, set max_read to 1 (a read of a single element is always contiguous)
    // To enable contiguous read of the entire contents of the buffer, set max_read to 0
    // The default is to disable this feature (max_read = 1)
    // This feature is only necessary if you plan to access elements using the reference returned by front(),
    // and has no impact, positive or negative, when using other methods (except for increased memory footprint).
    BoundedBuffer(size_t buffer_capacity, size_t max_read=1) :
            buf(buffer_capacity+(max_read==0? buffer_capacity-1 : max_read-1)), buf_capacity(buffer_capacity), maximum_read((max_read==0? 1 : max_read)), buf_size(0), read_ptr(
                    0), write_ptr(0) {
        /* buf is sized to be the capacity requested plus just enough extra memory to support the
         * contiguous memory requirement of max_read. Since there will always be at least a single
         * sample in the main buffer memory, the additional capacity is one less than max_read. In
         * the case of max_read = 0, that's used to essentially double the buffer for the maximum
         * amount of contiguous memory.
         */
    }

    //blocking write - blocks when full
    size_t write(const T* data, size_t size) {
        if (size == 0)
            return 0;
        boost::mutex::scoped_lock lock(m_mutex);
        m_not_full.wait(lock,
                boost::bind(&BoundedBuffer<T>::is_not_full, this));
        size = std::min(size, buf_capacity - buf_size);
        const size_t size_write1 = std::min(size, buf_capacity - write_ptr);
        memcpy(&buf[write_ptr], data, size_write1 * sizeof(T));
        memcpy(&buf[0], data + size_write1, (size - size_write1) * sizeof(T));
        update(write_ptr, size);
        buf_size += size;
        lock.unlock();
        m_not_empty.notify_one();
        return size;
    }

    //blocking read - blocks when empty
    size_t read(T* data, size_t size) {
        if (size == 0)
            return 0;
        boost::mutex::scoped_lock lock(m_mutex);
        m_not_empty.wait(lock,
                boost::bind(&BoundedBuffer<T>::is_not_empty, this));
        size = std::min(size, buf_size);
        const size_t size_read1 = std::min(size, buf_capacity - read_ptr);
        memcpy(data, &buf[read_ptr], size_read1 * sizeof(T));
        memcpy(data + size_read1, &buf[0], (size - size_read1) * sizeof(T));
        update(read_ptr, size);
        buf_size -= size;
        lock.unlock();
        m_not_full.notify_one();
        return size;
    }

    //non-blocking write
    size_t trywrite(const T* data, size_t size) {
        if (size == 0)
            return 0;
        boost::mutex::scoped_lock lock(m_mutex);
        if (is_full())
            return 0;
        size = std::min(size, buf_capacity - buf_size);
        const size_t size_write1 = std::min(size, buf_capacity - write_ptr);
        memcpy(&buf[write_ptr], data, size_write1 * sizeof(T));
        memcpy(&buf[0], data + size_write1, (size - size_write1) * sizeof(T));
        update(write_ptr, size);
        buf_size += size;
        lock.unlock();
        m_not_empty.notify_one();
        return size;
    }

    //non-blocking read
    size_t tryread(T* data, size_t size) {
        if (size == 0)
            return 0;
        boost::mutex::scoped_lock lock(m_mutex);
        if (is_empty())
            return 0;
        size = std::min(size, buf_size);
        const size_t size_read1 = std::min(size, buf_capacity - read_ptr);
        memcpy(data, &buf[read_ptr], size_read1 * sizeof(T));
        memcpy(data + size_read1, &buf[0], (size - size_read1) * sizeof(T));
        update(read_ptr, size);
        buf_size -= size;
        lock.unlock();
        m_not_full.notify_one();
        return size;
    }

    //blocking read - blocks when empty
    // to allow direct memory access, this function ensures the maximum read
    // of the buffer can be accessed in contiguous memory from read_ptr to
    // read_ptr+max_read-1
    // This version uses the minimum of maximum_read and custom_max
    T& front(size_t custom_max) {
        custom_max = std::min(custom_max, maximum_read);
        boost::mutex::scoped_lock lock(m_mutex);
        m_not_empty.wait(lock,
                boost::bind(&BoundedBuffer<T>::is_not_empty, this));
        if(buf.size() < custom_max)
            throw std::runtime_error("Error: called front() on empty sequence with fewer than requested elements.");
        if ( read_ptr > write_ptr && read_ptr > (buf_capacity-custom_max)) {
            // need to make copy to avoid needing to wrap around
            size_t copy_size = std::min(write_ptr,read_ptr+custom_max-buf_capacity);
            memcpy(&buf[buf_capacity], &buf[0], copy_size * sizeof(T));
        }
        return buf.at(read_ptr);
    }

    //blocking read - blocks when empty
    // no guarantee of contiguous reads
    T& front() {
        return front(1);
    }

    // blocking skip - Skip data that's already been read using front() ref
    size_t skip(size_t size) {
        if (size == 0)
            return 0;
        boost::mutex::scoped_lock lock(m_mutex);
        m_not_empty.wait(lock,
                boost::bind(&BoundedBuffer<T>::is_not_empty, this));
        size = std::min(size, buf_size);
        update(read_ptr, size);
        buf_size -= size;
        lock.unlock();
        m_not_full.notify_one();
        return size;
    }

    size_t size() {
        boost::mutex::scoped_lock lock(m_mutex);
        return buf_size;
    }

    bool empty() {
        boost::mutex::scoped_lock lock(m_mutex);
        return buf_size == 0;
    }

    bool full() {
        boost::mutex::scoped_lock lock(m_mutex);
        return buf_size == buf_capacity;
    }

    size_t capacity() {
        boost::mutex::scoped_lock lock(m_mutex);
        return buf_capacity;
    }

    void dump() {
        boost::mutex::scoped_lock lock(m_mutex);
        if (buf_size == 0) {
            std::cout << "dump: empty!" << std::endl;
        } else if (write_ptr > read_ptr) {
            for (size_t ii = 0; ii < buf_size; ii++) {
                std::cout << "dump: bb[" << ii << "]=" << buf[read_ptr + ii]
                        << std::endl;
            }
        } else {
            size_t ii = 0;
            for (; read_ptr + ii < buf_capacity; ii++) {
                std::cout << "dump: bb[" << ii << "]=" << buf[read_ptr + ii]
                        << std::endl;
            }
            for (size_t jj = ii; jj - ii < write_ptr; jj++) {
                std::cout << "dump: bb[" << jj << "]=" << buf[jj - ii]
                        << std::endl;
            }
        }
    }

private:
    BoundedBuffer(const BoundedBuffer&);             // Disabled copy constructor.
    BoundedBuffer& operator =(const BoundedBuffer&); // Disabled assign operator.

    bool is_not_empty() const {
        return buf_size > 0;
    }

    bool is_empty() const {
        return buf_size == 0;
    }

    bool is_not_full() const {
        return buf_size < buf_capacity;
    }

    bool is_full() const {
        return buf_size >= buf_capacity;
    }

    void update(size_t& ptr, size_t size) {
        if (size >= buf_capacity - ptr)
            ptr = ptr + size - buf_capacity;
        else
            ptr = ptr + size;
    }

    std::vector<T> buf;
    size_t buf_capacity;
    size_t maximum_read;
    size_t buf_size;
    size_t read_ptr;
    size_t write_ptr;

    boost::mutex m_mutex;
    boost::condition m_not_empty;
    boost::condition m_not_full;

};

#endif /* __RH_BOUNDEDBUFFER_H__ */

