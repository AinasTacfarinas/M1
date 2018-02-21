node puissance
  (x: int;
  n: int)
returns
  (res: int);

let
  res = (if (n < 0) then -1 else (if (n = 0) then 1 else (if (n = 1) then x 
  else 1)));
tel

