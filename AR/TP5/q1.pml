#define N 3 /* Number of processes */
#define L 10 /* Buffer size */
#define NIL (N+1) /* for an undefined value */

#define dem_sc node[1]@ask_sc
#define ln_sc node[1]@in_SC

mtype = {req, tk};
/* le processus i recoit ses messages dans canal[i] */

chan canal[N] = [L] of {mtype, byte}; /* pour un message de type tk,
on mettra la valeur associee a 0 */

byte SharedVar = 0; /* la variable partagee */

inline Initialisation ( ) {
/* initialise les variables locales, sauf reqId, val et typ_mes */
	printf("Init\n");
	father = Initial_Token_Holder;
	next = NIL;
	requesting = false;
	if
		::(father == id)->token = true;father = NIL;
		::else->token=false;
	fi;
}

inline Request_CS ( ) {
/* operations effectuees lors d’une demande de SC */
	printf("Request\n");
	ask_sc : requesting = true;
	if
		::(father != NIL)->canal[father] ! req,id; father=NIL;
		::else->skip;
	fi;
}

inline Release_CS ( ) {
/* operations effectuees en sortie de SC */
	printf("Release\n");
	requesting=false;
	printf("%d\n",next);
	if
		::(next!=NIL)->canal[next] ! tk,NIL; token = false; next = NIL;
		::else->skip;
	fi;
}

inline Receive_Request_CS ( ) {
/* traitement de la reception d’une requete */
	printf("%d Receive_req %d\n",id,val);
	if
		::(father==NIL)->if
							::(requesting==true)->next=val;
							::else->token=false;canal[val] ! tk,NIL;
						fi;
		::else->canal[father] ! req,val;
	fi;
	father=val;
	
}

inline Receive_Token () {
/* traitement de la reception du jeton */
	token = true;
}


proctype node(byte id; byte Initial_Token_Holder){
	bit requesting; /* indique si le processus a demande la SC ou pas */
	bit token; /* indique si le processus possede le jeton ou non */
	byte father; /* probable owner */
	byte next; /* next node to whom send the token */
	byte val; /* la valeur contenue dans le message */
	mtype typ_mes; /* le type du message recu */
	byte reqId; /* l’Id du demandeur, pour une requete */
	chan canal_rec = canal[id]; /* un alias pour mon canal de reception */
	xr canal_rec; /* je dois etre le seul a le lire */
	/* Chaque processus execute une boucle infinie */
	Initialisation ();
	do
		:: ((token == true) && empty(canal_rec) && (requesting == true)) -> printf("SC\n");
		/* on oblige le detenteur du jeton a consulter les messages recus */
			in_SC :
		/* acces a la ressource critique (actions sur SharedVar),
		puis sortie de SC */
		SharedVar++;
		assert(SharedVar==1);
		SharedVar--;
		Release_CS ( );
		:: canal_rec ? typ_mes(val) ->	if
											::(typ_mes==tk)->Receive_Token ();
											::(typ_mes==req)->Receive_Request_CS();
										fi;
		/* traitement du message recu */
		:: (requesting == false) -> /* demander la SC */
			Request_CS ( );
	od ;
}


/* Cree un ensemble de N processus */
init {
	byte proc;
	atomic {
		proc=0;
		do
			:: proc <N ->
				run node(proc, 0);
				proc++
			:: proc == N -> break
		od
	}
}

ltl_vivacite { always(dem_sc
