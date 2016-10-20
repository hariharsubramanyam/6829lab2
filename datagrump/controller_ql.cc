#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

#define EPOCH 50
#define THROUGHPUT_EWMA 0.2
#define RTT_EWMA 0.2
#define TIMEOUT 100

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug ) : 
  debug_(debug),
  rtt_(),
  throughput_(),
  num_packets_in_epoch_(0),
  start_of_last_epoch_(timestamp_ms()) 
{
  rtt_.set_alpha(RTT_EWMA);
  throughput_.set_alpha(THROUGHPUT_EWMA);
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  // Let the window size equal the throughput delay product.
  unsigned int the_window_size = 50;

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
  // Update RTT estimate.
  rtt_.update(timestamp_ack_received - send_timestamp_acked);

  // Indicate that another packet arrived during this epoch.
  num_packets_in_epoch_++;

  // Get current time.
  uint64_t now = timestamp_ms();

  // If the epoch is over...
  if (now - start_of_last_epoch_ >= EPOCH) {

    // Update the throughput estimate. the constant 10.0 is to measure in packets per second.
    throughput_.update(10.0 * ((double)num_packets_in_epoch_) / ((double)EPOCH));
    cerr << "delay: " << rtt_.get() / 2 << " and throughput " << throughput_.get() << endl;

    // Update the start of the epoch and indicate that no packets arrived during this epoch.
    start_of_last_epoch_ = now;
    num_packets_in_epoch_ = 0;
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
  return TIMEOUT; /* timeout of one second */
}
