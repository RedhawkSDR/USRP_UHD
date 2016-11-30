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
#ifndef CUSTOMSTRUCTS_H_
#define CUSTOMSTRUCTS_H_

#include <bulkio/bulkio.h>
#include <vector>

template <typename DATA_TYPE>
struct inputMetadata {
public:
    inputMetadata() {
        clear();
    }
    void clear() {
        m_num_samples = m_num_samples_ = 0;
        bulkio::sri::zeroTime(m_timestamp);
        bulkio::sri::zeroTime(m_timestamp_);
        m_EOS = false;
        m_sri_changed = false;
        bulkio::sri::zeroSRI(m_sri);
        m_data = 0;
    }

    // used for new block with new sri
    void set(size_t samps, const BULKIO::PrecisionUTCTime& ts, bool eos, const BULKIO::StreamSRI& sri) {
        m_num_samples = m_num_samples_ = samps;
        m_timestamp = m_timestamp_ = ts;
        m_EOS = eos;
        m_sri_changed = true;
        m_sri = sri;
        m_data = 0; // this must be explicitly set.
    }

    // used for new block with same sri
    void update(size_t samps, const BULKIO::PrecisionUTCTime& ts, bool eos) {
        m_num_samples = m_num_samples_ = samps;
        m_timestamp = m_timestamp_ = ts;
        m_EOS = eos;
        m_sri_changed = false;
        // keep m_sri unchanged.
        m_data = 0; // this must be explicitly set.
    }

    // used to combine two consecutive blocks
    size_t add(size_t samps, bool eos) {
        if (m_EOS) return 0;
        m_num_samples += samps;
        m_num_samples_ += samps;
        m_EOS = eos;
        return samps;
    }

    // used to combine two consecutive blocks
    size_t add(const inputMetadata& block) {
        return add(block.size(), block.eos());
    }

    // used to set address of start of data when reading data
    void set(DATA_TYPE* data) {
        // all metadata remains unchanged.
        m_data = data;
    }

    // used to update after consuming data
    void consume(size_t samps) {
        m_num_samples -= samps;
        m_timestamp = bulkio::time::utils::addSampleOffset(m_timestamp_, (m_num_samples_-m_num_samples)/(m_sri.mode+1), m_sri.xdelta);
        m_num_samples==0 ? m_data=0 : m_data+=samps;
        m_sri_changed = false;
    }

    // used to update after consuming data
    void consume() {
        consume(m_num_samples);
    }

    // used to get number of samples consumed from data block
    size_t total_consumed() const {
        return m_num_samples_-m_num_samples;
    }

    // used to get remaining number of samples
    size_t size() const {
        return m_num_samples;
    }

    // used to get timestamp of next remaining sample
    BULKIO::PrecisionUTCTime timestamp() const {
        return m_timestamp;
    }

    bool eos() const {
        return m_EOS;
    }

    bool sri_changed() const {
        return m_sri_changed;
    }

    void sri_changed(bool sri_changed) {
        m_sri_changed = sri_changed;
    }

    BULKIO::StreamSRI sri() const {
        return m_sri;
    }

    // used to get original number of samples associated with data block
    size_t total_samples() const {
        return m_num_samples_;
    }

    DATA_TYPE* data() const {
        return m_data;
    }

    DATA_TYPE* data() {
        return m_data;
    }

private:
    // current values
    size_t m_num_samples;
    BULKIO::PrecisionUTCTime m_timestamp;
    bool m_EOS;
    bool m_sri_changed;
    BULKIO::StreamSRI m_sri;
    DATA_TYPE* m_data;

    // original values
    size_t m_num_samples_;
    BULKIO::PrecisionUTCTime m_timestamp_;

};

#endif // CUSTOMSTRUCTS_H_
