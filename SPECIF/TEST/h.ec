node h
  (x: bool;
  y: bool)
returns
  (res: bool);

let
  res = ((if y then (current ((x) when y)) else false) -> (current ((x) when y)
  ));
tel

