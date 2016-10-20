#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <deque>

/* Congestion controller interface */
class Sarsa {
private:
  size_t num_states_;
  size_t num_actions_;
  double alpha_;
  double epsilon_;
  double gamma_;
  bool verbose_;
  // Given state s and action a, table_[s][a] is the reward attained 
  // by taking action a in state s.
  std::vector<std::vector<double> > table_;
public:
  Sarsa(size_t num_states,
            size_t num_actions,
            double alpha,
            double epsilon,
            double gamma,
            bool verbose) :
    num_states_(num_states), num_actions_(num_actions), alpha_(alpha), 
    epsilon_(epsilon), gamma_(gamma), verbose_(verbose), table_() {
      srand((unsigned)time(NULL));
      for (size_t i = 0; i < num_states; i++) {
        table_.push_back(std::vector<double>(num_actions));
        for (size_t j = 0; j < num_actions; j++) {
          table_[i][j] = 0;
        }
      }
  }

  void prefer_action(size_t state, size_t action, double value) {
    for (size_t act = 0; act < num_actions_; act++) {
      table_[state][act] = 0;
    }
    table_[state][action] = value;
  }

  size_t act_greedily(size_t state) {
    size_t best_action = 0;
    double best_value = table_[state][0];
    for (size_t action = 1; action < num_actions_; action++) {
      if (table_[state][action] > best_value) {
        best_action = action;
        best_value = table_[state][action];
      }
    }
    if (verbose_) {
      std::cout << "Taking greedy action " << best_action << " with value " << best_value << std::endl;
    }
    return best_action;
  }
  
  size_t act_eps_greedily(size_t state) {
    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    if (r < epsilon_) {
      // If we behave non-greedily, sample an action.
      size_t action = rand() % num_actions_;
      if (verbose_) {
        std::cout << "Taking random action " << action << std::endl;
      }
      return action;
    } else {
      // Otherwise, pick the optimal action.
      return act_greedily(state);
    }
  }
  
  void update_with_reward(size_t state, size_t action, size_t next_state, double reward) {
    double old_value = table_[state][action];
    table_[state][action] += alpha_ * (reward + gamma_ * table_[next_state][act_eps_greedily(next_state)] - table_[state][action]);
    if (verbose_) {
      std::cout << "Updating value from " << old_value << " to " << table_[state][action] << std::endl;
    }
  }
};

class FixedQueue {
private:
  size_t size_;
  std::deque<int> deque_;
public:
  FixedQueue(size_t size) : size_(size), deque_() {}
  void add(double val) {
    if (deque_.size() == size_) {
      deque_.pop_front();
    }
    deque_.push_back(val);
  }
  
  size_t to_state(int base) {
    size_t result = 0;
    size_t power = 0;
    for (std::deque<int>::const_iterator i = deque_.cbegin();
          i != deque_.cend(); i++) {
      result += (*i) * std::pow(base, power);
      power++;
    }
    return result;
  }
};

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
  int last_state_;
  int last_action_;
  double cwnd_;
  Sarsa q_;

  void act(int action);

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
};

#endif
