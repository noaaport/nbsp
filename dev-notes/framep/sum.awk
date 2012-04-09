# The files nbspd.status-0_4p{3,4} were taken for one hour each
# using the version with the original pctl, and with the active pce
# in the pctl. This script calculates the total number of frames
# and products missed in one hour for each file. The results are:
#
# 0_4p3: 890 102
# 0_4p4: 795 84

BEGIN {
  frames = 0;
  prod = 0;
}

END {
  print frames, prod;
}

{
  frames += $4;
  prod += $9;
}
