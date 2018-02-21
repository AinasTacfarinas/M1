node avant
  (E1: bool;
  E2: bool;
  C: bool)
returns
  (S: bool);

var
  V6_S1: bool;
  V7_S2: bool;

let
  S = (if C then V6_S1 else V7_S2);
  V6_S1 = (false -> (if E1 then (not (pre V6_S1)) else (pre V6_S1)));
  V7_S2 = (false -> (if E2 then (not (pre V7_S2)) else (pre V7_S2)));
tel

