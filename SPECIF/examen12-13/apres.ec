node apres
  (E1: bool;
  E2: bool;
  C: bool)
returns
  (S: bool);

let
  S = (false -> (if (if C then E1 else E2) then (not (pre S)) else (pre S)));
tel

