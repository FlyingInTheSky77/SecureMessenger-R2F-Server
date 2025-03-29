#pragma once

#include <ctime>
#include <math.h>

#include <gmp.h>

class SessionKey
{
public:
    SessionKey();
    int getIntermediateNumberForClient() const;
    void calculateSessionKey( const int& intermediate_number_from_client );
    int getSessionKey() const;

private:
    int randRange( int low,int high );
    mpz_t p_mpz_mod_;
    mpz_t g_mpz_;
    int random_number_;
    int intermediate_number_;
    mpz_t intermediate_number_mpz_;
    mpz_t session_key_mpz_;
    int session_key_;
};
