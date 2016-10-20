#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

#define EPOCH 100
#define THROUGHPUT_EWMA 0.7
#define RTT_EWMA 0.1
#define TIMEOUT 100

#define VERY_LOW 0
#define LOW 1
#define MED 2
#define HIGH 3
#define VERY_HIGH 4

#define NUM_ACTIONS 9

using namespace std;

int discretize_throughput(double throughput) {
  if (throughput < 3) {
    return LOW;
  } else if (throughput < 5) {
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

double compute_score(double throughput, double delay) {
  int d_throughput = discretize_throughput(throughput);
  int d_delay = discretize_delay(delay);
  if (d_throughput == LOW && d_delay == HIGH) {
    return 0;
  } else if (d_throughput == MED && d_delay == HIGH) {
    return 1;
  } else if (d_throughput == HIGH && d_delay == HIGH) {
    return 3;
  } else if (d_throughput == LOW && d_delay == LOW) {
    return 5;
  } else if (d_throughput == MED && d_delay == LOW) {
    return 20;
  } else {
    return 30; 
  }
}


void Controller::act(int action) {
  double relative = 1.0 * action / NUM_ACTIONS;
  cwnd_ = 30 * relative;
  cwnd_ = std::max(1.0, cwnd_);
}

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
  q_(9, NUM_ACTIONS, 0.4, 0.05, 0.9, true)
{
  rtt_.set_alpha(RTT_EWMA);
  throughput_.set_alpha(THROUGHPUT_EWMA);
  double greedy_value = 900;
  q_.prefer_action(state_for_discretized_pair(LOW, LOW), 7, greedy_value);
  q_.prefer_action(state_for_discretized_pair(LOW, MED), 4, greedy_value);
  q_.prefer_action(state_for_discretized_pair(LOW, HIGH), 2, greedy_value);
  q_.prefer_action(state_for_discretized_pair(MED, LOW), 6, greedy_value);
  q_.prefer_action(state_for_discretized_pair(MED, MED), 4, greedy_value);
  q_.prefer_action(state_for_discretized_pair(MED, HIGH), 1, greedy_value);
  q_.prefer_action(state_for_discretized_pair(HIGH, LOW), 3, greedy_value);
  q_.prefer_action(state_for_discretized_pair(HIGH, MED), 2, greedy_value);
  q_.prefer_action(state_for_discretized_pair(HIGH, HIGH), 9, greedy_value);
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

    // Update the throughput estimate.
    double to_packets_per_second = 10.0;
    throughput_.update(to_packets_per_second * ((double)num_packets_in_epoch_) / ((double)EPOCH));
    //cerr << "delay: " << rtt_.get() / 2 << " and throughput " << throughput_.get() << endl;

    // Update the start of the epoch and indicate that no packets arrived during this epoch.
    start_of_last_epoch_ = now;
    num_packets_in_epoch_ = 0;

    // Since the epoch is over, pick a new action for the next state.
    double score = compute_score(throughput_.get(), rtt_.get() / 2);
    //cerr << "score: " << score << endl;
    int state = state_for_discretized_pair(
      discretize_delay(rtt_.get() / 2),
      discretize_throughput(throughput_.get())
    );
    if (last_action_ >= 0 && last_state_ >= 0) {
      q_.update_with_reward(last_state_, last_action_, state, score);
    }
    int action = q_.act_eps_greedily(state);
    act(action);
    cerr << "tp " << throughput_.get() << " and delay " << rtt_.get() / 2 << endl;
    cerr << last_state_ << " => " << state << " "
      << last_action_ << " => " << action << " "
      << "rew=" << score << " and cwnd=" << cwnd_ << endl;
    last_state_ = state;
    last_action_ = action;
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
