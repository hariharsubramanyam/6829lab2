#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <deque>
#include <unordered_map>

/* Congestion controller interface */
class Ewma {
private:
  double alpha_;
  double value_;
  bool got_first_val_;
public:
  Ewma() : alpha_(1), value_(0), got_first_val_(false) {};
  void set_alpha(double alpha) {
    alpha_ = alpha;
  }
  double get() {
    return value_;
  }
  double update(double value) {
    value_ = got_first_val_ ? alpha_ * value + (1 - alpha_) * value_ : value;
    got_first_val_ = true;
    return value_;
  }
};

class Controller
{
private:
  bool debug_; /* Enables debugging output */

  /* Add member variables here */
  Ewma rtt_;
  Ewma throughput_;
  uint64_t num_packets_in_epoch_;
  uint64_t start_of_last_epoch_;
  double cwnd_;
  uint64_t timestamp_of_mult_decrease_;
  double ai_;
  std::unordered_map<uint64_t, uint64_t> send_time_for_packet_;
public:
  /* Public interface for the congestion controller */
  /* You can change these if you prefer, but will need to change
     the call site as well (in sender.cc) */

  /* Default constructor */
  Controller( const bool debug );

  /* Get current window size, in datagrams */
  unsigned int window_size( void );

  /* A datagram was sent */
  void datagram_was_sent( const uint64_t sequence_number,
			  const uint64_t send_timestamp );

  /* An ack was received */
  void ack_received( const uint64_t sequence_number_acked,
		     const uint64_t send_timestamp_acked,
		     const uint64_t recv_timestamp_acked,
		     const uint64_t timestamp_ack_received );

  /* How long to wait (in milliseconds) if there are no acks
     before sending one more datagram */
  unsigned int timeout_ms( void );

  void multiplicative_decrease();
  void multiplicative_decrease(double md_const);

  void additive_increase();
};

#endif
