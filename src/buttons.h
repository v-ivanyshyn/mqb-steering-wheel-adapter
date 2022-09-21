//Heat wheel 0x25 go to 0x29 Back/Return up to 1000 ms holding for heating works on the phone
if (pressed_button == 0x25) {
      startPressed = millis();
      idleTime = startPressed - endPressed;
      if (idleTime >= 50 && idleTime < 1000) { pressed_button = 0x29; }
      if (idleTime >= 1000) { pressed_button = 0x23; }
  } else {
      endPressed = millis(); holdTime = endPressed - startPressed; if (holdTime >= 500 && holdTime < 1500) { } if (holdTime >= 3000) {}
  }
