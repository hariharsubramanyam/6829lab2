#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

#define RTT_ALPHA 0.1
#define MIN_CWND 1.0
#define THRESHOLD 110
#define MD_CONST 0.5
#define AI_CONST 1
#define MD_BUFFER_TIME 500

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ), rtt_( -1 ), cwnd_( MIN_CWND ),
    timestamp_of_last_md_ ( timestamp_ms() )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = (unsigned int) cwnd_;

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
  if (rtt_ <= THRESHOLD) {
    cwnd_ += AI_CONST / cwnd_;
  } else if (timestamp_ms() - timestamp_of_last_md_ >= MD_BUFFER_TIME) {
    timestamp_of_last_md_ = timestamp_ms();
    cwnd_ *= MD_CONST;
    cwnd_ = max(cwnd_, MIN_CWND);
  }

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
