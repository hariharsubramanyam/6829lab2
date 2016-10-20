#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

#define EPOCH 50
#define THROUGHPUT_EWMA 0.1
#define RTT_EWMA 0.1
#define TIMEOUT 100

#define VERY_LOW 0
#define LOW 1
#define MED 2
#define HIGH 3
#define VERY_HIGH 4

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug ) : 
  debug_(debug),
  rtt_(),
  throughput_(),
  num_packets_in_epoch_(0),
  start_of_last_epoch_(timestamp_ms()),
  last_state_(-1),
  last_action_(-1),
  cwnd_(1),
  q_(9, 11, 0.4, 0.2, 0.9, false)
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

int discretize_throughput(double throughput) {
  if (throughput < 2) {
    return LOW;
  } else if (throughput < 4) {
    return MED;
  } else {
    return HIGH;
  }
}

int discretize_delay(double delay) {
  if (delay < 80) {
    return LOW;
  } else if (delay < 200) {
    return MED;
  } else {
    return HIGH;
  }
}

int state_for_discretized_pair(int tp, int delay) {
  if (tp == LOW && delay == LOW) {
    return 0;
  } else if (tp == LOW && delay == MED) {
    return 1;
  } else if (tp == LOW && delay == HIGH) {
    return 2;
  } else if (tp == MED && delay == LOW) {
    return 3;
  } else if (tp == MED && delay == MED) {
    return 4;
  } else if (tp == MED && delay == HIGH) {
    return 5;
  } else if (tp == HIGH && delay == LOW) {
    return 6;
  } else if (tp == HIGH && delay == MED) {
    return 7;
  } else {
    return 8;
  }
}

int cwnd_for_action(int action) {
  if (action == 0) {
    return 1;
  } else if (action == 1) {
    return 3;
  } else if (action == 2) {
    return 4;
  } else if (action == 3) {
    return 5;
  } else if (action == 4) {
    return 9;
  } else if (action == 5) {
    return 12;
  } else if (action == 6) {
    return 15;
  } else if (action == 7) {
    return 18;
  } else if (action == 8) {
    return 21;
  } else if (action == 9) {
    return 23;
  } else {
    return 24;
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
    //cerr << "delay: " << rtt_.get() / 2 << " and throughput " << throughput_.get() << endl;

    // Update the start of the epoch and indicate that no packets arrived during this epoch.
    start_of_last_epoch_ = now;
    num_packets_in_epoch_ = 0;
  }

  double score = throughput_.get() / rtt_.get();
  //cerr << "score: " << score << endl;
  int state = state_for_discretized_pair(
    discretize_delay(rtt_.get() / 2),
    discretize_throughput(throughput_.get())
  );
  if (last_action_ >= 0 && last_state_ >= 0) {
    q_.update_with_reward(last_state_, last_action_, state, score);
  }
  int action = q_.act_eps_greedily(state);
  cwnd_ = cwnd_for_action(action);
  last_state_ = state;
  last_action_ = action;

  cerr << "Came from " << last_state_ << " and " << last_action_
    << " to " << state << " and " << action
    << " with reward " << score << " and cwnd " << cwnd_ << endl;

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
