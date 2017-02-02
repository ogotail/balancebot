// =============================================================================
// ===                            PID                                        ===
// =============================================================================

#include "PID.h"

//************   Coefficients   ************

// kPID Stabilite
float Kps = 39;
float Kis = 6.6;
float Kds = 59.5;
float Vertical = 1.9;

// kPID Deplacement
float Kpd = 0.3;
float Kid = 0;
float Kdd = 2;
int Origine = 0;

// kPID Rotation
float Kpr = 0;
float Kir = 0;
float Kdr = 0;
int Direction = 0;

//************   Variables   ************

// variable pid Stabilisation
float erreur_s ;
float P_s ;
float I_s ;
float D_s ;
float PID_s;
float somme_erreurs_s = 0;
float erreur_precedente_s = 0;

// variable pid deplacement
float erreur_d ;
float P_d ;
float I_d ;
float D_d ;
float PID_d;
float somme_erreur_d = 0;
int erreur_precedente_d = 0;

// variable pid rotation
float erreur_r ;
float P_r ;
float I_r ;
float D_r ;
float PID_r;
float somme_erreur_r = 0;
int erreur_precedente_r = 0;

//************   Creation   ************
PID::PID(){/*nothing to Creation*/}

//************   Destruction   ************
PID::~PID(){/*nothing to destruct*/}

//************   Modification des Coefficients   ************
void PID::set( String data ){
    String commande = data.substring( 0, data.indexOf(' ') );
    float val = data.substring( data.indexOf(' ') ).toFloat();
    if ( commande == "kPs") Kps = val;
    else if ( commande == "kIs" ) Kis = val ;
    else if ( commande == "kDs" ) Kds = val ;
    else if ( commande == "kPd" ) Kpd = val ;
    else if ( commande == "kId" ) Kid = val ;
    else if ( commande == "kDd" ) Kdd = val ;
    else if ( commande == "kPr" ) Kpr = val ;
    else if ( commande == "kIr" ) Kir = val ;
    else if ( commande == "kDr" ) Kdr = val ;
    else if ( commande == "Vertical" ) Vertical = val ;
    else if ( commande == "Origine" ) Origine = int( val );
    else if ( commande == "Direction" ) Direction = int( val );
}

//************   Envoie des Coefficients   ************
String PID::get(){
    String msg = "PID";
    msg += " kPs " + String( Kps );
    msg += " kIs " + String( Kis );
    msg += " kDs " + String( Kds );
    msg += " kPd " + String( Kpd );
    msg += " kId " + String( Kid );
    msg += " kDd " + String( Kdd );
    msg += " kPr " + String( Kpr );
    msg += " kIr " + String( Kir );
    msg += " kDr " + String( Kdr );
    msg += " Vertical " + String( Vertical );
    msg += " Origine " + String( Origine );
    msg += " Direction " + String( Direction );
    msg += " " ;
    return msg ;
}

//************   Calcul du PID de Stabilisation   ************
float PID::stab(float Angle, float correction){
    // joue sur la vitesse / direction des moteurs pour maintenir l equilibre
    // entree : angle => vitesse moteurs
    // P
    erreur_s = Vertical - Angle - correction ;
    P_s = Kps * erreur_s ;
    // I
    somme_erreurs_s = constrain( somme_erreurs_s + erreur_s / 10 , -100, 100 );
    I_s = Kis * somme_erreurs_s ;
    // D
    D_s = Kds * ( erreur_s - erreur_precedente_s );
    erreur_precedente_s = erreur_s ;
    // PID
    // limitation du PID
    PID_s = constrain( (P_s + I_s + D_s) , -255, 255 );
    return PID_s ;
}

String PID::get_sta(){
    return "Stability "
            + String( millis() ) + " "
            + String( erreur_s ) + " "
            + String( PID_s ) + " "
            + String( P_s ) + " "
            + String( I_s ) + " "
            + String( D_s ) + " "
            ;
}

//************   Calcul du PID de deplacement   ************
float PID::dep( int EncL, int EncR, int consigne ){
    // joue sur l angle pour maintenir la position / atteindre une position
    // entree : valeurs des encodeurs + consigne de deplacement
    // => sortie : correction d angle
    // P
    erreur_d = - Origine - (EncL+ EncR)/2 - consigne ;
    P_d = Kpd * erreur_d ;
    // I
    somme_erreur_d = constrain( somme_erreur_d + erreur_d / 10, -100, 100 );
    I_d = Kid * somme_erreur_d ;
    // D
    D_d = Kdd * ( erreur_d - erreur_precedente_d );
    erreur_precedente_d = erreur_d ;
    // PID
    // limitation du PID
    PID_d = constrain( (P_d + I_d + D_d) / 10 , -5, 5 );
    return PID_d ;
}

String PID::get_dep(){
    return "Deplacement "
            + String( millis() ) + " "
            + String( erreur_d ) + " "
            + String( PID_d ) + " "
            + String( P_d ) + " "
            + String( I_d ) + " "
            + String( D_d ) + " "
            ;
}

//************   Calcul du PID de rotation   ************
float PID::rot( int EncL, int EncR, int consigne ){
    // joue sur la vitesse des moteurs gauche/droit pour maintenir la direction
    // entree : valeurs des encodeurs + consigne de rotation
    // => sortie : correction de rotation ( +G -D)
    // P
    erreur_r = Direction - consigne - EncL + EncR ;
    P_r = Kpr * erreur_r ;
    // I
    somme_erreur_r = constrain( somme_erreur_r + erreur_r, -100, 100 );
    I_r = Kir * somme_erreur_r ;
    // D
    D_r = Kdr * ( erreur_r = - erreur_precedente_r) ;
    erreur_precedente_r = erreur_r ;
    // PID
    // limitation du PID
    PID_r = constrain(  P_r + I_r + D_r , -20, 20);
    return PID_r ;
}

String PID::get_rot(){
    return "Rotation "
            + String( millis() ) + " "
            + String( erreur_r ) + " "
            + String( PID_r ) + " "
            + String( P_r ) + " "
            + String( I_r ) + " "
            + String( D_r ) + " "
            ;
}




// Code en reserve

//************   Mise a jour des vitesses   ************
//int PID::updateSpeed(float angle, float correction){
//    // si chute
//    if ( angle > 30 || angle < -30 ){
//        // arret des moteurs
//        ////Serial.print("chute\t");
//        //if ( SEND == "Stability" ){
//             //sendUdp( "Stability " + String( millis() ) + " " + String( -angle ) + " 0 0 0 0 ");
//        //}
//        return 0;
//    }
//    // sinon calcul le PID
//    else{
//        //calcul du PID
//        return stab( angle, correction );
//    }
//}

//************   Calcul du PID de Stabilisation   ************
//float stab(float Angle, float correction = 0 ){
//    float sta = Pid.stab( Angle, correction);
//    // envoie les valeurs
//    if ( SEND == "Stability" ){sendUdp( Pid.get_sta());}
//    return sta ;
//}

//************   Calcul du PID de deplacement   ************
//float dep( int consigne = 0 ){
//    float de = Pid.dep( EncL.read(), EncR.read(), consigne );
//    if ( SEND == "Deplacement" ){sendUdp( Pid.get_dep() );}
//    return de ;
//}

//************   Calcul du PID de rotation   ************
//float rot( int consigne = 0){
//    float ro = Pid.rot( EncL.read(), EncR.read(), consigne );
//    // envoie les valeurs
//    if ( SEND == "Rotation" ){sendUdp( Pid.get_rot() );}
//    return ro ;
//}
