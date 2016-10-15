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
  echo "Using threshold"
fi
