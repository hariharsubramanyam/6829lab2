if [ "$1" = "a" ]; then
  echo "Using fixed_window"
  cp sender_fixed_window.cc sender.cc
  cp controller_fixed_window.cc controller.cc 
  cp controller_fixed_window.hh controller.hh
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
  echo "Using ad-hoc"
  cp sender_custom.cc sender.cc
  cp controller_custom.cc controller.cc 
  cp controller_custom.hh controller.hh
elif [ "$1" = "dprime" ]; then
  echo "Using Sarsa"
  cp sender_sarsa.cc sender.cc
  cp controller_sarsa.cc controller.cc 
  cp controller_sarsa.hh controller.hh
fi
