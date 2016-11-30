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
// Class that handles a SDDS packet, assuming little-endian arch (Tru64, x86 Linux)
// Defines a swap8() function, ala htonll()
// Uses class XMTime for XMidas time representation
//
// 050314 RDA Created
//

#ifndef SDDS_PACKET
#define SDDS_PACKET

// cout, endl
#include <iostream>
// hex, dec
#include <iomanip>
// floor
#include <cmath>

// ntohl() and friends
#include <netinet/in.h>

// Covers for the lack of ntohll for 8-byte values
inline uint64_t swap8(uint64_t val){
  uint8_t *v = (uint8_t *)&val;
  return
    (uint64_t(v[0])<<56) |
    (uint64_t(v[1])<<48) |
    (uint64_t(v[2])<<40) |
    (uint64_t(v[3])<<32) |
    (uint64_t(v[4])<<24) |
    (uint64_t(v[5])<<16) |
    (uint64_t(v[6])<< 8) |
    (uint64_t(v[7]));
}

// A special time class for SDDS time, based on 250 picosecond tics.
// For periodic updates, this should be more efficient and accurate
// than dealing with XMTime and the associated conversions
const double SDDSTime_tic   = 250e-12;
const double SDDSTime_two32 = 4294967296.0;  // 2**32

class SDDSTime {
 public:
  // Copy constructor
  SDDSTime (const SDDSTime& other) :
    ps250_(other.ps250_),
    pf250_(other.pf250_)
    { }

  // Assignment operator
  SDDSTime& operator= (const SDDSTime& other) {
    ps250_ = other.ps250_;
    pf250_ = other.pf250_;
    return *this;
  }

  // Empty constructor
  SDDSTime (void) {
      ps250_ = 0;
      pf250_ = 0;
  }

  // Construct a time using seconds
  SDDSTime (double s) {
    double pfrac = s*4000000000.0;
    ps250_ = uint64_t(std::floor(pfrac));
    pf250_ = uint32_t((pfrac - std::floor(pfrac))*SDDSTime_two32);
    //printf( "td: %12llu %12u %16.12lf\n", ps250_, pf250_, pfrac);
  }

  // Construct from raw values
  SDDSTime (uint64_t ps, uint32_t pf) :
    ps250_(ps),
    pf250_(pf)
    { }

  // Construct a time from integral and fractional seconds
  SDDSTime (double integral, double fractional) {
    double pfrac = fractional*4000000000.0;
    ps250_ = uint64_t(integral)*4000000000LL + uint64_t(floor(pfrac));
    pf250_ = uint32_t((pfrac - floor(pfrac))*SDDSTime_two32);
  }

  // ///// Inspectors
  uint64_t ps250 () const  { return ps250_; }
  uint32_t pf250 () const  { return pf250_; }
  double seconds () const  { return ps250_*SDDSTime_tic + pf250_*(SDDSTime_tic/SDDSTime_two32); }

  // ///// Relational operators
  bool operator< (const SDDSTime& t) const
    { return (ps250_ == t.ps250_) ?
	(pf250_ < t.pf250_) : (ps250_ < t.ps250_); }


  bool operator== (const SDDSTime& t) const
    { return ps250_ == t.ps250_  &&  pf250_ == t.pf250_; }

  // ///// Addition/Difference operators
  friend inline SDDSTime operator+ (const SDDSTime& t1, const SDDSTime& t2);
  inline SDDSTime& operator+= (const SDDSTime& t);

  friend inline SDDSTime operator- (const SDDSTime& t1, const SDDSTime& t2);
  inline SDDSTime& operator-= (const SDDSTime& t);

 private:
  uint64_t ps250_;  // 250 psec tics
  uint32_t pf250_;  // fractional 250 psec tics
};

inline SDDSTime& SDDSTime::operator+= (const SDDSTime& other) {
  uint64_t frac = uint64_t(pf250_) + other.pf250_;
  ps250_ += other.ps250_ + (frac>>32);
  pf250_ =  frac;
  return *this;
}

// only supports differences with a positive result
inline SDDSTime& SDDSTime::operator-= (const SDDSTime& other) {
  if ( pf250_ >= other.pf250_ ) {
    pf250_ -= other.pf250_;
    ps250_ -= other.ps250_;
  } else {
    pf250_ -= other.pf250_;
    ps250_ -= other.ps250_ + 1;
  }
  return *this;
}

inline SDDSTime operator+ (const SDDSTime& t1, const SDDSTime& t2) {
  SDDSTime temp(t1);
  uint64_t frac = uint64_t(temp.pf250_) + t2.pf250_;
  temp.ps250_ += t2.ps250_ + (frac>>32);
  temp.pf250_ =  frac;
  return temp;
}


inline SDDSTime operator- (const SDDSTime& t1, const SDDSTime& t2) {
  SDDSTime temp(t1);
  if ( temp.pf250_ >= t2.pf250_ ) {
    temp.pf250_ -= t2.pf250_;
    temp.ps250_ -= t2.ps250_;
  } else {
    temp.pf250_ -= t2.pf250_;
    temp.ps250_ -= t2.ps250_ + 1;
  }
  return temp;
}



// Assume we're little endian (x86, Tru64) TODO - why? what part of this file assumes this?
const int SDDS_psize = 1080;
const int SDDS_payload_size = 1024;
class SDDSpacket {
 public:
  //======================== SDDS header (raw network order)
  // Format Identifier
  unsigned dmode:3;
  unsigned ss:1;  //SRI eventually (property for now)
  unsigned of:1;  //property default = 0
  unsigned pp:1;  //0 hard coded
  unsigned sos:1;
  unsigned sf:1;  //property forever default 1

  unsigned bps:5;  //SRI eventually (property for now)
  //unsigned unused:2;
  unsigned vw:1;  // very wideband - indicates that the 'freq' value is actually x16 what is stored.
  unsigned snp:1; // snapshot data - according to Bob Arnt (sp?), this is a flag that indicates that the packetized data was reconstituted from a snapshot file
  unsigned cx:1;  //SRI eventually (property for now) *1 for complex

  // Frame sequence
  uint16_t seq;

  // Time Tag
  uint16_t msptr;  //contains [msv, ttv, sscv] bits at the top bits

  uint16_t msdel;

  //one TT value, 64 bit  create SDDS_Time object, pass to set_SDDS_TIME function
  uint64_t  ttag;  //get from SRI
  uint32_t  ttage; //get from SRI

  // Clock info
  int32_t  dFdT;  //will come from SRI  set_dFdT()
  uint64_t  freq;  //will come from SRI  set_freq()

  uint16_t ssd[2];  //hard code 0
  uint8_t aad[20]; //hard code 0
  //======================== SDDS data
  int8_t data[1024];
//  int16_t data[512];

  //======================== SDDS methods
  uint16_t get_seq(void) { return ntohs(seq); }
  void   set_seq(uint16_t val) { seq = htons(val); }

  uint32_t get_msptr(void) { return ntohs(msptr)&0x07FF; }
  void   set_msptr(uint16_t val) { msptr = (htons(val&0x07FF) | (msptr & 0x00E0)); }
  void   clear_msptr(void) { msptr &= 0x00E0; msdel = 0; }

  uint32_t get_msdel(void) { return ntohs(msdel); }
  void   set_msdel(uint16_t val) { msdel = htons(val); }

  int get_msv (void) { return (msptr&0x0080); }
  int get_ttv (void) { return (msptr&0x0040); }
  int get_sscv(void) { return (msptr&0x0020); }
  void set_msv (int flag) { flag ? msptr |= 0x0080 : msptr &= 0xFF7F ;  }
  void set_ttv (int flag) { flag ? msptr |= 0x0040 : msptr &= 0xFFBF ;  }
  void set_sscv(int flag) { flag ? msptr |= 0x0020 : msptr &= 0xFFDF ;  }

  long long int get_ttag(void)          { return swap8(ttag); }
  void          set_ttag(long long val) { ttag = swap8(val); }

  SDDSTime get_SDDSTime(void) {
    return SDDSTime( swap8(ttag), htonl(ttage) );
  }

  void set_SDDSTime(SDDSTime t) {
    ttag  = swap8(t.ps250());
    ttage = htonl(t.pf250());
    //printf( "tt: %12llu %12u\n", t.ps250(), t.pf250() );
  }

  double get_freq(void) {
    return double(swap8(freq)) * 1.3552527156068805e-11;
  }// 125 MHz/2^63
  void   set_freq(double val) {
    freq = swap8( uint64_t(val * 73786976294.838211) );
  }// 2^63/125 MHz


  double get_dfdt(void) {
    return ntohl(dFdT)
      * 9.3132257461547852e-10; // 2 Hz/2^31
  }
  void   set_dfdt(double val) {
    dFdT = ntohl(int32_t(1073741824.0*val));
  }

  void printPacket( void ) {
    using namespace std;

    //static XMTime tlast(0.0);
    cout <<"-------"<<dec<<endl;
    cout << " sf:" << sf;
    cout << " sos:" << sos;
    cout << " pp:" << pp;
    cout << " of:" << of;
    cout << " ss:" << ss;
    cout << " dmode:" << dmode;
    cout << " bps:" << int(bps);
    cout << " cx:" << cx;
    cout << " seq:" << dec <<get_seq();
    cout << endl;
    cout << " msv:" << get_msv();
    cout << " ttv:" << get_ttv();
    cout << " sscv:" << get_sscv();
    cout << " msptr:" << get_msptr();
    cout << " msdel:" << get_msdel();
    //cout << " ttag:" << hex<<get_ttag();
    cout << " ttag:" << hex<<swap8(ttag);
    cout << " ttage:" << hex<<htonl(ttage) << endl;
    //XMTime tim = get_time();
    //cout << tim << "   " << tim-tlast << endl;
    //tlast = tim;
    cout << " dFdT:" << get_dfdt();
    cout << " freq:" << get_freq();
    cout << endl;
  }

};

#endif
