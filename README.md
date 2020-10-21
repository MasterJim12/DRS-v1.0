# DRS-v1.0
# Aero Code
  The main goal of the code is to provide functionality while keeping it readable for future team members that may not have a coding background.

# Board Design
  The goal of this board is to connect external servos and limit switches to the circuit board. It is also meant to handle power distribution between the components.

# Version 1.0 Issues
  With testing we found that pressing the button to activate the DRS system has bounce. This results in a less than ideal flap of the wings until the signal smooths out. 
  (A solution might be to add interupts to the code).
  The servos make noise and get hot after some time even if they aren't changin position. (This might be because of rubbing of wing elements which may be causing extra strain on the servos / I may need to design a BEC for power delivery to the servos)
