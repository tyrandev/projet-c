#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
char *clear = "cls";
#elif __unix__ || __APPLE__ && __MACH__
char *clear = "clear";
#endif

/*****************************************************************************/
/*                                     STRUCTS                               */
/*
 * int id : L'identifiant unique de la personne
 * char nom[25] : Le nom de la personne (25 caracteres maximum)
 * char prenom[25] : Le prenom de la personne (25 caracteres maximum)
 * int formateur : 1 si la personne est un formateur, 0 si la personne est un etudiant
 * int nb_formations : Le nombre de formations auquel la personne participera
 * int formations[30] : Vecteur qui stockera les identifiants des differentes formations auquel la personne participera
 * (On suppose dans une annee, une personne ne peut participer qu'a 30 formations maximum)
 * int nb_jours_indisponible : Si la personne est un formateur, il se peut qu'il/elle ait des jours d'indisponibile,
 * cette variable va stocker le nombre de jours ou cette personne est indisponile (maximum 7)
 * int jours_indisponibles[7] : Le vecteur qui stockera les jours auquel le formateur ne sera pas disponible
 * (1 - lundi, 2 - mardi, etc...)
 * int reduction : Si la personne est un etudiant, il se peut qu'il ait une reduction sur son minerval,
 * 1 s'il a droit a une reduction, 0 si pas
 * int val_reduction : Le pourcentage de reduction auquel un etudiant a droit
 */
typedef struct personne
{
    int id;
    char nom[25];
    char prenom[25];
    int formateur;
    int nb_formations;
    int formations[30];
    int nb_jours_indisponible;
    int jours_indisponible[7];
    int reduction;
    int val_reduction;
} personne;

/*
 * Cette structure sert a devenir les differents noeuds qui seront stockes dans la base de donnee,
 * soit la structure db_personne.
 * Voici ce qui represent chaque partie de la structure:
 * personne *p : Le pointeur de la personne qui sera stocke dans ce nud lors de sa creation.
 * struct noeud_db_personne *next : qui contiendra le tete lors qu'on creera un nouveau nud, sinon NULL.
 */
typedef struct noeud_db_personne
{
    personne *p;
    struct noeud_db_personne *next;
} noeud_db_personne;

/*
 * Cette structure sert a contenir tous les differentes noeuds noeud_db_personne.
 * C'est a partir de cette structure que l'on stockera les differentes noeuds qui eux-memes stockeront
 * leurs personnes respectives.
 * noeud_db_personne *head : La tete de la liste chainee qui stockera toutes les personnes.
 */
typedef struct db_personne
{
    noeud_db_personne *head;
} db_personne;

/*
 * Cette structure va stocker les differentes personnes qui participeront a une formation specifique.
 * personne *p : La personne qui participera a la formation.
 * struct noeud_formation *next : Le noeud de pour la prochaine personne qui sera stockee.
 */
typedef struct noeud_formation
{
    personne *p;
    struct noeud_formation *next;
} noeud_formation;

/*
 * Cette structure sert a stocker toutes les informations qui composent une formations.
 * Voici ce que chaque partie represente:
 * int id : L'identifiant unique de la formation.
 * char nom[40] : Le nom de la formation (40 caracteres maximum).
 * float prix : Le cout de la formation.
 * int nb_jours : Le nombre de jours par semaine ou cette formation a cours.
 * int jours[7] : Vecteur contenant les jours ou la formation a cours.
 * float heures[24] : Le nombre d'heures du debut de la formation.
 * float durees[10] : Les differentes durees du cours lors de la semaine.
 * int nb_prerequis : Le nombre de prerequis pour avoir acces a cette formation.
 * int prerequis[10] : Vecteur contenant les identifiants des formations qui seraient des prerequis.
 * noeud_formation *head : Etant donne qu'une formation stocke des personnes,
 * elle-meme est une liste chainee qui stockera un nombre indetermine de participants.
 */
typedef struct formation
{
    int id;
    char nom[40];
    float prix;
    int nb_jours;
    int jours[7];
    float heures[24];
    float durees[10];
    int nb_prerequis;
    int prerequis[10];
    noeud_formation *head;
} formation;

/*
 * Cette structure suit la meme logique que la structure noeud_db_personne.
 * Elle sert a stocker les differentes formations, qui eux-memes stockeront les personnes a leurs tour.
 * formation *f : La formation qui sera stockee dans la base de donnees.
 * struct noeud_db_formation *next : La prochaine formation qui sera stockee dans la base de donnees.
 * NULL si pas de prochaine formation.
 */
typedef struct noeud_db_formation
{
    formation *f;
    struct noeud_db_formation *next;
} noeud_db_formation;

/*
 * Cette structure aussi suit la meme logique que la structure db_formation.
 * Elle sert de tete pour la la liste chainee et c'est a partir de cette structure-ci que l'on demarrera
 * les differentes interactions avec la base de donnees des formations.
 * noeud_db_formation *head : La tete de la liste chainee qui stockera les differentes formations.
 */
typedef struct db_formation
{
    noeud_db_formation *head;
} db_formation;

/*                                    FIN STRUCTS                            */
/*****************************************************************************/

/*****************************************************************************/
/*                                    PERSONNE                               */
/*
 * Cette fonction sert a creer un pointeur qui permettra d'initialiser les differentes informations presentes
 * dans la structure personne.
 * Lors de l'initialisation d'une personne, on n'aura besoin que du nom de famille de la personne,
 * son prenom et s'il/elle est un formateur ou pas.
 * Le reste des informations est manipule par la suite lors des differentes interactions.
 */
personne *creer_personne( char nom[], char prenom[], int formateur )
{
    personne *e = ( personne * ) calloc( sizeof( personne ), sizeof( personne ) );
    strcpy( e->nom, nom );
    strcpy( e->prenom, prenom );
    e->formateur = formateur;
    return e;
}

/*
 * Cette fonction sert a afficher les informations de base qui caracterisent une personne.
 * De maniere generale, son identifiant, son nom de famille, son prenom et s'il est formateur ou etudiant.
 */
void afficher_personne( personne *p )
{
    personne *tmp = p;
    printf( "* %2d %-25s %-25s %-10s            *\n",
            tmp->id, tmp->nom, tmp->prenom, tmp->formateur ? "Formateur" : "Etudiant" );
}

/*
 * Cette fonction sert a initialiser le pointer noeud_db_personne *head dans la structure db_personne a NULL,
 * afin que l'on puisse commencer a faire des manipulations avec cette structure.
 */
db_personne *creer_db_personne()
{
    db_personne *db = ( db_personne * ) calloc( sizeof( db_personne ), sizeof( db_personne ) );
    db->head = NULL;
    return db;
}

/*
 * Cette fonction sert a initialiser un pointeur noeud_db_personne *ndb qui stockera personne *p
 * dans la base de donnees db_personne *db.
 * Ici, l'ajout dans la liste chainee a lieu par le mecanisme suivant:
 * On initialise le noeud temporaire que l'on ajoutera a la base de donnees.
 * On associe p au pointeur p present dans la structure noeud_db_personne.
 * On initialise le prochain noeud de la liste *next a NULL.
 * Si la tete *head de la base de donnee est NULL, alors la tete devient le nouveau noeud.
 * On arrete la fonction d'ajout la.
 * Sinon, on fait une copie de la tete dans le noeud *next que l'on avait initialise a NULL.
 * On declare la tete comme etant le noeud temporaire que l'on a initialise.
 */
void ajouter_db_personne( db_personne *db, personne *p )
{
    noeud_db_personne *ndb = ( noeud_db_personne * ) calloc( sizeof( noeud_db_personne ), sizeof( noeud_db_personne ) );
    ndb->p = p;
    ndb->next = NULL;
    if( db->head == NULL )
    {
        db->head = ndb;
        return;
    }
    ndb->next = db->head;
    db->head = ndb;
}

/*
 * Cette fonction sert a supprimer une personne de la base de donnees a partir de son identifiant.
 * La demarche faite dans cette fonction est la suivant:
 * On verife que la tete de la base de donnees dbp->head ne soit pas NULL, si oui, on arrete la fonction.
 * On verifie si le premier element de la liste correspond a l'id de la personne que l'on souhaite supprimer.
 * Si oui:
 * On cree un noeud temporaire qui stockera la personne suivante dans la liste.
 * On libere l'espace memoire occupe par head avec la fonction free(dbp->head).
 * On attribue a dbp->head le noeud temporaire que l'on avait cree.
 * On arrete la fonction.
 * Sinon, on parcourt l'entierete de la liste jusqu'au moment ou l'on trouve la personne qui a le meme id que
 * l'id en parametre.
 * Si on le trouve, on pivote l'element qui suit vers l'element que l'on vient de supprimer.
 * On arrete la fonction, si reussite, on obtient 1, si pas, on obtient 0.
 */
int supprimer_db_personne( db_personne *dbp, int id )
{
    noeud_db_personne *ndbp = dbp->head;
    if( ndbp == NULL )
    {
        return 0;
    }
    noeud_db_personne *tmp = NULL;
    if( ndbp->p->id == id )
    {
        tmp = dbp->head->next;
        free( dbp->head );
        dbp->head = tmp;
        return 1;
    }
    while( ndbp != NULL )
    {
        if( ndbp->next == NULL )
        {
            if( ndbp->p->id == id )
            {
                return 1;
            }
            return 0;
        }
        if( ndbp->next->p->id == id )
        {
            tmp = ndbp->next;
            ndbp->next = tmp->next;
            free( tmp );
            return 1;
        }
        ndbp = ndbp->next;
    }
    return 0;
}

/*
 * Cette fonction parcourt l'entierete de la base de donnees db_personne *db.Tant que la tete n'est pas NULL,
 * on affichera les informations que l'on souhaite afficher de chaque personne presente dans la base de donnees.
 */
void afficher_db_personne( db_personne *db )
{
    db_personne *tmpdb = db;
    noeud_db_personne *tmpndb = tmpdb->head;
    printf( "* %2s %-25s %-25s %-9s             *\n", "ID", "Nom", "Prenom", "Statut" );
    printf( "* ---------------------------------------------------------------------------- *\n" );
    while( tmpndb != NULL )
    {
        afficher_personne( tmpndb->p );
        tmpndb = tmpndb->next;
    }
}

/*
 * Cette fonction renvoie NULL si une formation avec un nom specifique n'existe pas dans
 * la base de donnees db_formation *dbf.
 * Sinon, la fonction retourne la formation trouvee.
 */
personne *get_personne( db_personne *db, char nom[], char prenom[], int formateur )
{
    db_personne *tmpdb = db;
    noeud_db_personne *tmpndb = tmpdb->head;
    while( tmpndb != NULL )
    {
        if( strcmp( tmpndb->p->nom, nom ) == 0 &&
            strcmp( tmpndb->p->prenom, prenom ) == 0 &&
            tmpndb->p->formateur == formateur )
        {
            return tmpndb->p;
        }
        tmpndb = tmpndb->next;
    }
    return NULL;
}

/*                                 FIN PERSONNE                              */
/*****************************************************************************/

/*****************************************************************************/
/*                                     FORMATION                             */
/*
 * Cette fonction sert a creer un pointeur qui permettra d'initialiser les differentes informations presentes dans
 * la structure formation.
 * Lors de l'initialisation d'une formation, on n'aura besoin que du nom de la formation et de son prix.
 * Le reste des informations est manipule par la suite lors des differentes interactions.
 */
formation *creer_formation( char nom[], float prix )
{
    formation *tmp = ( formation * ) calloc( sizeof( formation), sizeof( formation ) );
    strcpy( tmp->nom, nom );
    tmp->prix = prix;
    tmp->head = NULL;
    return tmp;
}
/*
 * Cette fonction sert a initialiser un pointeur noeud_formation *nf qui stockera personne *p qui participera
 * dans formation *f. Ici,
 * l'ajout dans la liste chainee a lieu par le mecanisme suivant:
 * On initialise le noeud temporaire que l'on ajoutera dans la formation.
 * On associe p au pointeur p present dans la structure noeud_formation.
 * On initialise le prochain noeud de la liste *next a NULL.
 * Si la tete *head de la formation est NULL, alors la tete devient le nouveau noeud. On arrete la fonction d'ajout la.
 * Sinon, on fait une copie de la tete dans le noeud *next que l'on avait initialise a NULL.
 * On declare la tete comme etant le noeud temporaire que l'on a initialise.
 */
int ajouter_formation( formation *f, personne *p )
{
    noeud_formation *nf = ( noeud_formation * ) calloc( sizeof( noeud_formation ), sizeof( noeud_formation) );
    nf->p = p;
    nf->next = NULL;
    if( f->head == NULL )
    {
        f->head = nf;
        return 1;
    }
    noeud_formation *tmpnf = f->head;
    while( tmpnf != NULL )
    {
        if( tmpnf->p->id == p->id )
        {
            return 0;
        }
        tmpnf = tmpnf->next;
    }
    nf->next = f->head;
    f->head = nf;
    return 1;
}

/*
 * Cette fonction sert a supprimer une personne de la fonction a partir de son identifiant.
 * La demarche faite dans cette fonction est la suivant:
 * On verife que la tete de la fonction f->head ne soit pas NULL, si oui, on arrete la fonction.
 * On verifie si le premier element de la liste correspond a l'id de la personne que l'on souhaite supprimer.
 * Si oui:
 * On cree un noeud temporaire qui stockera la personne suivante dans la liste.
 * On libere l'espace memoire occupe par head avec la fonction free(f->head).
 * On attribue a f->head le noeud temporaire que l'on avait cree.
 * On arrete la fonction.
 * Sinon, on parcourt l'entierete de la liste jusqu'au moment ou l'on trouve la personne
 * qui a le meme id que l'id en parametre.
 * Si on le trouve, on pivote l'element qui suit vers l'element que l'on vient de supprimer.
 * On arrete la fonction, si reussite, on obtient 1, si pas, on obtient 0.
 */
int supprimer_personne_de_formation( formation *f, int id )
{
    formation *tmpf = f;
    noeud_formation *tmp = NULL;
    if( f == NULL )
    {
        printf( "Formation pas trouvee\n" );
        return 0;
    }
    noeud_formation *headf = tmpf->head;
    if( headf == NULL )
    {
        return 0;
    }
    if( f->head->p->id == id )
    {
        tmp = f->head->next;
        free( f->head );
        f->head = tmp;
        return 1;
    }
    while( headf != NULL )
    {
        if( headf->next == NULL )
        {
            if( headf->p->id == id )
            {
                return 1;
            }
            return 0;
        }
        if( headf->next->p->id == id )
        {
            tmp = headf->next;
            headf->next = tmp->next;
            free( tmp );
            return 1;
        }
        headf = headf->next;
    }
    return 0;
}

/*
 * Cette fonction sert a afficher les informations de base qui caracterisent une formation.
 * De maniere generale, son identifiant, son nom, son prix, ainsi que les personnes qui y participent.
 */
void afficher_formation( formation *f )
{
    formation *tmp = f;
    noeud_formation *tmpnf = tmp->head;
    printf( "ID: %d - Nom formation: %s\n", tmp->id, tmp->nom );
    printf( "Participants dans la formation:\n" );
    printf( "Formateurs:\n" );
    while( tmpnf != NULL )
    {
        if( tmpnf->p->formateur == 1 )
        {
            printf( "%2d %s %s", tmpnf->p->id, tmpnf->p->nom, tmpnf->p->prenom );
            if( tmpnf != NULL ) printf( "\n" );
        }
        tmpnf = tmpnf->next;
    }
    tmpnf = tmp->head;
    printf( "Etudiants:\n" );
    while( tmpnf != NULL )
    {
        if( tmpnf->p->formateur == 0 )
        {
            printf( "%2d %s %s", tmpnf->p->id, tmpnf->p->nom, tmpnf->p->prenom );
            if( tmpnf != NULL ) printf( "\n" );
        }
        tmpnf = tmpnf->next;
    }
    printf( "\n" );
}

/*
 * Cette fonction sert a initialiser le pointer noeud_db_formation *head dans la structure db_formation a NULL,
 * afin que l'on puisse commencer a faire des manipulations avec cette structure.
 */
db_formation *creer_db_formation()
{
    db_formation *db = ( db_formation * ) calloc( sizeof( db_formation ), sizeof( db_formation ) );
    db->head = NULL;
    return db;
}

/*
 * Cette fonction sert a initialiser un pointeur noeud_db_formation *ndb qui stockera formation *f dans
 * la base de donnees db_formation *db.
 * Ici, l'ajout dans la liste chainee a lieu par le mecanisme suivant:
 * On initialise le noeud temporaire que l'on ajoutera a la base de donnees.
 * On associe f au pointeur f present dans la structure noeud_db_formation.
 * On initialise le prochain noeud de la liste *next a NULL.
 * Si la tete *head de la base de donnee est NULL, alors la tete devient le nouveau noeud.
 * On arrete la fonction d'ajout la.
 * Sinon, on fait une copie de la tete dans le noeud *next que l'on avait initialise a NULL.
 * On declare la tete comme etant le noeud temporaire que l'on a initialise.
 */
void ajouter_db_formation( db_formation *db, formation *f )
{
    noeud_db_formation *ndb = ( noeud_db_formation * ) calloc( sizeof( noeud_db_formation ), sizeof( noeud_db_formation ) );
    ndb->f = f;
    ndb->next = NULL;
    if( db->head == NULL )
    {
        db->head = ndb;
        return;
    }
    ndb->next = db->head;
    db->head = ndb;
}

/*
 * Cette fonction sert a supprimer une formation de la base de donnees a partir de son identifiant.
 * La demarche faite dans cette fonction est la suivant:
 * On verife que la tete de la base de donnees dbf->head ne soit pas NULL, si oui, on arrete la fonction.
 * On verifie si le premier element de la liste correspond a l'id de la formation que l'on souhaite supprimer.
 * Si oui:
 * On cree un noeud temporaire qui stockera la formation suivante dans la liste.
 * On libere l'espace memoire occupe par head avec la fonction free(dbf->head).
 * On attribue a dbf->head le noeud temporaire que l'on avait cree.
 * On arrete la fonction.
 * Sinon, on parcourt l'entierete de la liste jusqu'au moment ou l'on trouve la formation qui
 * a le meme id que l'id en parametre.
 * Si on le trouve, on pivote l'element qui suit vers l'element que l'on vient de supprimer.
 * On arrete la fonction, si reussite, on obtient 1, si pas, on obtient 0.
 */
int supprimer_db_formation( db_formation *dbf, int id )
{
    noeud_db_formation *tmp = NULL;
    if( dbf->head == NULL )
    {
        return 0;
    }
    if( dbf->head->f->id == id )
    {
        tmp = dbf->head->next;
        free( dbf->head );
        dbf->head = tmp;
        return 1;
    }
    noeud_db_formation *tmpndbf = dbf->head;
    while( tmpndbf != NULL )
    {
        if( tmpndbf->next == NULL )
        {
            if( tmpndbf->f->id == id )
            {
                return 1;
            }
            return 0;
        }
        if( tmpndbf->next->f->id == id )
        {
            tmp = tmpndbf->next;
            tmpndbf->next = tmpndbf->next->next;
            free( tmp );
            return 1;
        }
        tmpndbf = tmpndbf->next;
    }
    return 0;
}

/*
 * Cette fonction renvoie NULL si une formation avec un nom specifique n'existe pas dans
 * la base de donnees db_formation *dbf.
 * Sinon, la fonction retourne la formation trouvee.
 */
formation *get_formation( db_formation *dbf, char nom_formation[] )
{
    db_formation *tmpdbf = dbf;
    noeud_db_formation *tmpndbf = tmpdbf->head;
    while( tmpndbf != NULL )
    {
        if( strcmp( tmpndbf->f->nom, nom_formation ) == 0 )
        {
            return tmpndbf->f;
        }
        tmpndbf = tmpndbf->next;
    }
    return NULL;
}

/*
 * Cette fonction sert a initialiser un pointeur noeud_db_formation *ndb qui stockera formation *f dans
 * la base de donnees db_formation *db.
 * Ici, l'ajout dans la liste chainee a lieu par le mecanisme suivant:
 * On initialise le noeud temporaire que l'on ajoutera a la base de donnees.
 * On associe f au pointeur f present dans la structure noeud_db_formation.
 * On initialise le prochain noeud de la liste *next a NULL.
 * Si la tete *head de la base de donnee est NULL, alors la tete devient le nouveau noeud.
 * On arrete la fonction d'ajout la.
 * Sinon, on fait une copie de la tete dans le noeud *next que l'on avait initialise a NULL.
 * On declare la tete comme etant le noeud temporaire que l'on a initialise.
 */
void afficher_db_formation( db_formation *dbf )
{
    int i;
    db_formation *tmpdbf = dbf;
    noeud_db_formation *tmpndbf = tmpdbf->head;
    char jour[7][9] = { "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche" };
    for( i = 1; i <= 7; i++ )
    {
        tmpndbf = tmpdbf->head;
        printf( "********************************************************************************\n" );
        printf( "Cours du: %s\n", jour[i - 1] );
        printf( "********************************************************************************\n" );
        while( tmpndbf != NULL )
        {
            formation *tmp = tmpndbf->f;
            int j;
            for( j = 0; j < tmp->nb_jours; j++ )
            {
                if( tmp->jours[j] == i )
                {
                    afficher_formation( tmp );
                    printf( "De: %.2f - A %.2f\n", tmp->heures[j], tmp->heures[j] + tmp->durees[j] );
                    printf( "Prerequis: " );
                    if( tmp->nb_prerequis > 0 )
                    {
                        db_formation *tmpdb = dbf;
                        noeud_db_formation *tmp_prereq_ndbf = tmpdb->head;
                        while( tmp_prereq_ndbf != NULL )
                        {
                            formation *tmp_prereq = tmp_prereq_ndbf->f;
                            int k;
                            for( k = 0; k < tmp->nb_prerequis; k++ )
                            {
                                if( tmp->prerequis[k] == tmp_prereq->id )
                                {
                                    printf( "%s ", tmp_prereq->nom );
                                }
                            }
                            tmp_prereq_ndbf = tmp_prereq_ndbf->next;
                        }
                        printf( "\n\n" );
                    }
                    else
                    {
                        printf( "Aucun\n\n" );
                    }
                }
            }
            tmpndbf = tmpndbf->next;
        }
    }
}

/*                             FIN FORMATION                                 */
/*****************************************************************************/

/*****************************************************************************/
/*                           FONCTIONS GENERALES                             */

void menu_creer_formation( db_formation *f )
{
    db_formation *tmpdbf = f;
    int i;
    char nom[40];
    float prix;
    printf( "* Nom de la formation: " );
    fgets( nom, 40, stdin );
    if( strlen( nom ) > 0 && nom[ strlen( nom ) - 1 ] == '\n' )
    {
        nom[ strlen( nom ) - 1 ] = '\0';
    }
    formation *tmp = get_formation( f, nom );
    if( tmp == NULL )
    {
        printf( "* Cout de la formation: " );
        while( scanf( "%f", &prix ) != 1 )
        {
            printf( "* Option %.2f - INVALIDE: Min 0\n", prix );
            printf( "* Cout de la formation: " );
            scanf( "%f", &prix );
            getchar();
        }
        while( prix < 0 )
        {
            printf( "* Option %.2f - INVALIDE: Min 0\n", prix );
            printf( "* Cout de la formation: " );
            scanf( "%f", &prix );
            getchar();
        }
        formation *tmpf = creer_formation( nom, prix );
        if( tmpdbf->head == NULL )
        {
            tmpf->id = 1;
        }
        else
        {
            tmpf->id = tmpdbf->head->f->id + 1;
        }
        char choix_prerequis[4];
        printf( "* Est-ce que la formation a des prerequis ? (o/n) " );
        scanf( "%s", choix_prerequis );
        getchar();
        while( strcmp( choix_prerequis, "o" ) != 0 && strcmp( choix_prerequis, "oui" ) != 0 &&
               strcmp( choix_prerequis, "n" ) != 0 && strcmp( choix_prerequis, "non" ) != 0 )
        {
            printf( "* Options %s - INVALIDE: o pour oui et n pour non s'il vous plait\n", choix_prerequis );
            printf( "* Est-ce que la formation a des prerequis ? (o/n) " );
            scanf( "%s", choix_prerequis );
            getchar();
        }
        if( strcmp( choix_prerequis, "o" ) == 0 || strcmp( choix_prerequis, "oui" ) == 0 )
        {
            noeud_db_formation *tmpndbf = tmpdbf->head;
            printf( "* %2s %-40s                                  *\n", "ID", "Nom" );
            printf( "* ---------------------------------------------------------------------------- *\n" );
            while( tmpndbf != NULL )
            {
                printf( "* %2d %-40s                                  *\n", tmpndbf->f->id, tmpndbf->f->nom );
                tmpndbf = tmpndbf->next;
            }
            tmpndbf = tmpdbf->head;
            int nb_prerequis;
            printf( "* Combien de prerequis faut-il ? " );
            while( scanf( "%d", &nb_prerequis ) != 1 )
            {
                printf( "* Option %d - INVALIDE: Max %d prerequis Min 0\n", nb_prerequis, tmpndbf->f->id );
                printf( "* Combien de prerequis faut-il ? " );
                scanf( "%d", &nb_prerequis );
                getchar();
            }
            getchar();
            while( nb_prerequis > tmpndbf->f->id || nb_prerequis < 0 )
            {
                printf( "* Option %d - INVALIDE: Max %d prerequis Min 0\n", nb_prerequis, tmpndbf->f->id );
                printf( "* Combien de prerequis faut-il ? " );
                scanf( "%d", &nb_prerequis );
                getchar();
            }
            tmpf->nb_prerequis = nb_prerequis;
            for( i = 0; i < nb_prerequis; i++ )
            {
                int id_prerequis;
                printf( "* ID du prerequis N.%d a rajouter: ", i + 1 );
                while( scanf( "%d", &id_prerequis ) != 1 || nb_prerequis > tmpndbf->f->id )
                {
                    printf( "* Option %d - INVALIDE: veullez choisir 1 dans la liste\n", id_prerequis );
                    printf( "* ID du prerequis N.%d a rajouter: ", i + 1 );
                    scanf( "%d", &id_prerequis );
                    getchar();
                }
                tmpf->prerequis[i] = id_prerequis;
            }
        }
        int nb_jours;
        printf( "* Combien de fois par semaine a la formation lieu ? " );
        while( scanf( "%d", &nb_jours ) != 1 )
        {
            printf( "Option %d - INVALIDE: Max 7 jours Min 1 jour\n", nb_jours );
            printf( "* Combien de fois par semaine a la formation lieu ? " );
            scanf( "%d", &nb_jours );
            getchar();
        }
        getchar();
        while( nb_jours > 7 || nb_jours <= 0 )
        {
            printf( "Option %d - INVALIDE: Max 7 jours Min 1 jour\n", nb_jours );
            printf( "* Combien de fois par semaine a laformation lieu ? " );
            scanf( "%d", &nb_jours );
            getchar();
        }
        tmpf->nb_jours = nb_jours;
        printf( "* 1. lundi\n* 2. mardi\n* 3. mercredi\n* 4. jeudi\n* 5. vendredi\n* 6. samedi\n* 7. dimanche\n" );
        for ( i = 0; i < tmpf->nb_jours; i++ )
        {
            int jour;
            printf( "* Quel jour de la semaine a la formation N. %d lieu ? ", i + 1 );
            while( scanf( "%d", &jour ) != 1 || jour < 0 || jour > 7  )
            {
                printf( "* Option %d - INVALIDE: Max 7 Min 1\n", i );
                printf( "* Quel jour de la semaine a la formation N. %d lieu ? ", i + 1 );
                scanf( "%d", &jour );
                getchar();
            }
            tmpf->jours[i] = jour;
            float heure;
            printf( "* A quelle heure debute la formation (ex. 08.15) ? " );
            while( scanf( "%f", &heure ) != 1 || heure > 18 || heure < 6 )
            {
                printf( "* Option %.2f - INVALIDE: MIN 6 MAX 18\n", heure );
                printf( "* A quelle heure debute la formation (ex. 08.15) ? " );
                scanf( "%f", &heure);
                getchar();
            }
            tmpf->heures[i] = heure;
            float duree;
            printf( "* Combien d'heures dure la formation ? " );
            while( scanf( "%f", &duree ) != 1 || duree > 8 || duree < 1 )
            {
                printf( "* Option %.2f - INVALIDE: MIN 1 heures MAX 8 heures\n", duree );
                printf( "* Combien d'heures dure la formation ? " );
                scanf( "%f", &duree);
                getchar();
            }
            tmpf->durees[i] = duree;
        }
        char confirmation[4];
        printf( "* Etes vous sur de vouloir ajouter la formation %s avec prix %.2f a la base de donnees ? (o/n) ",
                tmpf->nom, tmpf->prix );
        scanf( "%s", confirmation );
        while( strcmp( confirmation, "o" ) != 0 && strcmp( confirmation, "oui" ) != 0 &&
               strcmp( confirmation, "n" ) != 0 && strcmp( confirmation, "non" ) != 0 )
        {
            printf( "Veuillez inserer o / oui - n / non : " );
            scanf( "%s", confirmation );
        }
        if( strcmp( confirmation, "o" ) == 0 || strcmp( confirmation, "oui" ) == 0 )
        {
            ajouter_db_formation( tmpdbf, tmpf );
            system( clear );
            printf( "* %s a ete ajoutee a la base de donnees avec succes *\n", tmpf->nom );
        }
        else
        {
            system( clear );
            printf( "* %s n'a PAS ete ajoutee a la base de donnees *\n", tmpf->nom );
        }
    }
    else
    {
        system( clear );
        printf( "* Formation deja existente dans la base de donnees *\n" );
    }
}

void menu_creer_personne( db_personne *p )
{
    db_personne *tmpdbp = p;
    char nom[25], prenom[25], choix_formateur[4];
    int formateur, nb_jours_indisponible = 0, jours_indisponible[7], reduction = 0, pourcent_reduction;
    printf( "* Nom de famille de la personne: " );
    scanf( "%s", nom );
    printf( "* Prenom de la personne: " );
    scanf( "%s", prenom );
    printf( "* Est-ce que cette personne est un formateur ou un etudiant ? (f/e) " );
    scanf( "%s", choix_formateur );
    int i;
    for( i = 0; choix_formateur[i]; i++ )
    {
        choix_formateur[i] = tolower( choix_formateur[i] );
    }
    while( strcmp( choix_formateur, "f" ) != 0 && strcmp( choix_formateur, "formateur" ) != 0 &&
           strcmp( choix_formateur, "e" ) != 0 && strcmp( choix_formateur, "etudiant" ) != 0 )
    {
        printf( "* Option %s - INVALIDE: f ou e seulement sont acceptees                       *\n",
                choix_formateur );
        printf( "* Est-ce que cette personne est un formateur ou un etudiant ? (f/e) " );
        scanf( "%s", choix_formateur );
        for( i = 0; choix_formateur[i]; i++ )
        {
            choix_formateur[i] = tolower( choix_formateur[i] );
        }
    }
    if( strcmp( choix_formateur, "f" ) == 0 || strcmp( choix_formateur, "formateur" ) == 0 )
    {
        formateur = 1;
        char choix_indisponible[4];
        printf( "* Est-ce que ce formateur as des jours ou il/elle est indisponible ? (o/n) " );
        scanf( "%s", choix_indisponible );
        getchar();
        while( strcmp( choix_indisponible, "o" ) != 0 && strcmp( choix_indisponible, "oui" ) != 0 &&
               strcmp( choix_indisponible, "n" ) != 0 && strcmp( choix_indisponible, "non" ) != 0 )
        {
            printf( "* Option %s - INVALIDE: o ou n sont acceptees                                  *\n",
                    choix_indisponible );
            printf( "* Est-ce que ce formateur as des jours ou il/elle est indisponible ? (o/n) " );
            scanf( "%s", choix_indisponible );
            getchar();
        }
        if( strcmp( choix_indisponible, "o" ) == 0 || strcmp( choix_indisponible, "oui" ) == 0 )
        {
            printf( "* Combien de jours serait ce formateur indisponible ? " );
            scanf( "%d", &nb_jours_indisponible );
            getchar();
            while( nb_jours_indisponible < 0 )
            {
                printf( "* Option %d - INVALIDE: Max 7 jours - Min 0                                    *\n",
                        nb_jours_indisponible );
                printf( "* Combien de jours serait ce formateur indisponible ? " );
                scanf( "%d", &nb_jours_indisponible );
                getchar();
            }
            if( nb_jours_indisponible > 0 )
            {
                while ( nb_jours_indisponible > 7 || nb_jours_indisponible < 0)
                {
                    printf( "* Option %d - INVALIDE: Max 7 jours - Min 0                                    *\n",
                            nb_jours_indisponible );
                    printf( "* Combien de jours serait ce formateur indisponible ? " );
                    scanf( "%d", &nb_jours_indisponible );
                    getchar();
                }
                printf( "* 1. lundi\n* 2. mardi\n* 3. mercredi\n* 4. jeudi\n* 5. vendredi\n* 6. samedi\n* 7. dimanche\n" );
                for ( i = 0; i < nb_jours_indisponible; i++ )
                {
                    int jour;
                    printf( "* Quel serait son jour d'indisponibilite N.%d ? ", i + 1 );
                    scanf( "%d", &jour );
                    jours_indisponible[ i ] = jour;
                }
            }
        }
    }
    else
    {
        formateur = 0;
        printf( "* Est-ce que cet etudiant beneficie d'une reduction ? (o/n) " );
        char choix_reduction[4];
        scanf( "%s", choix_reduction );
        getchar();
        while( strcmp( choix_reduction, "o" ) != 0 && strcmp( choix_reduction, "oui" ) != 0 &&
               strcmp( choix_reduction, "n" ) != 0 && strcmp( choix_reduction, "non" ) != 0 )
        {
            printf( "* Option %s - INVALIDE: o ou n sont acceptees                                  *\n",
                    choix_reduction );
            printf( "* Est-ce que cet etudiant beneficie d'une reduction ? (o/n) " );
            scanf( "%s", choix_reduction );
            getchar();
        }
        if( strcmp( choix_reduction, "o" ) == 0 || strcmp( choix_reduction, "oui" ) == 0 )
        {
            printf( "* Combien de pourcentage de reduction beneficie cet etudiant ? " );
            scanf( "%d", &pourcent_reduction );
            getchar();
            while ( pourcent_reduction > 100 || pourcent_reduction < 0 )
            {
                printf( "* Option %d - INVALIDE: Max 100 pourcent - Min 0                               *\n",
                        nb_jours_indisponible );
                printf( "* Combien de pourcentage de reduction beneficie cet etudiant ? " );
                scanf( "%d", &pourcent_reduction );
            }
            if( pourcent_reduction > 0 )
            {
                reduction = 1;
            }
        }
    }
    personne *tmpp = creer_personne( nom, prenom, formateur );
    if( tmpdbp->head == NULL )
    {
        tmpp->id = 1;
    }
    else
    {
        tmpp->id = tmpdbp->head->p->id + 1;
    }
    if( tmpp->formateur == 0 )
    {
        tmpp->reduction = reduction;
        if( reduction == 1 )
        {
            tmpp->val_reduction = pourcent_reduction;
        }
    }
    else
    {
        tmpp->nb_jours_indisponible = nb_jours_indisponible;
        if( nb_jours_indisponible > 0 )
        {
            memcpy( tmpp->jours_indisponible, jours_indisponible, sizeof(tmpp->jours_indisponible) );
        }
    }
    char confirmation[4];
    printf( "* Etes vous sur de vouloir ajouter %s: %s %s a la base de donnees ? (o/n) ",
            tmpp->formateur ? "formateur" : "etudiant", tmpp->prenom, tmpp->nom );
    scanf( "%s", confirmation );
    while( strcmp( confirmation, "o" ) != 0 && strcmp( confirmation, "oui" ) != 0 &&
           strcmp( confirmation, "n" ) != 0 && strcmp( confirmation, "non" ) != 0 )
    {
        printf( "Veuillez inserer o / oui - n / non : " );
        scanf( "%s", confirmation );
    }
    if( strcmp( confirmation, "o" ) == 0 || strcmp( confirmation, "oui" ) == 0 )
    {
        ajouter_db_personne( p, tmpp );
        system( clear );
        printf( "* %s %s a ete ajoute(e) a la base de donnees avec succes *\n", tmpp->nom, tmpp->prenom );
    }
    else
    {
        system( clear );
        printf( "* %s %s n'a PAS ete ajoute(e) a la base de donnees *\n", tmpp->nom, tmpp->prenom );
    }
}

int menu_creer( db_formation *f, db_personne *p )
{
    int choix;
    do
    {
        db_formation *tmpdbf = f;
        db_personne *tmpdbp = p;
        printf( "********************************************************************************\n" );
        printf( "* MENU AJOUT                                                                   *\n" );
        printf( "********************************************************************************\n" );
        printf( "* 1. Ajouter une nouvelle personne a la base de donnees                        *\n" );
        printf( "* 2. Ajouter une nouvelle formation a la base de donneees                      *\n" );
        printf( "* 0. Retour                                                                    *\n" );
        printf( "********************************************************************************\n" );
        printf( "* Que voudriez-vous ajouter a la base de donnees ? " );
        scanf( "%d", &choix );
        getchar();
        switch( choix )
        {
            case 1:
                menu_creer_personne( tmpdbp );
                break;
            case 2:
                menu_creer_formation( tmpdbf );
                break;
            case 0:
                system( clear );
                break;
            default:
                system( clear );
                printf( "/!\\ Option %d - INVALIDE /!\\\n", choix );
                break;
        }
    } while( choix != 0 );
    return 0;
}

void menu_ajouter_formation( db_formation *f, db_personne *p )
{
    db_formation *tmpdbf =f;
    noeud_db_formation *tmpndbf = tmpdbf->head;
    db_personne *tmpdbp = p;
    noeud_db_personne *tmpndbp = tmpdbp->head;
    printf( "********************************************************************************\n" );
    printf( "* MENU ATTRIBUTION                                                             *\n" );
    printf( "********************************************************************************\n" );
    printf( "********************************************************************************\n" );
    printf( "* Liste des cours                                                              *\n" );
    printf( "********************************************************************************\n" );
    printf( "* %2s %-40s                                  *\n", "ID", "Nom" );
    printf( "* ---------------------------------------------------------------------------- *\n" );
    while( tmpndbf != NULL )
    {
        formation *tmpf = tmpndbf->f;
        printf( "* %2d %-40s                                  *\n", tmpf->id, tmpf->nom );
        tmpndbf = tmpndbf->next;
    }
    tmpndbf = tmpdbf->head;
    printf( "*  0 Retour                                                                    *\n" );
    printf( "********************************************************************************\n" );
    int cours;
    printf( "* A quelle formation voudriez vous attribuer une personne? " );
    scanf( "%d", &cours );
    getchar();
    while( cours < 0 && cours > tmpndbf->f->id )
    {
        printf( "* Valeur %d - INVALIDE! *\n", cours );
        printf( "* A quelle formation voudriez vous attribuer une personne? " );
        scanf( "%d", &cours );
        getchar();
    }
    if( cours <= 0 )
    {
        system( clear );
        return;
    }
    while( tmpndbf != NULL )
    {
        if( cours == tmpndbf->f->id )
        {
            printf( "********************************************************************************\n" );
            printf( "* Liste des personnes                                                          *\n" );
            printf( "********************************************************************************\n" );
            printf( "* Formation choisie: %-40s                  *\n", tmpndbf->f->nom );
            printf( "********************************************************************************\n" );
            afficher_db_personne( tmpdbp );
            printf( "*  0 Retour                                                                    *\n" );
            printf( "********************************************************************************\n" );
            int p;
            printf( "* Qui voudriez vous attribuer a la formation : %s ? ", tmpndbf->f->nom );
            scanf( "%d", &p );
            getchar();
            while( p < 0 && p > tmpndbp->p->id )
            {
                printf( "* Valeur %d - INVALIDE\n", p );
                printf( "* Qui voudriez vous attribuer a la formation : %s ? ", tmpndbf->f->nom );
                scanf( "%d", &p );
                getchar();
            }
            if( p <= 0 )
            {
                system( clear );
                return;
            }
            while ( tmpndbp != NULL )
            {
                if ( p == tmpndbp->p->id )
                {
                    char confirmation[4];
                    printf( "* Etes vous sur de vouloir attribuer %s %s a la formation %s ? (o/n) ",
                            tmpndbp->p->nom, tmpndbp->p->prenom, tmpndbf->f->nom );
                    scanf( "%s", confirmation );
                    while( strcmp( confirmation, "o" ) != 0 && strcmp( confirmation, "oui" ) != 0 &&
                           strcmp( confirmation, "n" ) != 0 && strcmp( confirmation, "non" ) != 0 )
                    {
                        printf( "Veuillez inserer o / oui - n / non : " );
                        scanf( "%s", confirmation );
                    }
                    if( strcmp( confirmation, "o" ) == 0 || strcmp( confirmation, "oui" ) == 0 )
                    {
                        int res = ajouter_formation( tmpndbf->f, tmpndbp->p );
                        if ( res == 1 )
                        {
                            tmpndbp->p->nb_formations += 1;
                            tmpndbp->p->formations[ tmpndbp->p->nb_formations - 1 ] = tmpndbf->f->id;
                            system( clear );
                            printf( "* %s %s a ete attribue(e) a la formation %s avec succes *\n",
                                    tmpndbp->p->nom, tmpndbp->p->prenom, tmpndbf->f->nom );
                            break;
                        }
                        system( clear );
                        printf( "* %s %s est deja present dans la formation %s *\n" ,
                                tmpndbp->p->nom, tmpndbp->p->prenom, tmpndbf->f->nom );
                        break;
                    }
                    else
                    {
                        system( clear );
                        printf( "* %s %s n'a PAS ete attribue(e) a la formation %s *\n" ,
                                tmpndbp->p->nom, tmpndbp->p->prenom, tmpndbf->f->nom );
                    }
                }
                tmpndbp = tmpndbp->next;
            }
            break;
        }
        tmpndbf = tmpndbf->next;
    }
}

void menu_supprimer_personne( db_formation *dbf, db_personne *dbp )
{
    int idp;
    db_formation *tmpdbf = dbf;
    noeud_db_formation *tmpndbf = tmpdbf->head;
    db_personne *tmpdbp = dbp;
    noeud_db_personne *tmpndbp = tmpdbp->head;
    afficher_db_personne( tmpdbp );
    printf( "*  0 Retour                                                                    *\n" );
    printf( "* Quelle personne voudriez vous supprimer entierement ? " );
    scanf( "%d", &idp );
    getchar();
    while( idp < 0 && idp > tmpndbp->p->id )
    {
        printf( "* Option %d - INVALIDE\n", idp );
        printf( "* Quelle personne voudriez vous supprimer entierement ? " );
        scanf( "%d", &idp );
        getchar();
    }
    if( idp <= 0 )
    {
        system( clear );
        return;
    }
    while( tmpndbf != NULL )
    {
        formation *tmpf = tmpndbf->f;
        noeud_formation *tmpnf = tmpf->head;
        while( tmpnf != NULL )
        {
            if( tmpnf->p->id == idp )
            {
                supprimer_personne_de_formation( tmpf, tmpnf->p->id );
                break;
            }
            tmpnf = tmpnf->next;
        }
        tmpndbf = tmpndbf->next;
    }
    tmpndbf = tmpdbf->head;
    tmpndbp = tmpdbp->head;
    while( tmpndbp != NULL )
    {
        if( tmpndbp->p->id == idp )
        {
            char nom[40], prenom[40];
            strcpy( nom, tmpndbp->p->nom );
            strcpy( prenom, tmpndbp->p->prenom );
            char confirmation[4];
            printf( "* Etes vous sur de vouloir supprimer %s %s entierement de la base de donnees ? (o/n) ",
                    nom, prenom );
            scanf( "%s", confirmation );
            while( strcmp( confirmation, "o" ) != 0 && strcmp( confirmation, "oui" ) != 0 &&
                   strcmp( confirmation, "n" ) != 0 && strcmp( confirmation, "non" ) != 0 )
            {
                printf( "Veuillez inserer o / oui - n / non : " );
                scanf( "%s", confirmation );
            }
            if( strcmp( confirmation, "o" ) == 0 || strcmp( confirmation, "oui" ) == 0 )
            {
                supprimer_db_personne( tmpdbp, idp );
                system( clear );
                printf( "* %s %s a ete supprime(e) entierement de la base de donnees *\n",
                        nom, prenom );
            }
            else
            {
                system( clear );
                printf( "* %s %s n'a PAS ete supprimer de la base de donnees *\n",
                        nom, prenom );
            }
            break;
        }
        tmpndbp = tmpndbp->next;
    }
    tmpndbf = tmpdbf->head;
    tmpndbp = tmpdbp->head;
}

void menu_supprimer_formation( db_formation *dbf, db_personne *dbp )
{
    int idf;
    db_formation *tmpdbf = dbf;
    noeud_db_formation *tmpndbf = tmpdbf->head;
    db_personne *tmpdbp = dbp;
    noeud_db_personne *tmpndbp = tmpdbp->head;
    printf( "********************************************************************************\n" );
    printf( "* MENU SUPPRESSION : Liste des formations                                      *\n" );
    printf( "********************************************************************************\n" );
    printf( "* %2s %-40s                                  *\n", "ID", "Nom" );
    printf( "* ---------------------------------------------------------------------------- *\n" );
    while( tmpndbf != NULL )
    {
        formation *tmpf = tmpndbf->f;
        printf( "* %2d %-40s                                  *\n", tmpf->id, tmpf->nom );
        tmpndbf = tmpndbf->next;
    }
    tmpndbf = tmpdbf->head;
    printf( "*  0 Retour                                                                    *\n" );
    printf( "********************************************************************************\n" );
    printf( "* Quelle formation voudriez vous supprimer? " );
    scanf( "%d", &idf );
    getchar();
    printf( "********************************************************************************\n" );
    while( idf < 0 && idf > tmpndbf->f->id )
    {
        printf( "* Option %d - INVALIDE\n", idf );
        printf( "* Quelle formation voudriez vous supprimer? " );
        scanf( "%d", &idf );
        getchar();
    }
    if( idf <= 0 )
    {
        system( clear );
        return;
    }
    while( tmpndbf != NULL )
    {
        formation *tmpf = tmpndbf->f;
        if( idf == tmpf->id )
        {
            char confirmation[4];
            printf( "* Etes vous sur de vouloir supprimer %s entierement de la base de donnees ? (o/n) ",
                    tmpf->nom );
            scanf( "%s", confirmation );
            while( strcmp( confirmation, "o" ) != 0 && strcmp( confirmation, "oui" ) != 0 &&
                   strcmp( confirmation, "n" ) != 0 && strcmp( confirmation, "non" ) != 0 )
            {
                printf( "Veuillez inserer o / oui - n / non : " );
                scanf( "%s", confirmation );
            }
            if( strcmp( confirmation, "o" ) == 0 || strcmp( confirmation, "oui" ) == 0 )
            {
                supprimer_db_formation( tmpdbf, idf );
                while ( tmpndbp != NULL )
                {
                    personne *tmpp = tmpndbp->p;
                    int k;
                    for ( k = 0; k < tmpp->nb_formations - 1; k++ )
                    {
                        if ( tmpp->formations[ k ] == tmpf->id )
                        {
                            int l;
                            for ( l = k; l < tmpp->nb_formations; l++ )
                            {
                                tmpp->formations[ l ] = tmpp->formations[ l + 1 ];
                            }
                            tmpp->nb_formations -= 1;
                            break;
                        }
                    }
                    tmpndbp = tmpndbp->next;
                }
                system( clear );
                printf( "* %s a ete supprimee de la base de donnees *\n", tmpf->nom );
            }
            else
            {
                system( clear );
                printf( "* %s n'a PAS ete supprimee de la base de donnees *\n", tmpf->nom );
            }
            break;
        }
        tmpndbf = tmpndbf->next;
    }
    tmpndbf = tmpdbf->head;
}

int menu_supprimer_personne_de_formation( db_formation *dbf )
{
    int idf;
    db_formation *tmpdbf = dbf;
    noeud_db_formation *tmpndbf = tmpdbf->head;
    printf( "********************************************************************************\n" );
    printf( "* MENU SUPPRESSION : Liste des formations                                      *\n" );
    printf( "********************************************************************************\n" );
    while ( tmpndbf != NULL )
    {
        formation *tmpf = tmpndbf->f;
        printf( "* %2d %-40s                                  *\n", tmpf->id, tmpf->nom );
        tmpndbf = tmpndbf->next;
    }
    tmpndbf = tmpdbf->head;
    printf( "*  0 Retour                                                                    *\n" );
    printf( "* De quelle formation voudriez vous supprimer quelqu'un ? " );
    scanf( "%d", &idf );
    getchar();
    while( idf < 0 && idf > tmpndbf->f->id )
    {
        printf( "* Option %d - INVALIDE\n", idf );
        printf( "* De quelle formation voudriez vous supprimer quelqu'un ? " );
        scanf( "%d", &idf );
        getchar();
    }
    if ( idf <= 0 )
    {
        system( clear );
        return 0;
    }
    while( tmpndbf != NULL )
    {
        if( idf == tmpndbf->f->id )
        {
            printf( "Cours choisi: %s\n", tmpndbf->f->nom );
            formation *tmpf = tmpndbf->f;
            noeud_formation *tmpnf = tmpf->head;
            if( tmpnf == NULL )
            {
                system( clear );
                printf( "* /!\\ La formation est vide /!\\                                        *\n" );
                return 0;
            }
            while( tmpnf != NULL )
            {
                personne *tmpp = tmpnf->p;
                printf( "* %2d %-25s %-25s %-10s            *\n",
                        tmpp->id, tmpp->nom, tmpp->prenom, tmpp->formateur ? "Formateur" : "Etudiant" );
                tmpnf = tmpnf->next;
            }
            tmpnf = tmpf->head;
            printf( "*  0 Retour                                                                    *\n" );
            int idp;
            printf( "* Quelle personne voudriez vous supprimer de cette formation ? " );
            scanf( "%d", &idp );
            getchar();
            while( idp < 0 && idp > tmpnf->p->id )
            {
                printf( "* Option %d - INVALIDE\n", idp );
                printf( "* Quelle personne voudriez vous supprimer de cette formation ? " );
                scanf( "%d", &idp );
                getchar();
            }
            if( idp <= 0 )
            {
                system( clear );
                return 0;
            }
            while( tmpnf != NULL )
            {
                personne *tmpp = tmpnf->p;
                if( tmpp->id == idp )
                {
                    char confirmation[4];
                    printf( "* Etes vous sur de vouloir supprimer %s %s de la formation %s ? (o/n) ",
                            tmpp->nom, tmpp->prenom, tmpf->nom );
                    scanf( "%s", confirmation );
                    while( strcmp( confirmation, "o" ) != 0 && strcmp( confirmation, "oui" ) != 0 &&
                           strcmp( confirmation, "n" ) != 0 && strcmp( confirmation, "non" ) != 0 )
                    {
                        printf( "Veuillez inserer o / oui - n / non : " );
                        scanf( "%s", confirmation );
                    }
                    if( strcmp( confirmation, "o" ) == 0 || strcmp( confirmation, "oui" ) == 0 )
                    {
                        supprimer_personne_de_formation( tmpf, tmpp->id );
                        int k;
                        for ( k = 0; k < tmpp->nb_formations - 1; k++ )
                        {
                            if ( tmpp->formations[ k ] == tmpf->id )
                            {
                                int l;
                                for ( l = k; l < tmpp->nb_formations; l++ )
                                {
                                    tmpp->formations[ l ] = tmpp->formations[ l + 1 ];
                                }
                                tmpp->nb_formations -= 1;
                                break;
                            }
                        }
                        system( clear );
                        printf( "* %s %s a ete supprime de la formation %s avec succes *\n",
                                tmpp->nom, tmpp->prenom, tmpf->nom );
                        printf( "\n\n" );
                        afficher_formation( tmpf );
                        return 1;
                    }
                    else
                    {
                        system( clear );
                        printf( "* %s %s n'a PAS ete supprime de la formation %s *\n",
                                tmpp->nom, tmpp->prenom, tmpf->nom );
                    }
                }
                tmpnf = tmpnf->next;
            }
            tmpnf = tmpf->head;
        }
        tmpndbf = tmpndbf->next;
    }
    tmpndbf = tmpdbf->head;
    return 0;
}

int menu_supprimer( db_formation *dbf, db_personne *dbp )
{
    int choix;
    do
    {
        db_formation *tmpdbf = dbf;
        db_personne *tmpdbp = dbp;
        printf( "********************************************************************************\n" );
        printf( "* MENU SUPPRESSION                                                             *\n" );
        printf( "********************************************************************************\n" );
        printf( "* 1. Supprimer une personne de la base de donnees                              *\n" );
        printf( "* 2. Supprimer une formation de la base de donnes                              *\n" );
        printf( "* 3. Supprimer une personne specifique d'une formation specifique              *\n" );
        printf( "* 0. Retour                                                                    *\n" );
        printf( "********************************************************************************\n" );
        printf ("* Que voudriez vous supprimer ? " );
        scanf( "%d", &choix );
        getchar();
        switch( choix )
        {
            case 1:
                menu_supprimer_personne( tmpdbf, tmpdbp );
                break;
            case 2:
                menu_supprimer_formation( tmpdbf, tmpdbp );
                break;
            case 3:
                menu_supprimer_personne_de_formation( tmpdbf );
                break;
            case 0:
                system( clear );
                break;
            default:
                system( clear );
                break;
        }
    } while( choix != 0 );
    return 0;
}

int menu_affichage( db_formation *f, db_personne *p )
{
    int choix;
    do
    {
        db_formation *tmpdbf = f;
        db_personne *tmpdbp = p;
        noeud_db_formation *tmpndbf = tmpdbf->head;
        printf( "********************************************************************************\n" );
        printf( "* MENU AFFICHAGE                                                               *\n" );
        printf( "********************************************************************************\n" );
        printf( "* 1. Liste des personnes                                                       *\n" );
        printf( "* 2. Liste des formations                                                      *\n" );
        printf( "* 3. Planning de la semaine                                                    *\n" );
        printf( "* 0. Retour                                                                    *\n" );
        printf( "********************************************************************************\n" );
        printf( "* Que voudriez-vous afficher ? " );
        scanf( "%d", &choix );
        getchar();
        switch( choix )
        {
            case 1:
                system( clear );
                afficher_db_personne( tmpdbp );
                break;
            case 2:
                system( clear );
                printf( "* %2s %-40s %-6s                           *\n", "ID", "Nom", "Prix" );
                printf( "* ---------------------------------------------------------------------------- *\n" );
                while( tmpndbf != NULL )
                {
                    printf( "* %2d %-40s %6.2f                           *\n",
                            tmpndbf->f->id, tmpndbf->f->nom, tmpndbf->f->prix );
                    tmpndbf = tmpndbf->next;
                }
                tmpndbf = tmpdbf->head;
                break;
            case 3:
                system( clear );
                afficher_db_formation( tmpdbf );
                break;
            case 0:
                system( clear );
                break;
            default:
                system( clear );
                printf( "/!\\ Option %d - INVALIDE /!\\\n", choix );
                break;
        }
    } while( choix != 0 );
    return 0;
}

void ecrire_planning( db_formation *dbf )
{
    FILE *fres = fopen( "CaculliTyranowski.res", "w" );
    int i;
    db_formation *tmpdbf = dbf;
    noeud_db_formation *tmpndbf = tmpdbf->head;
    char jour[7][9] = { "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche" };
    for( i = 1; i <= 7; i++ )
    {
        tmpndbf = tmpdbf->head;
        fprintf( fres, "********************************************************************************\n" );
        fprintf( fres, "Cours du: %s\n", jour[i - 1] );
        fprintf( fres, "********************************************************************************\n" );
        while( tmpndbf != NULL )
        {
            formation *tmp = tmpndbf->f;
            int j;
            for( j = 0; j < tmp->nb_jours; j++ )
            {
                if( tmp->jours[j] == i )
                {
                    formation *tmp = tmpndbf->f;
                    noeud_formation *tmpnf = tmp->head;
                    fprintf( fres, "ID: %d - Nom formation: %s\n", tmp->id, tmp->nom );
                    fprintf( fres, "Participants dans la formation:\n" );
                    fprintf( fres, "Formateurs:\n" );
                    while( tmpnf != NULL )
                    {
                        if( tmpnf->p->formateur == 1 )
                        {
                            fprintf( fres, "%2d %s %s", tmpnf->p->id, tmpnf->p->nom, tmpnf->p->prenom );
                            if( tmpnf != NULL ) fprintf( fres, "\n" );
                        }
                        tmpnf = tmpnf->next;
                    }
                    tmpnf = tmp->head;
                    fprintf( fres, "Etudiants:\n" );
                    while( tmpnf != NULL )
                    {
                        if( tmpnf->p->formateur == 0 )
                        {
                            fprintf( fres, "%2d %s %s", tmpnf->p->id, tmpnf->p->nom, tmpnf->p->prenom );
                            if( tmpnf != NULL ) fprintf( fres, "\n" );
                        }
                        tmpnf = tmpnf->next;
                    }
                    fprintf( fres, "\n" );
                    fprintf( fres, "De: %.2f - A %.2f\n", tmp->heures[j], tmp->heures[j] + tmp->durees[j] );
                    fprintf( fres, "Prerequis: " );
                    if( tmp->nb_prerequis > 0 )
                    {
                        db_formation *tmpdb = dbf;
                        noeud_db_formation *tmp_prereq_ndbf = tmpdb->head;
                        while( tmp_prereq_ndbf != NULL )
                        {
                            formation *tmp_prereq = tmp_prereq_ndbf->f;
                            int k;
                            for( k = 0; k < tmp->nb_prerequis; k++ )
                            {
                                if( tmp->prerequis[k] == tmp_prereq->id )
                                {
                                    fprintf( fres, "%s ", tmp_prereq->nom );
                                }
                            }
                            tmp_prereq_ndbf = tmp_prereq_ndbf->next;
                        }
                        fprintf( fres, "\n\n" );
                    }
                    else
                    {
                        fprintf( fres, "Aucun\n\n" );
                    }
                }
            }
            tmpndbf = tmpndbf->next;
        }
    }
    fclose( fres );
}

/*
 * Menu permettant a l'utilisateur d'interagir avec le programme
 */
int menu( db_formation *f, db_personne *p )
{
    int choix;
    do
    {
        printf( "********************************************************************************\n" );
        printf( "* MENU PRINCIPALE                                                              *\n" );
        printf( "********************************************************************************\n" );
        db_formation *tmpdbf = f;
        db_personne *tmpdbp = p;
        printf( "* 1: Afficher la liste des etudiants/formateurs ou la liste des formations     *\n" );
        printf( "* 2: Ajouter une nouvelle personne ou formation a la base de donnees           *\n" );
        printf( "* 3: Attribuer une personne a une formation                                    *\n" );
        printf( "* 4: Supprimer une formation, une personne ou une personne d'une formation     *\n" );
        printf( "* 0: Quitter le programme                                                      *\n" );
        printf( "********************************************************************************\n" );
        printf( "* Que voudriez-vous faire ? " );
        scanf( "%d", &choix );
        getchar();
        switch ( choix )
        {
            case 1:
                system( clear );
                menu_affichage( tmpdbf, tmpdbp );
                break;
            case 2:
                system( clear );
                menu_creer( tmpdbf, tmpdbp );
                break;
            case 3:
                system( clear );
                menu_ajouter_formation( tmpdbf, tmpdbp );
                break;
            case 4:
                system( clear );
                menu_supprimer( tmpdbf, tmpdbp );
                break;
            case 0:
                printf( "Voulez vous sauvegarder les changements ? (o/n) " );
                char choix_sauvegarde[4];
                scanf( "%s", choix_sauvegarde );
                if( strcmp( choix_sauvegarde, "o" ) == 0 || strcmp( choix_sauvegarde, "oui" ) == 0 )
                {
                    FILE *fdat_f = fopen( "CaculliTyranowskiFormation.dat", "w" );
                    FILE *fdat_p = fopen( "CaculliTyranowskiPersonne.dat", "w" );
                    noeud_db_formation *tmpndbf = tmpdbf->head;
                    noeud_db_personne *tmpndbp = tmpdbp->head;
                    db_formation *out_f = creer_db_formation();
                    while ( tmpndbf != NULL )
                    {
                        ajouter_db_formation( out_f, tmpndbf->f );
                        tmpndbf = tmpndbf->next;
                    }
                    tmpndbf = out_f->head;
                    db_personne *out_p = creer_db_personne();
                    while ( tmpndbp != NULL )
                    {
                        ajouter_db_personne( out_p, tmpndbp->p );
                        tmpndbp = tmpndbp->next;
                    }
                    tmpndbp = out_p->head;
                    while ( tmpndbp != NULL )
                    {
                        personne *tmpp = tmpndbp->p;
                        fprintf( fdat_p, "%02d %-24s %-24s %d   %d   ",
                                 tmpp->id, tmpp->nom, tmpp->prenom, tmpp->formateur, tmpp->nb_formations );
                        int i;
                        for ( i = 0; i < tmpp->nb_formations; i++ )
                        {
                            fprintf( fdat_p, "%d ", tmpp->formations[ i ] );
                        }
                        if ( tmpp->formateur == 0 )
                        {
                            fprintf( fdat_p, "   %d   ", tmpp->reduction );
                            if ( tmpp->reduction > 0 )
                            {
                                fprintf( fdat_p, "%d", tmpp->val_reduction );
                            }
                        } else
                        {
                            fprintf( fdat_p, "  %d  ", tmpp->nb_jours_indisponible );
                            for ( i = 0; i < tmpp->nb_jours_indisponible; i++ )
                            {
                                fprintf( fdat_p, "%d ", tmpp->jours_indisponible[ i ] );
                            }
                        }
                        fprintf( fdat_p, "\n" );
                        tmpndbp = tmpndbp->next;
                    }
                    tmpndbp = tmpdbp->head;
                    while ( tmpndbf != NULL )
                    {
                        int i;
                        formation *tmpf = tmpndbf->f;
                        fprintf( fdat_f, "%02d %d ", tmpf->id, tmpf->nb_prerequis );
                        if ( tmpf->nb_prerequis > 0 )
                        {
                            for ( i = 0; i < tmpf->nb_prerequis; i++ )
                            {
                                fprintf( fdat_f, "%d ", tmpf->prerequis[ i ] );
                            }
                            fprintf( fdat_f, "%d   ", tmpf->nb_jours );
                        } else
                        {
                            fprintf( fdat_f, "  %d   ", tmpf->nb_jours );
                        }
                        for ( i = 0; i < tmpf->nb_jours; i++ )
                        {
                            fprintf( fdat_f, "%d   %.2f   %.2f   ",
                                     tmpf->jours[ i ], tmpf->heures[ i ], tmpf->durees[ i ] );
                        }
                        fprintf( fdat_f, "%.2f %-s\n", tmpf->prix, tmpf->nom );
                        tmpndbf = tmpndbf->next;
                    }
                    tmpndbf = tmpdbf->head;
                    fclose( fdat_f );
                    fclose( fdat_p );
                    ecrire_planning( tmpdbf );
                    printf( "Changements sauvegardes!\n" );
                }
                printf( "Fermeture du programme...\n" );
                printf( "Au revoir!\n" );
                break;
            default:
                system( clear );
                printf( "/!\\ Option %d - INVALIDE /!\\\n", choix );
                break;
        }
    } while ( choix != 0 );
    return 0;
}

int main( void )
{
    system( clear );
    printf( "Projet par Giorgio Caculli et Jedrzej Tyranowski\n" );

    FILE *fdat_f = fopen( "CaculliTyranowskiFormation.dat", "r" );
    FILE *fdat_p = fopen( "CaculliTyranowskiPersonne.dat", "r" );

    db_personne *dbp = creer_db_personne();
    db_formation *dbf = creer_db_formation();

    int i = 1;

    while( !feof( fdat_p ) )
    {
        char nom[25], prenom[25];
        int formateur;
        int id;
        fscanf( fdat_p, "%d %24s %24s %d", &id, nom, prenom, &formateur );
        if( feof( fdat_p ) )
        {
            break;
        }
        personne *tmp = creer_personne( nom, prenom, formateur );
        fscanf( fdat_p, "%d", &tmp->nb_formations );
        tmp->id = id;
        int j;
        for( j = 0; j < tmp->nb_formations; j++ )
        {
            fscanf( fdat_p, "%d", &tmp->formations[j] );
        }
        if(  formateur == 0 )
        {
            fscanf( fdat_p, "%d", &tmp->reduction );
            if( tmp->reduction == 1 )
            {
                fscanf( fdat_p, "%d", &tmp->val_reduction );
            }
        }
        else if( formateur == 1 )
        {
            fscanf( fdat_p, "%d", &tmp->nb_jours_indisponible );
            for( j = 0; j < tmp->nb_jours_indisponible; j++ )
            {
                fscanf( fdat_p, "%d", &tmp->jours_indisponible[j] );
            }
        }
        ajouter_db_personne( dbp, tmp );
        i += 1;
    }

    i = 1;

    while( !feof( fdat_f ) )
    {
        int id;
        int nb_prerequis, nb_jours;
        float prix;
        char nom_formation[40];
        fscanf( fdat_f, "%d %d", &id, &nb_prerequis );
        int prerequis[ nb_prerequis ];
        int j;
        for( j = 0; j < nb_prerequis; j++ )
        {
            fscanf( fdat_f, "%d", &prerequis[j] );
        }
        fscanf( fdat_f, "%d", &nb_jours );
        int jours[ nb_jours ];
        float heures[ nb_jours ];
        float durees[ nb_jours ];
        for( j = 0; j < nb_jours; j++ )
        {
            fscanf( fdat_f, "%d %f %f", &jours[j], &heures[j], &durees[j] );
        }
        fscanf( fdat_f, "%f ", &prix );
        fgets( nom_formation, 40, fdat_f );
        if( feof( fdat_f ) )
        {
            break;
        }
        if( strlen( nom_formation ) > 0 && nom_formation[ strlen( nom_formation ) - 1 ] == '\n' )
        {
            nom_formation[ strlen( nom_formation ) - 1 ] = '\0';
        }
        formation *tmp = creer_formation( nom_formation, prix );
        tmp->id = id;
        tmp->nb_prerequis = nb_prerequis;
        for( j = 0; j < tmp->nb_prerequis; j++ )
        {
            tmp->prerequis[j] = prerequis[j];
        }
        tmp->nb_jours = nb_jours;
        for( j = 0; j < tmp->nb_jours; j++ )
        {
            tmp->jours[j] = jours[j];
            tmp->heures[j] = heures[j];
            tmp->durees[j] = durees[j];
        }
        ajouter_db_formation( dbf, tmp );
        i += 1;
    }

    db_personne *tmpdbp = dbp;
    noeud_db_personne *tmpndbp = tmpdbp->head;

    while( tmpndbp != NULL )
    {
        db_formation *tmpdbf = dbf;
        noeud_db_formation *tmpndbf = tmpdbf->head;
        while( tmpndbf != NULL )
        {
            int j;
            for( j = 0; j < tmpndbp->p->nb_formations; j++ )
            {
                if( tmpndbp->p->formations[j] == tmpndbf->f->id )
                {
                    ajouter_formation( tmpndbf->f, tmpndbp->p );
                }
            }
            tmpndbf = tmpndbf->next;
        }
        tmpndbp = tmpndbp->next;
    }

    fclose( fdat_p );
    fclose( fdat_f );

    menu( dbf, dbp );

    return 0;
}
