node tramway
  (dp: bool;
  es: bool;
  fp: bool)
returns
  (en_station: bool;
  porte_demandee: bool;
  porte_ouverte: bool;
  ouvrir_porte: bool;
  fermer_porte: bool;
  porte_ok: bool;
  attention_depart: bool;
  accepter_demande: bool;
  depart_iminent: bool;
  depart: bool);

var
  V74_level: bool;
  V82_level: bool;

let
  assert (true -> ((ouvrir_porte or (pre porte_ouverte)) or (not porte_ouverte)
  ));
  assert (porte_ouverte => ((pre ouvrir_porte) or (pre porte_ouverte)));
  assert ((not porte_ouverte) => ((pre fermer_porte) or (pre (not porte_ouverte
  ))));
  assert (not (V74_level and depart));
  assert (not (V82_level and depart));
  en_station = es;
  porte_demandee = (dp -> (if porte_ouverte then false else (if ((pre 
  en_station) and (not en_station)) then false else (if (false -> (dp and (not 
  (pre porte_demandee)))) then true else (if attention_depart then false else 
  (pre porte_demandee))))));
  porte_ouverte = (false -> (if (false -> (pre fermer_porte)) then false else 
  (if (not (pre porte_ouverte)) then (false -> (((pre ouvrir_porte) and (pre 
  accepter_demande)) and (pre en_station))) else (pre porte_ouverte))));
  ouvrir_porte = (false -> ((accepter_demande and en_station) and 
  porte_demandee));
  fermer_porte = (attention_depart and porte_ouverte);
  porte_ok = (depart_iminent and (not porte_ouverte));
  attention_depart = (if (false -> (fp and en_station)) then true else false);
  accepter_demande = (true -> (if depart_iminent then false else (if depart 
  then true else (false -> (pre accepter_demande)))));
  depart_iminent = (if attention_depart then true else (if depart then false 
  else (false -> (pre depart_iminent))));
  depart = ((pre en_station) and (not en_station));
  V74_level = (porte_ok -> (if false then true else (if attention_depart then 
  false else (pre V74_level))));
  V82_level = (attention_depart -> (if false then true else (if (en_station and 
  (not (pre en_station))) then false else (pre V82_level))));
tel

