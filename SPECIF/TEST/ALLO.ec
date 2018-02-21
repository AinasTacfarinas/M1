node ALLO
  (C: bool)
returns
  (S: int);

var
  V7_S1: int;
  V8_S2: int;

let
  S = (if C then V7_S1 else V8_S2);
  V7_S1 = (0 -> (if C then ((pre V7_S1) + 1) else (pre V7_S1)));
  V8_S2 = (0 -> (if (not C) then ((pre V8_S2) + 1) else (pre V8_S2)));
tel

