#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

#define TIMEOUT 100
#define AI_CONST 1
#define MD_CONST 2
#define MIN_WINDOW 1
#define MD_BUFFER_TIME 1000

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ), window_size_( MIN_WINDOW ), fraction_of_window_size_( 0 ), 
    timestamp_of_mult_decrease_ ( timestamp_ms() )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << window_size_ 
   << " with fraction " << fraction_of_window_size_ << endl;
  }

  return window_size_;
}

void Controller::additive_increase() {
  fraction_of_window_size_ += AI_CONST;
  if (fraction_of_window_size_ >= window_size_) {
    window_size_++;
    fraction_of_window_size_ -= window_size_;
  }
  cerr << "Doing additive increase " << window_size_ << endl;
}

void Controller::multiplicative_decrease() {
  // If we have recently done a multiplicative decrease, don't do anything.
  if (timestamp_ms() - timestamp_of_mult_decrease_ < MD_BUFFER_TIME){
    return;
  }

  timestamp_of_mult_decrease_ = timestamp_ms();
  fraction_of_window_size_ = 0;
  window_size_ = max(
      (unsigned int) MIN_WINDOW, 
      (unsigned int) (window_size_ /  MD_CONST)
  );
  cerr << "Doing multiplicative decrease " << window_size_ << endl;
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
  // We'll assume we had packet loss if the packet took more than timeout_ms().
  if (timestamp_ack_received - send_timestamp_acked < timeout_ms()) {
    additive_increase();
  }  else {
    multiplicative_decrease();
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
