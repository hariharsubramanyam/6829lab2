#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

#define RTT_ALPHA 0.2
#define MIN_CWND 1

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ), rtt_( -1 ), cwnd_( MIN_CWND )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = cwnd_;

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

void Controller::update_cwnd_() {
  if (rtt_ > 200) {
    cwnd_ = 5;
  } else if (rtt_ > 150) {
    cwnd_ = 10;
  } else if (rtt_ > 100) {
    cwnd_ = 15;
  } else if (rtt_ > 75) {
    cwnd_ = 20;
  } else if (rtt_ > 40) {
    cwnd_ = 25;
  } else {
    cwnd_ =  35;
  }
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
  /* Default: take no action */
  double curr_rtt = timestamp_ack_received - send_timestamp_acked;
  if (rtt_ < 0) {
    rtt_ = curr_rtt;
  } else {
    rtt_ = RTT_ALPHA * curr_rtt + (1 - RTT_ALPHA) * rtt_;
  }
  update_cwnd_();

  cerr << "RTT " << rtt_ << " and cwnd: " << cwnd_ << endl;

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
