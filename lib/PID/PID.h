#ifndef PID_H
#define PID_H

#include <Arduino.h>

class PID{
    private:
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

    public:
        void set( String data );
        String get();
        float stab(float Angle, float correction = 0 );
        String get_sta();
        float dep( int EncL, int EncR, int consigne = 0 );
        String get_dep();
        float rot( int EncL, int EncR, int consigne = 0 );
        String get_rot();
};

#endif
