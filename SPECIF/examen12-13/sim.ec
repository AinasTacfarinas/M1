node sim
  (E1: bool;
  E2: bool;
  C: bool)
returns
  (S: bool);

var
  V76_S1: bool;
  V77_S2: bool;
  V83_S: bool;

let
  S = ((if C then V76_S1 else V77_S2) = V83_S);
  V76_S1 = (false -> (if E1 then (not (pre V76_S1)) else (pre V76_S1)));
  V77_S2 = (false -> (if E2 then (not (pre V77_S2)) else (pre V77_S2)));
  V83_S = (false -> (if (if C then E1 else E2) then (not (pre V83_S)) else (pre 
  V83_S)));
tel

