if [ "$1" = "a" ]; then
  echo "Using original code"
  cp sender_original.cc sender.cc
  cp controller_original.cc controller.cc 
  cp controller_original.hh controller.hh
elif [ "$1" = "b" ]; then
  echo "Using AIMD"
  cp sender_aimd.cc sender.cc
  cp controller_aimd.cc controller.cc 
  cp controller_aimd.hh controller.hh
elif [ "$1" = "c" ]; then
  echo "Using delay-triggered"
  cp sender_delay.cc sender.cc
  cp controller_delay.cc controller.cc 
  cp controller_delay.hh controller.hh
elif [ "$1" = "d" ]; then
  echo "Using Q-learning"
  cp sender_sarsa.cc sender.cc
  cp controller_sarsa.cc controller.cc 
  cp controller_sarsa.hh controller.hh
fi
