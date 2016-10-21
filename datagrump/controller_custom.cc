#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

#define EPOCH 100
#define THROUGHPUT_EWMA 0.7
#define RTT_EWMA 0.3
#define TIMEOUT 100
#define AI_CONST 1.0
#define MD_CONST 2
#define MIN_WINDOW 1
#define MD_BUFFER_TIME 200

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug ) : 
  debug_(debug),
  rtt_(),
  throughput_(),
  num_packets_in_epoch_(0),
  start_of_last_epoch_(timestamp_ms()),
  cwnd_(MIN_WINDOW),
  timestamp_of_mult_decrease_(timestamp_ms()),
  ai_(AI_CONST)
{
  rtt_.set_alpha(RTT_EWMA);
  throughput_.set_alpha(THROUGHPUT_EWMA);
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  // Let the window size equal the throughput delay product.
  unsigned int the_window_size = cwnd_;//50;

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

void Controller::additive_increase() {
  ai_ += 0.001;
  cwnd_ += (1.0 * ai_) / cwnd_;
}

void Controller::multiplicative_decrease(double md_const) {
  // If we have recently done a multiplicative decrease, don't do anything.
  ai_ = AI_CONST;
  if (timestamp_ms() - timestamp_of_mult_decrease_ < MD_BUFFER_TIME){
    return;
  }

  timestamp_of_mult_decrease_ = timestamp_ms();
  cwnd_ = max((double)MIN_WINDOW, cwnd_ /  md_const);
  cerr << "Cutting window by ratio " << md_const << " to " <<  cwnd_ << endl;
}

void Controller::multiplicative_decrease() {
  multiplicative_decrease(MD_CONST);
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
  // If the RTT is too high, cut the congestion window aggressively.
  double ratio = (1.0 * timestamp_ack_received - send_timestamp_acked) / rtt_.get();
  if (ratio > 2) {
    multiplicative_decrease(ratio * MD_CONST);
    if (ratio > 4) {
      cerr << "Outage" << endl;
      cwnd_ = 1;
    }
  }

  // Update RTT estimate.
  if (ratio > 4) {
    rtt_.update(timestamp_ack_received - send_timestamp_acked);
  }
  if (timestamp_ack_received - send_timestamp_acked < timeout_ms()) {
    additive_increase();
  }  else {
    multiplicative_decrease();
  }

  // Indicate that another packet arrived during this epoch.
  num_packets_in_epoch_++;

  // Get current time.
  uint64_t now = timestamp_ms();

  // If the epoch is over...
  if (now - start_of_last_epoch_ >= EPOCH) {

    // Update the throughput estimate.
    double to_packets_per_second = 10.0;
    throughput_.update(to_packets_per_second * ((double)num_packets_in_epoch_) / ((double)EPOCH));
    //cerr << "delay: " << rtt_.get() / 2 << " and throughput " << throughput_.get() << endl;

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