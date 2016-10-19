#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

#define MIN_CWND 1
#define DESIRED_RTT 80
#define KP 0.002
#define KI 0
#define KD 0

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ), rtt_( -1 ), cwnd_( MIN_CWND ), 
    time_of_last_update_( timestamp_ms() ), last_error_( DESIRED_RTT ),
    sum_error_( 0 )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = (unsigned int) max((double) MIN_CWND, cwnd_);

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }
}

void Controller::control_update_() {
  uint64_t now = timestamp_ms();

  double error = DESIRED_RTT - rtt_;

  uint64_t dt = now - time_of_last_update_;
  double derivativeError = (error - last_error_) / (dt);

  sum_error_ += error * dt;

  cerr << "full " << KP * error + KI * sum_error_ << endl;

  cwnd_ += KP * error;
  cwnd_ = min(max((double) MIN_CWND, cwnd_), 40.0);

  last_error_ = error;
  time_of_last_update_ = now;
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  rtt_ = timestamp_ack_received - send_timestamp_acked;
  control_update_();
  cerr << "RTT " << rtt_ << " and cwnd " << cwnd_ << endl;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
